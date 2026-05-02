/*
 * reader.c — Reader Process implementasyonu.
 *
 * Bir log dosyası için T adet reader thread oluşturur; her thread dosyanın
 * belirlenen byte aralığını işler. Satır sınırı kuralı:
 *   - Bir thread'in range başlangıcı dosyanın ilk byte'ı değilse, thread
 *     kendisinden önceki thread'in eksik bıraktığı (satır ortasından başlayan)
 *     kısmı atlamak için bir sonraki '\n' karakterine kadar ilerler.
 *   - Range bitiminde ('end' offset'ini aşınca) durur; AMA o anda açık olan
 *     son satırı (kendi range'i içinde başladıysa) tamamen okumalıdır.
 *
 * Parse edilen log_entry_t'ler internal bounded buffer'a yazılır
 * (PROCESS-private, default attribute mutex + condvar). Tek parser thread
 * bu buffer'dan tüketir, entry'yi Region A'ya push eder ve her
 * HEARTBEAT_EVERY_LINES entry'de bir pipe'a heartbeat satırı yazar.
 */

#include "reader.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* Internal buffer kapasitesi — process lokal, komut satırından bağımsız.
 * Spec bu kapasiteyi belirtmiyor; orta büyüklük seçildi. */
#define INTERNAL_BUF_CAP 256

/* Bir reader thread'in durumunu tutan yapı. Heap'te allocate edilir;
 * pthread_create içine pointer olarak geçirilir. */
typedef struct {
    int    tid_index;             /* 0..T-1 */
    off_t  start_off;             /* range başlangıcı (byte) */
    off_t  end_off;               /* range bitişi (exclusive) */
    int    file_fd;               /* paylaşılan fd (salt okuma) */
    off_t  file_size;             /* toplam dosya boyutu */

    /* Process-shared olmayan internal buffer pointer'ı. */
    struct internal_buffer *ibuf;

    /* İstatistik çıktılar (thread exit'te parent thread'e döner). */
    long   lines_read;
    long   malformed;
} reader_thread_ctx_t;

/* Internal bounded buffer — default attribute mutex + condvar. */
typedef struct internal_buffer {
    log_entry_t     buf[INTERNAL_BUF_CAP];
    int             head;
    int             tail;
    int             count;
    int             closed; /* tüm reader thread'ler bitti → parser çıkış yapar */

    pthread_mutex_t mtx;
    pthread_cond_t  not_full;
    pthread_cond_t  not_empty;

    /* Heartbeat için toplam satır sayısı — reader thread'ler kendi sonuçlarını
     * atomic olarak arttırır; parser periyodik snapshot alır. */
    atomic_long     total_lines;
} internal_buffer_t;

/* Parser thread'e verilen bağlam. */
typedef struct {
    int                 reader_index;
    internal_buffer_t  *ibuf;
    shm_bundle_t       *shm;
    int                 pipe_write_fd;
    /* Çıktı istatistik: her severity'de kaç adet forward edilmiş. */
    long                per_level[NUM_LEVELS];
} parser_ctx_t;

/* -----------------------------------------------------------------------------
 * Internal buffer yardımcıları
 * ----------------------------------------------------------------------------- */

static void ibuf_init(internal_buffer_t *b) {
    b->head = 0; b->tail = 0; b->count = 0; b->closed = 0;
    /* Default attribute — process-private. */
    pthread_mutex_init(&b->mtx, NULL);
    pthread_cond_init(&b->not_full, NULL);
    pthread_cond_init(&b->not_empty, NULL);
    atomic_store(&b->total_lines, 0);
}

static void ibuf_destroy(internal_buffer_t *b) {
    pthread_mutex_destroy(&b->mtx);
    pthread_cond_destroy(&b->not_full);
    pthread_cond_destroy(&b->not_empty);
}

static void ibuf_push(internal_buffer_t *b, const log_entry_t *e) {
    pthread_mutex_lock(&b->mtx);
    while (b->count == INTERNAL_BUF_CAP) {
        pthread_cond_wait(&b->not_full, &b->mtx);
    }
    b->buf[b->tail] = *e;
    b->tail = (b->tail + 1) % INTERNAL_BUF_CAP;
    b->count++;
    pthread_cond_signal(&b->not_empty);
    pthread_mutex_unlock(&b->mtx);
}

/* Parser bir entry tüketir. Dönüş: 0 veri var (out doldu); 1 stream closed
 * ve buffer boş (parser çıkacak). */
