/* Analyzer Process implementasyonu.
 *
 * Süreç şu bileşenleri barındırır:
 *   - W worker thread (pthread).
 *   - Her worker thread başına pthread_key_t tabanlı TLS alanı:
 *       * per-keyword ağırlıklı skor dizisi (MAX_KEYWORDS)
 *       * per-source lokal hit tablosu (basit linear array)
 *       * toplam entry / toplam ağırlıklı skor
 *   - pthread_barrier_t: tüm worker'lar iş bitiminde burada buluşur.
 *   - Lowest-TID reporting: syscall(SYS_gettid) ile alınan TID'lerin en
 *     küçüğü olan worker, Region C'ye sonucu yazıp sem_post yapar.
 *   - TLS destructor: thread exit'te tetiklenir, process-lokal aggregate'a
 *     mutex altında flush yapar. Raporlayıcı worker destructor'ın kendi
 *     TLS'ini OTOMATİK çağırmasını istemediğimiz için pthread_setspecific(NULL)
 *     ile devre dışı bırakır, bunun yerine manuel helper ile flush eder.
 *
 * Bu dosyada process-local (yani shm'de olmayan) yardımcı yapılar var;
 * her Analyzer Process kendi kopyasında çalışır.
 */

#include "analyzer.h"

#include <errno.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

/* Per-worker lokal hash benzeri tablonun max kapasitesi. Aynı Analyzer
 * içinde tipik SOURCE çeşitliliği birkaç yüzü geçmez; 256 yeterli. */
#define PER_WORKER_SRC_CAP 256

/* Process-lokal aggregate source tablosu; tüm worker'ların per-source
 * katkıları burada birleşir. Raporlayıcı top-3'ü bu yapıdan çıkarır. */
#define AGGREGATE_SRC_CAP 1024

/* -----------------------------------------------------------------------------
 * Tipler
 * ----------------------------------------------------------------------------- */

/* Worker thread'in TLS'inde tuttuğu skor kaydı. pthread_setspecific ile
 * saklanır; destructor bu struct'ı alır ve flush eder. */
typedef struct {
    double per_keyword[MAX_KEYWORDS];

    /* Kaynak hit tablosu: aynı source adı için hits toplanır. Lineer arama;
     * küçük tablo olduğu için hızlıdır. Gerekirse hash'e geçilir. */
    struct {
        char src[SOURCE_LEN];
        long hits; /* ağırlıklı hit sayısı: raw_count × level_weight */
    } src_tbl[PER_WORKER_SRC_CAP];
    int  src_count;

    long   total_entries;       /* bu worker'ın gördüğü entry sayısı */
    double total_weighted;      /* bu worker'ın toplam ağırlıklı skoru */

    int    worker_idx;          /* level_result_t.per_thread_score indexi */
} tls_scores_t;

/* Process-lokal aggregate: tüm worker'ların destructor'larıyla birleşen veri.
 * shm'de değil; Analyzer process kendi heap'inde tutar. agg_mutex ile korunur. */
typedef struct {
    double per_keyword[MAX_KEYWORDS];

    struct {
        char src[SOURCE_LEN];
        long hits;
    } src_tbl[AGGREGATE_SRC_CAP];
    int  src_count;

    long   total_entries;
    double total_weighted;

    double per_thread_score[MAX_WORKERS];

    /* Kaç worker destructor'ı flush yaptı (raporlayıcı dahil). */
    int    flushed_count;

    /* Raporlayıcı dışında flush biten worker sayısı; raporlayıcı bunu
     * W-1'e ulaşana kadar cond_wait ile bekler. */
    pthread_mutex_t mtx;
    pthread_cond_t  cond;
} analyzer_aggregate_t;

/* Worker thread'e geçirilen context. */
typedef struct {
    int               level_idx;
    int               worker_idx;
    int               num_keywords;
    char            **keywords;
    shm_bundle_t     *shm;

    pthread_key_t    *tls_key;          /* ortak key pointer */
    pthread_barrier_t*barrier;
    analyzer_aggregate_t *agg;

    /* TID kayıt tablosu (aynı pointer'ı paylaşırlar). */
    pid_t            *tid_table;        /* size = W */
    int               num_workers;

    /* Raporlayıcı seçimi: en küçük TID olan worker_idx. Barrier sonrası
     * hesaplanır ve aynı şekilde okunur. */
    int              *reporter_worker_idx;  /* atomic-like, ancak barrier sonrası stable */
} worker_ctx_t;

