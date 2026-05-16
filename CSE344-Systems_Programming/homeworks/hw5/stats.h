#ifndef STATS_H
#define STATS_H

int write_stats_file(const char *path,
                     int  total_orders,
                     long completed,
                     long cancelled,
                     long total_time_ms);

#endif