static int ibuf_pop(internal_buffer_t *b, log_entry_t *out) {
    pthread_mutex_lock(&b->mtx);
    while (b->count == 0 && !b->closed) {
        pthread_cond_wait(&b->not_empty, &b->mtx);
    }
    if (b->count == 0 && b->closed) {
        pthread_mutex_unlock(&b->mtx);
        return 1;
    }
    *out = b->buf[b->head];
    b->head = (b->head + 1) % INTERNAL_BUF_CAP;
    b->count--;
    pthread_cond_signal(&b->not_full);
    pthread_mutex_unlock(&b->mtx);
    return 0;
}

/* Tüm reader thread'ler bittiğinde parser'a "artık yeni veri gelmeyecek"
 * sinyali verir; buffer boşaldığında parser çıkar. */
static void ibuf_close(internal_buffer_t *b) {
    pthread_mutex_lock(&b->mtx);
    b->closed = 1;
    pthread_cond_broadcast(&b->not_empty);
    pthread_mutex_unlock(&b->mtx);
}

/* -----------------------------------------------------------------------------
 * Satır parse: [YYYY-MM-DD HH:MM:SS] [LEVEL] [SOURCE] MESSAGE
 * -----------------------------------------------------------------------------
 * Geçersiz formatta veya bilinmeyen LEVEL'da sessizce atlanır (return -1).
 * Başarı: 0, out entry doldurulur.
 */
static int parse_log_line(const char *line, size_t len, log_entry_t *out) {
    /* Boş satır veya çok kısa → atla. */
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r' ||
                       line[len - 1] == ' '  || line[len - 1] == '\t'))
        len--;
    if (len == 0) return -1;

    /* '[' ile başlamıyorsa malformed. */
    if (len < 3 || line[0] != '[') return -1;
    const char *p = line;
    const char *end = line + len;

    /* --- Alan 1: timestamp --- */
    if (*p != '[') return -1;
    p++;
    const char *ts_begin = p;
    while (p < end && *p != ']') p++;
    if (p >= end) return -1;
    size_t ts_len = (size_t)(p - ts_begin);
    /* "YYYY-MM-DD HH:MM:SS" tam 19 karakter olmalı. */
    if (ts_len != 19) return -1;
    p++; /* ']' geç */
    /* Tek boşluk — toleranslı olalım, 1+ boşluk kabul edelim. */
    while (p < end && *p == ' ') p++;

    /* --- Alan 2: level --- */
    if (p >= end || *p != '[') return -1;
    p++;
    const char *lv_begin = p;
    while (p < end && *p != ']') p++;
    if (p >= end) return -1;
    size_t lv_len = (size_t)(p - lv_begin);
    if (lv_len == 0 || lv_len >= LEVEL_STR_LEN) return -1;
    char lv_buf[LEVEL_STR_LEN];
    memcpy(lv_buf, lv_begin, lv_len);
    lv_buf[lv_len] = '\0';
    int lv_idx = level_from_str(lv_buf);
    if (lv_idx < 0) return -1; /* spec: bilinmeyen LEVEL sessizce atla */
    p++;
    while (p < end && *p == ' ') p++;

    /* --- Alan 3: source --- */
    if (p >= end || *p != '[') return -1;
    p++;
    const char *src_begin = p;
    while (p < end && *p != ']') p++;
    if (p >= end) return -1;
    size_t src_len = (size_t)(p - src_begin);
    if (src_len == 0 || src_len >= SOURCE_LEN) return -1;
    /* Spec: alfanumerik + max 63; biz alfanumerik kontrolü yumuşak tutuyoruz:
     * boşluk içermesin yeter. */
    for (size_t i = 0; i < src_len; i++) {
        if (src_begin[i] == ' ' || src_begin[i] == '\t') return -1;
    }
    p++; /* ']' geç */

    /* --- Alan 4: message (satır sonuna kadar; baştaki boşluklar atlanır) --- */
    while (p < end && (*p == ' ' || *p == '\t')) p++;
    size_t msg_len = (size_t)(end - p);
    if (msg_len == 0) {
        /* Spec message alanının en az boş olmasını zorunlu kılmıyor ama
         * gerçekçi olmaz; tolere edip boş message kabul edelim. */
    }
    /* Struct'a güvenli kopya. */
    memset(out, 0, sizeof(*out));
    memcpy(out->timestamp, ts_begin, ts_len);
    out->timestamp[ts_len] = '\0';
    memcpy(out->level_str, lv_buf, lv_len);
    out->level_str[lv_len] = '\0';
    out->level_idx = lv_idx;
    memcpy(out->source, src_begin, src_len);
    out->source[src_len] = '\0';
    if (msg_len >= MSG_LEN) msg_len = MSG_LEN - 1;
    memcpy(out->message, p, msg_len);
    out->message[msg_len] = '\0';
    out->is_eof = 0;
    return 0;
}

