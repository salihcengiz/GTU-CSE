/*  Ortak tipler, sabitler ve yardımcılar
 *
 * Bu başlık dosyası proje genelinde kullanılan sabit boyutlu veri tiplerini
 * (log_entry_t, level_result_t), severity seviyesi enum'unu, keyword/worker
 * üst sınırlarını ve ağırlık tablosunu barındırır. Tüm paylaşımlı bellek
 * bölgeleri bu tipleri sabit boyutlu array olarak kullanır; dolayısıyla bu
 * dosyadaki boyutlar süreçler arasında mutlaka aynı olmak zorundadır.
 */

#ifndef HW4_COMMON_H
#define HW4_COMMON_H

/* POSIX başlıklarının doğru ifade edilmesi için özellik makrolarını erken
 * tanımlıyoruz; pthread_barrier, clock_gettime, sigaction gibi sembollerin
 * gözükmesi için _POSIX_C_SOURCE >= 200809L olması gerekiyor. */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

/* Sabitler */

/* Maksimum keyword sayısı; -k bayrağı en fazla bu kadar virgülle ayrılmış
 * terim kabul eder. Shared memory struct'larının boyutu da bu sabite bağlı. */
#define MAX_KEYWORDS 8

/* Maksimum worker thread sayısı (bir Analyzer process içindeki worker üst
 * sınırı). level_result_t.per_thread_score dizisi bu sabite göre boyutlanır. */
#define MAX_WORKERS 64

/* Severity sayısı (ERROR, WARN, INFO, DEBUG) — her zaman tam olarak 4. */
#define NUM_LEVELS 4

/* Watchdog thread'inin snapshot basma periyodu (saniye). */
#define WATCHDOG_PERIOD_SEC 3

/* Reader thread'lerinin heartbeat yazma aralığı; parser thread her bu kadar
 * entry'de bir pipe'a heartbeat satırı yazar. */
#define HEARTBEAT_EVERY_LINES 50

/* Binary checkpoint dosyasının header sihirli sayısı; dosya bütünlüğünün
 * doğrulanması için kullanılır (ödev spesifikasyonu). */
#define BIN_MAGIC    0xC5E3440BU
#define BIN_VERSION  1U

/* Varsayılan timeout (saniye) — -T bayrağı ile üzerine yazılır. Dispatcher
 * ve Aggregator bu değeri pthread_cond_timedwait çağrılarında kullanır. */
#define DEFAULT_TIMEOUT_SEC 10

/* log_entry_t içindeki string alanların boyutları. Bu sabitler değişirse
 * shared memory layout tamamen bozulur; sürecler arası uyum için paylaşıyoruz. */
#define TS_LEN       20   /* "YYYY-MM-DD HH:MM:SS" + NUL */
#define LEVEL_STR_LEN 8
#define SOURCE_LEN   64   /* alfanumerik SOURCE + NUL (maks 63 karakter) */
#define MSG_LEN      512  /* mesaj gövdesi + NUL; taşarsa truncate edilir */

/* Region C içindeki top-3 source raporu için sabit. */
#define TOP_SRC_N 3

/* Severity Enum + Ağırlık Tablosu */

/* Severity seviyeleri. Değerler ödev tablosuyla ve çıktı sırasıyla
 * eşleşiyor: ERROR=0, WARN=1, INFO=2, DEBUG=3. */
typedef enum {
    LEVEL_ERROR = 0,
    LEVEL_WARN  = 1,
    LEVEL_INFO  = 2,
    LEVEL_DEBUG = 3
} log_level_t;

/* Seviye ağırlıkları: ERROR=4, WARN=3, INFO=2, DEBUG=1.
 * common.c olmadığı için static const olarak inline bırakıyoruz; her
 * translation unit kendi kopyasına sahip olacak ancak değerler aynı. */
static const int LEVEL_WEIGHT[NUM_LEVELS] = { 4, 3, 2, 1 };

