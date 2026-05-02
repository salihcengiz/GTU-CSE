/*
 * aggregator.c — Aggregator Process implementasyonu.
 *
 * Görevleri:
 *   1. Region C'deki 4 semaphore'u sırayla sem_wait + agg_cond timedwait ile
 *      bekler. Bir level için hem semaphore hem de cond üzerinden bekleme
 *      ödevin "pthread_cond_timedwait zorunlu" gereksinimini karşılar
 *      (sem_wait ödev tarafındaki semaphore kullanımını karşılar; timedwait
 *      "bu sürede gelmediyse terminasyon durumu olabilir" kontrolünü sağlar).
 *   2. Region D'den high-priority entry'leri tüketir ve kendi başına ağırlıklı
 *      skoru hesaplar.
 *   3. Level'leri toplam ağırlıklı skorlarına göre azalan sırada sıralar.
 *   4. İki çıktı dosyası üretir:
 *      - İnsan-okunur text (sabit formatlı, sağa yaslı kolonlar, 2 boşluk sep)
 *      - Binary checkpoint: .tmp dosyasına yaz → rename(2) ile atomik olarak
 *        hedefe geçir.
 */

#include "aggregator.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

/* Binary checkpoint header yapısı — layout sabit. */
typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint32_t version;
    uint32_t num_levels;
    uint32_t num_keywords;
    double   total_weighted;
    double   high_priority_weighted;
} bin_header_t;

/* -----------------------------------------------------------------------------
 * Yardımcı: tek level'in Region C'ye gelmesini bekle (sem_wait + timedwait).
 * ----------------------------------------------------------------------------- */

static int wait_level_result(region_c_t *c, int lv, int timeout_sec) {
    /* Önce semaphore üzerinde timedwait dene — bu ödevin sem_wait kullanımı.
     * sem_timedwait absolute zaman alır; CLOCK_REALTIME kullanılır. */
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += (timeout_sec > 0 ? timeout_sec : DEFAULT_TIMEOUT_SEC);

    for (;;) {
        int rc = sem_timedwait(&c->sems[lv], &ts);
        if (rc == 0) break;                 /* Analyzer post etti */
        if (errno == EINTR) continue;       /* sinyal kesintisi → devam */
        if (errno == ETIMEDOUT) {
            /* Timeout: "belki ready_mask set edildi ama sem henüz
             * görülmedi" — highly unlikely; hata dön. */
            return -1;
        }
        return -1;
    }

    /* İkinci aşama: agg_mutex altında ready bayrağını cond_timedwait ile
     * doğrula. Ödev "pthread_cond_timedwait" kullanımını zorunlu kıldığı
     * için bu adımı her zaman koşuyoruz — pratikte ready zaten set olur. */
    pthread_mutex_lock(&c->agg_mutex);
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += (timeout_sec > 0 ? timeout_sec : DEFAULT_TIMEOUT_SEC);
    while (!c->results[lv].ready) {
        int rc = pthread_cond_timedwait(&c->agg_cond, &c->agg_mutex, &ts);
        if (rc == ETIMEDOUT) {
            pthread_mutex_unlock(&c->agg_mutex);
            return -1;
        }
        if (rc != 0) {
            pthread_mutex_unlock(&c->agg_mutex);
            return -1;
        }
    }
    pthread_mutex_unlock(&c->agg_mutex);
    return 0;
}

/* -----------------------------------------------------------------------------
 * High-priority drainer — Region D'den gelen kayıtları Dispatcher ile
 * eş zamanlı tüketir. Aggregator ana thread'i Region C'yi beklerken bu
 * yardımcı thread Region D'yi boşaltır; aksi halde D dolunca Dispatcher
 * bloke olur, Region A dolar ve sistem kilitlenir.
 * -----------------------------------------------------------------------------
 */
typedef struct {
    shm_bundle_t *shm;
    char        **keywords;
    int           num_keywords;
    double        total;           /* atomic olarak değil — thread join sonrası okunur */
} hp_drain_ctx_t;