/* -----------------------------------------------------------------------------
 * Yardımcılar: per-worker ve aggregate src_tbl işlemleri
 * ----------------------------------------------------------------------------- */

/* src kaydını worker TLS tablosuna ekle veya var olanı artır. */
static void tls_src_add(tls_scores_t *t, const char *src, long weighted_hits) {
    if (weighted_hits == 0) return;
    for (int i = 0; i < t->src_count; i++) {
        if (strcmp(t->src_tbl[i].src, src) == 0) {
            t->src_tbl[i].hits += weighted_hits;
            return;
        }
    }
    if (t->src_count < PER_WORKER_SRC_CAP) {
        /* snprintf ile güvenli kopya — her zaman NUL ile bitirir ve
         * truncation uyarısı üretmez. */
        snprintf(t->src_tbl[t->src_count].src, SOURCE_LEN, "%s", src);
        t->src_tbl[t->src_count].hits = weighted_hits;
        t->src_count++;
    }
    /* Tablo dolduysa sessizce düşür — kaybı minimize etmek için son gelen
     * nadir source'lar. Gerçekçi testlerde bu sınır dolmaz. */
}

/* Aggregate tablosuna ekle/merge — agg->mtx tutulurken çağrılmalı. */
static void agg_src_add(analyzer_aggregate_t *a, const char *src, long hits) {
    if (hits == 0) return;
    for (int i = 0; i < a->src_count; i++) {
        if (strcmp(a->src_tbl[i].src, src) == 0) {
            a->src_tbl[i].hits += hits;
            return;
        }
    }
    if (a->src_count < AGGREGATE_SRC_CAP) {
        snprintf(a->src_tbl[a->src_count].src, SOURCE_LEN, "%s", src);
        a->src_tbl[a->src_count].hits = hits;
        a->src_count++;
    }
}

/* TLS scores → aggregate merge. agg->mtx tutarken çağrılır. */
static void tls_flush_into_agg(tls_scores_t *t, analyzer_aggregate_t *a) {
    for (int k = 0; k < MAX_KEYWORDS; k++) {
        a->per_keyword[k] += t->per_keyword[k];
    }
    for (int i = 0; i < t->src_count; i++) {
        agg_src_add(a, t->src_tbl[i].src, t->src_tbl[i].hits);
    }
    a->total_entries  += t->total_entries;
    a->total_weighted += t->total_weighted;
    if (t->worker_idx >= 0 && t->worker_idx < MAX_WORKERS) {
        a->per_thread_score[t->worker_idx] = t->total_weighted;
    }
}

/* -----------------------------------------------------------------------------
 * Global (dosya-kapsamlı) aggregate pointer'ı — TLS destructor'ın ulaşması
 * için gerekli. Bu bir process-local değişken; her Analyzer Process kendi
 * kopyasını tutar. fork sonrası bu adres Analyzer Process'in kendi adres
 * alanındadır (farklı proseslerin aynı değişkeni karıştırması söz konusu
 * değil).
 * ----------------------------------------------------------------------------- */

static analyzer_aggregate_t *g_agg = NULL;
static pthread_key_t         g_tls_key; /* tek key — flush tetikleyici */

/* TLS destructor — thread exit'te çağrılır. Raporlayıcı kendi TLS'ini
 * manuel flush ettikten sonra pthread_setspecific(key, NULL) yaptığı için
 * destructor onun için çağrılmaz (veya çağrılırsa val NULL olur). */
static void tls_destructor(void *val) {
    if (!val) return;
    tls_scores_t *t = (tls_scores_t *)val;
    if (!g_agg) { free(t); return; }

    pthread_mutex_lock(&g_agg->mtx);
    tls_flush_into_agg(t, g_agg);
    g_agg->flushed_count++;
    pthread_cond_broadcast(&g_agg->cond);
    pthread_mutex_unlock(&g_agg->mtx);

    free(t);
}

/* -----------------------------------------------------------------------------
 * Worker thread
 * ----------------------------------------------------------------------------- */