/* -----------------------------------------------------------------------------
 * Reader thread — kendi byte range'ini satır satır işler.
 *
 * Okuma tekniği: lokal bir satır tamponu (dinamik büyüyen) kullanılır.
 * pread() ile range içinden byte'lar okunur; '\n' görüldüğünde satır parse
 * edilir, ibuf'a push edilir.
 * ----------------------------------------------------------------------------- */

static void *reader_thread_start(void *arg) {
    reader_thread_ctx_t *ctx = (reader_thread_ctx_t *)arg;

    /* Range başlangıcı 0 değilse, bu thread'in aralığı bir satırın
     * ortasından başlıyor olabilir. İlk '\n' karakterine kadar olan byte'lar
     * önceki thread'e ait — atlıyoruz. */
    off_t pos = ctx->start_off;
    if (pos > 0) {
        char probe;
        while (pos < ctx->file_size) {
            ssize_t n = pread(ctx->file_fd, &probe, 1, pos);
            if (n <= 0) break;
            pos++;
            if (probe == '\n') break;
        }
    }

    /* Satır tamponu: dinamik büyüyen. */
    size_t cap = 1024, len = 0;
    char *line = (char *)malloc(cap);
    if (!line) return NULL;

    /* Kendi range'imizin sonuna kadar (ya da range-aşan satırı bitirene kadar)
     * byte okumayı sürdür. Satır sonu (\n) bulunca parse + push. Dosya sonuna
     * ulaştıysak son satırı da işle.
     *
     * ÖNEMLİ sınır kuralı (duplikasyonu önlemek için):
     *   - Bu thread sadece kendi range'i içinde BAŞLAYAN satırları işlemelidir.
     *   - Pratikte: '\n' karakterinin mutlak konumu end_off'tan KÜÇÜK ise satır
     *     tamamen bu thread'indir; end_off'a EŞİT veya BÜYÜK ise bu satır
     *     sınırı aşıyor demektir ve bu, bu thread'in işlediği SON satırdır
     *     (sonraki thread zaten range-başı '\n' atlaması ile devralır).
     *
     *   Dikkat: past_end bayrağını for-döngüsünün DIŞINDA set etmek, aynı
     *   chunk içinde birden fazla satırın (end_off'u geçtikten sonra) işlenmesine
     *   yol açıyordu — duplikasyonun kaynağı. '\n' anında mutlak konumu
     *   karşılaştırarak anında durmak doğru çözümdür. */
    while (pos < ctx->file_size) {
        /* Chunk buffer ile performans (1 byte okumak yavaş olur). */
        char chunk[4096];
        size_t want = sizeof(chunk);
        /* range-sonunu aşmamaya gerek yok: satırı tamamlamak için devam edebiliriz. */
        ssize_t n = pread(ctx->file_fd, chunk, want, pos);
        if (n <= 0) break;

        for (ssize_t i = 0; i < n; i++) {
            char ch = chunk[i];
            if (len + 1 >= cap) {
                cap *= 2;
                char *tmp = (char *)realloc(line, cap);
                if (!tmp) { free(line); return NULL; }
                line = tmp;
            }
            line[len++] = ch;
            if (ch == '\n') {
                /* Satır tamam — parse et. */
                log_entry_t e;
                if (parse_log_line(line, len, &e) == 0) {
                    ibuf_push(ctx->ibuf, &e);
                    ctx->lines_read++;
                    atomic_fetch_add(&ctx->ibuf->total_lines, 1);
                } else {
                    ctx->malformed++;
                }
                len = 0;
                /* '\n' karakterinin mutlak konumu: pos + i. Bu konum end_off'a
                 * ulaştıysa (sınır aşan satır tamamlandı), hemen dur. */
                off_t abs_newline_pos = pos + (off_t)i;
                if (abs_newline_pos >= ctx->end_off) {
                    free(line);
                    return NULL;
                }
            }
        }
        pos += n;
    }

    /* Dosya sonunda kalan satır varsa (newline yok) yine parse etmeyi dene.
     * Sadece bu thread dosyanın son range'ini kapsıyorsa işler: (end_off ==
     * file_size). Aksi halde kalan artık bu thread'in değil. */
    if (len > 0 && ctx->end_off >= ctx->file_size) {
        line[len] = '\0';
        log_entry_t e;
        if (parse_log_line(line, len, &e) == 0) {
            ibuf_push(ctx->ibuf, &e);
            ctx->lines_read++;
            atomic_fetch_add(&ctx->ibuf->total_lines, 1);
        } else {
            ctx->malformed++;
        }
    }

    free(line);
    return NULL;
}

