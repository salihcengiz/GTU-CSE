/*
 * shm.h — Paylaşımlı bellek bölgelerinin (A, B×4, C, D) tanımı ve API'si.
 *
 * Bu modül dört farklı mantıksal paylaşımlı bölgeyi tek bir anonim mmap
 * çağrısı içinde tutar; tüm mutex/cond/sem primitifleri fork öncesi parent
 * tarafından PTHREAD_PROCESS_SHARED ile başlatılır. Bu dosya ve shm.c, süreç
 * içi iletişimin omurgasıdır; değişiklik yaparken Context.md bölümleri ile
 * senkron tutulmalıdır.
 */

#ifndef HW4_SHM_H
#define HW4_SHM_H

#include "common.h"

/* -----------------------------------------------------------------------------
 * Region A — Dispatcher input queue
 * ----------------------------------------------------------------------------- */

/* Tüm Reader parser thread'lerinin yazdığı, Dispatcher'ın okuduğu tek giriş
 * kuyruğu. Çoklu üretici / tek tüketici düzeni; bu yüzden not_full_a ve
 * not_empty_a için iki ayrı condvar yeterli. `buf[]` flexible array member
 * olarak tanımlı; tam boyut parent tarafından mmap sırasında ayarlanır. */
typedef struct {
    pthread_mutex_t input_mutex;          /* PROCESS_SHARED: tüm alanları korur */
    pthread_cond_t  not_full_a;           /* PROCESS_SHARED */
    pthread_cond_t  not_empty_a;          /* PROCESS_SHARED */

    int capacity;                         /* -a ile geçilen değer */
    int head;                             /* bir sonraki üretim indexi */
    int tail;                             /* bir sonraki tüketim indexi */
    int count;                            /* mevcut eleman sayısı */

    /* Her severity için kaç Reader'ın EOF marker'ı gelmiş. Dispatcher bu
     * sayıyı total_readers ile karşılaştırarak o level için Region B'ye
     * tek EOF marker yayımlayıp yayımlamayacağını belirler. */
    int eof_count_per_level[NUM_LEVELS];

    /* Parent fork öncesi toplam Reader süreci sayısını buraya yazar; fork
     * sonrası yalnız okuma yapılır. */
    int total_readers;

    /* Flexible buffer: parent tam capacity kadar alan rezerve eder. */
    log_entry_t buf[];
} region_a_t;

/* -----------------------------------------------------------------------------
 * Region B — Her severity için ayrı bir circular buffer
 * ----------------------------------------------------------------------------- */

/* Tek üretici (Dispatcher) / çoklu tüketici (o level'in W worker'ı) düzeni.
 * Her Analyzer yalnız kendi level'inin buffer'ına bakar; bu sayede level
 * arası yarış yok. */
typedef struct {
    pthread_mutex_t level_mutex;          /* PROCESS_SHARED */
    pthread_cond_t  not_full_b;           /* PROCESS_SHARED: Dispatcher için */
    pthread_cond_t  not_empty_b;          /* PROCESS_SHARED: worker'lar için */

    int capacity;                         /* -b ile geçilen değer */
    int head;
    int tail;
    int count;

    /* Dispatcher bu level için tüm Reader'lardan EOF aldığını belirlediğinde
     * 1 yapar; workerlar `eof_posted && count==0` gördüğünde çıkar. */
    int eof_posted;

    log_entry_t buf[];
} region_b_level_t;

/* -----------------------------------------------------------------------------
 * Region C — Analyzer → Aggregator sonuç alanı
 * ----------------------------------------------------------------------------- */

/* 4 level_result_t + 4 sem_t + 1 mutex/cond. Level_result_t içindeki alanlar
 * reporting thread ve TLS destructor'lar tarafından agg_mutex altında yazılır;
 * Aggregator sem_wait + timedwait ile bekler. */
typedef struct {
    level_result_t  results[NUM_LEVELS];
    sem_t           sems[NUM_LEVELS];     /* unnamed, pshared=1 */
    pthread_mutex_t agg_mutex;            /* PROCESS_SHARED */
    pthread_cond_t  agg_cond;             /* PROCESS_SHARED: timedwait için */

    /* Her bitin 1 olması o seviyenin sonucunun Region C'de yazılı olduğunu
     * gösterir. sem_wait başarılı olduğunda bu bit zaten set olmalıdır. */
    int results_ready_mask;
} region_c_t;

/* -----------------------------------------------------------------------------
 * Region D — High-priority buffer
 * ----------------------------------------------------------------------------- */