static void *high_priority_drain_main(void *arg) {
    hp_drain_ctx_t *ctx = (hp_drain_ctx_t *)arg;
    ctx->total = 0.0;
    for (;;) {
        log_entry_t e;
        int rc = region_d_pop_or_done(ctx->shm->d, &e);
        if (rc == 1) break;              /* dispatcher_done + boş */
        if (rc != 0) break;              /* hata */
        if (e.is_eof) continue;          /* güvenlik */
        int lv = e.level_idx;
        if (lv < 0 || lv >= NUM_LEVELS) continue;
        int w = LEVEL_WEIGHT[lv];
        for (int k = 0; k < ctx->num_keywords; k++) {
            size_t c = count_overlapping(e.message, ctx->keywords[k]);
            ctx->total += (double)c * (double)w;
        }
    }
    return NULL;
}

/* -----------------------------------------------------------------------------
 * Text output
 * -----------------------------------------------------------------------------
 * Format PDF §10.1'e uyar. Kolonlar sağa yaslı, aralarında tam 2 boşluk.
 * Floatlar %.1f. Level tablosu sırası weighted_score DESC.
 */

/* Basit karşılaştırma yapıcı — qsort için. */
typedef struct {
    int idx;                  /* orijinal level indexi */
    double weighted;
} level_sort_entry_t;

static int cmp_weighted_desc(const void *a, const void *b) {
    const level_sort_entry_t *x = (const level_sort_entry_t *)a;
    const level_sort_entry_t *y = (const level_sort_entry_t *)b;
    if (y->weighted > x->weighted) return 1;
    if (y->weighted < x->weighted) return -1;
    return 0;
}

/* Kolon genişliklerini önden hesaplayıp sağa yaslı yazmak için küçük bir
 * yardımcı. 2-boşluk ayracı ayrı fprintf'te konuyor. */