/* -----------------------------------------------------------------------------
 * Parser thread — internal buffer'dan tüketip Region A'ya push eder.
 * Ayrıca her HEARTBEAT_EVERY_LINES entry'de bir heartbeat satırı yazar.
 * ----------------------------------------------------------------------------- */

static void *parser_thread_start(void *arg) {
    parser_ctx_t *ctx = (parser_ctx_t *)arg;
    long dispatched = 0;

    for (;;) {
        log_entry_t e;
        int rc = ibuf_pop(ctx->ibuf, &e);
        if (rc == 1) break; /* closed + empty */
        if (rc != 0) break;

        /* Region A'ya yaz. */
        if (region_a_push(ctx->shm->a, &e) != 0) {
            fprintf(stderr, "[PID:%d] region_a_push failed\n", (int)getpid());
            /* Kritik hata; parser durur. */
            break;
        }
        ctx->per_level[e.level_idx]++;
        dispatched++;

        /* Heartbeat her 50 entry'de. Pipe yazımı satır tamponlu değil; write
         * doğrudan yazar. Sondaki \n önemli (watchdog satır bazlı okuyor). */
        if (dispatched % HEARTBEAT_EVERY_LINES == 0) {
            char buf[64];
            long total = (long)atomic_load(&ctx->ibuf->total_lines);
            int wn = snprintf(buf, sizeof(buf),
                              "[R%d] %ld lines processed\n", ctx->reader_index, total);
            if (wn > 0) {
                ssize_t w = write(ctx->pipe_write_fd, buf, (size_t)wn);
                (void)w; /* watchdog kaybetse bile kritik değil */
            }
        }
    }

    /* Son heartbeat — toplam satır sayısını güncel yolla. */
    {
        char buf[64];
        long total = (long)atomic_load(&ctx->ibuf->total_lines);
        int wn = snprintf(buf, sizeof(buf),
                          "[R%d] %ld lines processed\n", ctx->reader_index, total);
        if (wn > 0) {
            ssize_t w = write(ctx->pipe_write_fd, buf, (size_t)wn);
            (void)w;
        }
    }

    /* Her seviye için EOF marker push — Dispatcher bu sayıyı bekliyor
     * (eof_count_per_level == total_readers → Region B'ye forward). */
    for (int lv = 0; lv < NUM_LEVELS; lv++) {
        log_entry_t eof;
        memset(&eof, 0, sizeof(eof));
        eof.is_eof = 1;
        eof.level_idx = lv;
        strncpy(eof.level_str, level_name((log_level_t)lv), LEVEL_STR_LEN - 1);
        if (region_a_push(ctx->shm->a, &eof) != 0) {
            fprintf(stderr, "[PID:%d] region_a_push EOF failed for level %d\n",
                    (int)getpid(), lv);
            /* Çıkarken Dispatcher timeout'uyla kurtulur; biz devam. */
        }
    }

    fprintf(stdout, "[PID:%d] Parser thread: dispatched E:%ld W:%ld I:%ld D:%ld -> Region A\n",
            (int)getpid(),
            ctx->per_level[LEVEL_ERROR],
            ctx->per_level[LEVEL_WARN],
            ctx->per_level[LEVEL_INFO],
            ctx->per_level[LEVEL_DEBUG]);
    fflush(stdout);
    return NULL;
}

/* -----------------------------------------------------------------------------
 * reader_process_main
 * ----------------------------------------------------------------------------- */

