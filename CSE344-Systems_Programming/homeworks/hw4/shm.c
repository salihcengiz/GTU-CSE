/*
 * shm.c — Paylaşımlı bellek bölgelerinin oluşturulması, başlatılması,
 * circular buffer yardımcıları ve temizlik kodu.
 *
 * Tek `mmap(MAP_SHARED|MAP_ANONYMOUS)` çağrısı ile monolitik bir bellek bloğu
 * ayırır; bu bloğu offsetlere bölerek 4 mantıksal bölgeye dönüştürür.
 * Tüm mutex/cond/sem fork öncesi parent tarafından PROCESS_SHARED ile
 * başlatılır ve destroy yalnız parent tarafında çağrılır.
 */

#include "shm.h"

#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <unistd.h>

/* -----------------------------------------------------------------------------
 * Yardımcı: hizalama
 * ----------------------------------------------------------------------------- */

/* Verilen offset'i belirtilen alignment değerine yukarı yuvarlar.
 * Shared bellek içindeki bölge başlangıçlarını cache line'a yaslamak yarışı
 * azaltır; burada 64-byte (olası cache line boyutu) ile çalışıyoruz. */
static size_t align_up(size_t off, size_t alignment) {
    size_t rem = off % alignment;
    return rem ? (off + alignment - rem) : off;
}

#define SHM_ALIGN 64

/* region_a_t / region_b_level_t / region_d_t flexible array üyeleri içerdiği
 * için sizeof() sadece header'ı verir. Tam boyut = header + capacity * entry. */
static size_t region_a_full_size(int cap) {
    return align_up(sizeof(region_a_t) + (size_t)cap * sizeof(log_entry_t), SHM_ALIGN);
}
static size_t region_b_level_full_size(int cap) {
    return align_up(sizeof(region_b_level_t) + (size_t)cap * sizeof(log_entry_t), SHM_ALIGN);
}
static size_t region_d_full_size(int cap) {
    return align_up(sizeof(region_d_t) + (size_t)cap * sizeof(log_entry_t), SHM_ALIGN);
}
static size_t region_c_full_size(void) {
    return align_up(sizeof(region_c_t), SHM_ALIGN);
}

/* -----------------------------------------------------------------------------
 * Yardımcı: PROCESS_SHARED mutex / cond init
 * ----------------------------------------------------------------------------- */

/* Tek bir mutex'i PROCESS_SHARED attribute ile başlatır. Bu attribute olmadan
 * fork sonrası child'larda mutex davranışı tanımsızdır (ödev penaltısı -30). */
static int init_pshared_mutex(pthread_mutex_t *m) {
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr) != 0) return -1;
    /* PTHREAD_PROCESS_SHARED: mutex farklı süreçlerden kullanılabilir. */
    if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) != 0) {
        pthread_mutexattr_destroy(&attr);
        return -1;
    }
    int rc = pthread_mutex_init(m, &attr);
    pthread_mutexattr_destroy(&attr);
    return rc == 0 ? 0 : -1;
}

/* Condvar için aynı PROCESS_SHARED ayarı. Ayrıca CLOCK_MONOTONIC kullanmak
 * saatten etkilenmemek için tercih edilir; ancak POSIX'in portable yolu
 * CLOCK_REALTIME ile timedwait. Biz realtime kullanıyoruz (Debian 12'de
 * sorunsuz) — ödev gereksinimi explicit saat belirtmiyor. */
static int init_pshared_cond(pthread_cond_t *c) {
    pthread_condattr_t attr;
    if (pthread_condattr_init(&attr) != 0) return -1;
    if (pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) != 0) {
        pthread_condattr_destroy(&attr);
        return -1;
    }
    int rc = pthread_cond_init(c, &attr);
    pthread_condattr_destroy(&attr);
    return rc == 0 ? 0 : -1;
}

/* -----------------------------------------------------------------------------
 * shm_bundle_init
 * ----------------------------------------------------------------------------- */

