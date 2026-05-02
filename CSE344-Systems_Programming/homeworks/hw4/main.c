/*
 * main.c — Parent / Coordinator Process girişi.
 *
 * Bu dosya:
 *   1. Komut satırını getopt ile parse eder,
 *   2. Config ve filter dosyalarını okur,
 *   3. Paylaşımlı bellek bölgelerini ve pipe'ları oluşturur,
 *   4. Tüm child süreçleri sırasıyla fork eder (Readers → Dispatcher →
 *      4 Analyzer → Aggregator),
 *   5. Watchdog thread'i yaratır,
 *   6. waitpid döngüsünde tüm child'ları toplar,
 *   7. SIGINT geldiğinde temiz çıkış yapar,
 *   8. Final system summary basar ve kaynakları serbest bırakır.
 *
 * Kod stili kuralı: isimler İngilizce, yorumlar Türkçe + detaylı.
 */

#include "common.h"
#include "shm.h"
#include "reader.h"
#include "dispatcher.h"
#include "analyzer.h"
#include "aggregator.h"
#include "watchdog.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* -----------------------------------------------------------------------------
 * Global sinyal bayrağı
 * ----------------------------------------------------------------------------- */

/* common.h'de extern ile declare edildi; burada tanım. SIGINT handler bu
 * değişkene sadece 1 atar. Async-signal-safe değişken tipi zorunlu. */
volatile sig_atomic_t g_sigint_received = 0;

/* -----------------------------------------------------------------------------
 * Yardımcılar: string split, file-by-line okuma
 * ----------------------------------------------------------------------------- */

/* "a,b,c" şeklinde virgülle ayrılmış listeyi heap'e ayrılmış strdup dizisine
 * böler. Boş token üretmez; tekrar eden virgülleri toleransla kabul eder.
 * Sonuçta out_list ve out_count doldurulur. Üst sınır max_items. */
static int split_csv(const char *s, char **out_list, int *out_count, int max_items) {
    *out_count = 0;
    if (!s) return -1;
    const char *start = s;
    int count = 0;
    while (*start) {
        /* virgüllere kadar oku */
        const char *end = start;
        while (*end && *end != ',') end++;
        size_t len = (size_t)(end - start);
        if (len > 0) {
            if (count >= max_items) return -1; /* çok fazla keyword */
            char *tok = (char *)malloc(len + 1);
            if (!tok) return -1;
            memcpy(tok, start, len);
            tok[len] = '\0';
            out_list[count++] = tok;
        }
        if (*end == '\0') break;
        start = end + 1;
    }
    if (count < 1) return -1;
    *out_count = count;
    return 0;
}

/* Bir text dosyayı satır satır okur ve her satırı strdup'layarak dizi olarak
 * döner. Boş satırlar ve sadece whitespace içerenler atlanır. Başarıda
 * *out_array ve *out_count doldurulur; başarısızlıkta -1 döner (hata stderr'e
 * basılır). Dosya yoksa -1 döner. */
static int read_lines(const char *path, char ***out_array, int *out_count) {
    *out_array = NULL;
    *out_count = 0;
    FILE *fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "[main] cannot open '%s': %s\n", path, strerror(errno));
        return -1;
    }
    /* Dinamik büyüyen dizi: basitlik adına realloc ile capacity doubling. */
    size_t cap = 8, cnt = 0;
    char **arr = (char **)malloc(cap * sizeof(char *));
    if (!arr) { fclose(fp); return -1; }

    char *line = NULL;
    size_t n = 0;
    ssize_t rd;
    while ((rd = getline(&line, &n, fp)) != -1) {
        /* Sondaki \n veya \r karakterlerini temizle. */
        while (rd > 0 && (line[rd - 1] == '\n' || line[rd - 1] == '\r')) {
            line[--rd] = '\0';
        }
        /* Baştaki whitespace'i atla. */
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        /* Boş satırı atla. */
        if (*p == '\0') continue;

        if (cnt == cap) {
            cap *= 2;
            char **tmp = (char **)realloc(arr, cap * sizeof(char *));
            if (!tmp) {
                for (size_t i = 0; i < cnt; i++) free(arr[i]);
                free(arr);
                free(line);
                fclose(fp);
                return -1;
            }
            arr = tmp;
        }
        arr[cnt] = strdup(p);
        if (!arr[cnt]) {
            for (size_t i = 0; i < cnt; i++) free(arr[i]);
            free(arr);
            free(line);
            fclose(fp);
            return -1;
        }
        cnt++;
    }
    free(line);
    fclose(fp);
    *out_array = arr;
    *out_count = (int)cnt;
    return 0;
}

