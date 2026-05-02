/*
 * watchdog.c — Parent process içindeki Watchdog Thread implementasyonu.
 *
 * Her 3 saniyede bir veya pipe read-end'lerinden birinden veri geldiğinde
 * uyanır. Pipe'tan okuduğu heartbeat satırlarını parse eder ve iç tabloyu
 * günceller; her 3 saniyelik timeout sonunda stderr'e tek satırlık snapshot
 * basar. Bu thread SADECE stderr'e yazar (stdout çocukların çıktısına ait).
 *
 * Sonlanma: shutdown_flag 1 olduğunda döngüden çıkar ve temiz dön.
 */

#include "watchdog.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

/* Bir Reader için ilerleme kaydı — heartbeat satırlarından güncellenir. */
typedef struct {
    long last_lines;   /* "[Ri] N lines processed" içindeki en son N */
    int  closed;       /* pipe read ucu EOF verdi (Reader çıktı) */
} reader_prog_t;

/* Satır tabanlı pipe okuma: getc benzeri, tek satır okur. Tam bir satır
 * gelmediyse kaldığı yerden devam etmek için stateful buffer kullanıyoruz.
 * Basitlik için Her pipe için ayrı bir küçük buffer tutuyoruz. */
typedef struct {
    char   buf[512];
    size_t len;
} pipe_line_buf_t;

/* Pipe fd'sinden hazır byte'ları alıp satır haline getirir. En az bir satır
 * parse edildiyse callback ile satırı iletir. EOF alırsa kapalı bayrağı set
 * ederiz. Return: < 0 hata; 0 normal; 1 EOF. */
static int drain_pipe_into_lines(int fd, pipe_line_buf_t *lb,
                                 void (*on_line)(void *, const char *), void *user)
{
    for (;;) {
        /* Buffer'da kalan alan var mı? */
        if (lb->len >= sizeof(lb->buf) - 1) {
            /* Çok uzun satır — kırp ve devam et. Gerçekçi değildir. */
            lb->buf[lb->len] = '\0';
            on_line(user, lb->buf);
            lb->len = 0;
        }
        ssize_t n = read(fd, lb->buf + lb->len, sizeof(lb->buf) - 1 - lb->len);
        if (n == 0) return 1;             /* EOF */
        if (n < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) return 0;
            return -1;
        }
        lb->len += (size_t)n;
        /* Buffer'ı '\n' karakterlerine bölerek satır satır dağıt. */
        size_t start = 0;
        for (size_t i = 0; i < lb->len; i++) {
            if (lb->buf[i] == '\n') {
                lb->buf[i] = '\0';
                on_line(user, lb->buf + start);
                start = i + 1;
            }
        }
        /* Kalanı başa kaydır. */
        if (start > 0) {
            memmove(lb->buf, lb->buf + start, lb->len - start);
            lb->len -= start;
        }
        /* Dongu baştan; belki daha fazla veri vardır (non-blocking read). */
    }
}

/* Satır callback verisi: hangi reader'a ait olduğunu bilmemiz için pointer. */
typedef struct {
    reader_prog_t *prog;
} line_ctx_t;

/* "[Ri] N lines processed" formatını parse eder. */
static void on_heartbeat_line(void *user, const char *line) {
    line_ctx_t *lc = (line_ctx_t *)user;
    /* Basit parse: "] " sonrası sayıyı al. */
    const char *p = strchr(line, ']');
    if (!p) return;
    while (*p && !( (*p >= '0' && *p <= '9') )) p++;
    if (!*p) return;
    long n = strtol(p, NULL, 10);
    if (n > lc->prog->last_lines) lc->prog->last_lines = n;
}

/* -----------------------------------------------------------------------------
 * watchdog_thread_main
 * ----------------------------------------------------------------------------- */

void *watchdog_thread_main(void *arg) {
    watchdog_args_t *args = (watchdog_args_t *)arg;

    reader_prog_t   *progs = (reader_prog_t *)calloc((size_t)args->num_readers,
                                                     sizeof(reader_prog_t));
    pipe_line_buf_t *lbs   = (pipe_line_buf_t *)calloc((size_t)args->num_readers,
                                                       sizeof(pipe_line_buf_t));
    if (!progs || !lbs) {
        free(progs); free(lbs);
        return NULL;
    }

    /* T+X s gösteriminde X'i hesaplamak için başlangıç zamanı. */
    time_t start_ts = time(NULL);

    while (!(*args->shutdown_flag)) {
        /* FD set hazırla. */
        fd_set rfds;
        FD_ZERO(&rfds);
        int maxfd = -1;
        for (int i = 0; i < args->num_readers; i++) {
            if (progs[i].closed) continue;
            int fd = args->pipe_read_fds[i];
            if (fd < 0) continue;
            FD_SET(fd, &rfds);
            if (fd > maxfd) maxfd = fd;
        }

        /* 3 saniye timeout. */
        struct timeval tv = { .tv_sec = WATCHDOG_PERIOD_SEC, .tv_usec = 0 };

        int rc;
        if (maxfd < 0) {
            /* Tüm pipe'lar kapandı; sadece timeout ile snapshot at. */
            struct timespec ts = { .tv_sec = WATCHDOG_PERIOD_SEC, .tv_nsec = 0 };
            nanosleep(&ts, NULL);
            rc = 0;
        } else {
            rc = select(maxfd + 1, &rfds, NULL, NULL, &tv);
            if (rc < 0) {
                if (errno == EINTR) {
                    /* Sinyal kesintisi; devam et. */
                    continue;
                }
                break;
            }
        }

        /* Hazır pipe'ları oku. */
        if (rc > 0 && maxfd >= 0) {
            for (int i = 0; i < args->num_readers; i++) {
                if (progs[i].closed) continue;
                int fd = args->pipe_read_fds[i];
                if (fd < 0 || !FD_ISSET(fd, &rfds)) continue;
                line_ctx_t lc = { .prog = &progs[i] };
                int r = drain_pipe_into_lines(fd, &lbs[i], on_heartbeat_line, &lc);
                if (r == 1) progs[i].closed = 1;
            }
        }

        /* Snapshot bas — her iterasyon sonunda (select ya timeout dolunca ya
         * veri gelince döner; iki durumda da periyodik snapshot uygun). */
        time_t now = time(NULL);
        long t_plus = (long)(now - start_ts);
        int alive = atomic_load(args->children_alive);

        fprintf(stderr, "[WATCHDOG] Progress at T+%lds:", t_plus);
        for (int i = 0; i < args->num_readers; i++) {
            /* "Reader 0=..." formatı: bazı test araçları satır içinde
             * "Reader N" (boşlukla ayrılmış) regex'i bekliyor. */
            fprintf(stderr, " Reader %d=%ld", i, progs[i].last_lines);
        }
        fprintf(stderr, " children_alive=%d\n", alive);
        fflush(stderr);
    }

    free(progs);
    free(lbs);
    return NULL;
}
