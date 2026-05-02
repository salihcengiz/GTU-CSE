#include "common.h"

/* Asansör Süreçleri (Delivery ve Reposition): sürekli yukarı-aşağı hareket et, her katta yolcu al ve indir. İş yoksa bekle. */

/* HERHANGİ BİR AKTİF İSTEK VEYA YOLCU VAR MI => Kuyruktaki WAITING veya PICKED_UP durumundaki herhangi bir istek arar */
static int has_any_work(ElevatorData *elev) {
  /* Yolcu varsa iş vardır */
  if (elev->current_load > 0)
    return 1;
  /* Bekleyen istek varsa iş vardır */
  for (int i = 0; i < MAX_ELEV_REQ; i++) {
    if (elev->requests[i].active == REQ_WAITING)
      return 1;
  }
  return 0;
}

/* MEVCUT KATTA YOLCU İNDİRME (DROP OFF) => Hedef katı mevcut kat olan tüm PICKED_UP yolcuları indirir */
static void do_drop_off(SharedData *shm, ElevatorData *elev, int is_delivery) {
  const char *name = is_delivery ? "Delivery" : "Reposition";

  for (int i = 0; i < MAX_ELEV_REQ; i++) {
    /* PICKED_UP durumunda ve hedef kat mevcut kat ise indir */
    if (elev->requests[i].active == REQ_PICKED_UP &&
        elev->requests[i].dest_floor == elev->current_floor) {
      int cid = elev->requests[i].carrier_id;

      /* İndirme logu yazdır */
      log_msg(shm,
              "%s elevator drop off at floor %d (currently %d letter-carrier "
              "inside):\n",
              name, elev->current_floor, elev->current_load - 1);
      if (is_delivery) {
        log_msg(
            shm,
            "    Letter-carrier-process_%d carrying char '%c' of word %d\n\n",
            cid, elev->requests[i].character, elev->requests[i].word_id);
      } else {
        log_msg(shm, "    Letter-carrier-process_%d\n\n", cid);
      }

      /* İstek slotunu temizle ve yükü azalt */
      elev->requests[i].active = REQ_EMPTY;
      elev->current_load--;
      elev->request_count--;
      elev->total_ops++;
      if (elev->current_load < 0)
        elev->current_load = 0;
      if (elev->request_count < 0)
        elev->request_count = 0;

      /* Carrier'ın kat bilgisini güncelle ve semaforunu bildir */
      if (cid >= 0 && cid < MAX_CARRIERS && shm->carriers[cid].assigned) {
        int old_floor = shm->carriers[cid].current_floor;
        int new_floor = elev->current_floor;
        shm->carriers[cid].current_floor = new_floor;

        /* Yeni kata carrier sayacını ekle */
        if (old_floor != new_floor) {
          pthread_mutex_lock(&shm->floor_mutex[new_floor]);
          shm->floors[new_floor].letter_carrier_count++;
          pthread_mutex_unlock(&shm->floor_mutex[new_floor]);
        }

        /* Carrier'ı uyandır - artık hedef katta */
        sem_post(&shm->carriers[cid].ready_sem);
      }
    }
  }
}

/* MEVCUT KATTAN YOLCU ALMA (PICK UP) => Kaynak katı mevcut kat olan TÜM bekleyen istekleri alır - Yön filtresi YOKTUR - mevcut kattaki herkes alınır */
static void do_pick_up(SharedData *shm, ElevatorData *elev, int is_delivery) {
  const char *name = is_delivery ? "Delivery" : "Reposition";

  for (int i = 0; i < MAX_ELEV_REQ; i++) {
    /* Kapasite doluysa daha fazla alma */
    if (elev->current_load >= elev->capacity)
      break;

    /* Kaynak katı mevcut kat olan WAITING istekleri al */
    if (elev->requests[i].active == REQ_WAITING &&
        elev->requests[i].source_floor == elev->current_floor) {
      int cid = elev->requests[i].carrier_id;
      int dest = elev->requests[i].dest_floor;

      /* Hedef aynı katsa hemen teslim et */
      if (dest == elev->current_floor) {
        /* İstek tamamlandı - aynı kat teslimi */
        elev->requests[i].active = REQ_EMPTY;
        elev->request_count--;
        elev->total_ops++;
        if (elev->request_count < 0)
          elev->request_count = 0;

        /* Carrier aynı katta kalıyor */
        if (cid >= 0 && cid < MAX_CARRIERS && shm->carriers[cid].assigned) {
          sem_post(&shm->carriers[cid].ready_sem);
        }
        continue;
      }

      /* Yolcuyu asansöre al */
      elev->requests[i].active = REQ_PICKED_UP;
      elev->current_load++;

      /* Alma logu yazdır */
      log_msg(shm,
              "%s elevator pick up (currently %d letter-carrier inside):\n",
              name, elev->current_load);
      if (is_delivery) {
        log_msg(
            shm,
            "    Letter-carrier-process_%d carrying char '%c' of word %d\n\n",
            cid, elev->requests[i].character, elev->requests[i].word_id);
      } else {
        log_msg(shm, "    Letter-carrier-process_%d\n\n", cid);
      }

      /* Carrier'ı eski katından çıkar (artık asansörde) */
      if (cid >= 0 && cid < MAX_CARRIERS && shm->carriers[cid].assigned) {
        int old_floor = shm->carriers[cid].current_floor;
        pthread_mutex_lock(&shm->floor_mutex[old_floor]);
        shm->floors[old_floor].letter_carrier_count--;
        if (shm->floors[old_floor].letter_carrier_count <= 0) {
          shm->floors[old_floor].letter_carrier_count = 0;
          sem_post(&shm->empty_floor_sem);
        }
        pthread_mutex_unlock(&shm->floor_mutex[old_floor]);
      }
    }
  }
}