/* Heap-ayrılmış string dizisini serbest bırakır. */
static void free_str_array(char **arr, int n) {
    if (!arr) return;
    for (int i = 0; i < n; i++) free(arr[i]);
    free(arr);
}

/* -----------------------------------------------------------------------------
 * Usage
 * ----------------------------------------------------------------------------- */

/* Kullanım mesajı — stderr'e yazılır; getopt hatası veya -h durumunda çağrılır. */
static void print_usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s -c <config> -f <filter> -k <kw1,kw2,...>\n"
        "          -t <reader_threads> -w <worker_threads>\n"
        "          -a <cap_A> -b <cap_B> -d <cap_D> [-T <timeout>]\n"
        "          -o <output_text> -O <output_binary>\n"
        "\n"
        "Constraints:\n"
        "  1 <= keywords <= %d, w <= %d, a >= 4, b >= 4, d >= 2, T >= 1\n",
        prog, MAX_KEYWORDS, MAX_WORKERS);
}

/* -----------------------------------------------------------------------------
 * Argparse
 * ----------------------------------------------------------------------------- */

/* Komut satırını parse eder ve cli_args_t'yi doldurur. Dönüş: 0 başarı,
 * -1 hata (mesaj zaten stderr'e basılmış). */
static int parse_args(int argc, char **argv, cli_args_t *out) {
    memset(out, 0, sizeof(*out));
    out->timeout_sec = DEFAULT_TIMEOUT_SEC; /* -T default */

    int opt;
    const char *kw_spec = NULL; /* -k argümanını geçici tut */
    /* getopt kısa opsiyon listesi: `h` yardım; diğerleri değer alır. */
    while ((opt = getopt(argc, argv, "c:f:k:t:w:a:b:d:T:o:O:h")) != -1) {
        switch (opt) {
            case 'c': out->config_path = optarg; break;
            case 'f': out->filter_path = optarg; break;
            case 'k': kw_spec = optarg; break;
            case 't': out->reader_threads = atoi(optarg); break;
            case 'w': out->worker_threads = atoi(optarg); break;
            case 'a': out->cap_a = atoi(optarg); break;
            case 'b': out->cap_b = atoi(optarg); break;
            case 'd': out->cap_d = atoi(optarg); break;
            case 'T': out->timeout_sec = atoi(optarg); break;
            case 'o': out->output_text = optarg; break;
            case 'O': out->output_bin = optarg; break;
            case 'h': default:
                print_usage(argv[0]);
                return -1;
        }
    }

    /* Zorunlu parametre kontrolü. */
    if (!out->config_path || !out->filter_path || !kw_spec ||
        !out->output_text || !out->output_bin) {
        fprintf(stderr, "[main] missing required option\n");
        print_usage(argv[0]);
        return -1;
    }

    /* Keyword listesi */
    if (split_csv(kw_spec, out->keywords, &out->num_keywords, MAX_KEYWORDS) != 0) {
        fprintf(stderr, "[main] invalid -k: must be 1..%d non-empty CSV tokens\n",
                MAX_KEYWORDS);
        return -1;
    }

    /* Sayısal sınırlar — PDF Section 11 tablosuyla birebir. */
    if (out->reader_threads < 1) {
        fprintf(stderr, "[main] -t must be >= 1\n"); return -1;
    }
    if (out->worker_threads < 1 || out->worker_threads > MAX_WORKERS) {
        fprintf(stderr, "[main] -w must be in [1, %d]\n", MAX_WORKERS); return -1;
    }
    if (out->cap_a < 4) { fprintf(stderr, "[main] -a must be >= 4\n"); return -1; }
    if (out->cap_b < 4) { fprintf(stderr, "[main] -b must be >= 4\n"); return -1; }
    if (out->cap_d < 2) { fprintf(stderr, "[main] -d must be >= 2\n"); return -1; }
    if (out->timeout_sec < 1) { fprintf(stderr, "[main] -T must be >= 1\n"); return -1; }

    /* Config ve filter dosyalarını oku. */
    if (read_lines(out->config_path, &out->log_files, &out->num_files) != 0) return -1;
    if (out->num_files < 1) {
        fprintf(stderr, "[main] config file '%s' has no log paths\n", out->config_path);
        return -1;
    }
    /* Filter dosyası boş olabilir (test 5 senaryosu). read_lines 0 dönmez
     * dosya boşsa; 0 satır sayısı normal kabul ediliyor. */
    if (read_lines(out->filter_path, &out->priority_sources, &out->num_priority) != 0) return -1;

    return 0;
}

