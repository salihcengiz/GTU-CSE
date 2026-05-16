#ifndef PARSER_H
#define PARSER_H

#include "cargo.h"

int load_orders_from_file(const char *path, order_t **arr_out, int *count_out);

#endif
