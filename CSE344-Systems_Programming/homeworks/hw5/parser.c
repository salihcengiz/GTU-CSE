#define _POSIX_C_SOURCE 200809L

#include "parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int is_blank_line(const char *s) {
    while (*s) {
        if (!isspace((unsigned char)*s)) return 0;
        s++;
    }
    return 1;
}

static int is_valid_name(const char *s) {
    if (!s || !*s) return 0;
    size_t len = strlen(s);
    if (len > MAX_NAME_LEN) return 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c == '_')) return 0;
    }
    return 1;
}

static int parse_priority(const char *s, priority_t *out) {
    if (strcmp(s, "EXPRESS") == 0)  { *out = PRIO_EXPRESS;  return 1; }
    if (strcmp(s, "STANDARD") == 0) { *out = PRIO_STANDARD; return 1; }
    if (strcmp(s, "ECONOMY") == 0)  { *out = PRIO_ECONOMY;  return 1; }
    return 0;
}

static int has_only_trailing_whitespace(const char *line, int consumed) {
    const char *p = line + consumed;
    while (*p) {
        if (!isspace((unsigned char)*p)) return 0;
        p++;
    }
    return 1;
}

int load_orders_from_file(const char *path, order_t **arr_out, int *count_out) {
    if (!path || !arr_out || !count_out) return -1;

    FILE *fp = fopen(path, "r");
    if (!fp) return -1;

    size_t   cap = 16;
    size_t   sz  = 0;
    order_t *arr = (order_t *)malloc(sizeof(order_t) * cap);
    if (!arr) { fclose(fp); return -1; }

    char  *line = NULL;
    size_t bufsz = 0;
    ssize_t nread;

    while ((nread = getline(&line, &bufsz, fp)) != -1) {
        if (is_blank_line(line)) continue;

        int  id;
        char name[MAX_NAME_LEN + 2];
        char prio_str[16];
        int  duration;
        int  consumed = 0;

        int matched = sscanf(line, " %d %33s %15s %d%n",
                             &id, name, prio_str, &duration, &consumed);
        if (matched != 4) continue;
        if (!has_only_trailing_whitespace(line, consumed)) continue;

        if (id <= 0) continue;
        if (duration <= 0) continue;
        if (!is_valid_name(name)) continue;

        priority_t prio;
        if (!parse_priority(prio_str, &prio)) continue;

        if (sz == cap) {
            size_t   new_cap = cap * 2;
            order_t *new_arr = (order_t *)realloc(arr, sizeof(order_t) * new_cap);
            if (!new_arr) {
                free(arr);
                free(line);
                fclose(fp);
                return -1;
            }
            arr = new_arr;
            cap = new_cap;
        }

        arr[sz].id             = id;
        strncpy(arr[sz].name, name, MAX_NAME_LEN);
        arr[sz].name[MAX_NAME_LEN] = '\0';
        arr[sz].priority       = prio;
        arr[sz].duration_units = duration;
        sz++;
    }

    free(line);
    fclose(fp);

    *arr_out   = arr;
    *count_out = (int)sz;
    return 0;
}
