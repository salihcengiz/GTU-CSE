#ifndef CARGO_H
#define CARGO_H

#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>

#define MAX_NAME_LEN 32

typedef enum {
    PRIO_EXPRESS  = 1,
    PRIO_STANDARD = 2,
    PRIO_ECONOMY  = 3
} priority_t;

typedef struct {
    int        id;
    char       name[MAX_NAME_LEN + 1];
    priority_t priority;
    int        duration_units;
} order_t;

typedef struct {
    order_t *data;
    size_t   size;
    size_t   cap;
} pqueue_t;

typedef struct {
    long completed;
    long total_time_ms;
} courier_stats_t;

extern pqueue_t        g_queue;
extern pthread_mutex_t g_queue_mutex;
extern pthread_cond_t  g_queue_cond;
extern pthread_mutex_t g_log_mutex;

extern _Atomic long g_completed_orders;
extern _Atomic long g_cancelled_orders;
extern _Atomic long g_total_delivery_time_ms;
extern _Atomic int  g_shutdown;
extern _Atomic int  g_natural_shutdown;

extern int              g_num_couriers;
extern int              g_total_orders;
extern courier_stats_t *g_courier_stats;
extern pthread_t        g_signal_tid;

const char *priority_to_str(priority_t p);

void log_shift_start(int couriers, int orders);
void log_order_queued(const order_t *o);
void log_courier_waiting(int courier_id);
void log_delivery_start(int courier_id, const order_t *o);
void log_delivery_complete(int courier_id, const order_t *o, long duration_ms);
void log_shift_over(int courier_id);
void log_sigint_received(int pending_orders);
void log_order_cancelled(const order_t *o);
void log_shift_end(long completed, long cancelled, long total_time_ms);
void log_shutdown_complete(void);

#endif
