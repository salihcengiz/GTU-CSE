#define _POSIX_C_SOURCE 200809L

#include "courier.h"

#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "cargo.h"
#include "priority_queue.h"

static void deliver_sleep(int duration_units) {
    long total_ms = (long)duration_units * 500L;
    struct timespec req;
    req.tv_sec  = total_ms / 1000;
    req.tv_nsec = (total_ms % 1000) * 1000000L;
    while (nanosleep(&req, &req) == -1) {
        continue;
    }
}

void *courier_run(void *arg) {
    courier_arg_t *ca         = (courier_arg_t *)arg;
    int            courier_id = ca->courier_id;
    int            idx        = ca->index;

    for (;;) {
        pthread_mutex_lock(&g_queue_mutex);

        long done      = atomic_load(&g_completed_orders);
        long cancelled = atomic_load(&g_cancelled_orders);
        if (done + cancelled >= (long)g_total_orders) {
            atomic_store(&g_shutdown, 1);
            pthread_cond_broadcast(&g_queue_cond);
        }

        if (pq_size(&g_queue) == 0) {
            if (!atomic_load(&g_shutdown)) {
                log_courier_waiting(courier_id);
                while (pq_size(&g_queue) == 0 && !atomic_load(&g_shutdown)) {
                    pthread_cond_wait(&g_queue_cond, &g_queue_mutex);
                }
            }
            if (atomic_load(&g_shutdown) && pq_size(&g_queue) == 0) {
                pthread_mutex_unlock(&g_queue_mutex);
                break;
            }
        }

        order_t ord;
        if (pq_pop_min(&g_queue, &ord) != 0) {
            pthread_mutex_unlock(&g_queue_mutex);
            continue;
        }

        log_delivery_start(courier_id, &ord);
        pthread_mutex_unlock(&g_queue_mutex);

        deliver_sleep(ord.duration_units);

        long ms = (long)ord.duration_units * 500L;
        atomic_fetch_add(&g_completed_orders, 1);
        atomic_fetch_add(&g_total_delivery_time_ms, ms);
        g_courier_stats[idx].completed     += 1;
        g_courier_stats[idx].total_time_ms += ms;

        log_delivery_complete(courier_id, &ord, ms);
    }

    log_shift_over(courier_id);
    return NULL;
}