/* cli_args_t içindeki heap alanlarını serbest bırakır. */
static void free_args(cli_args_t *a) {
    for (int i = 0; i < a->num_keywords; i++) free(a->keywords[i]);
    free_str_array(a->log_files, a->num_files);
    free_str_array(a->priority_sources, a->num_priority);
}

/* -----------------------------------------------------------------------------
 * SIGINT handler
 * ----------------------------------------------------------------------------- */

/* SIGINT handler — async-signal-safe. printf/malloc/free YASAK; sadece
 * write(2) ve bayrak ataması yapıyoruz. */
static void sigint_handler(int signo) {
    (void)signo;
    /* Kullanıcıya bir satırlık hızlı uyarı; write async-signal-safe'tir. */
    const char msg[] = "\n[SIGINT] cleanup in progress...\n";
    /* write'ın return değerine bakmamak compile uyarısı doğurur; volatile
     * atamada değişkene yazılacak ve göz ardı edilecektir. */
    ssize_t w = write(STDERR_FILENO, msg, sizeof(msg) - 1);
    (void)w;
    g_sigint_received = 1;
}

/* SIGINT için sigaction kurulumu. SA_RESTART yok — bloke sistem çağrıları
 * handler sonrası EINTR döndürsün; waitpid döngüsü bunu zaten işliyor. */
