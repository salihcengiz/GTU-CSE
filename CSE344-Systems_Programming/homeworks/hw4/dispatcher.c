/*
 * dispatcher.c — Dispatcher Process implementasyonu.
 *
 * Döngü:
 *   - Region A'dan timedwait ile entry çek
 *   - entry EOF ise: eof_count_per_level[level]++; eğer bu sayı == total_readers
 *     olduysa o seviyenin Region B buffer'ına EOF forward et (tek sefer)
 *   - entry normal ise:
 *       * SOURCE filter ise Region D'ye kopya push
 *       * Region B[level]'a push
 *   - timeout olursa: tüm eof_count_per_level == total_readers mi? Evetse çık
 *   - çıkarken Region D'nin dispatcher_done=1 yap, worker'ları uyandır
 */

#include "dispatcher.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/* Priority source listesini küçük bir linear-search set olarak kullanıyoruz.
 * Typical filter boyutu küçük (max birkaç düzine) olduğundan hash gerekmiyor. */
static int is_priority(const char *src, char **priority_sources, int n) {
    for (int i = 0; i < n; i++) {
        if (strcmp(src, priority_sources[i]) == 0) return 1;
    }
    return 0;
}

int dispatcher_process_main(shm_bundle_t *shm,
                            char **priority_sources,
                            int num_priority,
                            int timeout_sec)
{
    fprintf(stdout, "[PID:%d] Dispatcher started.\n", (int)getpid());
    fflush(stdout);

    /* Her seviye için EOF Region B'ye forward edildi mi bayrağı — tek seferlik. */
    int eof_forwarded[NUM_LEVELS] = {0, 0, 0, 0};
    long routed = 0, priority_copies = 0;
    int timeouts = 0;

    for (;;) {
        log_entry_t e;
        int rc = region_a_pop_timed(shm->a, &e, timeout_sec);
        if (rc == -1) {
            fprintf(stderr, "[PID:%d] region_a_pop_timed error\n", (int)getpid());
            break;
        }
        if (rc == 1) {
            /* Timeout: sonlanma şartını kontrol et. */
            int all_seen = 1;
            /* eof_count_per_level okuması kilitsiz; atomic olmayan int
             * yazmalarında okumayı da defansif olarak mutex altında yapıyoruz. */
            pthread_mutex_lock(&shm->a->input_mutex);
            for (int l = 0; l < NUM_LEVELS; l++) {
                if (shm->a->eof_count_per_level[l] < shm->a->total_readers) {
                    all_seen = 0;
                    break;
                }
            }
            int a_empty = (shm->a->count == 0);
            pthread_mutex_unlock(&shm->a->input_mutex);
            timeouts++;
            if (all_seen && a_empty) break;
            /* Aksi halde döngüye devam — kuyrukta hala veri/EOF var. */
            continue;
        }

        if (e.is_eof) {
            /* EOF marker — seviyenin sayaç artır. Tüm Reader'lar gelmişse
             * Region B'ye forward et. */
            int lv = e.level_idx;
            if (lv < 0 || lv >= NUM_LEVELS) continue; /* defensive */

            pthread_mutex_lock(&shm->a->input_mutex);
            shm->a->eof_count_per_level[lv]++;
            int ready = (shm->a->eof_count_per_level[lv] == shm->a->total_readers);
            pthread_mutex_unlock(&shm->a->input_mutex);

            if (ready && !eof_forwarded[lv]) {
                eof_forwarded[lv] = 1;
                /* Region B[lv]'e tek EOF marker push + eof_posted=1 set. */
                log_entry_t eof_mark = e;
                /* region_b_push mutex'ini kendisi alır; bu mutex'i tutarken
                 * eof_posted'ı da tek transaction'da set etmemiz gerekiyor ki
                 * tüketici bekleme penceresinde "buffer boş + eof_posted" ile
                 * çıkabilsin. Bu yüzden push'u kendi mantığımızla yapıyoruz. */
                region_b_level_t *b = shm->b[lv];
                pthread_mutex_lock(&b->level_mutex);
                while (b->count == b->capacity) {
                    pthread_cond_wait(&b->not_full_b, &b->level_mutex);
                }
                b->buf[b->tail] = eof_mark;
                b->tail = (b->tail + 1) % b->capacity;
                b->count++;
                b->eof_posted = 1;
                pthread_cond_broadcast(&b->not_empty_b);
                pthread_mutex_unlock(&b->level_mutex);
            }
            continue;
        }

        /* Normal entry — routing. */
        int lv = e.level_idx;
        if (lv < 0 || lv >= NUM_LEVELS) continue; /* defensive */

        /* Priority ise Region D'ye kopya push. Region D doluysa bekleriz
         * (busy-wait yok); Aggregator bu buffer'dan tüketir. */
        int is_hp = (num_priority > 0 &&
                     is_priority(e.source, priority_sources, num_priority));
        if (is_hp) {
            region_d_push(shm->d, &e);
            priority_copies++;
        }
#ifdef VERBOSE_DISPATCH
        fprintf(stdout,
                "[PID:%d] Routed entry to %s buffer. High-priority: %s (source: %s)\n",
                (int)getpid(), level_name((log_level_t)lv),
                is_hp ? "YES" : "NO", e.source);
#endif

        /* Region B[lv]'e push — tek üretici olduğumuz için eof_posted bayrağına
         * dokunmuyoruz. */
        if (region_b_push(shm->b[lv], &e) != 0) {
            fprintf(stderr, "[PID:%d] region_b_push failed for level %d\n",
                    (int)getpid(), lv);
            break;
        }
        routed++;
    }

    /* Çıkış: eğer bir sebepten EOF forward etmediysek de, son hamle olarak
     * her seviye için eof_posted=1 yapıp worker'ları uyandıralım. Bu sayede
     * anormal bir hata durumunda deadlock oluşmaz. */
    for (int lv = 0; lv < NUM_LEVELS; lv++) {
        region_b_level_t *b = shm->b[lv];
        pthread_mutex_lock(&b->level_mutex);
        if (!b->eof_posted) {
            b->eof_posted = 1;
            pthread_cond_broadcast(&b->not_empty_b);
        }
        pthread_mutex_unlock(&b->level_mutex);
    }

    /* Region D'yi "done" olarak işaretle ve Aggregator'ı uyandır. */
    pthread_mutex_lock(&shm->d->priority_mutex);
    shm->d->dispatcher_done = 1;
    pthread_cond_broadcast(&shm->d->not_empty_d);
    pthread_mutex_unlock(&shm->d->priority_mutex);

    fprintf(stdout, "[PID:%d] All EOF markers forwarded to Region B. Exiting. "
                    "(routed=%ld priority=%ld timeouts=%d)\n",
            (int)getpid(), routed, priority_copies, timeouts);
    fflush(stdout);
    return 0;
}
