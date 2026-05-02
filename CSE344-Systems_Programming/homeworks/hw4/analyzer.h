/*
 * analyzer.h — Analyzer Process'in başlığı.
 *
 * Her severity için bir Analyzer Process yaratılır (toplam 4). Her biri W
 * worker thread içerir; worker'lar pthread_key_t TLS ile per-keyword weighted
 * skoru biriktirir, pthread_barrier ile senkronlanır ve syscall(SYS_gettid)
 * ile seçilen lowest-TID worker raporlayıcı olarak Region C'ye yazar ve
 * ilgili semaphore'ı post eder.
 */

#ifndef HW4_ANALYZER_H
#define HW4_ANALYZER_H

#include "common.h"
#include "shm.h"

/*
 * Analyzer Process main.
 * level_idx        : 0..3 (ERROR/WARN/INFO/DEBUG).
 * keywords         : -k listesinin heap kopyası (pointerlar shared).
 * num_keywords     : 1..MAX_KEYWORDS.
 * worker_threads   : W — kaç worker spawn edilecek.
 */
int analyzer_process_main(shm_bundle_t *shm,
                          int level_idx,
                          char **keywords,
                          int num_keywords,
                          int worker_threads);

#endif /* HW4_ANALYZER_H */