/* Enum'u insan-okunur stringe çevirmek için yardımcı; parser ve çıktı
 * formatlayıcı tarafından kullanılır. */
static inline const char *level_name(log_level_t lv) {
    switch (lv) {
        case LEVEL_ERROR: return "ERROR";
        case LEVEL_WARN:  return "WARN";
        case LEVEL_INFO:  return "INFO";
        case LEVEL_DEBUG: return "DEBUG";
    }
    /* Spec dışı değer çağrıldıysa boş string dön (çağıran tarafın hatası). */
    return "";
}

/* String'ten enum'a çeviri; parser [LEVEL] token'ı için kullanır. Geçersiz
 * değer -1 döndürür → çağıran satırı sessizce atlamalıdır. */
static inline int level_from_str(const char *s) {
    if (!s) return -1;
    if (!strcmp(s, "ERROR")) return LEVEL_ERROR;
    if (!strcmp(s, "WARN"))  return LEVEL_WARN;
    if (!strcmp(s, "INFO"))  return LEVEL_INFO;
    if (!strcmp(s, "DEBUG")) return LEVEL_DEBUG;
    return -1;
}

/* log_entry_t — Shared memory üzerinde taşınan sabit boyutlu log kaydı */

/* Bu struct tüm paylaşımlı bölgelerde (A, B×4, D) circular buffer elemanı
 * olarak kullanılır. Sabit boyutlu olması zorunlu; mmap layout'u bu struct'ın
 * sizeof() değerine bağlı. Taşma durumunda alanlar truncate edilir (parser
 * asla overflow yapmamak için snprintf / memcpy + NUL terminatör kullanır). */
typedef struct {
    /* "YYYY-MM-DD HH:MM:SS" formatında zaman damgası + NUL. */
    char timestamp[TS_LEN];

    /* "ERROR"/"WARN"/"INFO"/"DEBUG" string kopyası (kolay debug için tutuluyor). */
    char level_str[LEVEL_STR_LEN];

    /* Sayısal seviye indeksi (0..3). Dispatcher routing için bunu kullanır. */
    int  level_idx;

    /* SOURCE alanı (alfanumerik, maks 63 karakter) + NUL. */
    char source[SOURCE_LEN];

    /* Mesaj gövdesi (maks MSG_LEN-1 karakter) + NUL. Daha uzun mesajlar
     * parser tarafında sessizce kırpılır; spec malformed bile saymıyor. */
    char message[MSG_LEN];

    /* EOF marker bayrağı: 1 ise bu entry bir normal log satırı değil,
     * "bu dosyanın bu seviyesi için EOF" anlamını taşır. level_idx yine geçerli,
     * diğer alanların içeriği önemsizdir (parser NUL doldurur). */
    int  is_eof;
} log_entry_t;

/* level_result_t — Analyzer → Aggregator sonuç yapısı (Region C) */

/* Her severity için bir adet bu struct bulunur. Boyut sabit; binary checkpoint
 * dosyasının layout'u doğrudan bu struct'ı fwrite eder. Dolayısıyla bu yapıyı
 * değiştirirsen binary dosya formatı da değişir ve grader script'i
 * kırılabilir. */
typedef struct {
    /* "ERROR"/"WARN"/"INFO"/"DEBUG" — sıralama sonrası çıktıyı kolaylaştırır. */
    char   level[LEVEL_STR_LEN];

    /* Bu seviyede işlenen toplam entry sayısı (tüm worker'ların katkısı). */
    long   total_entries;

    /* Bu seviye için toplam ağırlıklı skor (keyword cross-toplamı × weight). */
    double total_weighted_score;

    /* Keyword başına ağırlıklı skor; argparse sırasında -k ile verilen sıra. */
    double per_keyword_score[MAX_KEYWORDS];

    /* Worker başına toplam ağırlıklı katkı; raporun per-thread bölümü için. */
    double per_thread_score[MAX_WORKERS];

    /* Top-3 source alanları: isim + ağırlıklı hit sayısı. Değer olmazsa
     * hit=0 ve isim boş string. */
    char   top_source[TOP_SRC_N][SOURCE_LEN];
    long   top_source_hits[TOP_SRC_N];

    /* Aggregator'ın "bu sonuç hazır mı" kontrolü için bayrak. sem_t posta
     * ek olarak redundant bir fail-safe. */
    int    ready;
} level_result_t;

