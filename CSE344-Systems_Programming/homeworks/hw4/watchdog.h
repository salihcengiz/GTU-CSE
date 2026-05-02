/*
 * watchdog.h — Parent process içinde çalışan Watchdog thread'in başlığı.
 *
 * Watchdog her Reader Process'e karşılık gelen pipe read-end'ini select()
 * ile dinler, gelen heartbeat satırlarını parse eder ve 3 saniyede bir
 * stderr'e progress snapshot basar.
 */

#ifndef HW4_WATCHDOG_H
#define HW4_WATCHDOG_H

#include "common.h"
#include <stdatomic.h>

/* Watchdog thread'e parent tarafından geçirilen config struct. Thread
 * yaratıldıktan sonra tüm alanlar read-only olarak kullanılır. */
typedef struct {
    int               num_readers;     /* pipe sayısı */
    int              *pipe_read_fds;   /* num_readers boyutunda array */
    const char      **log_file_names;  /* görüntülenen isimler */
    atomic_int       *children_alive;  /* parent günceller, watchdog okur */
    volatile sig_atomic_t *shutdown_flag; /* 1 → watchdog çıkış */
} watchdog_args_t;

/* pthread_create entry point. start_routine imzasına uygun olacak şekilde
 * void* döner ve argüman olarak watchdog_args_t* alır. */
void *watchdog_thread_main(void *arg);

#endif /* HW4_WATCHDOG_H */