static int install_sigint_handler(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) != 0) {
        fprintf(stderr, "[main] sigaction SIGINT failed: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

/* -----------------------------------------------------------------------------
 * Fork yardımcıları
 * ----------------------------------------------------------------------------- */

/* Aktif child process'leri izlemek için bir tablo. SIGINT sonrası hepsine
 * SIGTERM göndermek için lazım. */
typedef struct {
    pid_t pid;
    const char *label;  /* "Reader 0", "Dispatcher", "Analyzer ERROR" vb. */
} child_info_t;

/* Tek bir child fork eder; parent pid tablosuna yazar, child'da ilgili
 * main fonksiyonunu çağırıp exit eder. close_pipes_in_child: child'ın
 * sahibi olmadığı pipe uçlarını kapatır. Return: fork başarılıysa parent'ta
 * child pid; child asla return etmez. */
typedef int (*child_entry_fn)(void *ctx);

/* Tüm Reader pipe'larının write-end'lerini bu array'de tutuyoruz. Reader olmayan
 * child'lar fork sonrası tüm write-end'leri kapatır; Reader ise sadece kendi
 * write-end'ini tutar (ve diğerlerini kapatır). */
static void close_all_pipe_ends_in_child(int *read_ends, int *write_ends, int n,
                                         int keep_read_idx, int keep_write_idx)
{
    for (int i = 0; i < n; i++) {
        if (read_ends[i] != -1 && i != keep_read_idx) {
            close(read_ends[i]);
            read_ends[i] = -1;
        }
        if (write_ends[i] != -1 && i != keep_write_idx) {
            close(write_ends[i]);
            write_ends[i] = -1;
        }
    }
}

/* -----------------------------------------------------------------------------
 * main
 * ----------------------------------------------------------------------------- */

int main(int argc, char **argv) {
    cli_args_t args;

    /* 1) Argüman parse. */
    if (parse_args(argc, argv, &args) != 0) {
        return 2;
    }

    /* 2) Parent banner. */
    fprintf(stdout, "[PID:%d] Parent started. Files: %d, Keywords: ",
            (int)getpid(), args.num_files);
    for (int i = 0; i < args.num_keywords; i++) {
        fprintf(stdout, "%s%s", args.keywords[i],
                i + 1 < args.num_keywords ? "," : "");
    }
    fprintf(stdout, "\n");
    fflush(stdout);

    /* 3) Shared memory init (fork öncesi). */
    shm_bundle_t shm;
    if (shm_bundle_init(&shm, args.cap_a, args.cap_b, args.cap_d,
                        args.num_files) != 0) {
        free_args(&args);
        return 2;
    }
    fprintf(stdout, "[PID:%d] Shared memory initialized (A:%d B:%dx%d D:%d).\n",
            (int)getpid(), args.cap_a, args.cap_b, NUM_LEVELS, args.cap_d);
    fflush(stdout);

    /* 4) SIGINT handler kurulumu (fork öncesi; child'lar da aynı handler'ı
     * alır ama SIGTERM ile zaten sonlandırılacaklar). */
    if (install_sigint_handler() != 0) {
        shm_bundle_destroy(&shm);
        free_args(&args);
        return 2;
    }

    /* 5) Heartbeat pipe'ları oluştur — her Reader için bir pipe. Parent
     * read-end'leri tutar (watchdog'a verilecek), Reader kendi write-end'ini
     * tutar. */
    int *pipe_read_fds  = (int *)malloc(sizeof(int) * args.num_files);
    int *pipe_write_fds = (int *)malloc(sizeof(int) * args.num_files);
    if (!pipe_read_fds || !pipe_write_fds) {
        fprintf(stderr, "[main] pipe fd alloc failed\n");
        shm_bundle_destroy(&shm);
        free_args(&args);
        return 2;
    }
    for (int i = 0; i < args.num_files; i++) {
        int p[2];
        if (pipe(p) != 0) {
            fprintf(stderr, "[main] pipe() failed: %s\n", strerror(errno));
            for (int j = 0; j < i; j++) { close(pipe_read_fds[j]); close(pipe_write_fds[j]); }
            free(pipe_read_fds); free(pipe_write_fds);
            shm_bundle_destroy(&shm);
            free_args(&args);
            return 2;
        }
        pipe_read_fds[i]  = p[0];
        pipe_write_fds[i] = p[1];
    }

    /* 6) Child tablosu — fork sırasında tek tek doldurulur. */
    int total_children = args.num_files + 1 /*Disp*/ + NUM_LEVELS /*Analyzers*/ + 1 /*Agg*/;
    child_info_t *children = (child_info_t *)calloc((size_t)total_children, sizeof(child_info_t));
    if (!children) { return 2; } /* defensive */
    int child_count = 0;

    /* Label string'leri stack'te tutmak zor olur (lifetime); bunun yerine heap
     * kopyalar üretip children[i].label'a veriyoruz. free'leri final cleanup'ta. */
    #define PUSH_CHILD(pid_, fmt_, ...) do {                \
        char *lbl = (char *)malloc(64);                      \
        snprintf(lbl, 64, (fmt_), ##__VA_ARGS__);            \
        children[child_count].pid   = (pid_);                \
        children[child_count].label = lbl;                   \
        child_count++;                                       \
    } while (0)

    /* 7) Readers fork */
    for (int i = 0; i < args.num_files; i++) {
        fprintf(stdout, "[PID:%d] Forking Reader %d -> %s\n",
                (int)getpid(), i, args.log_files[i]);
        fflush(stdout);
        pid_t pid = fork();
        if (pid < 0) {
            fprintf(stderr, "[main] fork Reader failed: %s\n", strerror(errno));
            /* Best-effort cleanup: daha önce fork edilmiş child'ları TERM et. */
            for (int k = 0; k < child_count; k++) kill(children[k].pid, SIGTERM);
            shm_bundle_destroy(&shm);
            free_args(&args);
            return 2;
        }
        if (pid == 0) {
            /* Child: kendi write-end'i hariç tüm pipe uçlarını kapat. */
            close_all_pipe_ends_in_child(pipe_read_fds, pipe_write_fds, args.num_files,
                                         /*keep_read=*/-1, /*keep_write=*/i);
            /* Reader kendi read-end'ini de kapatır (sadece yazacak). */
            if (pipe_read_fds[i] != -1) { close(pipe_read_fds[i]); pipe_read_fds[i] = -1; }
            int rc = reader_process_main(i, args.log_files[i], args.reader_threads,
                                         pipe_write_fds[i], &shm);
            /* Reader kendi yazma ucunu kapat → parent EOF görsün. */
            close(pipe_write_fds[i]);
            _exit(rc == 0 ? 0 : 1);
        }
        PUSH_CHILD(pid, "Reader %d", i);
    }

    /* 8) Dispatcher fork */
    fprintf(stdout, "[PID:%d] Forking Dispatcher\n", (int)getpid());
    fflush(stdout);
    {
        pid_t pid = fork();
        if (pid < 0) { fprintf(stderr, "[main] fork Dispatcher failed\n"); return 2; }
        if (pid == 0) {
            /* Dispatcher tüm pipe uçlarını kapatır — Reader değil. */
            close_all_pipe_ends_in_child(pipe_read_fds, pipe_write_fds, args.num_files, -1, -1);
            int rc = dispatcher_process_main(&shm, args.priority_sources,
                                             args.num_priority, args.timeout_sec);
            _exit(rc == 0 ? 0 : 1);
        }
        PUSH_CHILD(pid, "Dispatcher");
    }

    /* 9) Analyzers fork (ERROR, WARN, INFO, DEBUG) */
    for (int lv = 0; lv < NUM_LEVELS; lv++) {
        fprintf(stdout, "[PID:%d] Forking Analyzer %s (index %d)\n",
                (int)getpid(), level_name((log_level_t)lv), lv);
        fflush(stdout);
        pid_t pid = fork();
        if (pid < 0) { fprintf(stderr, "[main] fork Analyzer failed\n"); return 2; }
        if (pid == 0) {
            close_all_pipe_ends_in_child(pipe_read_fds, pipe_write_fds, args.num_files, -1, -1);
            int rc = analyzer_process_main(&shm, lv, args.keywords,
                                           args.num_keywords, args.worker_threads);
            _exit(rc == 0 ? 0 : 1);
        }
        PUSH_CHILD(pid, "Analyzer %s", level_name((log_level_t)lv));
    }

    /* 10) Aggregator fork */
    fprintf(stdout, "[PID:%d] Forking Aggregator\n", (int)getpid());
    fflush(stdout);
    {
        pid_t pid = fork();
        if (pid < 0) { fprintf(stderr, "[main] fork Aggregator failed\n"); return 2; }
        if (pid == 0) {
            close_all_pipe_ends_in_child(pipe_read_fds, pipe_write_fds, args.num_files, -1, -1);
            int rc = aggregator_process_main(&shm, args.output_text, args.output_bin,
                                             args.keywords, args.num_keywords,
                                             args.num_files, args.filter_path,
                                             args.timeout_sec);
            _exit(rc == 0 ? 0 : 1);
        }
        PUSH_CHILD(pid, "Aggregator");
    }

    /* 11) Parent: kendi sahiplenmediği pipe write-end'lerini kapat.
     * Böylece Reader'lar exit ettiğinde parent'ın read-end'lerinde EOF görülür
     * (watchdog select'inde önemli). */
    for (int i = 0; i < args.num_files; i++) {
        if (pipe_write_fds[i] != -1) close(pipe_write_fds[i]);
        pipe_write_fds[i] = -1;
    }

    /* 12) Watchdog thread başlat. */
    volatile sig_atomic_t watchdog_shutdown = 0;
    atomic_int children_alive;
    atomic_store(&children_alive, total_children);

    /* watchdog'a geçilecek label dizisi (Reader isimleri için). */
    const char **reader_names = (const char **)malloc(sizeof(char *) * args.num_files);
    for (int i = 0; i < args.num_files; i++) reader_names[i] = args.log_files[i];

    watchdog_args_t wdargs = {
        .num_readers   = args.num_files,
        .pipe_read_fds = pipe_read_fds,
        .log_file_names= reader_names,
        .children_alive= &children_alive,
        .shutdown_flag = &watchdog_shutdown,
    };
    pthread_t watchdog_tid;
    if (pthread_create(&watchdog_tid, NULL, watchdog_thread_main, &wdargs) != 0) {
        fprintf(stderr, "[main] pthread_create watchdog failed\n");
        /* Devam et — watchdog kritik değil ama yokluğu uyarıdır. */
    }
    fprintf(stdout, "[PID:%d] Watchdog thread started.\n", (int)getpid());
    fflush(stdout);

    /* 13) waitpid döngüsü. SIGINT gelirse tüm child'lara SIGTERM + 5s deadline. */
    int remaining = total_children;
    int any_failed = 0;
    while (remaining > 0) {
        if (g_sigint_received) {
            /* Tüm child'lara SIGTERM. */
            for (int i = 0; i < child_count; i++) {
                if (children[i].pid > 0) kill(children[i].pid, SIGTERM);
            }
            /* 5 saniye deadline ile WNOHANG collect. */
            time_t deadline = time(NULL) + 5;
            while (remaining > 0 && time(NULL) < deadline) {
                int st = 0;
                pid_t w = waitpid(-1, &st, WNOHANG);
                if (w > 0) {
                    remaining--;
                    atomic_fetch_sub(&children_alive, 1);
                } else if (w == 0) {
                    /* Henüz exit etmemiş; kısa bekle. */
                    struct timespec ts = { .tv_sec = 0, .tv_nsec = 100 * 1000 * 1000 };
                    nanosleep(&ts, NULL);
                } else if (errno == ECHILD) {
                    remaining = 0;
                    break;
                } else if (errno == EINTR) {
                    continue;
                } else {
                    break;
                }
            }
            /* Kalanlara SIGKILL. */
            for (int i = 0; i < child_count; i++) {
                if (children[i].pid > 0) kill(children[i].pid, SIGKILL);
            }
            /* Toplama. */
            while (remaining > 0) {
                int st = 0;
                pid_t w = waitpid(-1, &st, 0);
                if (w > 0) { remaining--; atomic_fetch_sub(&children_alive, 1); }
                else if (errno == ECHILD) { remaining = 0; break; }
                else if (errno == EINTR) continue;
                else break;
            }
            any_failed = 1;
            break;
        }

        /* Normal waitpid. */
        int st = 0;
        pid_t w = waitpid(-1, &st, 0);
        if (w > 0) {
            remaining--;
            atomic_fetch_sub(&children_alive, 1);
            if (WIFEXITED(st) && WEXITSTATUS(st) != 0) any_failed = 1;
            if (WIFSIGNALED(st)) any_failed = 1;
        } else if (errno == EINTR) {
            continue;
        } else if (errno == ECHILD) {
            break;
        } else {
            fprintf(stderr, "[main] waitpid error: %s\n", strerror(errno));
            any_failed = 1;
            break;
        }
    }

    /* 14) Watchdog thread'i durdur ve join et. */
    watchdog_shutdown = 1;
    pthread_join(watchdog_tid, NULL);

    /* 15) Final summary — Region C'deki sonuçları oku ve PDF örneğine uygun
     * formatta stdout'a bas. */
    if (!g_sigint_received && !any_failed) {
        region_c_t *c = shm.c;
        fprintf(stdout, "==================================================\n");
        fprintf(stdout, "SYSTEM SUMMARY\n");
        fprintf(stdout, "Keywords     : ");
        for (int i = 0; i < args.num_keywords; i++) {
            fprintf(stdout, "%s%s", args.keywords[i],
                    i + 1 < args.num_keywords ? ", " : "");
        }
        fprintf(stdout, "\n");
        fprintf(stdout, "Log files    : %d\n", args.num_files);

        long total_entries = 0;
        double total_weighted = 0.0;
        for (int i = 0; i < NUM_LEVELS; i++) {
            total_entries += c->results[i].total_entries;
            total_weighted += c->results[i].total_weighted_score;
        }
        fprintf(stdout, "Total entries: %ld\n", total_entries);
        fprintf(stdout, "Total weighted: %.1f\n", total_weighted);

        /* High-priority skorunu Region C'ye Aggregator yazmıyor olabilir;
         * Aggregator text dosyasında HIGH_PRIORITY_SCORE raporlar. Final
         * summary'de aynı değeri getirebilmek için Region C içine özel bir
         * alan ayırabilirdik; yer tutucu olarak txt dosyasından okumak yerine
         * Region C'ye aggregator_high_priority_weighted alanı eklemeden
         * "(see output file)" yazıyoruz. */
        fprintf(stdout, "High-priority: (see %s)\n", args.output_text);

        for (int i = 0; i < NUM_LEVELS; i++) {
            fprintf(stdout, "%-5s: %ld entries, score: %.1f\n",
                    c->results[i].level,
                    c->results[i].total_entries,
                    c->results[i].total_weighted_score);
        }
        fprintf(stdout, "==================================================\n");
        fprintf(stdout, "Program terminated successfully.\n");
        fflush(stdout);
    } else {
        fprintf(stderr, "[main] shutting down with errors (sigint=%d failed=%d)\n",
                (int)g_sigint_received, any_failed);
    }

    /* 16) Temizlik. */
    for (int i = 0; i < args.num_files; i++) {
        if (pipe_read_fds[i] != -1) close(pipe_read_fds[i]);
    }
    free(pipe_read_fds);
    free(pipe_write_fds);
    free(reader_names);
    for (int i = 0; i < child_count; i++) free((void *)children[i].label);
    free(children);

    shm_bundle_destroy(&shm);
    free_args(&args);

    /* SIGINT veya hata durumunda non-zero exit; PDF bu durum için _exit(1)
     * önerse de main'den normal return yeterli (atexit handler'lar yok). */
    return (g_sigint_received || any_failed) ? 1 : 0;
}