/* Dispatcher yazar, Aggregator tüketir. Priority filter'a uyan her entry'nin
 * bir kopyası buraya yazılır; Aggregator sadece bu buffer üzerinden
 * "high-priority weighted score" toplamını hesaplar. */
typedef struct {
    pthread_mutex_t priority_mutex;       /* PROCESS_SHARED */
    pthread_cond_t  not_full_d;           /* PROCESS_SHARED */
    pthread_cond_t  not_empty_d;          /* PROCESS_SHARED */

    int capacity;
    int head;
    int tail;
    int count;

    /* Dispatcher tamamlandığında 1 yapar; Aggregator bunu ve count==0'ı
     * görünce yüksek-öncelik aşamasını sonlandırır. */
    int dispatcher_done;

    log_entry_t buf[];
} region_d_t;

/* -----------------------------------------------------------------------------
 * shm_bundle_t — tek mmap içinde tüm bölgelerin pointer'larını tutan yapı
 * ----------------------------------------------------------------------------- */

/* Parent, mmap ettiği monolitik bloğu 4 bölgeye bölüp pointer'ları bu yapıda
 * saklar. Child süreçler fork sonrası aynı yapıyı parent'tan kalıt alır;
 * pointer'lar zaten shared belleğe işaret ediyor. shm_base + total_size
 * alanları munmap için gereklidir. */
typedef struct {
    void           *base;             /* mmap bloğunun başı */
    size_t          total_size;       /* munmap için tam boyut */

    region_a_t     *a;                /* Region A başlangıcı */
    region_b_level_t *b[NUM_LEVELS];  /* 4 ayrı Region B level'i */
    region_c_t     *c;                /* Region C başlangıcı */
    region_d_t     *d;                /* Region D başlangıcı */

    /* Aynı parametrelerin kolay erişimi için kopyalar (capacity alanları
     * struct içinde de var ama parent validasyon için ayrı tutuyor). */
    int             cap_a;
    int             cap_b;
    int             cap_d;
    int             total_readers;
} shm_bundle_t;

/* -----------------------------------------------------------------------------
 * API
 * ----------------------------------------------------------------------------- */

/*
 * Dört bölgeyi tek mmap ile ayırır, tüm primitifleri PROCESS_SHARED olarak
 * başlatır ve ilgili pointer'ları doldurur. Hata durumunda stderr'e bilgi
 * basıp -1 döner (çağıran tarafın nasıl sonlandırılacağına karar vermesi için).
 *
 * Zorunlu: bu fonksiyon **fork'tan önce** çağrılmalıdır.
 */
int shm_bundle_init(shm_bundle_t *out,
                    int cap_a, int cap_b, int cap_d,
                    int total_readers);

/*
 * Tüm primitifleri destroy eder ve munmap yapar. Sadece parent tarafından
 * çağrılmalı (tüm child'lar çıktıktan sonra). Çocukların da aynı paylaşımlı
 * belleği munmap etmesi önerilir (leak detection kolaylaşır), ama destroy
 * yalnız parent'tadır.
 */
void shm_bundle_destroy(shm_bundle_t *bundle);

/*
 * Circular buffer yardımcıları — her üç buffer tipine de uygun şekilde
 * genel imza ile tanımlanmış. İçerde uygun mutex + condvar kullanılır.
 * Tüm fonksiyonlar başarısızlıkta -1, başarıda 0 döner.
 */

/* Region A'ya üretici tarafından entry yazar. Doluysa not_full_a'da bekler. */
int region_a_push(region_a_t *a, const log_entry_t *e);

/* Region A'dan Dispatcher tarafından entry okur. `timeout_sec` > 0 ise
 * pthread_cond_timedwait ile bekler; timeout dolarsa ve spec koşulları
 * sağlanıyorsa 1 döner (=> timeout). 0 başarı, -1 hata. */
int region_a_pop_timed(region_a_t *a, log_entry_t *out, int timeout_sec);

/* Region B level yazma (tek üretici Dispatcher). */
int region_b_push(region_b_level_t *b, const log_entry_t *e);

/* Region B level okuma (çoklu tüketici worker). Buffer boşsa ve eof_posted
 * set ise 1 döner (=> stream bitti). 0 başarı, -1 hata. */
int region_b_pop_or_eof(region_b_level_t *b, log_entry_t *out);

/* Region D yazma (Dispatcher). */
int region_d_push(region_d_t *d, const log_entry_t *e);

/* Region D okuma (Aggregator). dispatcher_done + count==0 → 1 döner. */
int region_d_pop_or_done(region_d_t *d, log_entry_t *out);

#endif /* HW4_SHM_H */
