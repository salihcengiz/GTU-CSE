#ifndef COURIER_H
#define COURIER_H

typedef struct {
    int courier_id;
    int index;
} courier_arg_t;

void *courier_run(void *arg);

#endif
