#include "common.h"

/* Letter-Carrier Süreci => Varış katlarından karakterleri alıp sıralama katlarına taşır */ 
/* Asansör sistemiyle entegre çalışır, katlar arası hareket edebilir */

/* ASANSÖR İSTEĞİ GÖNDERME FONKSİYONU => Delivery veya reposition asansörü için istek kuyruğuna ekleme yapar
 * İsteğin eklenmesi mutex ile korunur, ardından asansör semaforla uyandırılır */
static int submit_elevator_request(SharedData *shm, int is_delivery, int carrier_id, int src, int dst, int word_id, char character, int char_idx) {
  
  /* Hangi asansör? Delivery mi reposition mu? */
  ElevatorData *elev = is_delivery ? &shm->delivery_elev : &shm->reposition_elev;
  pthread_mutex_t *mtx = is_delivery ? &shm->delivery_mutex : &shm->reposition_mutex;
  sem_t *sem = is_delivery ? &shm->delivery_sem : &shm->reposition_sem;
  
  /* Asansör kuyruğunu kilitle */
  pthread_mutex_lock(mtx);
  
  /* Kuyrukta boş slot bul */
  int slot = -1;
  for (int i = 0; i < MAX_ELEV_REQ; i++) {
    if (elev->requests[i].active == REQ_EMPTY) {
      slot = i;
      break;
    }
  }
  /* Kuyruk doluysa hata döndür */
  if (slot == -1) {
    pthread_mutex_unlock(mtx);
    return -1;
  }
  /* İstek bilgilerini doldur */
  elev->requests[slot].active = REQ_WAITING;    /* İstek bekleme durumunda */
  elev->requests[slot].carrier_id = carrier_id; /* İsteği yapan carrier */
  elev->requests[slot].source_floor = src;      /* Kalkış katı */
  elev->requests[slot].dest_floor = dst;        /* Varış katı */
  elev->requests[slot].word_id = word_id; /* Taşınan karakterin kelime ID'si */
  elev->requests[slot].character = character; /* Taşınan karakter */
  elev->requests[slot].char_index = char_idx; /* Karakterin orijinal indeksi */
  elev->request_count++;                      /* Toplam istek sayısını artır */
  pthread_mutex_unlock(mtx);
  /* Asansörü uyandır (engelleniyorsa sem_wait'ten çıkar) */
  sem_post(sem);
  return 0;
}

/* KARAKTERİ SIRALAMA ALANINA YERLEŞTIRME => Teslim edilen karakter, hedef kelime'nin sıralama alanına ilk boş ve sabitlenmemiş slota yerleştirilir */
static void place_char_in_sorting_area(SharedData *shm, int word_idx, char ch, int orig_idx) {
  WordEntry *word = &shm->words[word_idx];
  /* Kelime mutex'ini kilitle - sıralama alanına atomik erişim */
  pthread_mutex_lock(&shm->word_mutex[word_idx]);
  /* İlk boş ve sabitlenmemiş slotu bul */
  for (int i = 0; i < word->word_length; i++) {
    if (!word->occupied[i] && !word->fixed[i]) {
      /* Karakteri bu slota yerleştir */
      word->sorting_area[i] = ch;
      word->sort_orig_idx[i] = orig_idx;
      word->occupied[i] = 1;
      word->chars_delivered++;
      break;
    }
  }
  pthread_mutex_unlock(&shm->word_mutex[word_idx]);
  /* Sorting process'lere yeni karakter geldiğini bildir */
  sem_post(&shm->sorting_work_sem[word->sorting_floor]);
}

/* LETTER-CARRIER ANA FONKSİYONU => Her letter-carrier süreci fork() sonrası bu fonksiyonu çağırır
 * Süreç katlar arası dinamik olarak hareket edebilir */
