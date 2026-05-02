/*
 * dispatcher.h — Dispatcher Process'in başlığı.
 *
 * Dispatcher tek bir process olup Region A'dan entry'leri okur ve seviyeye
 * göre Region B'nin ilgili level buffer'ına, ayrıca SOURCE filter'a uyuyorsa
 * Region D'ye kopya yazar. EOF marker'ları sayar ve tüm Reader'lardan gelen
 * EOF'lar tamamlanınca her level için Region B'ye tek EOF marker forward eder.
 */

#ifndef HW4_DISPATCHER_H
#define HW4_DISPATCHER_H

#include "common.h"
#include "shm.h"

/*
 * Dispatcher Process main.
 * priority_sources : -f dosyasından okunmuş isim listesi (parent heap'inde;
 *                    fork sonrası aynı adresten erişilir).
 * num_priority     : adet.
 * timeout_sec      : -T değeri; Region A üzerinde timedwait için.
 */
int dispatcher_process_main(shm_bundle_t *shm,
                            char **priority_sources,
                            int num_priority,
                            int timeout_sec);

#endif /* HW4_DISPATCHER_H */