static int write_text_output(const char *path,
                             region_c_t *c,
                             char **keywords, int num_keywords,
                             int num_files,
                             double total_weighted, double high_priority_weighted,
                             const level_sort_entry_t *sorted)
{
    FILE *fp = fopen(path, "w");
    if (!fp) {
        fprintf(stderr, "[agg] cannot open '%s' for write: %s\n", path, strerror(errno));
        return -1;
    }

    /* Header bölümü */
    fprintf(fp, "KEYWORD_LIST: ");
    for (int i = 0; i < num_keywords; i++) {
        fprintf(fp, "%s%s", keywords[i], i + 1 < num_keywords ? "," : "");
    }
    fprintf(fp, "\n");
    fprintf(fp, "FILES: %d\n", num_files);
    fprintf(fp, "TOTAL_WEIGHTED_SCORE: %.1f\n", total_weighted);
    fprintf(fp, "HIGH_PRIORITY_SCORE: %.1f\n", high_priority_weighted);

    /* Levels table */
    fprintf(fp, "# Levels sorted by total_weighted_score DESC\n");

    /* Kolon genişlikleri — header satırı ve en geniş değer arasındaki maks. */
    int w_level = 5; /* "LEVEL" */
    for (int i = 0; i < NUM_LEVELS; i++) {
        int l = (int)strlen(c->results[i].level);
        if (l > w_level) w_level = l;
    }
    int w_entries = 7;       /* "ENTRIES" */
    int w_wscore  = 14;      /* "WEIGHTED_SCORE" */
    int *w_kw = (int *)malloc(sizeof(int) * (size_t)num_keywords);
    for (int k = 0; k < num_keywords; k++) {
        int kl = (int)strlen(keywords[k]);
        if (kl < 5) kl = 5; /* en az "xxx.x" */
        w_kw[k] = kl;
    }

    /* Satırları string'e yazıp kolon sınırlarını genişletelim — bir pass
     * yapıp sonra ikinci pass'ta basmak yerine tek pass'ta doğrudan basalım.
     * Kolon genişliği zaten float'lar için yeterli (1 decimal → en fazla
     * 10-11 char). */
    (void)num_keywords; /* unused warning önle */

    /* Başlık satırı — LEVEL kolonu sola hizalı (test araçları "^WARN/^INFO"
     * gibi satır başı regex'leriyle değer çekiyor; sağa hizalama olunca
     * 4 karakterli WARN/INFO başında bir boşluk kalıyordu). */
    fprintf(fp, "%-*s  %*s  %*s",
            w_level, "LEVEL", w_entries, "ENTRIES", w_wscore, "WEIGHTED_SCORE");
    for (int k = 0; k < num_keywords; k++) {
        fprintf(fp, "  %*s", w_kw[k], keywords[k]);
    }
    fprintf(fp, "\n");

    /* Veri satırları — sıralı sırayla */
    for (int s = 0; s < NUM_LEVELS; s++) {
        int lv = sorted[s].idx;
        level_result_t *r = &c->results[lv];
        fprintf(fp, "%-*s  %*ld  %*.1f",
                w_level, r->level,
                w_entries, r->total_entries,
                w_wscore, r->total_weighted_score);
        for (int k = 0; k < num_keywords; k++) {
            fprintf(fp, "  %*.1f", w_kw[k], r->per_keyword_score[k]);
        }
        fprintf(fp, "\n");
    }
    free(w_kw);

    /* Top-3 sources per level */
    fprintf(fp, "# Top-3 sources per level\n");
    for (int s = 0; s < NUM_LEVELS; s++) {
        int lv = sorted[s].idx;
        level_result_t *r = &c->results[lv];
        fprintf(fp, "%s", r->level);
        for (int t = 0; t < TOP_SRC_N; t++) {
            if (r->top_source[t][0] == '\0') continue;
            fprintf(fp, " %s:%ld", r->top_source[t], r->top_source_hits[t]);
        }
        fprintf(fp, "\n");
    }

    /* Per-thread contributions */
    fprintf(fp, "# Per-thread contributions (weighted score)\n");
    for (int s = 0; s < NUM_LEVELS; s++) {
        int lv = sorted[s].idx;
        level_result_t *r = &c->results[lv];
        fprintf(fp, "%s", r->level);
        /* Sıfır olmayan katkıları yaz — analizci sayısı max W; boşluk ile ayır. */
        for (int w = 0; w < MAX_WORKERS; w++) {
            if (r->per_thread_score[w] != 0.0) {
                fprintf(fp, " thread_%d:%.1f", w, r->per_thread_score[w]);
            }
        }
        fprintf(fp, "\n");
    }

    if (fflush(fp) != 0) {
        fprintf(stderr, "[agg] fflush failed\n");
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return 0;
}

/* -----------------------------------------------------------------------------
 * Binary output + atomic rename
 * ----------------------------------------------------------------------------- */

static int write_binary_output(const char *final_path,
                               region_c_t *c,
                               int num_keywords,
                               double total_weighted,
                               double high_priority_weighted)
{
    /* Geçici dosya adı: <final>.tmp */
    size_t flen = strlen(final_path);
    char *tmp_path = (char *)malloc(flen + 5);
    if (!tmp_path) return -1;
    memcpy(tmp_path, final_path, flen);
    memcpy(tmp_path + flen, ".tmp", 5);

    FILE *fp = fopen(tmp_path, "wb");
    if (!fp) {
        fprintf(stderr, "[agg] cannot open '%s' for write: %s\n", tmp_path, strerror(errno));
        free(tmp_path);
        return -1;
    }

    bin_header_t h;
    h.magic                  = BIN_MAGIC;
    h.version                = BIN_VERSION;
    h.num_levels             = (uint32_t)NUM_LEVELS;
    h.num_keywords           = (uint32_t)num_keywords;
    h.total_weighted         = total_weighted;
    h.high_priority_weighted = high_priority_weighted;

    /* Tek fwrite ile header. */
    size_t wn = fwrite(&h, sizeof(h), 1, fp);
    if (wn != 1) {
        fprintf(stderr, "[agg] header fwrite partial (%zu)\n", wn);
        fclose(fp); unlink(tmp_path); free(tmp_path);
        return -1;
    }

    /* Her level_result_t için ayrı fwrite. Region C'deki orijinal sıra
     * (ERROR, WARN, INFO, DEBUG) korunur. */
    for (int i = 0; i < NUM_LEVELS; i++) {
        wn = fwrite(&c->results[i], sizeof(level_result_t), 1, fp);
        if (wn != 1) {
            fprintf(stderr, "[agg] level %d fwrite partial (%zu)\n", i, wn);
            fclose(fp); unlink(tmp_path); free(tmp_path);
            return -1;
        }
    }

    /* Durable sync: fflush + fsync (mümkünse). */
    if (fflush(fp) != 0) {
        fprintf(stderr, "[agg] fflush binary failed: %s\n", strerror(errno));
        fclose(fp); unlink(tmp_path); free(tmp_path);
        return -1;
    }
    int fd = fileno(fp);
    if (fd >= 0) {
        /* fsync hata verse bile best-effort devam — asıl atomikliği rename sağlar. */
        (void)fsync(fd);
    }
    fclose(fp);

    /* Atomik rename — aynı dosya sisteminde yol varsa atomiktir (POSIX). */
    if (rename(tmp_path, final_path) != 0) {
        fprintf(stderr, "[agg] rename('%s' -> '%s') failed: %s\n",
                tmp_path, final_path, strerror(errno));
        unlink(tmp_path);
        free(tmp_path);
        return -1;
    }

    free(tmp_path);
    return 0;
}

/* -----------------------------------------------------------------------------
 * aggregator_process_main
 * ----------------------------------------------------------------------------- */

int aggregator_process_main(shm_bundle_t *shm,
                            const char *output_text_path,
                            const char *output_bin_path,
                            char **keywords,
                            int num_keywords,
                            int num_files,
                            const char *filter_path,
                            int timeout_sec)
{
    (void)filter_path; /* header metninde kullanılabilir; şimdilik sadece bilgi */

    fprintf(stdout, "[PID:%d] Aggregator started. Waiting for %d levels...\n",
            (int)getpid(), NUM_LEVELS);
    fflush(stdout);

    /* Region D drainer thread'i başlat — Dispatcher ile eş zamanlı D'yi
     * tüket ki Dispatcher priority push'larda blok olmasın. */
    hp_drain_ctx_t hp_ctx = {
        .shm = shm, .keywords = keywords, .num_keywords = num_keywords, .total = 0.0,
    };
    pthread_t hp_tid;
    int hp_started = (pthread_create(&hp_tid, NULL, high_priority_drain_main, &hp_ctx) == 0);
    if (!hp_started) {
        fprintf(stderr, "[agg] pthread_create high-priority drain failed\n");
        /* Sentezle devam: D'yi ana thread sonradan tüketir (deadlock riski). */
    }

    /* 4 level için sonuç bekle. */
    for (int i = 0; i < NUM_LEVELS; i++) {
        if (wait_level_result(shm->c, i, timeout_sec) != 0) {
            fprintf(stderr, "[PID:%d] Aggregator: level %d timed out\n",
                    (int)getpid(), i);
            /* Best-effort devam — eksik değerlerle output yazmaktansa hata
             * raporlayıp çık. */
            return -1;
        }
        fprintf(stdout, "[PID:%d] %s result received.\n",
                (int)getpid(), level_name((log_level_t)i));
        fflush(stdout);
    }

    fprintf(stdout, "[PID:%d] All results received. Writing output files...\n",
            (int)getpid());
    fflush(stdout);

    /* High-priority drainer thread'in bitmesini bekle — Dispatcher exit olunca
     * dispatcher_done=1 set edilir; drainer D tamamen boşalınca return eder. */
    if (hp_started) {
        pthread_join(hp_tid, NULL);
    } else {
        /* Fallback: ana thread synchron tüketir. */
        high_priority_drain_main(&hp_ctx);
    }
    double high_priority_weighted = hp_ctx.total;

    /* Toplam ağırlıklı skor = tüm level'ların toplamı. */
    double total_weighted = 0.0;
    for (int i = 0; i < NUM_LEVELS; i++) {
        total_weighted += shm->c->results[i].total_weighted_score;
    }

    /* Level'leri ağırlıklı skorlarına göre azalan sırada sırala. */
    level_sort_entry_t sorted[NUM_LEVELS];
    for (int i = 0; i < NUM_LEVELS; i++) {
        sorted[i].idx      = i;
        sorted[i].weighted = shm->c->results[i].total_weighted_score;
    }
    qsort(sorted, NUM_LEVELS, sizeof(sorted[0]), cmp_weighted_desc);

    /* Text output */
    if (write_text_output(output_text_path, shm->c, keywords, num_keywords,
                          num_files, total_weighted, high_priority_weighted,
                          sorted) != 0) {
        return -1;
    }

    /* Binary output + atomic rename */
    if (write_binary_output(output_bin_path, shm->c, num_keywords,
                            total_weighted, high_priority_weighted) != 0) {
        return -1;
    }

    fprintf(stdout, "[PID:%d] Output files written: %s, %s\n",
            (int)getpid(), output_text_path, output_bin_path);
    fprintf(stdout, "Output files written.\n"); /* PDF'deki sabit satır */
    fprintf(stdout, "[PID:%d] Aggregator exiting.\n", (int)getpid());
    fflush(stdout);

    return 0;
}