void letter_carrier_main(SharedData *shm, int carrier_id, int initial_floor) {
  /* Rasgele sayı üretecini bu süreç için benzersiz hale getir */
  srand(time(NULL) ^ getpid() ^ carrier_id);
  int current_floor = initial_floor; /* Başlangıç katı */
  /* Sistem çalıştığı sürece görev döngüsü */
  while (shm->system_running) {
    int found_task = 0; /* Görev bulundu mu bayrağı */
    int word_idx = -1;  /* Bulunan görevin kelime indeksi */
    int char_idx = -1;  /* Bulunan görevin karakter indeksi */
    
    /* MEVCUT KATTAN GÖREV ARAMA => Bu katta admitted olan ve henüz tüm karakterleri 
       alınmamış kelimelerden rasgele bir karakter görevi seç */

    /* Önce bu katta görev içeren kelimeleri bul */
    int candidate_words[MAX_WORDS]; /* Uygun kelime indeksleri */
    int num_candidates = 0;         /* Uygun kelime sayısı */
    for (int i = 0; i < shm->num_words; i++) {
      /* Kelime bu katta mı ve aktif mi kontrol et */
      if (shm->words[i].admitted && !shm->words[i].all_chars_picked &&
          shm->words[i].arrival_floor == current_floor) {
        /* Bu kelimede alınmamış karakter var mı kontrol et */
        int has_unclaimed = 0;
        for (int j = 0; j < shm->words[i].word_length; j++) {
          if (!shm->words[i].char_tasks[j].claimed &&
              !shm->words[i].char_tasks[j].delivered) {
            has_unclaimed = 1;
            break;
          }
        }
        if (has_unclaimed) {
          candidate_words[num_candidates++] = i;
        }
      }
    }

    /* RASGELE KELİME VE KARAKTER SEÇİMİ => PDF'ye göre letter-carrier rasgele kelime seçer,
     * sonra o kelimeden rasgele karakter alır */
    if (num_candidates > 0) {
      /* Rasgele bir kelime seç */
      int rand_word = candidate_words[rand() % num_candidates];
      /* O kelimeden alınmamış karakterleri bul */
      int unclaimed_chars[MAX_WORD_LENGTH];
      int num_unclaimed = 0;
      for (int j = 0; j < shm->words[rand_word].word_length; j++) {
        if (!shm->words[rand_word].char_tasks[j].claimed &&
            !shm->words[rand_word].char_tasks[j].delivered) {
          unclaimed_chars[num_unclaimed++] = j;
        }
      }
      if (num_unclaimed > 0) {
        /* Rasgele bir karakter seç */
        int rand_char = unclaimed_chars[rand() % num_unclaimed];
        /* KARAKTERİ ATOMİK OLARAK SAHİPLEN => Floor mutex ile korunarak aynı karakterin birden fazla
         * carrier tarafından alınması önlenir */
        pthread_mutex_lock(&shm->floor_mutex[current_floor]);
        /* Tekrar kontrol et (başka carrier almış olabilir) */
        if (!shm->words[rand_word].char_tasks[rand_char].claimed &&
            !shm->words[rand_word].char_tasks[rand_char].delivered) {
          /* Karakteri sahiplen */
          shm->words[rand_word].char_tasks[rand_char].claimed = 1;
          shm->words[rand_word].char_tasks[rand_char].carrier_id = carrier_id;
          word_idx = rand_word;
          char_idx = rand_char;
          found_task = 1;
          /* Kelimeden alınan karakter sayısını güncelle */
          shm->words[rand_word].chars_picked++;
          /* Tüm karakterler alındı mı kontrol et */
          if (shm->words[rand_word].chars_picked >=
              shm->words[rand_word].word_length) {
            shm->words[rand_word].all_chars_picked = 1;
            /* Varış katı kapasitesini serbest bırak (sorting katı farklıysa) */
            if (shm->words[rand_word].arrival_floor !=
                shm->words[rand_word].sorting_floor) {
              shm->floors[shm->words[rand_word].arrival_floor]
                  .active_word_count--;
              /* Kapasite boşaldığı için word-carrier'ları bildir */
              sem_post(&shm->word_available_sem);
            }
          }
        }
        pthread_mutex_unlock(&shm->floor_mutex[current_floor]);
      }
    }
    /* GÖREV BULUNAMADI - YENİDEN KONUMLANMA VEYA BEKLEME => İş yoksa reposition asansörü ile rasgele kata git
     * veya tek kat varsa bekle */
    if (!found_task) {
      /* Sistem hâlâ çalışıyor mu kontrol et */
      if (!shm->system_running)
        break;
      /* Sistemde hiç bekleyen görev var mı kontrol et */
      int any_pending = 0;
      for (int i = 0; i < shm->num_words; i++) {
        if (shm->words[i].admitted && !shm->words[i].all_chars_picked) {
          any_pending = 1;
          break;
        }
      }
      if (!any_pending) {
        /* Hiç taşınacak karakter kalmadı - semaforla bekle */
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += 200000000L; /* 200ms timeout */
        if (ts.tv_nsec >= 1000000000L) {
          ts.tv_sec += 1;
          ts.tv_nsec -= 1000000000L;
        }
        sem_timedwait(&shm->letter_work_sem[current_floor], &ts);
        continue;
      }
      /* İş var ama bu katta değil - reposition asansörü kullan */
      if (shm->num_floors > 1) {
        /* Rasgele bir hedef kat seç (mevcut kattan farklı) */
        int target_floor = rand() % shm->num_floors;
        while (target_floor == current_floor && shm->num_floors > 1) {
          target_floor = rand() % shm->num_floors;
        }
        log_msg(
            shm,
            "Letter-carrier-process_%d found no available task on floor %d\n",
            carrier_id, current_floor);
        log_msg(shm,
                "Letter-carrier-process_%d requested reposition elevator from "
                "floor %d\n",
                carrier_id, current_floor);
        /* Reposition asansörü isteği gönder */
        submit_elevator_request(shm, 0, carrier_id, current_floor, target_floor,
                                0, 0, 0);
        /* Asansör teslimini bekle (kişisel semafor) */
        sem_wait(&shm->carriers[carrier_id].ready_sem);
        /* Sistem kapanıyor olabilir */
        if (!shm->system_running)
          break;
        /* Kat değişikliğini güncelle */
        current_floor = shm->carriers[carrier_id].current_floor;
        log_msg(shm, "Letter-carrier-process_%d resumed work on floor %d\n",
                carrier_id, current_floor);
      } else {
        /* Tek katlı sistem - reposition anlamsız, bekle */
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += 100000000L; /* 100ms */
        if (ts.tv_nsec >= 1000000000L) {
          ts.tv_sec += 1;
          ts.tv_nsec -= 1000000000L;
        }
        sem_timedwait(&shm->letter_work_sem[current_floor], &ts);
      }
      continue;
    }
    /* GÖREV BULUNDU - KARAKTERİ TAŞI => Hedef kat aynıysa doğrudan yerleştir,
     * farklıysa delivery asansörü kullan */
    CharTask *task = &shm->words[word_idx].char_tasks[char_idx];
    int dest = task->dest_floor;
    log_msg(shm,
            "Letter-carrier-process_%d selected char '%c' of word %d from "
            "floor %d\n",
            carrier_id, task->character, task->word_id, current_floor);
    if (dest == current_floor) {
      /* AYNI KAT - DOĞRUDAN YERLEŞTİRME => Asansöre gerek yok, karakter hemen sıralama alanına konur */
      log_msg(shm, "Destination is same floor -> direct placement\n");
      /* Karakter görevini teslim edildi olarak işaretle */
      task->delivered = 1;
      /* Karakteri sıralama alanına yerleştir */
      place_char_in_sorting_area(shm, word_idx, task->character,
                                 task->original_index);
      log_msg(shm,
              "Letter-carrier-process_%d brought char '%c' of word %d to floor "
              "%d\n\n",
              carrier_id, task->character, task->word_id, current_floor);
      /* Taşınan karakter istatistiğini güncelle */
      pthread_mutex_lock(&shm->global_mutex);
      shm->total_chars_transported++;
      pthread_mutex_unlock(&shm->global_mutex);
    } else {
      /* FARKLI KAT - DELİVERY ASANSÖRÜ KULLAN => İstek gönder, asansörün teslimini bekle */
      log_msg(shm,
              "Letter-carrier-process_%d requested delivery elevator from "
              "floor %d to floor %d\n\n",
              carrier_id, current_floor, dest);
      /* Delivery asansörüne istek gönder */
      submit_elevator_request(shm, 1, carrier_id, current_floor, dest,
                              task->word_id, task->character,
                              task->original_index);
      /* Asansör teslimini bekle (kişisel semafor üzerinde engelle) */
      sem_wait(&shm->carriers[carrier_id].ready_sem);
      /* Sistem kapanıyor olabilir */
      if (!shm->system_running)
        break;
      /* Kat bilgilerini güncelle */
      current_floor = shm->carriers[carrier_id].current_floor;
      /* Karakter görevini teslim edildi olarak işaretle */
      task->delivered = 1;
      /* Karakteri sıralama alanına yerleştir */
      place_char_in_sorting_area(shm, word_idx, task->character,
                                 task->original_index);
      log_msg(shm,
              "Letter-carrier-process_%d brought char '%c' of word %d to floor "
              "%d\n\n",
              carrier_id, task->character, task->word_id, current_floor);
      /* Taşınan karakter istatistiğini güncelle */
      pthread_mutex_lock(&shm->global_mutex);
      shm->total_chars_transported++;
      pthread_mutex_unlock(&shm->global_mutex);
    }
  }
}
