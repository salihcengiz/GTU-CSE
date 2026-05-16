#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cargo.h"
#include "courier.h"
#include "parser.h"
#include "priority_queue.h"
#include "signal_thread.h"
#include "stats.h"

pqueue_t        g_queue;
pthread_mutex_t g_queue_mutex;
pthread_cond_t  g_queue_cond;
pthread_mutex_t g_log_mutex;

_Atomic long g_completed_orders        = 0;
_Atomic long g_cancelled_orders        = 0;
_Atomic long g_total_delivery_time_ms  = 0;
_Atomic int  g_shutdown                = 0;
_Atomic int  g_natural_shutdown        = 0;

int              g_num_couriers   = 0;
int              g_total_orders   = 0;
courier_stats_t *g_courier_stats  = NULL;
pthread_t        g_signal_tid;

static void print_usage(const char *prog) {
    fprintf(stderr,
            "Usage: %s -n <num_couriers> -i <orders.txt> -s <stats.txt>\n",
            prog ? prog : "cargoGTU");
}

static int parse_positive_int(const char *s, int *out) {
    if (!s || !*s) return -1;
    char *end = NULL;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') return -1;
    if (v <= 0 || v > 1000000) return -1;
    *out = (int)v;
    return 0;
}

int main(int argc, char *argv[]) {
    const char *input_path = NULL;
    const char *stats_path = NULL;
    int         num_couriers = -1;

    int opt;
    while ((opt = getopt(argc, argv, "n:i:s:")) != -1) {
        switch (opt) {
            case 'n':
                if (parse_positive_int(optarg, &num_couriers) != 0) {
                    print_usage(argv[0]);
                    return 1;
                }
                break;
            case 'i':
                input_path = optarg;
                break;
            case 's':
                stats_path = optarg;
                break;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    if (num_couriers < 1 || !input_path || !stats_path) {
        print_usage(argv[0]);
        return 1;
    }

    if (access(input_path, R_OK) != 0) {
        fprintf(stderr, "Error: cannot read input file '%s'\n", input_path);
        return 1;
    }

    g_num_couriers = num_couriers;

    sigset_t block_set;
    sigemptyset(&block_set);
    sigaddset(&block_set, SIGINT);
    if (pthread_sigmask(SIG_BLOCK, &block_set, NULL) != 0) {
        fprintf(stderr, "Error: failed to set signal mask\n");
        return 1;
    }

    if (pthread_mutex_init(&g_queue_mutex, NULL) != 0 ||
        pthread_mutex_init(&g_log_mutex, NULL)   != 0 ||
        pthread_cond_init(&g_queue_cond, NULL)   != 0) {
        fprintf(stderr, "Error: failed to init synchronization primitives\n");
        return 1;
    }

    if (pq_init(&g_queue, 64) != 0) {
        fprintf(stderr, "Error: failed to init priority queue\n");
        return 1;
    }

    g_courier_stats = (courier_stats_t *)calloc((size_t)num_couriers,
                                                sizeof(courier_stats_t));
    if (!g_courier_stats) {
        fprintf(stderr, "Error: out of memory\n");
        pq_destroy(&g_queue);
        return 1;
    }

    order_t *parsed       = NULL;
    int      parsed_count = 0;
    if (load_orders_from_file(input_path, &parsed, &parsed_count) != 0) {
        fprintf(stderr, "Error: failed to load input file '%s'\n", input_path);
        free(g_courier_stats);
        pq_destroy(&g_queue);
        return 1;
    }

    g_total_orders = parsed_count;

    log_shift_start(num_couriers, parsed_count);

    for (int i = 0; i < parsed_count; ++i) {
        log_order_queued(&parsed[i]);
        if (pq_push(&g_queue, &parsed[i]) != 0) {
            fprintf(stderr, "Error: failed to enqueue order id=%d\n",
                    parsed[i].id);
            free(parsed);
            free(g_courier_stats);
            pq_destroy(&g_queue);
            return 1;
        }
    }

    free(parsed);
    parsed = NULL;

    if (pthread_create(&g_signal_tid, NULL, signal_thread_run, NULL) != 0) {
        fprintf(stderr, "Error: failed to create signal thread\n");
        free(g_courier_stats);
        pq_destroy(&g_queue);
        return 1;
    }

    pthread_t     *courier_tids = (pthread_t *)calloc((size_t)num_couriers,
                                                      sizeof(pthread_t));
    courier_arg_t *courier_args = (courier_arg_t *)calloc((size_t)num_couriers,
                                                          sizeof(courier_arg_t));
    if (!courier_tids || !courier_args) {
        fprintf(stderr, "Error: out of memory\n");
        free(courier_tids);
        free(courier_args);
        atomic_store(&g_natural_shutdown, 1);
        pthread_kill(g_signal_tid, SIGINT);
        pthread_join(g_signal_tid, NULL);
        free(g_courier_stats);
        pq_destroy(&g_queue);
        return 1;
    }

    for (int i = 0; i < num_couriers; ++i) {
        courier_args[i].courier_id = i + 1;
        courier_args[i].index      = i;
        if (pthread_create(&courier_tids[i], NULL, courier_run,
                           &courier_args[i]) != 0) {
            fprintf(stderr, "Error: failed to create courier thread %d\n",
                    i + 1);
            atomic_store(&g_shutdown, 1);
            pthread_mutex_lock(&g_queue_mutex);
            pthread_cond_broadcast(&g_queue_cond);
            pthread_mutex_unlock(&g_queue_mutex);
            for (int j = 0; j < i; ++j) {
                pthread_join(courier_tids[j], NULL);
            }
            free(courier_tids);
            free(courier_args);
            atomic_store(&g_natural_shutdown, 1);
            pthread_kill(g_signal_tid, SIGINT);
            pthread_join(g_signal_tid, NULL);
            free(g_courier_stats);
            pq_destroy(&g_queue);
            return 1;
        }
    }

    for (int i = 0; i < num_couriers; ++i) {
        pthread_join(courier_tids[i], NULL);
    }

    free(courier_tids);
    free(courier_args);

    atomic_store(&g_natural_shutdown, 1);
    pthread_kill(g_signal_tid, SIGINT);
    pthread_join(g_signal_tid, NULL);

    long completed = atomic_load(&g_completed_orders);
    long cancelled = atomic_load(&g_cancelled_orders);
    long total_ms  = atomic_load(&g_total_delivery_time_ms);

    log_shift_end(completed, cancelled, total_ms);

    if (write_stats_file(stats_path, g_total_orders,
                         completed, cancelled, total_ms) != 0) {
        fprintf(stderr, "Error: failed to write stats file '%s'\n", stats_path);
    }

    log_shutdown_complete();

    pthread_mutex_destroy(&g_queue_mutex);
    pthread_mutex_destroy(&g_log_mutex);
    pthread_cond_destroy(&g_queue_cond);
    pq_destroy(&g_queue);
    free(g_courier_stats);

    return 0;
}