static void *analyzer_worker_main(void *arg) {
    worker_ctx_t *ctx = (worker_ctx_t *)arg;

    /* TID kaydı: bu worker'ın kernel TID'ini paylaşılan tabloya yaz. */
    pid_t my_tid = (pid_t)syscall(SYS_gettid);
    ctx->tid_table[ctx->worker_idx] = my_tid;

    fprintf(stdout, "[PID:%d][TID:%d] Worker %d started.\n",
            (int)getpid(), (int)my_tid, ctx->worker_idx);
    fflush(stdout);

    /* TLS alanı ayır ve setspecific. */
    tls_scores_t *t = (tls_scores_t *)calloc(1, sizeof(*t));
    if (!t) {
        fprintf(stderr, "[PID:%d] worker %d calloc failed\n", (int)getpid(), ctx->worker_idx);
        return NULL;
    }
    t->worker_idx = ctx->worker_idx;
    pthread_setspecific(*ctx->tls_key, t);

    /* İş döngüsü: Region B[level]'dan entry çek, skorla, TLS'e ekle.
     * Keyword uzunluklarını count_overlapping kendi içinde hesaplıyor;
     * burada önceden cache'lemiyoruz (optimize edilirse eklenecektir). */
    region_b_level_t *b = ctx->shm->b[ctx->level_idx];
    int lv_weight = LEVEL_WEIGHT[ctx->level_idx];

    for (;;) {
        log_entry_t e;
        int rc = region_b_pop_or_eof(b, &e);
        if (rc == 1) break; /* stream bitti */
        if (rc != 0) {
            fprintf(stderr, "[PID:%d] region_b_pop_or_eof error\n", (int)getpid());
            break;
        }
        if (e.is_eof) {
            /* Forwarded EOF marker; işlemeden geç. eof_posted ayrıca set
             * edilmiş olacak, sonraki pop 1 dönecek. */
            continue;
        }

        /* Message üzerinde her keyword için örtüşen arama. */
        long per_entry_weighted = 0;
        long any_keyword_hit = 0;
        for (int k = 0; k < ctx->num_keywords; k++) {
            /* count_overlapping common.h içinde inline yardımcı. */
            size_t cnt = count_overlapping(e.message, ctx->keywords[k]);
            if (cnt > 0) {
                double w = (double)cnt * (double)lv_weight;
                t->per_keyword[k] += w;
                per_entry_weighted += (long)cnt * (long)lv_weight;
                any_keyword_hit += (long)cnt;
            }
        }
        t->total_entries++;
        t->total_weighted += (double)per_entry_weighted;

        /* per-source: eşleşen keyword sayısı × weight toplamı bu source'a
         * yazılır. Spec "per-source match count" diyor; biz ağırlıklı hit
         * tutuyoruz (top-3 sıralaması da ağırlıklı anlamlı). Değer 0 ise
         * kayıt açılmasın. */
        if (any_keyword_hit > 0) {
            tls_src_add(t, e.source, per_entry_weighted);
        }
    }

    fprintf(stdout, "[PID:%d][TID:%d] Worker %d done. Entries: %ld, Weighted score: %.1f\n",
            (int)getpid(), (int)my_tid, ctx->worker_idx,
            t->total_entries, t->total_weighted);
    fflush(stdout);

    /* Barrier: tüm worker'lar buluşur — TID kayıtları ve iş tamamen bitti. */
    pthread_barrier_wait(ctx->barrier);

    /* Raporlayıcı seçimi: en küçük TID olan worker. */
    pid_t min_tid = ctx->tid_table[0];
    int   min_idx = 0;
    for (int i = 1; i < ctx->num_workers; i++) {
        if (ctx->tid_table[i] < min_tid) {
            min_tid = ctx->tid_table[i];
            min_idx = i;
        }
    }
    *ctx->reporter_worker_idx = min_idx; /* aynı değeri tüm worker'lar yazar; race yok */

    if (ctx->worker_idx != min_idx) {
        /* Bu thread raporlayıcı değil — normal return ile çık. TLS destructor
         * otomatik tetiklenecek ve flush yapacak. */
        return NULL;
    }

    /* --- Raporlayıcı akışı --- */
    fprintf(stdout, "[PID:%d][TID:%d] ** Reporting thread (lowest TID). Level: %s **\n",
            (int)getpid(), (int)my_tid, level_name((log_level_t)ctx->level_idx));
    fflush(stdout);

    /* Diğer (W-1) worker'ın destructor flush'larını bekle. Busy-wait yasak;
     * cond_wait ile bekliyoruz. */
    pthread_mutex_lock(&ctx->agg->mtx);
    while (ctx->agg->flushed_count < (ctx->num_workers - 1)) {
        pthread_cond_wait(&ctx->agg->cond, &ctx->agg->mtx);
    }

    /* Kendi TLS'imizi manuel flush et (lock zaten tutuluyor). */
    tls_flush_into_agg(t, ctx->agg);
    ctx->agg->flushed_count++;
    /* Aggregate artık tam. Hemen Region C'ye yaz. */

    /* Top-3 source hesapla — selection sort benzeri 3 pass. */
    char top_src[TOP_SRC_N][SOURCE_LEN];
    long top_hits[TOP_SRC_N] = {0, 0, 0};
    for (int t_i = 0; t_i < TOP_SRC_N; t_i++) top_src[t_i][0] = '\0';

    /* Basit seçim: her adımda kalanların en büyüğünü al. src_count küçük.
     *
     * Determinizm için ikinci anahtar olarak kaynak adını (strcmp artan)
     * kullanıyoruz: eşit hit sayısı durumunda alfabetik olarak ilk gelen
     * seçilsin. Böylece farklı koşularda eşitlikler aynı sonucu verir. */
    int used_flags[AGGREGATE_SRC_CAP];
    memset(used_flags, 0, sizeof(used_flags));
    for (int t_i = 0; t_i < TOP_SRC_N; t_i++) {
        int best = -1;
        long best_hits = -1;
        for (int i = 0; i < ctx->agg->src_count; i++) {
            if (used_flags[i]) continue;
            long h = ctx->agg->src_tbl[i].hits;
            if (h > best_hits) {
                best_hits = h;
                best = i;
            } else if (h == best_hits && best >= 0) {
                /* Eşit hit — adı alfabetik küçük olanı tercih et. */
                if (strcmp(ctx->agg->src_tbl[i].src,
                           ctx->agg->src_tbl[best].src) < 0) {
                    best = i;
                }
            }
        }
        if (best < 0 || best_hits <= 0) break;
        used_flags[best] = 1;
        snprintf(top_src[t_i], SOURCE_LEN, "%s", ctx->agg->src_tbl[best].src);
        top_hits[t_i] = ctx->agg->src_tbl[best].hits;
    }
    pthread_mutex_unlock(&ctx->agg->mtx);

    /* Region C'ye yaz — agg_mutex altında. */
    region_c_t *c = ctx->shm->c;
    pthread_mutex_lock(&c->agg_mutex);
    level_result_t *r = &c->results[ctx->level_idx];
    snprintf(r->level, LEVEL_STR_LEN, "%s", level_name((log_level_t)ctx->level_idx));
    r->total_entries       = ctx->agg->total_entries;
    r->total_weighted_score= ctx->agg->total_weighted;
    for (int k = 0; k < MAX_KEYWORDS; k++) {
        r->per_keyword_score[k] = ctx->agg->per_keyword[k];
    }
    for (int w = 0; w < MAX_WORKERS; w++) {
        r->per_thread_score[w] = ctx->agg->per_thread_score[w];
    }
    for (int t_i = 0; t_i < TOP_SRC_N; t_i++) {
        snprintf(r->top_source[t_i], SOURCE_LEN, "%s", top_src[t_i]);
        r->top_source_hits[t_i] = top_hits[t_i];
    }
    r->ready = 1;
    c->results_ready_mask |= (1 << ctx->level_idx);
    /* Aggregator hem sem_wait hem cond_timedwait kullanabilsin diye broadcast. */
    pthread_cond_broadcast(&c->agg_cond);
    pthread_mutex_unlock(&c->agg_mutex);

    /* İlgili semaphore'u post et. */
    sem_post(&c->sems[ctx->level_idx]);

    fprintf(stdout, "[PID:%d][TID:%d] Total entries: %ld | Total weighted score: %.1f\n",
            (int)getpid(), (int)my_tid, r->total_entries, r->total_weighted_score);
    fflush(stdout);

    /* Otomatik destructor'ın kendi TLS'imize tekrar flush yapmasını engelle. */
    pthread_setspecific(*ctx->tls_key, NULL);
    free(t); /* TLS belleği biz serbest bırakıyoruz */

    return NULL;
}

