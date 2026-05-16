#include "cargo.h"

#include <pthread.h>
#include <stdio.h>

const char *priority_to_str(priority_t p) {
    switch (p) {
        case PRIO_EXPRESS:  return "EXPRESS";
        case PRIO_STANDARD: return "STANDARD";
        case PRIO_ECONOMY:  return "ECONOMY";
        default:            return "UNKNOWN";
    }
}

void log_shift_start(int couriers, int orders) {
    pthread_mutex_lock(&g_log_mutex);
    printf("[CARGOGTU] SHIFT_START couriers=%d orders=%d\n", couriers, orders);
    fflush(stdout);
    pthread_mutex_unlock(&g_log_mutex);
}

void log_order_queued(const order_t *o) {
    pthread_mutex_lock(&g_log_mutex);
    printf("[CARGOGTU] ORDER_QUEUED id=%d recipient=%s priority=%s duration=%d\n",
           o->id, o->name, priority_to_str(o->priority), o->duration_units);
    fflush(stdout);
    pthread_mutex_unlock(&g_log_mutex);
}

void log_courier_waiting(int courier_id) {
    pthread_mutex_lock(&g_log_mutex);
    printf("[COURIER-%d] WAITING\n", courier_id);
    fflush(stdout);
    pthread_mutex_unlock(&g_log_mutex);
}

void log_delivery_start(int courier_id, const order_t *o) {
    pthread_mutex_lock(&g_log_mutex);
    printf("[COURIER-%d] DELIVERY_START id=%d recipient=%s priority=%s\n",
           courier_id, o->id, o->name, priority_to_str(o->priority));
    fflush(stdout);
    pthread_mutex_unlock(&g_log_mutex);
}

void log_delivery_complete(int courier_id, const order_t *o, long duration_ms) {
    pthread_mutex_lock(&g_log_mutex);
    printf("[COURIER-%d] DELIVERY_COMPLETE id=%d recipient=%s duration=%ldms\n",
           courier_id, o->id, o->name, duration_ms);
    fflush(stdout);
    pthread_mutex_unlock(&g_log_mutex);
}

void log_shift_over(int courier_id) {
    pthread_mutex_lock(&g_log_mutex);
    printf("[COURIER-%d] SHIFT_OVER\n", courier_id);
    fflush(stdout);
    pthread_mutex_unlock(&g_log_mutex);
}

void log_sigint_received(int pending_orders) {
    pthread_mutex_lock(&g_log_mutex);
    printf("[CARGOGTU] SIGINT_RECEIVED pending_orders=%d\n", pending_orders);
    fflush(stdout);
    pthread_mutex_unlock(&g_log_mutex);
}

void log_order_cancelled(const order_t *o) {
    pthread_mutex_lock(&g_log_mutex);
    printf("[CARGOGTU] ORDER_CANCELLED id=%d recipient=%s priority=%s\n",
           o->id, o->name, priority_to_str(o->priority));
    fflush(stdout);
    pthread_mutex_unlock(&g_log_mutex);
}

void log_shift_end(long completed, long cancelled, long total_time_ms) {
    pthread_mutex_lock(&g_log_mutex);
    printf("[CARGOGTU] SHIFT_END completed=%ld cancelled=%ld total_time=%ldms\n",
           completed, cancelled, total_time_ms);
    fflush(stdout);
    pthread_mutex_unlock(&g_log_mutex);
}

void log_shutdown_complete(void) {
    pthread_mutex_lock(&g_log_mutex);
    printf("[CARGOGTU] SHUTDOWN_COMPLETE\n");
    fflush(stdout);
    pthread_mutex_unlock(&g_log_mutex);
}
