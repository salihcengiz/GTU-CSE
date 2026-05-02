/*
 * aggregator.h — Aggregator Process'in başlığı.
 *
 * Aggregator 4 Analyzer Process'in Region C'deki sonucunu (sem_wait +
 * pthread_cond_timedwait karışımı) bekler, Region D'deki high-priority
 * entry'leri analiz eder ve iki çıktı dosyasını (insan-okunur text + binary
 * checkpoint) yazar. Binary dosya atomic rename ile finalize edilir.
 */

#ifndef HW4_AGGREGATOR_H
#define HW4_AGGREGATOR_H

#include "common.h"
#include "shm.h"

/*
 * Aggregator Process main.
 */
int aggregator_process_main(shm_bundle_t *shm,
                            const char *output_text_path,
                            const char *output_bin_path,
                            char **keywords,
                            int num_keywords,
                            int num_files,
                            const char *filter_path,
                            int timeout_sec);

#endif /* HW4_AGGREGATOR_H */