int shm_bundle_init(shm_bundle_t *out,
                    int cap_a, int cap_b, int cap_d,
                    int total_readers)
{
    if (!out) return -1;
    /* Girdi validasyonu — ödev PDF'indeki sınırlar. Parent argparse zaten
     * kontrol etse de burada bir daha kontrol ediyoruz (defansif). */
    if (cap_a < 4 || cap_b < 4 || cap_d < 2 || total_readers < 1) return -1;

    /* Tüm bölgelerin toplam boyutunu hesapla. align_up her bölgeyi 64 byte'a
     * yuvarladığı için bölge pointer'ları hizalı olacaktır. */
    size_t off = 0;
    size_t size_a   = region_a_full_size(cap_a);
    size_t size_b   = region_b_level_full_size(cap_b);  /* bir level */
    size_t size_c   = region_c_full_size();
    size_t size_d   = region_d_full_size(cap_d);

    /* Layout: [A] [B0] [B1] [B2] [B3] [C] [D] */
    size_t total = size_a + NUM_LEVELS * size_b + size_c + size_d;

    /* MAP_ANONYMOUS + MAP_SHARED: disk backing yok, süreçler arası paylaşımlı. */
    void *base = mmap(NULL, total, PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (base == MAP_FAILED) {
        fprintf(stderr, "[shm] mmap failed: %s\n", strerror(errno));
        return -1;
    }

    /* Tüm bloğu sıfırla — özellikle count/head/tail gibi int alanlar 0 başlamalı. */
    memset(base, 0, total);

    /* Pointer yerleştirme */
    region_a_t *a = (region_a_t *)((char *)base + off);
    off += size_a;

    region_b_level_t *b[NUM_LEVELS];
    for (int i = 0; i < NUM_LEVELS; i++) {
        b[i] = (region_b_level_t *)((char *)base + off);
        off += size_b;
    }

    region_c_t *c = (region_c_t *)((char *)base + off);
    off += size_c;

    region_d_t *d = (region_d_t *)((char *)base + off);
    off += size_d;

    /* Region A alanlarını doldur */
    a->capacity        = cap_a;
    a->head            = 0;
    a->tail            = 0;
    a->count           = 0;
    a->total_readers   = total_readers;
    for (int i = 0; i < NUM_LEVELS; i++) a->eof_count_per_level[i] = 0;
    if (init_pshared_mutex(&a->input_mutex) != 0) goto fail;
    if (init_pshared_cond(&a->not_full_a)   != 0) goto fail;
    if (init_pshared_cond(&a->not_empty_a)  != 0) goto fail;

    /* Region B (×4) */
    for (int i = 0; i < NUM_LEVELS; i++) {
        b[i]->capacity    = cap_b;
        b[i]->head        = 0;
        b[i]->tail        = 0;
        b[i]->count       = 0;
        b[i]->eof_posted  = 0;
        if (init_pshared_mutex(&b[i]->level_mutex) != 0) goto fail;
        if (init_pshared_cond(&b[i]->not_full_b)   != 0) goto fail;
        if (init_pshared_cond(&b[i]->not_empty_b)  != 0) goto fail;
    }

    /* Region C */
    for (int i = 0; i < NUM_LEVELS; i++) {
        /* results[i].level alanını şimdiden dolduralım; reporting thread
         * üzerine yazabilir ama hazır olsa iyidir. */
        strncpy(c->results[i].level, level_name((log_level_t)i), LEVEL_STR_LEN - 1);
        c->results[i].level[LEVEL_STR_LEN - 1] = '\0';
        c->results[i].total_entries        = 0;
        c->results[i].total_weighted_score = 0.0;
        for (int k = 0; k < MAX_KEYWORDS; k++)
            c->results[i].per_keyword_score[k] = 0.0;
        for (int w = 0; w < MAX_WORKERS; w++)
            c->results[i].per_thread_score[w] = 0.0;
        for (int t = 0; t < TOP_SRC_N; t++) {
            c->results[i].top_source[t][0] = '\0';
            c->results[i].top_source_hits[t] = 0;
        }
        c->results[i].ready = 0;
        /* Unnamed pshared semaphore — pshared=1 */
        if (sem_init(&c->sems[i], 1, 0) != 0) {
            fprintf(stderr, "[shm] sem_init C[%d] failed: %s\n", i, strerror(errno));
            goto fail;
        }
    }
    c->results_ready_mask = 0;
    if (init_pshared_mutex(&c->agg_mutex) != 0) goto fail;
    if (init_pshared_cond(&c->agg_cond)   != 0) goto fail;

    /* Region D */
    d->capacity        = cap_d;
    d->head            = 0;
    d->tail            = 0;
    d->count           = 0;
    d->dispatcher_done = 0;
    if (init_pshared_mutex(&d->priority_mutex) != 0) goto fail;
    if (init_pshared_cond(&d->not_full_d)      != 0) goto fail;
    if (init_pshared_cond(&d->not_empty_d)     != 0) goto fail;

    /* Bundle doldur */
    out->base         = base;
    out->total_size   = total;
    out->a            = a;
    for (int i = 0; i < NUM_LEVELS; i++) out->b[i] = b[i];
    out->c            = c;
    out->d            = d;
    out->cap_a        = cap_a;
    out->cap_b        = cap_b;
    out->cap_d        = cap_d;
    out->total_readers= total_readers;

    return 0;

fail:
    /* Kısmi başlatılmış primitifleri zorla sıfırlayıp mmap'i geri bırak.
     * destroy çağrısı burada çalıştırılmaz çünkü bazı primitifler init
     * edilmemiş olabilir; en güvenlisi munmap sonrası OS temizliği. */
    fprintf(stderr, "[shm] init failed; unmapping region\n");
    munmap(base, total);
    return -1;
}

/* -----------------------------------------------------------------------------
 * shm_bundle_destroy
 * ----------------------------------------------------------------------------- */

void shm_bundle_destroy(shm_bundle_t *bundle) {
    if (!bundle || !bundle->base) return;

    /* Normal sonlanmada primitifleri destroy edip kernel-side kaydını
     * bırakmak zararsızdır, ancak SIGINT/SIGTERM ile sonlandırılmış bir
     * child hâlâ bir process-shared mutex veya cond tutuyor olabilir; bu
     * durumda pthread_mutex_destroy/pthread_cond_destroy bazı glibc
     * sürümlerinde iç futex üzerinde kilitlenip süresiz bekler.
     *
     * Tüm pthread primitifleri bu monolitik paylaşımlı bellek bloğunda
     * gömülü olduğundan, munmap bu kaynakları bütünüyle geri verir. Linux
     * üzerinde pthread_mutex_destroy / pthread_cond_destroy / sem_destroy
     * çağrılarının munmap'ten önce yapılması zorunlu değildir; process-shared
     * primitifler munmap ile güvenle serbest bırakılabilir.
     *
     * Bu yüzden burada destroy çağrılarını ATLAYIP, doğrudan munmap ile
     * tüm bölgeyi serbest bırakıyoruz. Böylece anormal sonlanmada da
     * parent temiz şekilde çıkabilir. */

    if (munmap(bundle->base, bundle->total_size) != 0) {
        fprintf(stderr, "[shm] munmap warning: %s\n", strerror(errno));
    }
    bundle->base = NULL;
    bundle->total_size = 0;
}

/* -----------------------------------------------------------------------------
 * Circular buffer yardımcıları
 * -----------------------------------------------------------------------------
 * Bu yardımcıların hepsi kendi mutex'lerini alır, ilgili condvar üzerinde
 * bekler, veriyi yazar/okur ve karşı tarafı sinyalle uyandırır. Busy-wait
 * yasak; her bekleme gerçek bir cond_wait üzerinden.
 */

/* -------- Region A -------- */

int region_a_push(region_a_t *a, const log_entry_t *e) {
    if (!a || !e) return -1;
    if (pthread_mutex_lock(&a->input_mutex) != 0) return -1;
    /* Buffer doluysa not_full_a üzerinde bekle. */
    while (a->count == a->capacity) {
        /* pthread_cond_wait mutex'i atomik olarak bırakır ve uyanırken geri alır. */
        if (pthread_cond_wait(&a->not_full_a, &a->input_mutex) != 0) {
            pthread_mutex_unlock(&a->input_mutex);
            return -1;
        }
    }
    /* Tail pozisyonuna kopyala; tail'i dairesel arttır. */
    a->buf[a->tail] = *e;
    a->tail = (a->tail + 1) % a->capacity;
    a->count++;
    /* Tüketiciyi uyandır: broadcast değil signal yeterli çünkü tüketici tek
     * (Dispatcher). Ancak Dispatcher cond_timedwait yapabilir; spurious
     * wakeup predicate loop ile toleranslı. */
    pthread_cond_signal(&a->not_empty_a);
    pthread_mutex_unlock(&a->input_mutex);
    return 0;
}

int region_a_pop_timed(region_a_t *a, log_entry_t *out, int timeout_sec) {
    if (!a || !out) return -1;
    if (pthread_mutex_lock(&a->input_mutex) != 0) return -1;

    /* Timeout hesapla. CLOCK_REALTIME tabanlı; condvar default attribute
     * bu saati kullanır (setclock ile değiştirmedik). */
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += (timeout_sec > 0 ? timeout_sec : DEFAULT_TIMEOUT_SEC);

    /* Buffer boşsa timedwait ile bekle. Timeout dolup hala boşsa çağırana 1
     * dön — Dispatcher böylece "termination olabilir mi?" kontrolü yapar. */
    while (a->count == 0) {
        int rc = pthread_cond_timedwait(&a->not_empty_a, &a->input_mutex, &ts);
        if (rc == ETIMEDOUT) {
            pthread_mutex_unlock(&a->input_mutex);
            return 1; /* zaman aşımı */
        }
        if (rc != 0) {
            pthread_mutex_unlock(&a->input_mutex);
            return -1;
        }
    }

    /* Head pozisyonundan oku. */
    *out = a->buf[a->head];
    a->head = (a->head + 1) % a->capacity;
    a->count--;
    /* Üretici(leri) uyandır. */
    pthread_cond_signal(&a->not_full_a);
    pthread_mutex_unlock(&a->input_mutex);
    return 0;
}

/* -------- Region B -------- */

int region_b_push(region_b_level_t *b, const log_entry_t *e) {
    if (!b || !e) return -1;
    if (pthread_mutex_lock(&b->level_mutex) != 0) return -1;
    while (b->count == b->capacity) {
        if (pthread_cond_wait(&b->not_full_b, &b->level_mutex) != 0) {
            pthread_mutex_unlock(&b->level_mutex);
            return -1;
        }
    }
    b->buf[b->tail] = *e;
    b->tail = (b->tail + 1) % b->capacity;
    b->count++;
    /* Worker'ları (çoklu tüketici) uyandır — broadcast güvenli seçim çünkü
     * her worker potansiyel olarak bekliyor olabilir. */
    pthread_cond_broadcast(&b->not_empty_b);
    pthread_mutex_unlock(&b->level_mutex);
    return 0;
}

int region_b_pop_or_eof(region_b_level_t *b, log_entry_t *out) {
    if (!b || !out) return -1;
    if (pthread_mutex_lock(&b->level_mutex) != 0) return -1;
    /* Buffer boşsa iki durum: (a) henüz EOF gelmemiş → bekle;
     * (b) eof_posted ve count==0 → bitmiş → 1 dön. */
    while (b->count == 0) {
        if (b->eof_posted) {
            pthread_mutex_unlock(&b->level_mutex);
            return 1;
        }
        if (pthread_cond_wait(&b->not_empty_b, &b->level_mutex) != 0) {
            pthread_mutex_unlock(&b->level_mutex);
            return -1;
        }
    }
    *out = b->buf[b->head];
    b->head = (b->head + 1) % b->capacity;
    b->count--;
    /* EOF marker'ın da bir entry olarak geçmesi bu noktada; caller
     * is_eof alanına bakar. eof_posted=1 olmadan da EOF bayraklı entry
     * gelirse worker kendi iç mantığında işler (forwarded EOF). */
    pthread_cond_signal(&b->not_full_b);
    pthread_mutex_unlock(&b->level_mutex);
    return 0;
}

/* -------- Region D -------- */

int region_d_push(region_d_t *d, const log_entry_t *e) {
    if (!d || !e) return -1;
    if (pthread_mutex_lock(&d->priority_mutex) != 0) return -1;
    while (d->count == d->capacity) {
        if (pthread_cond_wait(&d->not_full_d, &d->priority_mutex) != 0) {
            pthread_mutex_unlock(&d->priority_mutex);
            return -1;
        }
    }
    d->buf[d->tail] = *e;
    d->tail = (d->tail + 1) % d->capacity;
    d->count++;
    pthread_cond_signal(&d->not_empty_d);
    pthread_mutex_unlock(&d->priority_mutex);
    return 0;
}

int region_d_pop_or_done(region_d_t *d, log_entry_t *out) {
    if (!d || !out) return -1;
    if (pthread_mutex_lock(&d->priority_mutex) != 0) return -1;
    while (d->count == 0) {
        if (d->dispatcher_done) {
            pthread_mutex_unlock(&d->priority_mutex);
            return 1;
        }
        if (pthread_cond_wait(&d->not_empty_d, &d->priority_mutex) != 0) {
            pthread_mutex_unlock(&d->priority_mutex);
            return -1;
        }
    }
    *out = d->buf[d->head];
    d->head = (d->head + 1) % d->capacity;
    d->count--;
    pthread_cond_signal(&d->not_full_d);
    pthread_mutex_unlock(&d->priority_mutex);
    return 0;
}