/* Argüman yapısı (CLI parse sonrası taşıyıcı) */

/* Komut satırı argümanları ve config/filter dosyalarından okunan değerler
 * tek bir struct altında toplanır; bu struct parent tarafından oluşturulur
 * ve tüm child süreçlerin global/stack state'ine kopyalanır (fork ile zaten
 * doğrudan kopyalanıyor — struct parent'ın heap/stack'inde tutulur). */
typedef struct {
    const char *config_path;   /* -c */
    const char *filter_path;   /* -f */
    const char *output_text;   /* -o */
    const char *output_bin;    /* -O */

    /* Keyword listesi: heap'te ayrılmış num_keywords adet null-terminated
     * string pointer. Parent allocate eder, child'lar aynı adrese erişir
     * (fork sonrası adresler aynı — COW ile korunur; sadece read). */
    char  *keywords[MAX_KEYWORDS];
    int    num_keywords;

    int reader_threads;        /* -t */
    int worker_threads;        /* -w */
    int cap_a;                 /* -a Region A */
    int cap_b;                 /* -b Region B (her level) */
    int cap_d;                 /* -d Region D */
    int timeout_sec;           /* -T */

    /* Config dosyasından okunan log dosyası yolları. */
    char **log_files;          /* heap array of strings */
    int    num_files;

    /* Filter dosyasından okunan priority SOURCE isimleri. */
    char **priority_sources;   /* heap array of strings */
    int    num_priority;
} cli_args_t;

/* Paylaşımlı global sinyal bayrağı (parent process içinde) */

/* SIGINT handler tarafından set edilen async-signal-safe bayrak. Sadece parent
 * process'te tanımlıdır; diğer süreçler SIGTERM ile sonlandırılır. Tanımı
 * main.c'de; diğer dosyalar extern olarak görür. */
extern volatile sig_atomic_t g_sigint_received;

/* Yardımcı: güvenli overlapping substring sayımı */

/* Örtüşen (overlapping) substring sayımı yapan yardımcı. "errorer" stringinde
 * "error" iki kez bulunur. strstr bunu kaçırır; bu yüzden 1-byte'lık kayan
 * pencere ile memcmp yaparız. Boş needle 0 döner. */
static inline size_t count_overlapping(const char *hay, const char *needle) {
    if (!hay || !needle) return 0;
    size_t klen = 0; while (needle[klen]) klen++;
    if (klen == 0) return 0;
    size_t hlen = 0; while (hay[hlen]) hlen++;
    if (klen > hlen) return 0;
    size_t c = 0;
    /* Pencereyi her adımda 1 kaydırıyoruz; klasik strstr-based yaklaşım
     * örtüşenleri kaçırırdı. */
    for (size_t i = 0; i + klen <= hlen; i++) {
        /* İlk karakter eşleşmeden memcmp'i çağırmayı atlayarak bir miktar
         * hızlanma sağlıyoruz — işlevsel fark yok. */
        if (hay[i] == needle[0]) {
            int ok = 1;
            for (size_t j = 1; j < klen; j++) {
                if (hay[i + j] != needle[j]) { ok = 0; break; }
            }
            if (ok) c++;
        }
    }
    return c;
}

/* Yardımcı: stderr'e hata yazıp errno ile exit(2) */

/* Ölümcül hata mesajı basıp exit(2) yapan küçük yardımcı makrosu.
 * printf ailesi async-signal-safe DEĞİLDİR; sadece normal kod yolunda
 * kullanılır (sinyal handler içinde değil!). */
#define DIE(...) do { \
    fprintf(stderr, "[FATAL] "); \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n"); \
    exit(2); \
} while (0)

#endif
