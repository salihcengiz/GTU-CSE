#define _POSIX_C_SOURCE 200809L

#include "signal_thread.h"

#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>

#include "cargo.h"
#include "priority_queue.h"

void *signal_thread_run(void *arg) {
    (void)arg;

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);

    int sig = 0;
    sigwait(&set, &sig);

    if (atomic_load(&g_natural_shutdown)) {
        return NULL;
    }

    atomic_store(&g_shutdown, 1);

    pthread_mutex_lock(&g_queue_mutex);

    int pending = (int)pq_size(&g_queue);
    log_sigint_received(pending);

    while (pq_size(&g_queue) > 0) {
        order_t cancelled;
        if (pq_pop_min(&g_queue, &cancelled) != 0) break;
        atomic_fetch_add(&g_cancelled_orders, 1);
        log_order_cancelled(&cancelled);
    }

    pthread_cond_broadcast(&g_queue_cond);
    pthread_mutex_unlock(&g_queue_mutex);

    return NULL;
}