/* -----------------------------------------------------------------------------
 * analyzer_process_main
 * ----------------------------------------------------------------------------- */

int analyzer_process_main(shm_bundle_t *shm,
                          int level_idx,
                          char **keywords,
                          int num_keywords,
                          int worker_threads)
{
    fprintf(stdout, "[PID:%d] Analyzer %s started. Workers: %d\n",
            (int)getpid(), level_name((log_level_t)level_idx), worker_threads);
    fflush(stdout);

    /* Aggregate init. */
    analyzer_aggregate_t *agg = (analyzer_aggregate_t *)calloc(1, sizeof(*agg));
    if (!agg) {
        fprintf(stderr, "[PID:%d] analyzer aggregate alloc failed\n", (int)getpid());
        return -1;
    }
    pthread_mutex_init(&agg->mtx, NULL);
    pthread_cond_init(&agg->cond, NULL);
    g_agg = agg; /* destructor için */

    /* TLS key yarat — destructor ile. */
    if (pthread_key_create(&g_tls_key, tls_destructor) != 0) {
        fprintf(stderr, "[PID:%d] pthread_key_create failed\n", (int)getpid());
        free(agg);
        return -1;
    }

    /* Barrier: W worker. */
    pthread_barrier_t barrier;
    if (pthread_barrier_init(&barrier, NULL, (unsigned)worker_threads) != 0) {
        fprintf(stderr, "[PID:%d] pthread_barrier_init failed\n", (int)getpid());
        pthread_key_delete(g_tls_key);
        free(agg);
        return -1;
    }

    /* TID tablosu (W boyut). */
    pid_t *tid_table = (pid_t *)calloc((size_t)worker_threads, sizeof(pid_t));
    int reporter_idx = 0;

    /* Worker'ları yarat. */
    pthread_t    *tids = (pthread_t *)calloc((size_t)worker_threads, sizeof(pthread_t));
    worker_ctx_t *ctxs = (worker_ctx_t *)calloc((size_t)worker_threads, sizeof(worker_ctx_t));
    if (!tid_table || !tids || !ctxs) {
        fprintf(stderr, "[PID:%d] analyzer alloc failed\n", (int)getpid());
        pthread_barrier_destroy(&barrier);
        pthread_key_delete(g_tls_key);
        free(tid_table); free(tids); free(ctxs); free(agg);
        return -1;
    }

    for (int i = 0; i < worker_threads; i++) {
        ctxs[i].level_idx          = level_idx;
        ctxs[i].worker_idx         = i;
        ctxs[i].num_keywords       = num_keywords;
        ctxs[i].keywords           = keywords;
        ctxs[i].shm                = shm;
        ctxs[i].tls_key            = &g_tls_key;
        ctxs[i].barrier            = &barrier;
        ctxs[i].agg                = agg;
        ctxs[i].tid_table          = tid_table;
        ctxs[i].num_workers        = worker_threads;
        ctxs[i].reporter_worker_idx= &reporter_idx;
        if (pthread_create(&tids[i], NULL, analyzer_worker_main, &ctxs[i]) != 0) {
            fprintf(stderr, "[PID:%d] pthread_create worker %d failed\n",
                    (int)getpid(), i);
            /* Temizlik: başarılı olanları join et. */
            for (int j = 0; j < i; j++) pthread_join(tids[j], NULL);
            pthread_barrier_destroy(&barrier);
            pthread_key_delete(g_tls_key);
            free(tid_table); free(tids); free(ctxs);
            pthread_mutex_destroy(&agg->mtx);
            pthread_cond_destroy(&agg->cond);
            free(agg);
            return -1;
        }
    }

    for (int i = 0; i < worker_threads; i++) {
        pthread_join(tids[i], NULL);
    }

    fprintf(stdout, "[PID:%d] Analyzer %s exiting.\n",
            (int)getpid(), level_name((log_level_t)level_idx));
    fflush(stdout);

    /* Temizlik. */
    pthread_barrier_destroy(&barrier);
    pthread_key_delete(g_tls_key);
    pthread_mutex_destroy(&agg->mtx);
    pthread_cond_destroy(&agg->cond);
    free(agg);
    g_agg = NULL;
    free(tid_table);
    free(tids);
    free(ctxs);

    return 0;
}
