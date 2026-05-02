/*
 * reader.h — Reader Process entry point'i ve ilgili yardımcıların başlığı.
 *
 * Reader Process, tek bir log dosyasını T adet reader thread ile paralel
 * işler; thread'ler dosyayı byte aralıklarına böler, her thread kendi
 * aralığındaki satırları parse ederek process-lokal bounded buffer'a
 * (default attribute mutex+condvar) yazar. Tek bir parser thread bu
 * buffer'dan tüketip Region A'ya push eder; ayrıca heartbeat pipe'ına
 * periyodik "X lines processed" yazar.
 */

#ifndef HW4_READER_H
#define HW4_READER_H

#include "common.h"
#include "shm.h"

/*
 * Bir Reader Process'in main fonksiyonu. Parent fork sonrası hemen bu
 * fonksiyonu çağırır (child tarafında). Tüm thread'ler burada yaratılır
 * ve join edilir; fonksiyon sonunda proses exit(0) olur.
 *
 * reader_index     : 0..num_files-1, process başına tekil index.
 * log_file_path    : işlenecek tek log dosyası.
 * reader_threads   : T — kaç reader thread spawn edileceği.
 * pipe_write_fd    : bu Reader'a atanmış heartbeat pipe'ın yazma ucu.
 *                    Diğer uçlar parent tarafında kapatılmıştır.
 * shm              : parent'ın başlatıp kalıt bıraktığı shared memory.
 */
int reader_process_main(int reader_index,
                        const char *log_file_path,
                        int reader_threads,
                        int pipe_write_fd,
                        shm_bundle_t *shm);

#endif /* HW4_READER_H */