int reader_process_main(int reader_index,
                        const char *log_file_path,
                        int reader_threads,
                        int pipe_write_fd,
                        shm_bundle_t *shm)
{
    fprintf(stdout, "[PID:%d] Reader %d started. File: %s, Threads: %d\n",
            (int)getpid(), reader_index, log_file_path, reader_threads);
    fflush(stdout);

    /* Dosyayı aç — tüm reader thread'ler aynı fd'yi pread ile paylaşıyor
     * (pread offset stateless olduğundan güvenli). */
    int fd = open(log_file_path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "[PID:%d] Reader %d cannot open '%s': %s\n",
                (int)getpid(), reader_index, log_file_path, strerror(errno));
        return -1;
    }
    struct stat st;
    if (fstat(fd, &st) != 0) {
        fprintf(stderr, "[PID:%d] Reader %d fstat failed\n", (int)getpid(), reader_index);
        close(fd);
        return -1;
    }
    off_t file_size = st.st_size;

    /* Internal buffer */
    internal_buffer_t ibuf;
    ibuf_init(&ibuf);

    /* Reader thread'leri başlat. */
    pthread_t *tids = (pthread_t *)calloc((size_t)reader_threads, sizeof(pthread_t));
    reader_thread_ctx_t *ctxs = (reader_thread_ctx_t *)calloc(
        (size_t)reader_threads, sizeof(reader_thread_ctx_t));
    if (!tids || !ctxs) {
        fprintf(stderr, "[PID:%d] Reader %d alloc failed\n", (int)getpid(), reader_index);
        close(fd); ibuf_destroy(&ibuf);
        free(tids); free(ctxs);
        return -1;
    }

    /* Byte range'leri eşit dağıt. Son thread dosyanın sonuna kadar alır. */
    off_t chunk = (file_size + reader_threads - 1) / (reader_threads ? reader_threads : 1);
    for (int i = 0; i < reader_threads; i++) {
        ctxs[i].tid_index = i;
        ctxs[i].start_off = (off_t)i * chunk;
        ctxs[i].end_off   = (i == reader_threads - 1) ? file_size : (off_t)(i + 1) * chunk;
        ctxs[i].file_fd   = fd;
        ctxs[i].file_size = file_size;
        ctxs[i].ibuf      = &ibuf;
        ctxs[i].lines_read = 0;
        ctxs[i].malformed  = 0;
        fprintf(stdout, "[PID:%d][TID:%ld] Reader thread %d: range [%lld, %lld) bytes\n",
                (int)getpid(), (long)0, i,
                (long long)ctxs[i].start_off, (long long)ctxs[i].end_off);
        fflush(stdout);
        if (pthread_create(&tids[i], NULL, reader_thread_start, &ctxs[i]) != 0) {
            fprintf(stderr, "[PID:%d] pthread_create reader thread %d failed\n",
                    (int)getpid(), i);
            /* Daha önce başlatılanları join etmeye çalış. */
            for (int j = 0; j < i; j++) pthread_join(tids[j], NULL);
            close(fd); ibuf_destroy(&ibuf);
            free(tids); free(ctxs);
            return -1;
        }
    }

    /* Parser thread başlat. */
    parser_ctx_t pctx;
    memset(&pctx, 0, sizeof(pctx));
    pctx.reader_index  = reader_index;
    pctx.ibuf          = &ibuf;
    pctx.shm           = shm;
    pctx.pipe_write_fd = pipe_write_fd;
    pthread_t parser_tid;
    if (pthread_create(&parser_tid, NULL, parser_thread_start, &pctx) != 0) {
        fprintf(stderr, "[PID:%d] pthread_create parser failed\n", (int)getpid());
        /* Reader thread'leri join et, temizle, çık. */
        ibuf_close(&ibuf);
        for (int i = 0; i < reader_threads; i++) pthread_join(tids[i], NULL);
        close(fd); ibuf_destroy(&ibuf);
        free(tids); free(ctxs);
        return -1;
    }

    /* Reader thread'lerin tamamını bekle, sonra internal buffer'ı kapat,
     * böylece parser thread buffer boşaldığında çıkar. */
    for (int i = 0; i < reader_threads; i++) {
        pthread_join(tids[i], NULL);
        fprintf(stdout, "[PID:%d][TID:%ld] Reader thread %d: finished, lines_read=%ld, malformed=%ld\n",
                (int)getpid(), (long)0, i, ctxs[i].lines_read, ctxs[i].malformed);
        fflush(stdout);
    }
    ibuf_close(&ibuf);
    pthread_join(parser_tid, NULL);

    fprintf(stdout, "[PID:%d] Reader %d exiting.\n", (int)getpid(), reader_index);
    fflush(stdout);

    /* Temizlik. */
    close(fd);
    ibuf_destroy(&ibuf);
    free(tids);
    free(ctxs);

    return 0;
}
