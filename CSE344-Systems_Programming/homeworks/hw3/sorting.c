#include "common.h"

/* TEK KELİMEYİ SIRALAMA FONKSİYONU => Kelime'nin sıralama alanındaki karakterleri kontrol eder
 * Doğru pozisyondakileri sabitler, yanlışları taşır/swap eder
 * Dönüş: 1 = en az bir işlem yapıldı, 0 = hiçbir şey yapılmadı */

/* Sorting (Sıralama) Süreci => Sıralama katlarında çalışır, karakterleri orijinal kelimeye göre
  doğru konumlarına yerleştirir ve kelime tamamlanmasını tespit eder */

static int sort_word(SharedData *shm, int word_idx, int sorter_id) {
  WordEntry *word = &shm->words[word_idx];
  int did_work = 0; /* İşlem yapıldı mı bayrağı */
  /* Her slot için sıralama kurallarını uygula */
  for (int i = 0; i < word->word_length; i++) {
    /* DURUM 1: BOŞ SLOT => Slot dolmamış, hiçbir şey yapma */
    if (!word->occupied[i]) {
      continue;
    }
    /* DURUM 2: SABİTLENMİŞ SLOT => Bu pozisyon zaten doğru ve sabitlenmiş, dokunma */
    if (word->fixed[i]) {
      continue;
    }
    /* Mevcut karakterin orijinal indeksini al */
    int orig_idx = word->sort_orig_idx[i];
    /* DURUM 3: DOĞRU POZİSYON => Karakter zaten doğru yerinde - sabitle */
    if (orig_idx == i) {
      word->fixed[i] = 1; /* Bu slotu sabit olarak işaretle */
      log_msg(
          shm, "Sorting-process_%d fixed char '%c' of word %d on floor %d\n",
          sorter_id, word->sorting_area[i], word->word_id, word->sorting_floor);
      did_work = 1;
      continue;
    }
    /* DURUM 4: YANLIŞ POZİSYON => Karakter yanlış yerde - hedef pozisyona taşınmalı veya swap yapılmalı
     * target_pos: bu karakterin gitmesi gereken yer (orijinal indeksi) */
    int target_pos = orig_idx;
    /* Hedef pozisyon geçerli aralıkta mı kontrol et */
    if (target_pos < 0 || target_pos >= word->word_length) {
      continue;
    }
    if (!word->occupied[target_pos]) {
      /* HEDEF BOŞ - DOĞRUDAN TAŞI (MOVE) => Karakteri mevcut slottan hedef slota taşı
       * Eski slot boşalır, yeni slotta karakter kontrol edilir */
      word->sorting_area[target_pos] = word->sorting_area[i];
      word->sort_orig_idx[target_pos] = word->sort_orig_idx[i];
      word->occupied[target_pos] = 1;
      /* Eski slotu temizle */
      word->sorting_area[i] = 0;
      word->sort_orig_idx[i] = -1;
      word->occupied[i] = 0;
      log_msg(
          shm,
          "Sorting-process_%d moved char '%c' of word %d to correct index\n",
          sorter_id, word->sorting_area[target_pos], word->word_id);
      /* Yeni pozisyonda doğru mu kontrol et ve sabitlenebiliyorsa sabitle */
      if (word->sort_orig_idx[target_pos] == target_pos) {
        word->fixed[target_pos] = 1;
        log_msg(shm,
                "Sorting-process_%d fixed char '%c' of word %d on floor %d\n",
                sorter_id, word->sorting_area[target_pos], word->word_id,
                word->sorting_floor);
      }
      did_work = 1;
    } else if (!word->fixed[target_pos]) {
      /* HEDEF DOLU VE SABİT DEĞİL - SWAP YAP => İki karakterin yerini değiştir
       * Her iki pozisyon da kontrol edilip sabitlenebilir */
      /* Geçici değişkenlerle swap */
      char tmp_char = word->sorting_area[i];
      int tmp_idx = word->sort_orig_idx[i];
      word->sorting_area[i] = word->sorting_area[target_pos];
      word->sort_orig_idx[i] = word->sort_orig_idx[target_pos];
      word->sorting_area[target_pos] = tmp_char;
      word->sort_orig_idx[target_pos] = tmp_idx;
      log_msg(shm, "Sorting-process_%d swap performed for word %d\n", sorter_id,
              word->word_id);
      /* Swap sonrası her iki pozisyonu da kontrol et */
      /* Hedef pozisyon (target_pos) artık doğru mu? */
      if (word->sort_orig_idx[target_pos] == target_pos) {
        word->fixed[target_pos] = 1;
        log_msg(shm, "Sorting-process_%d fixed one index of word %d\n",
                sorter_id, word->word_id);
      }
      /* Kaynak pozisyon (i) artık doğru mu? */
      if (word->sort_orig_idx[i] == i) {
        word->fixed[i] = 1;
        log_msg(shm, "Sorting-process_%d fixed one index of word %d\n",
                sorter_id, word->word_id);
      }
      did_work = 1;
    } else {
      /* HEDEF SABİTLENMİŞ - HİÇBİR ŞEY YAPMA => Hedef pozisyon sabitlenmiş, karakter taşınamaz
       * Sonraki dönüşte başka hedef bulunabilir */
      continue;
    }
  }
  return did_work;
}
/* KELİME TAMAMLANMA KONTROLÜ => Tüm fixed[] dizisi 1 ise kelime tamamlanmıştır
 * Dönüş: 1 = tamamlandı, 0 = henüz tamamlanmadı */
