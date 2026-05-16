#include "priority_queue.h"

#include <stdlib.h>
#include <string.h>

static int order_less(const order_t *a, const order_t *b) {
    if (a->priority != b->priority) return a->priority < b->priority;
    return a->id < b->id;
}

static void swap_orders(order_t *a, order_t *b) {
    order_t tmp = *a;
    *a = *b;
    *b = tmp;
}

static void sift_up(pqueue_t *q, size_t idx) {
    while (idx > 0) {
        size_t parent = (idx - 1) / 2;
        if (order_less(&q->data[idx], &q->data[parent])) {
            swap_orders(&q->data[idx], &q->data[parent]);
            idx = parent;
        } else {
            break;
        }
    }
}

static void sift_down(pqueue_t *q, size_t idx) {
    size_t n = q->size;
    for (;;) {
        size_t left  = 2 * idx + 1;
        size_t right = 2 * idx + 2;
        size_t best  = idx;
        if (left  < n && order_less(&q->data[left],  &q->data[best])) best = left;
        if (right < n && order_less(&q->data[right], &q->data[best])) best = right;
        if (best == idx) break;
        swap_orders(&q->data[idx], &q->data[best]);
        idx = best;
    }
}

int pq_init(pqueue_t *q, size_t initial_cap) {
    if (!q) return -1;
    if (initial_cap == 0) initial_cap = 16;
    q->data = (order_t *)malloc(sizeof(order_t) * initial_cap);
    if (!q->data) return -1;
    q->size = 0;
    q->cap  = initial_cap;
    return 0;
}

void pq_destroy(pqueue_t *q) {
    if (!q) return;
    free(q->data);
    q->data = NULL;
    q->size = 0;
    q->cap  = 0;
}

size_t pq_size(const pqueue_t *q) {
    return q ? q->size : 0;
}

int pq_push(pqueue_t *q, const order_t *o) {
    if (!q || !o) return -1;
    if (q->size == q->cap) {
        size_t   new_cap = q->cap * 2;
        order_t *new_buf = (order_t *)realloc(q->data, sizeof(order_t) * new_cap);
        if (!new_buf) return -1;
        q->data = new_buf;
        q->cap  = new_cap;
    }
    q->data[q->size] = *o;
    q->size++;
    sift_up(q, q->size - 1);
    return 0;
}

int pq_pop_min(pqueue_t *q, order_t *out) {
    if (!q || q->size == 0 || !out) return -1;
    *out = q->data[0];
    q->size--;
    if (q->size > 0) {
        q->data[0] = q->data[q->size];
        sift_down(q, 0);
    }
    return 0;
}
