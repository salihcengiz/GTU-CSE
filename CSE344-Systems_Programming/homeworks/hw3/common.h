#ifndef _GNU_SOURCE /* POSIX ve GNU uzantılarını etkinleştir (-std=c11 varsayılan olarak kapatır) */
#define _GNU_SOURCE /* getopt/optarg, MAP_ANONYMOUS, sigaction, clock_gettime, sem_timedwait, kill */
#endif

#ifndef COMMON_H
#define COMMON_H

// Ortak başlık dosyası: Paylaşılan veri yapıları, sabitler ve fonksiyon prototipleri
// Tüm süreçler bu başlık dosyasını kullanarak ortak veri yapılarına erişir

/* Standart C kütüphaneleri */
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* SABİTLER - Sistemin maksimum sınırları */
#define MAX_WORDS 512      /* Maksimum kelime sayısı */
#define MAX_WORD_LENGTH 64 /* Bir kelimenin maksimum karakter uzunluğu */
#define MAX_FLOORS 64      /* Maksimum kat sayısı */
#define MAX_CARRIERS 2048  /* Maksimum letter-carrier süreci sayısı */
#define MAX_ELEV_REQ 2048  /* Asansör istek kuyruğunun maksimum boyutu */
#define MAX_CHILDREN 4096 /* Toplam oluşturulabilecek maksimum child süreç sayısı */

/* Asansör yön sabitleri */
#define DIR_UP 1    /* Yukarı yön */
#define DIR_DOWN -1 /* Aşağı yön */
#define DIR_IDLE 0  /* Boşta - yön yok */

/* Asansör istek durumları */
#define REQ_EMPTY 0     /* İstek slotu boş, kullanılmıyor */
#define REQ_WAITING 1   /* İstek yapıldı, asansör bekleniyor */
#define REQ_PICKED_UP 2 /* Yolcu asansöre alındı, taşınıyor */
#define REQ_DONE 3      /* Teslim tamamlandı */

/* KARAKTERİSTİK GÖREV YAPISI => Her kelimeden ayrıştırılan her karakter için bir görev oluşturulur */
typedef struct {
  int word_id;        /* Bu karakterin ait olduğu kelimenin benzersiz kimliği */
  char character;     /* Taşınacak karakter (ör: 'a', 'p', vb.) */
  int original_index; /* Karakterin orijinal kelime içindeki konumu (0-tabanlı) */
  int source_floor;   /* Karakterin şu an bulunduğu kat (varış katı) */
  int dest_floor;     /* Karakterin gitmesi gereken kat (sıralama katı) */
  int claimed; /* 0: henüz alınmadı, 1: bir letter-carrier tarafından alındı */
  int delivered;  /* 0: henüz teslim edilmedi, 1: hedef kata teslim edildi */
  int carrier_id; /* Bu görevi alan letter-carrier'ın kimliği (-1: atanmamış) */
} CharTask;

/* KELİME GİRDİ YAPISI => Giriş dosyasından okunan her kelime için temel bilgileri tutar */
typedef struct {
  int word_id; /* Kelimenin benzersiz kimliği (giriş dosyasından) */
  char word[MAX_WORD_LENGTH]; /* Kelimenin kendisi (ör: "apple") */
  int word_length;            /* Kelimenin karakter uzunluğu */
  int sorting_floor; /* Kelimenin sıralanacağı kat (giriş dosyasından) */
  int arrival_floor; /* Kelimenin getirildiği kat (word-carrier'ın katı) */

  /* Durum bayrakları - başlangıçta hepsi 0 */
  int claimed;          /* 1: word-carrier tarafından alındı */
  int admitted;         /* 1: kata kabul edildi (kapasite uygun) */
  int completed;        /* 1: tüm karakterler doğru sıralandı */
  int all_chars_picked; /* 1: tüm karakterler varış katından alındı */

  /* Sıralama alanı - bu kelime için ayrılmış sıralama yapısı */
  char sorting_area[MAX_WORD_LENGTH]; /* Sıralama alanındaki karakterler */
  int sort_orig_idx[MAX_WORD_LENGTH]; /* Her slottaki karakterin orijinal indeksi */
  int occupied[MAX_WORD_LENGTH];      /* Slot dolu mu? (0: boş, 1: dolu) */
  int fixed[MAX_WORD_LENGTH]; /* Slot sabitlenmiş mi? (0: değil, 1: sabit) */
  int chars_delivered; /* Sıralama katına teslim edilen karakter sayısı */
  int chars_picked;    /* Varış katından alınan karakter sayısı */

  /* Karakter görevleri - her karakter için bir görev */
  CharTask
      char_tasks[MAX_WORD_LENGTH]; /* Bu kelimenin karakter görevleri dizisi */
} WordEntry;

/* KAT VERİ YAPISI => Her katın durumunu ve kapasitesini takip eder */
typedef struct {
  int floor_id;             /* Katın numarası (0'dan başlar) */
  int active_word_count;    /* Şu an bu katta aktif olan kelime sayısı */
  int max_capacity;         /* Bu kattaki maksimum aktif kelime kapasitesi */
  int letter_carrier_count; /* Bu kattaki mevcut letter-carrier süreci sayısı */
} FloorData;

/* ASANSÖR İSTEK YAPISI => Hem delivery hem de reposition asansörü için ortak istek formatı */
typedef struct {
  int active;       /* İstek aktif mi? (REQ_EMPTY, REQ_WAITING, REQ_PICKED_UP, REQ_DONE) */
  int carrier_id;   /* İsteği yapan letter-carrier'ın kimliği */
  int source_floor; /* Kalkış katı */
  int dest_floor;   /* Varış katı */

  /* Delivery asansörü için ek bilgi: taşınan karakter detayları */
  int word_id;    /* Taşınan karakterin kelime kimliği */
  char character; /* Taşınan karakter */
  int char_index; /* Karakterin orijinal indeksi */
} ElevReq;