static int check_completion(SharedData *shm, int word_idx) {
  WordEntry *word = &shm->words[word_idx];
  /* Tüm slotlar sabitlenmiş mi kontrol et */
  for (int i = 0; i < word->word_length; i++) {
    if (!word->fixed[i]) {
      return 0; /* En az bir slot henüz sabitlenmemiş */
    }
  }
  return 1; /* Tüm slotlar sabit - kelime tamamlanmış */
}
/* SORTING PROCESS ANA FONKSİYONU => Her sorting süreci fork() sonrası bu fonksiyonu çağırır
 * Atandığı sıralama katından asla ayrılmaz
 * Aynı kattaki herhangi bir kelime üzerinde çalışabilir */
void sorting_process_main(SharedData *shm, int id, int floor) {
  /* Rasgele sayı üretecini bu süreç için benzersiz hale getir */
  srand(time(NULL) ^ getpid() ^ id);
  /* Sistem çalıştığı sürece sıralama döngüsü */
  while (shm->system_running) {
    int did_any_work = 0; /* Bu taramada iş yapıldı mı */
    /* BU KATTAKİ TÜM KELİMELERİ TARA => sorting_floor değeri bu katla eşleşen kelimeleri bul
     * Admitted ve tamamlanmamış kelimeler üzerinde çalış */
    for (int i = 0; i < shm->num_words; i++) {
      /* Sistem durdu mu kontrol et */
      if (!shm->system_running)
        return;
      /* Bu kelime bu katta mı ve sıralamaya hazır mı? */
      if (shm->words[i].sorting_floor != floor)
        continue;
      if (!shm->words[i].admitted)
        continue;
      if (shm->words[i].completed)
        continue;
      /* PER-WORD KİLİTLEME (TRYLOCK) => Aynı anda sadece BİR sorting process bir kelime üzerinde çalışabilir
       * trylock kullanılır: kilitlenemezse bu kelimeyi atla, diğerine geç
       * Böylece deadlock ve gereksiz bekleme önlenir */
      if (pthread_mutex_trylock(&shm->word_mutex[i]) != 0) {
        continue; /* Başka sorter çalışıyor, atla */
      }
      /* Kelime üzerinde çalışıyoruz - sıralama kurallarını uygula */
      log_msg(shm, "Sorting-process_%d is scanning word %d on floor %d\n", id,
              shm->words[i].word_id, floor);
      int result = sort_word(shm, i, id);
      if (result) {
        did_any_work = 1;
      }
      /* TAMAMLANMA KONTROLÜ => Sıralama sonrası tüm pozisyonlar sabit mi kontrol et
       * Tamamlandıysa completed bayrağını set et ve istatistikleri güncelle */
      if (check_completion(shm, i)) {
        shm->words[i].completed = 1;
        log_msg(shm, "Word %d COMPLETED\n\n", shm->words[i].word_id);
        /* Sıralama katı kapasitesini serbest bırak */
        pthread_mutex_lock(&shm->floor_mutex[floor]);
        shm->floors[floor].active_word_count--;
        pthread_mutex_unlock(&shm->floor_mutex[floor]);
        /* Varış katı = sıralama katı olan durumda da kapasiteyi serbest bırak
         */
        if (shm->words[i].arrival_floor == shm->words[i].sorting_floor) {
          /* Zaten aynı kat, active_word_count sadece bir kez artırılmıştı */
          /* Yukarıda düşürdük, ek işlem gerekmez */
        }
        /* Tamamlanan kelime sayısını atomik güncelle */
        pthread_mutex_lock(&shm->global_mutex);
        shm->total_completed++;
        pthread_mutex_unlock(&shm->global_mutex);
        /* Word-carrier'lara kapasite boşaldığını bildir */
        sem_post(&shm->word_available_sem);
        /* Parent'a kelime tamamlandığını bildir */
        sem_post(&shm->completion_sem);
      }
      /* Kelime mutex'ini serbest bırak */
      pthread_mutex_unlock(&shm->word_mutex[i]);
    }
    /* İŞBULUNAMADIYSA BEKLE => Yeni karakter gelene kadar semaforla bekle
     * Meşgul bekleme (busy wait) yapılmaz */
    if (!did_any_work) {
      struct timespec ts;
      clock_gettime(CLOCK_REALTIME, &ts);
      /* 50 milisaniye zaman aşımı ile bekle */
      ts.tv_nsec += 50000000L;
      if (ts.tv_nsec >= 1000000000L) {
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000000000L;
      }
      sem_timedwait(&shm->sorting_work_sem[floor], &ts);
    }
  }
}