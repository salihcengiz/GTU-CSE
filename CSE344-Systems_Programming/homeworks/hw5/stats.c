#include "stats.h"

#include <stdio.h>

#include "cargo.h"

int write_stats_file(const char *path,
                     int  total_orders,
                     long completed,
                     long cancelled,
                     long total_time_ms) {
    if (!path) return -1;

    FILE *fp = fopen(path, "w");
    if (!fp) return -1;

    long avg = (completed > 0) ? (total_time_ms / completed) : 0L;

    fprintf(fp, "SHIFT_SUMMARY\n");
    fprintf(fp, "Total orders : %d\n", total_orders);
    fprintf(fp, "Completed : %ld\n", completed);
    fprintf(fp, "Cancelled : %ld\n", cancelled);
    fprintf(fp, "Total time : %ldms\n", total_time_ms);
    fprintf(fp, "Avg per order : %ldms\n", avg);
    fprintf(fp, "\n");
    fprintf(fp, "COURIER_STATS\n");
    for (int i = 0; i < g_num_couriers; ++i) {
        fprintf(fp, "Courier-%d completed=%ld total_time=%ldms\n",
                i + 1,
                g_courier_stats[i].completed,
                g_courier_stats[i].total_time_ms);
    }

    if (fclose(fp) != 0) return -1;
    return 0;
}
