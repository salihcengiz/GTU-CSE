#include "common.h"

/* Word-Carrier Süreci => Giriş dosyasındaki kelimeleri round-robin sırasıyla tarar,
  uygun kelimeleri seçer ve varış katına yerleştirir */

/* WORD-CARRIER ANA FONKSİYONU => Her word-carrier süreci fork() sonrası bu fonksiyonu çağırır
 * Süreç boyunca kendi katından ayrılmaz (arrival floor sabittir) */
void word_carrier_main(SharedData *shm, int id, int floor) {
  /* Rasgele sayı üretecini bu süreç için benzersiz hale getir */
  srand(time(NULL) ^ getpid() ^ id);
  /* Sistem çalıştığı sürece kelime tarama döngüsü */
  while (shm->system_running) {
    int found = 0;     /* Kelime bulundu mu bayrağı */
    int word_idx = -1; /* Bulunan kelimenin dizideki indeksi */
    /* ROUND-ROBİN KELİME TARAMA => Global mutex altında döngüsel olarak kelimeler taranır
     * Sahipsiz ve tamamlanmamış ilk kelime seçilir */
    pthread_mutex_lock(&shm->global_mutex);
    /* Tüm kelimeler tarandı mı kontrol etmek için sayaç */
    int scanned = 0;
    /* Tüm kelimeleri dolaşana kadar veya uygun kelime bulana kadar tara */
    while (scanned < shm->num_words) {
      int idx = shm->round_robin_index;
      /* Round-robin indeksini bir sonrakine ilerlet (döngüsel) */
      shm->round_robin_index = (shm->round_robin_index + 1) % shm->num_words;
      scanned++;
      /* Kelime uygun mu kontrol et: henüz alınmamış ve tamamlanmamış */
      if (shm->words[idx].claimed == 0 && shm->words[idx].admitted == 0 &&
          shm->words[idx].completed == 0) {
        /* Kelimeyi atomik olarak sahiplen (claimed = 1) */
        shm->words[idx].claimed = 1;
        word_idx = idx;
        found = 1;
        break;
      }
    }
    pthread_mutex_unlock(&shm->global_mutex);
    /* KELİME BULUNAMADI - BEKLEME => Eğer taranacak kelime yoksa semafor ile bekle
     * Meşgul bekleme (busy wait) yapılmaz */
    if (!found) {
      /* Tüm kelimeler tamamlandıysa döngüden çık */
      if (shm->total_completed >= shm->num_words) {
        break;
      }
      /* Yeni kelime hazır olana kadar bekle */
      struct timespec ts;
      clock_gettime(CLOCK_REALTIME, &ts);
      /* 200 milisaniye zaman aşımı ile bekle */
      ts.tv_nsec += 200000000L;
      if (ts.tv_nsec >= 1000000000L) {
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000000000L;
      }
      sem_timedwait(&shm->word_available_sem, &ts);
      continue;
    }
    /* KELİME BULUNDU - KABUL KONTROLÜ (ALL-OR-NOTHING ADMISSION) => Hem varış katı hem de sıralama katı kapasitesini atomik kontrol et
     * Deadlock önleme: kat mutexleri her zaman küçükten büyüğe kilitlenir */
    WordEntry *word = &shm->words[word_idx];
    int arrival = floor;               /* Bu word-carrier'ın katı */
    int sorting = word->sorting_floor; /* Kelimenin sıralama katı */
    log_msg(shm, "Word-carrier-process_%d claimed word %d\n", id,
            word->word_id);
    int admitted = 0; /* Kabul sonucu bayrağı */
    if (arrival == sorting) {
      /* AYNI KAT DURUMU => Varış ve sıralama katı aynıysa tek mutex yeterli
       * Kelime bu katta yalnızca 1 kapasite slotu kaplar */
      pthread_mutex_lock(&shm->floor_mutex[arrival]);
      if (shm->floors[arrival].active_word_count <
          shm->floors[arrival].max_capacity) {
        /* Kapasite müsait - kelimeyi kabul et */
        shm->floors[arrival].active_word_count++;
        admitted = 1;
      }
      pthread_mutex_unlock(&shm->floor_mutex[arrival]);
    } else {
      /* FARKLI KAT DURUMU => İki farklı katın kapasitesi atomik olarak kontrol edilir
       * Deadlock önleme: küçük numaralı kat her zaman önce kilitlenir */
      int first = (arrival < sorting) ? arrival : sorting;
      int second = (arrival < sorting) ? sorting : arrival;
      pthread_mutex_lock(&shm->floor_mutex[first]);
      pthread_mutex_lock(&shm->floor_mutex[second]);
      /* Her iki katta da yer var mı kontrol et */
      if (shm->floors[arrival].active_word_count <
              shm->floors[arrival].max_capacity &&
          shm->floors[sorting].active_word_count <
              shm->floors[sorting].max_capacity) {
        /* İkisi de müsait - her iki katta kapasiteyi artır */
        shm->floors[arrival].active_word_count++;
        shm->floors[sorting].active_word_count++;
        admitted = 1;
      }
      pthread_mutex_unlock(&shm->floor_mutex[second]);
      pthread_mutex_unlock(&shm->floor_mutex[first]);
    }
    /* KABUL SONUCUNUN İŞLENMESİ => Başarılı kabul: kelimeyi aktive et, karakter görevlerini hazırla
     * Başarısız kabul: kelimeyi serbest bırak, yeniden denenebilir yap */
    if (admitted) {
      /* Kelimeyi kabul et - tüm alanları güncelle */
      word->arrival_floor = arrival;
      word->admitted = 1;
      /* Karakter görevlerinin kaynak katını ayarla */
      for (int j = 0; j < word->word_length; j++) {
        word->char_tasks[j].source_floor = arrival;
      }
      log_msg(shm, "Word %d admitted to floor %d (sorting floor: %d)\n\n",
              word->word_id, arrival, sorting);
      /* Letter-carrier'lara yeni iş olduğunu bildir */
      sem_post(&shm->letter_work_sem[arrival]);
    } else {
      /* KABUL BAŞARISIZ - kelimeyi serbest bırak => claimed = 0 yaparak başka bir word-carrier'ın almasına izin ver
       * Retry sayacını artır */
      word->claimed = 0;
      word->arrival_floor = -1;
      /* İstatistikleri güncelle (global mutex altında) */
      pthread_mutex_lock(&shm->global_mutex);
      shm->total_retries++;
      pthread_mutex_unlock(&shm->global_mutex);
      /* Diğer word-carrier'lara kelime serbest kaldı bildir */
      sem_post(&shm->word_available_sem);
      log_msg(
          shm,
          "Word-carrier-process_%d released word %d (floor capacity full)\n",
          id, word->word_id);
      /* Kısa bekleme - sürekli denemeyi önle (busy wait değil) */
      usleep(10000); /* 10ms */
    }
  }
}
