#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include "cargo.h"

int    pq_init(pqueue_t *q, size_t initial_cap);
void   pq_destroy(pqueue_t *q);
size_t pq_size(const pqueue_t *q);
int    pq_push(pqueue_t *q, const order_t *o);
int    pq_pop_min(pqueue_t *q, order_t *out);

#endif