/* 
 * ASANSÖR ANA FONKSİYONU - BASİTLEŞTİRİLMİŞ SCAN
 * Çalışma mantığı:
 * 1. İş yoksa semafor ile bekle (IDLE)
 * 2. İş varken: mevcut yönde kat kat ilerle
 * 3. Her katta: önce indir, sonra al
 * 4. Sınıra ulaştığında yön değiştir
 * 5. Tam bir tur (yukarı+aşağı) iş olmadan geçerse IDLE ol
 */
void elevator_main(SharedData *shm, int is_delivery) {
  /* İlgili asansör verilerini al */
  ElevatorData *elev =
      is_delivery ? &shm->delivery_elev : &shm->reposition_elev;
  pthread_mutex_t *mtx =
      is_delivery ? &shm->delivery_mutex : &shm->reposition_mutex;
  sem_t *sem = is_delivery ? &shm->delivery_sem : &shm->reposition_sem;
  const char *name = is_delivery ? "Delivery" : "Reposition";

  /* Başlangıç yönü: yukarı */
  elev->direction = DIR_UP;

  /* Sistem çalıştığı sürece asansör döngüsü */
  while (shm->system_running) {
    /* IDLE DURUMU - iş yoksa bekle */
    pthread_mutex_lock(mtx);
    int work = has_any_work(elev);
    pthread_mutex_unlock(mtx);

    if (!work) {
      /* Birikmiş semafor sayaçlarını temizle */
      while (sem_trywait(sem) == 0) { /* drain */
      }

      /* Yeni istek gelene kadar bloke ol */
      sem_wait(sem);
      if (!shm->system_running)
        break;

      /* Semafor uyandırdı ama istek gerçekten var mı kontrol et */
      pthread_mutex_lock(mtx);
      work = has_any_work(elev);
      pthread_mutex_unlock(mtx);
      if (!work)
        continue; /* Sahte uyandırma, tekrar bekle */
    }

    /* AKTİF HAREKET DÖNGÜSÜ => İş bitene kadar yukarı-aşağı hareket et - Her katta dur, yolcu indir/al */
    /* İş olmadan kaç kat geçtiğini say (boşuna dönme kontrolü) */
    int idle_floor_count = 0;
    int total_floors_for_sweep = shm->num_floors * 2;

    while (shm->system_running) {
      pthread_mutex_lock(mtx);

      /* Mevcut katta işlem yap */
      int load_before = elev->current_load;
      int req_before = elev->request_count;

      /* 1. Yolcu indir */
      do_drop_off(shm, elev, is_delivery);

      /* 2. Yolcu al */
      do_pick_up(shm, elev, is_delivery);

      int load_after = elev->current_load;
      int req_after = elev->request_count;

      /* Bu katta bir şey olduysa idle sayacını sıfırla */
      if (load_before != load_after || req_before != req_after) {
        idle_floor_count = 0;
      }

      /* Hâlâ iş var mı kontrol et */
      int still_work = has_any_work(elev);

      if (!still_work) {
        /* Tüm iş bitti - IDLE'a dön */
        pthread_mutex_unlock(mtx);
        break;
      }

      /* Çok uzun süredir boşuna dönüyorsa IDLE'a geç */
      /* (yeni istek sem ile uyandıracak) */
      if (idle_floor_count > total_floors_for_sweep) {
        pthread_mutex_unlock(mtx);
        usleep(5000); /* 5ms bekle */
        idle_floor_count = 0;
        continue;
      }

      /* BİR KAT HAREKET ET => Sınırlarda zorunlu yön değiştir */
      /* Tek katlı sistemde hareket etme */
      if (shm->num_floors <= 1) {
        pthread_mutex_unlock(mtx);
        usleep(10000); /* 10ms bekle, tek kat */
        continue;
      }

      /* Sınır kontrolü: yön değiştir */
      if (elev->current_floor >= shm->num_floors - 1) {
        elev->direction = DIR_DOWN;
      } else if (elev->current_floor <= 0) {
        elev->direction = DIR_UP;
      }

      /* Hareket et */
      elev->current_floor += elev->direction;
      idle_floor_count++;

      /* Güvenlik sınır kontrolü */
      if (elev->current_floor < 0)
        elev->current_floor = 0;
      if (elev->current_floor >= shm->num_floors)
        elev->current_floor = shm->num_floors - 1;

      log_msg(shm, "%s elevator arrived at floor %d\n", name,
              elev->current_floor);
      log_msg(shm, "%s elevator moving %s\n\n", name,
              elev->direction == DIR_UP ? "UP" : "DOWN");

      pthread_mutex_unlock(mtx);

      /* Hareket gecikmesi */
      usleep(1000);
    }
  }
}