/* ASANSÖR VERİ YAPISI => Delivery ve reposition asansörlerinin durumunu takip eder */
typedef struct {
  int current_floor; /* Asansörün şu anki katı */
  int direction;     /* Hareket yönü (DIR_UP, DIR_DOWN, DIR_IDLE) */
  int capacity;      /* Maksimum taşıma kapasitesi */
  int current_load;  /* Şu anki yük (içerideki yolcu/karakter sayısı) */
  ElevReq requests[MAX_ELEV_REQ]; /* İstek kuyruğu */
  int request_count;              /* Kuyruktaki toplam aktif istek sayısı */
  int total_ops; /* Toplam asansör operasyonu sayısı (istatistik) */
} ElevatorData;

/* LETTER-CARRIER BİLGİ YAPISI => Her letter-carrier sürecinin paylaşılan bellek içindeki durumu */
typedef struct {
  int carrier_id;    /* Carrier'ın benzersiz kimliği */
  pid_t pid;         /* Carrier sürecinin PID'si */
  int current_floor; /* Carrier'ın şu anki katı */
  int assigned;      /* Bu slot aktif mi? (1: evet, 0: hayır) */
  sem_t ready_sem;   /* Asansör teslimi için kişisel semafor */
} CarrierInfo;

/* ANA PAYLAŞILAN BELLEK YAPISI => Tüm süreçlerin eriştiği merkezi veri yapısı */
typedef struct {
  /* Sistem yapılandırması */
  int num_floors;         /* Toplam kat sayısı */
  int num_words;          /* Toplam kelime sayısı */
  int words_per_floor;    /* Kat başına word-carrier sayısı (argümandan) */
  int carriers_per_floor; /* Kat başına letter-carrier sayısı (argümandan) */
  int sorters_per_floor;  /* Kat başına sorting process sayısı (argümandan) */
  int system_running;     /* Sistem çalışıyor mu? (1: evet, 0: hayır) */

  /* Kelime verileri */
  WordEntry words[MAX_WORDS]; /* Tüm kelimeler dizisi */
  int round_robin_index; /* Word-carrier'lar için döngüsel tarama indeksi */
  int total_completed;   /* Tamamlanan kelime sayısı */

  /* Kat verileri */
  FloorData floors[MAX_FLOORS]; /* Tüm katların durumu */

  /* Asansör verileri */
  ElevatorData delivery_elev;   /* Teslimat asansörünün durumu */
  ElevatorData reposition_elev; /* Yeniden konumlandırma asansörünün durumu */

  /* Letter-carrier bilgileri */
  CarrierInfo carriers[MAX_CARRIERS]; /* Tüm carrier'ların bilgileri */
  int next_carrier_id;                /* Sonraki atanacak carrier kimliği */
  int total_carrier_count;            /* Toplam carrier sayısı */

  /* SENKRONİZASYON PRİMİTİFLERİ => Tüm mutex ve semaforlar paylaşılan bellekte tutulur */

  /* Yazdırma senkronizasyonu - konsol çıktılarının karışmaması için */
  pthread_mutex_t print_mutex;

  /* Global mutex - round-robin indeksi koruması */
  pthread_mutex_t global_mutex;

  /* Kat başına mutex - her katın kapasitesini korur */
  pthread_mutex_t floor_mutex[MAX_FLOORS];

  /* Kelime başına mutex - sıralama alanını korur */
  pthread_mutex_t word_mutex[MAX_WORDS];

  /* Asansör mutexleri - asansör kuyruklarını korur */
  pthread_mutex_t delivery_mutex;
  pthread_mutex_t reposition_mutex;

  /* Carrier ID mutex - yeni carrier kimliği atanırken koruma */
  pthread_mutex_t carrier_id_mutex;

  /* Asansör semaforları - asansörleri uyandırmak için */
  sem_t delivery_sem;
  sem_t reposition_sem;

  /* İş mevcutluğu semaforları */
  sem_t word_available_sem; /* Kelime hazır olduğunda word-carrier'ları uyandırır */
  sem_t letter_work_sem[MAX_FLOORS]; /* Kat bazında letter-carrier iş bildirimi */
  sem_t sorting_work_sem[MAX_FLOORS]; /* Kat bazında sorting process iş bildirimi */
  sem_t completion_sem;  /* Kelime tamamlandığında parent'ı uyandırır */
  sem_t empty_floor_sem; /* Bir katta carrier kalmadığında parent'ı uyandırır */

  /* İstatistikler */
  int total_retries;           /* Toplam başarısız kabul denemesi sayısı */
  int total_chars_transported; /* Toplam taşınan karakter sayısı */

  /* Senkronizasyon bariyeri - başlatma aşaması için */
  sem_t start_sem;
  sem_t init_done_sem;
  int initial_phase_done;
} SharedData;

/* FONKSİYON PROTOTİPLERİ => Her modüldeki ana fonksiyonların bildirimleri */

/* Süreç ana fonksiyonları - fork() sonrası child'da çağrılır */
void word_carrier_main(SharedData *shm, int id, int floor);
void letter_carrier_main(SharedData *shm, int id, int floor);
void sorting_process_main(SharedData *shm, int id, int floor);
void elevator_main(SharedData *shm, int is_delivery);

/* Yardımcı fonksiyon: senkronize log mesajı yazdırır */
void log_msg(SharedData *shm, const char *format, ...);

#endif /* COMMON_H */
