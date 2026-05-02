#include "common.h"

/* Ana süreç - Koordinatör (Parent Process) => Sistemin başlatılması, izlenmesi ve sonlandırılmasından sorumludur*/ 
/* Global paylaşılan bellek işaretçisi - sinyal işleyiciden erişim için global */ 

static SharedData *g_shm = NULL;

/* Child süreçlerin PID'lerini tutan dizi ve sayacı */
static pid_t child_pids[MAX_CHILDREN];
static int child_count = 0;

/* Komut satırı parametreleri yapısı */
typedef struct {
  int num_floors;          /* -f: Toplam kat sayısı */
  int word_carriers;       /* -w: Kat başına word-carrier sayısı */
  int letter_carriers;     /* -l: Kat başına letter-carrier sayısı */
  int sorting_procs;       /* -s: Kat başına sorting process sayısı */
  int floor_capacity;      /* -c: Kat başına maksimum aktif kelime */
  int delivery_capacity;   /* -d: Delivery asansör kapasitesi */
  int reposition_capacity; /* -r: Reposition asansör kapasitesi */
  char input_file[256];    /* -i: Giriş dosyası yolu */
  char output_file[256];   /* -o: Çıkış dosyası yolu */
} Config;

/* LOG MESAJI YAZDIRMA FONKSİYONU => Tüm süreçler bu fonksiyonu kullanarak senkronize çıktı üretir
 * print_mutex ile korunur, böylece çıktılar birbiriyle karışmaz */
void log_msg(SharedData *shm, const char *format, ...) {
  /* Yazdırma kilidini al - diğer süreçler yazdıramaz */
  pthread_mutex_lock(&shm->print_mutex);
  /* PID bilgisiyle birlikte mesajı yazdır */
  printf("[PID:%d] ", getpid());
  va_list args;
  va_start(args, format);
  /* Değişken argüman listesiyle formatlanmış çıktı */
  vprintf(format, args);
  va_end(args);
  /* Çıktıyı hemen görüntüle (tamponlamayı boşalt) */
  fflush(stdout);
  /* Yazdırma kilidini serbest bırak */
  pthread_mutex_unlock(&shm->print_mutex);
}

/* KOMUT SATIRINI AYRIŞIR (PARSE) => getopt() kullanarak -f, -w, -l, -s, -c, -d, -r, -i, -o parametrelerini okur
 * Eksik veya geçersiz parametre varsa stderr'e hata yazdırıp çıkar */
static int parse_args(int argc, char *argv[], Config *cfg) {
  int opt;
  /* Tüm alanları başlangıçta -1 olarak işaretle (kontrol için) */
  memset(cfg, 0, sizeof(Config));
  cfg->num_floors = -1;
  cfg->word_carriers = -1;
  cfg->letter_carriers = -1;
  cfg->sorting_procs = -1;
  cfg->floor_capacity = -1;
  cfg->delivery_capacity = -1;
  cfg->reposition_capacity = -1;

  /* getopt ile her bir parametreyi ayrıştır */
  while ((opt = getopt(argc, argv, "f:w:l:s:c:d:r:i:o:")) != -1) {
    switch (opt) {
    case 'f':
      cfg->num_floors = atoi(optarg);
      break; /* Kat sayısı */
    case 'w':
      cfg->word_carriers = atoi(optarg);
      break; /* Word-carrier/kat */
    case 'l':
      cfg->letter_carriers = atoi(optarg);
      break; /* Letter-carrier/kat */
    case 's':
      cfg->sorting_procs = atoi(optarg);
      break; /* Sorting process/kat */
    case 'c':
      cfg->floor_capacity = atoi(optarg);
      break; /* Kat kapasitesi */
    case 'd':
      cfg->delivery_capacity = atoi(optarg);
      break; /* Delivery asansör kapasitesi */
    case 'r':
      cfg->reposition_capacity = atoi(optarg);
      break; /* Reposition asansör kapasitesi */
    case 'i':
      strncpy(cfg->input_file, optarg, 255);
      break; /* Giriş dosyası */
    case 'o':
      strncpy(cfg->output_file, optarg, 255);
      break; /* Çıkış dosyası */
    default:
      /* Bilinmeyen parametre tespit edildi */
      fprintf(
          stderr,
          "Usage: %s -f <floors> -w <word_carriers> -l <letter_carriers> "
          "-s <sorting_procs> -c <capacity> -d <delivery_cap> -r <repos_cap> "
          "-i <input> -o <output>\n",
          argv[0]);
      return -1;
    }
  }

  /* PARAMETRE DOĞRULAMA => Her parametre için geçerlilik kontrolü yapılır
   * Geçersiz parametre tespit edildiğinde stderr'e yazılır */
  if (cfg->num_floors < 1) {
    fprintf(stderr, "Error: Number of floors must be at least 1 (-f)\n");
    return -1;
  }
  if (cfg->num_floors > MAX_FLOORS) {
    fprintf(stderr, "Error: Number of floors cannot exceed %d (-f)\n",
            MAX_FLOORS);
    return -1;
  }
  if (cfg->word_carriers < 1) {
    fprintf(stderr, "Error: Word-carriers per floor must be at least 1 (-w)\n");
    return -1;
  }
  if (cfg->letter_carriers < 1) {
    fprintf(stderr,
            "Error: Letter-carriers per floor must be at least 1 (-l)\n");
    return -1;
  }
  if (cfg->sorting_procs < 1) {
    fprintf(stderr,
            "Error: Sorting processes per floor must be at least 1 (-s)\n");
    return -1;
  }
  if (cfg->floor_capacity < 1) {
    fprintf(stderr, "Error: Floor capacity must be at least 1 (-c)\n");
    return -1;
  }
  if (cfg->delivery_capacity < 1) {
    fprintf(stderr,
            "Error: Delivery elevator capacity must be positive (-d)\n");
    return -1;
  }
  if (cfg->reposition_capacity < 1) {
    fprintf(stderr,
            "Error: Reposition elevator capacity must be positive (-r)\n");
    return -1;
  }
  if (strlen(cfg->input_file) == 0) {
    fprintf(stderr, "Error: Input file must be specified (-i)\n");
    return -1;
  }
  if (strlen(cfg->output_file) == 0) {
    fprintf(stderr, "Error: Output file must be specified (-o)\n");
    return -1;
  }
  /* Giriş dosyasının var olup olmadığını kontrol et */
  if (access(cfg->input_file, R_OK) != 0) {
    fprintf(stderr, "Error: Cannot read input file: %s\n", cfg->input_file);
    return -1;
  }
  return 0;
}

/* GİRİŞ DOSYASINI OKUMA FONKSİYONU => Her satırda word_id, kelime ve sorting_floor bilgisini ayrıştırır
 * Okunan kelimeler paylaşılan belleğe yazılır */
static int read_input_file(const char *filename, SharedData *shm) {
  /* Dosyayı okuma modunda aç */
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    /* Dosya açılamadıysa hata bildir */
    perror("Error: Failed to open input file");
    return -1;
  }
  int count = 0;
  int wid;                    /* Geçici: kelime kimliği */
  char wbuf[MAX_WORD_LENGTH]; /* Geçici: kelime tamponu */
  int sf;                     /* Geçici: sıralama katı */

  /* Dosyadan satır satır oku: word_id kelime sorting_floor */
  while (fscanf(fp, "%d %s %d", &wid, wbuf, &sf) == 3) {
    /* Maksimum kelime kapasitesini aşmadığını kontrol et */
    if (count >= MAX_WORDS) {
      fprintf(stderr, "Warning: Maximum word count exceeded (%d)\n", MAX_WORDS);
      break;
    }
    /* Kelime bilgilerini paylaşılan belleğe yaz */
    shm->words[count].word_id = wid;
    strncpy(shm->words[count].word, wbuf, MAX_WORD_LENGTH - 1);
    shm->words[count].word[MAX_WORD_LENGTH - 1] = '\0';
    shm->words[count].word_length = (int)strlen(wbuf);
    shm->words[count].sorting_floor = sf;
    shm->words[count].arrival_floor =
        -1; /* Henüz atanmadı, word-carrier atayacak */

    /* Tüm bayrakları sıfırla */
    shm->words[count].claimed = 0;
    shm->words[count].admitted = 0;
    shm->words[count].completed = 0;
    shm->words[count].all_chars_picked = 0;
    shm->words[count].chars_delivered = 0;
    shm->words[count].chars_picked = 0;

    /* Sıralama alanını temizle - tüm slotlar boş ve sabitlenmemiş */
    memset(shm->words[count].sorting_area, 0, MAX_WORD_LENGTH);
    memset(shm->words[count].sort_orig_idx, -1, sizeof(int) * MAX_WORD_LENGTH);
    memset(shm->words[count].occupied, 0, sizeof(int) * MAX_WORD_LENGTH);
    memset(shm->words[count].fixed, 0, sizeof(int) * MAX_WORD_LENGTH);

    /* Her karakter için karakter görevi oluştur */
    int wlen = shm->words[count].word_length;
    for (int j = 0; j < wlen; j++) {
      shm->words[count].char_tasks[j].word_id = wid;
      shm->words[count].char_tasks[j].character = wbuf[j];
      shm->words[count].char_tasks[j].original_index = j;
      shm->words[count].char_tasks[j].source_floor =
          -1; /* Henüz bilinmiyor, admitted olunca atanır */
      shm->words[count].char_tasks[j].dest_floor = sf;
      shm->words[count].char_tasks[j].claimed = 0;
      shm->words[count].char_tasks[j].delivered = 0;
      shm->words[count].char_tasks[j].carrier_id = -1;
    }
    count++;
  }
  fclose(fp);

  /* En az bir kelime okunmuş olmalı */
  if (count == 0) {
    fprintf(stderr, "Error: Could not read words from input file\n");
    return -1;
  }

  /* Sorting floor değerlerinin geçerli aralıkta olduğunu kontrol et */
  /* (Not: num_floors henüz shm'de ayarlanmamış olabilir, çağrıdan önce
   * ayarlanmalı) */
  shm->num_words = count;
  return count;
}

/* PROCESS-SHARED MUTEX BAŞLATMA YARDIMCI FONKSİYONU => pthread_mutex'i MAP_SHARED bellekte kullanılabilir hale getirir */
static int init_pshared_mutex(pthread_mutex_t *mtx) {
  pthread_mutexattr_t attr;
  /* Mutex özniteliğini başlat */
  if (pthread_mutexattr_init(&attr) != 0) {
    perror("pthread_mutexattr_init error");
    return -1;
  }
  /* Süreçler arası paylaşım özniteliğini ayarla */
  if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) != 0) {
    perror("pthread_mutexattr_setpshared error");
    pthread_mutexattr_destroy(&attr);
    return -1;
  }
  /* Mutex'i oluştur */
  if (pthread_mutex_init(mtx, &attr) != 0) {
    perror("pthread_mutex_init error");
    pthread_mutexattr_destroy(&attr);
    return -1;
  }
  /* Öznitelik nesnesini temizle (artık gerekmez) */
  pthread_mutexattr_destroy(&attr);
  return 0;
}

/* PROCESS-SHARED SEMAFOR BAŞLATMA YARDIMCI FONKSİYONU => sem_init ile paylaşılan bellekte semafor oluşturur */
static int init_pshared_sem(sem_t *sem, int initial_value) {
  /* pshared=1: süreçler arası kullanılabilir, initial_value: başlangıç değeri
   */
  if (sem_init(sem, 1, initial_value) != 0) {
    perror("sem_init error");
    return -1;
  }
  return 0;
}

/* SENKRONİZASYON PRİMİTİFLERİNİ BAŞLATMA => Tüm mutex ve semaforları oluşturur ve başlangıç değerlerini atar */
static int init_sync(SharedData *shm) {
  /* Yazdırma mutex'ini başlat */
  if (init_pshared_mutex(&shm->print_mutex) != 0)
    return -1;
  /* Global round-robin mutex'ini başlat */
  if (init_pshared_mutex(&shm->global_mutex) != 0)
    return -1;
  /* Carrier ID mutex'ini başlat */
  if (init_pshared_mutex(&shm->carrier_id_mutex) != 0)
    return -1;
  /* Delivery asansör mutex'ini başlat */
  if (init_pshared_mutex(&shm->delivery_mutex) != 0)
    return -1;
  /* Reposition asansör mutex'ini başlat */
  if (init_pshared_mutex(&shm->reposition_mutex) != 0)
    return -1;

  /* Her kat için ayrı mutex oluştur */
  for (int i = 0; i < shm->num_floors; i++) {
    if (init_pshared_mutex(&shm->floor_mutex[i]) != 0)
      return -1;
  }

  /* Her kelime için ayrı mutex oluştur (sıralama koruma) */
  for (int i = 0; i < shm->num_words; i++) {
    if (init_pshared_mutex(&shm->word_mutex[i]) != 0)
      return -1;
  }

  /* Asansör semaforlarını başlat (başlangıçta 0: hiç istek yok) */
  if (init_pshared_sem(&shm->delivery_sem, 0) != 0)
    return -1;
  if (init_pshared_sem(&shm->reposition_sem, 0) != 0)
    return -1;

  /* Kelime mevcudiyet semaforunu başlat */
  if (init_pshared_sem(&shm->word_available_sem, 0) != 0)
    return -1;

  /* Tamamlanma ve boş kat semaforlarını başlat */
  if (init_pshared_sem(&shm->completion_sem, 0) != 0)
    return -1;
  if (init_pshared_sem(&shm->empty_floor_sem, 0) != 0)
    return -1;
  if (init_pshared_sem(&shm->start_sem, 0) != 0)
    return -1;
  if (init_pshared_sem(&shm->init_done_sem, 0) != 0)
    return -1;

  /* Kat bazında iş semaforlarını başlat */
  for (int i = 0; i < shm->num_floors; i++) {
    if (init_pshared_sem(&shm->letter_work_sem[i], 0) != 0)
      return -1;
    if (init_pshared_sem(&shm->sorting_work_sem[i], 0) != 0)
      return -1;
  }

  return 0;
}

/* SİNYAL İŞLEYİCİ (Ctrl+C / SIGINT) => Sistem temiz bir şekilde kapatılır, kaynaklar serbest bırakılır
 * Zombie süreçlerin kalmaması sağlanır */
static void signal_handler(int sig) {
  (void)sig; /* Kullanılmayan parametre uyarısını engelle */
  if (g_shm) {
    /* Sistemi durdur - tüm süreçler bu bayrağı kontrol eder */
    g_shm->system_running = 0;

    /* Tüm bekleyen semaforları uyandır ki süreçler çıkabilsin */
    sem_post(&g_shm->delivery_sem);
    sem_post(&g_shm->reposition_sem);
    sem_post(&g_shm->word_available_sem);
    sem_post(&g_shm->completion_sem);
    sem_post(&g_shm->empty_floor_sem);
    for (int i = 0; i < g_shm->num_floors; i++) {
      sem_post(&g_shm->letter_work_sem[i]);
      sem_post(&g_shm->sorting_work_sem[i]);
    }
    /* Tüm carrier semaforlarını uyandır */
    for (int i = 0; i < g_shm->total_carrier_count; i++) {
      if (g_shm->carriers[i].assigned) {
        sem_post(&g_shm->carriers[i].ready_sem);
      }
    }
  }

  /* Tüm child süreçlere SIGTERM gönder */
  for (int i = 0; i < child_count; i++) {
    if (child_pids[i] > 0) {
      kill(child_pids[i], SIGTERM);
    }
  }

  /* Tüm child süreçlerin sonlanmasını bekle (zombie önleme) */
  for (int i = 0; i < child_count; i++) {
    if (child_pids[i] > 0) {
      waitpid(child_pids[i], NULL, 0);
    }
  }

  /* Paylaşılan bellek alanını serbest bırak */
  if (g_shm) {
    /* Semaforları yok et */
    sem_destroy(&g_shm->delivery_sem);
    sem_destroy(&g_shm->reposition_sem);
    sem_destroy(&g_shm->word_available_sem);
    sem_destroy(&g_shm->completion_sem);
    sem_destroy(&g_shm->empty_floor_sem);
    /* Paylaşılan belleği serbest bırak */
    munmap(g_shm, sizeof(SharedData));
  }

  printf("\nProgram terminated via Ctrl+C.\n");
  _exit(0);
}

/* CHILD SÜREÇ KAYDETME YARDIMCISI => Her fork edilen sürecin PID'sini child_pids dizisine ekler */
static void register_child(pid_t pid) {
  if (child_count < MAX_CHILDREN) {
    child_pids[child_count++] = pid;
  }
}

/* LETTER-CARRIER OLUŞTURMA FONKSİYONU => Belirtilen katta yeni bir letter-carrier süreci oluşturur
 * Carrier bilgilerini paylaşılan belleğe kaydeder */
static pid_t create_letter_carrier(SharedData *shm, int floor, int display_id) {
  int cid;

  /* Carrier ID'sini atomik olarak al */
  pthread_mutex_lock(&shm->carrier_id_mutex);
  cid = shm->next_carrier_id++;
  /* Maksimum carrier sınırını aşmadığından emin ol */
  if (cid >= MAX_CARRIERS) {
    pthread_mutex_unlock(&shm->carrier_id_mutex);
    fprintf(stderr, "Error: Maximum carrier count reached\n");
    return -1;
  }
  /* Carrier bilgilerini paylaşılan belleğe yaz */
  shm->carriers[cid].carrier_id = cid;
  shm->carriers[cid].current_floor = floor;
  shm->carriers[cid].assigned = 1;
  /* Kişisel semafor başlat (asansör teslimi için) */
  init_pshared_sem(&shm->carriers[cid].ready_sem, 0);
  shm->total_carrier_count++;
  pthread_mutex_unlock(&shm->carrier_id_mutex);

  /* Kat üzerindeki carrier sayısını artır */
  pthread_mutex_lock(&shm->floor_mutex[floor]);
  shm->floors[floor].letter_carrier_count++;
  pthread_mutex_unlock(&shm->floor_mutex[floor]);

  /* Yeni süreci oluştur */
  fflush(stdout);
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork error (letter-carrier)");
    return -1;
  }
  if (pid == 0) {
    /* Child süreç: letter-carrier olarak çalışmaya başla */
    shm->carriers[cid].pid = getpid();
    log_msg(shm, "Letter-carrier-process_%d initialized on floor %d\n",
            display_id, floor);
    /* Başlatma aşamasında el sıkışması */
    sem_post(&shm->init_done_sem);
    /* Başlatma aşamasında değilse bekleme */
    if (!shm->initial_phase_done) {
      sem_wait(&shm->start_sem);
    }
    letter_carrier_main(shm, cid, floor);
    _exit(0);
  }
  /* Parent: child PID'sini kaydet */
  shm->carriers[cid].pid = pid;
  register_child(pid);
  return pid;
}

/* ÇIKIŞ DOSYASI OLUŞTURMA FONKSİYONU => Tüm kelimeler sorting_floor ve word_id'ye göre sıralanarak yazılır
 * Formatı: word_id kelime sorting_floor (tek boşlukla ayrılmış) */
static void generate_output(SharedData *shm, const char *filename) {
  /* Çıkış dosyasını yazma modunda aç */
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    perror("Error: Could not create output file");
    return;
  }

  /* Kelime indekslerini sıralama için geçici dizi oluştur */
  int indices[MAX_WORDS];
  for (int i = 0; i < shm->num_words; i++) {
    indices[i] = i;
  }

  /* Sıralama: önce sorting_floor, sonra word_id (basit kabarcık sıralama) */
  for (int i = 0; i < shm->num_words - 1; i++) {
    for (int j = 0; j < shm->num_words - i - 1; j++) {
      WordEntry *a = &shm->words[indices[j]];
      WordEntry *b = &shm->words[indices[j + 1]];
      /* Önce sorting_floor'a göre karşılaştır */
      if (a->sorting_floor > b->sorting_floor ||
          (a->sorting_floor == b->sorting_floor && a->word_id > b->word_id)) {
        /* Yer değiştir */
        int tmp = indices[j];
        indices[j] = indices[j + 1];
        indices[j + 1] = tmp;
      }
    }
  }

  /* Sıralı kelimeler dosyaya yazılır */
  for (int i = 0; i < shm->num_words; i++) {
    WordEntry *w = &shm->words[indices[i]];
    fprintf(fp, "%d %s %d\n", w->word_id, w->word, w->sorting_floor);
  }
  fclose(fp);
}

/* ÖZET BİLGİ YAZDIRMA FONKSİYONU => Sistem tamamlandığında terminale istatistikleri yazdırır */
static void print_summary(SharedData *shm) {
  printf("\n--------------------------------------------------\n\n");
  printf("All words have been transported and sorted...\n");
  printf("Output file is being created...\n\n");
  printf("System Summary:\n\n");
  printf("Total words: %d\n", shm->num_words);
  printf("Completed words: %d\n", shm->total_completed);
  printf("Retries: %d\n", shm->total_retries);
  printf("Characters transported: %d\n", shm->total_chars_transported);
  printf("Delivery elevator operations: %d\n", shm->delivery_elev.total_ops);
  printf("Reposition elevator operations: %d\n",
         shm->reposition_elev.total_ops);
  printf("\nProgram terminated successfully.\n");
}

/* SENKRONİZASYON PRİMİTİFLERİNİ TEMİZLEME => Tüm mutex ve semaforlar düzgün bir şekilde yok edilir */
static void cleanup_sync(SharedData *shm) {
  /* Tüm mutex'leri yok et */
  pthread_mutex_destroy(&shm->print_mutex);
  pthread_mutex_destroy(&shm->global_mutex);
  pthread_mutex_destroy(&shm->carrier_id_mutex);
  pthread_mutex_destroy(&shm->delivery_mutex);
  pthread_mutex_destroy(&shm->reposition_mutex);
  for (int i = 0; i < shm->num_floors; i++) {
    pthread_mutex_destroy(&shm->floor_mutex[i]);
  }
  for (int i = 0; i < shm->num_words; i++) {
    pthread_mutex_destroy(&shm->word_mutex[i]);
  }

  /* Tüm semaforları yok et */
  sem_destroy(&shm->delivery_sem);
  sem_destroy(&shm->reposition_sem);
  sem_destroy(&shm->word_available_sem);
  sem_destroy(&shm->completion_sem);
  sem_destroy(&shm->empty_floor_sem);
  sem_destroy(&shm->start_sem);
  sem_destroy(&shm->init_done_sem);
  for (int i = 0; i < shm->num_floors; i++) {
    sem_destroy(&shm->letter_work_sem[i]);
    sem_destroy(&shm->sorting_work_sem[i]);
  }
  /* Carrier semaforlarını yok et */
  for (int i = 0; i < shm->total_carrier_count; i++) {
    if (shm->carriers[i].assigned) {
      sem_destroy(&shm->carriers[i].ready_sem);
    }
  }
}

/* ANA FONKSİYON (main) => Programın giriş noktası - tüm bileşenleri sırasıyla başlatır */
int main(int argc, char *argv[]) {
  Config cfg;

  /* Rasgele sayı üretecini başlat (süreç dağıtımı için) */
  srand(time(NULL) ^ getpid());

  /* 1. Komut satırı parametrelerini ayrıştır */
  printf("Program is starting...\n");
  if (parse_args(argc, argv, &cfg) != 0) {
    return 1;
  }

  /* 2. Paylaşılan belleği oluştur (mmap ile anonim paylaşılan bellek) */
  printf("Input file is being read...\n");
  SharedData *shm = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE,
                         MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (shm == MAP_FAILED) {
    perror("mmap error");
    return 1;
  }
  /* Paylaşılan belleği sıfırla */
  memset(shm, 0, sizeof(SharedData));
  /* Global işaretçiyi ayarla (sinyal işleyici için) */
  g_shm = shm;

  /* 3. Sistem yapılandırmasını paylaşılan belleğe yaz */
  shm->num_floors = cfg.num_floors;
  shm->words_per_floor = cfg.word_carriers;
  shm->carriers_per_floor = cfg.letter_carriers;
  shm->sorters_per_floor = cfg.sorting_procs;
  shm->system_running = 1;    /* Sistem çalışmaya başlıyor */
  shm->round_robin_index = 0; /* Round-robin indeksi sıfırdan başlar */
  shm->next_carrier_id = 0;   /* İlk carrier ID'si 0 */
  shm->total_carrier_count = 0;

  /* 4. Giriş dosyasını oku ve kelimeleri paylaşılan belleğe yükle */
  int word_count = read_input_file(cfg.input_file, shm);
  if (word_count < 0) {
    munmap(shm, sizeof(SharedData));
    return 1;
  }

  /* 5. Sorting floor değerlerini doğrula */
  for (int i = 0; i < shm->num_words; i++) {
    if (shm->words[i].sorting_floor < 0 ||
        shm->words[i].sorting_floor >= cfg.num_floors) {
      fprintf(stderr,
              "Error: Invalid sorting_floor (%d) for word %d (must be between "
              "0-%d)\n",
              shm->words[i].sorting_floor, shm->words[i].word_id,
              cfg.num_floors - 1);
      munmap(shm, sizeof(SharedData));
      return 1;
    }
  }

  /* 6. Kat verilerini başlat */
  for (int i = 0; i < cfg.num_floors; i++) {
    shm->floors[i].floor_id = i;
    shm->floors[i].active_word_count = 0;
    shm->floors[i].max_capacity = cfg.floor_capacity;
    shm->floors[i].letter_carrier_count = 0;
  }

  /* 7. Asansör verilerini başlat */
  shm->delivery_elev.current_floor = 0;
  shm->delivery_elev.direction = DIR_IDLE;
  shm->delivery_elev.capacity = cfg.delivery_capacity;
  shm->delivery_elev.current_load = 0;
  shm->delivery_elev.request_count = 0;
  shm->delivery_elev.total_ops = 0;

  shm->reposition_elev.current_floor = 0;
  shm->reposition_elev.direction = DIR_IDLE;
  shm->reposition_elev.capacity = cfg.reposition_capacity;
  shm->reposition_elev.current_load = 0;
  shm->reposition_elev.request_count = 0;
  shm->reposition_elev.total_ops = 0;

  /* 8. Senkronizasyon primitiflerini başlat */
  printf("Shared memory is initialized...\n");
  printf("Synchronization primitives are created...\n");
  if (init_sync(shm) != 0) {
    munmap(shm, sizeof(SharedData));
    return 1;
  }

  /* 9. Sinyal işleyiciyi kaydet (Ctrl+C temiz sonlandırma) */
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = signal_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);

  /* 10. Süreçleri oluştur */
  printf("Processes are being created...\n\n");
  log_msg(shm, "Parent process started\n\n");

  int wc_display_id = 0; /* Word-carrier görüntü numarası */
  int lc_display_id = 0; /* Letter-carrier görüntü numarası */
  int sp_display_id = 0; /* Sorting process görüntü numarası */
  pid_t pid;

  /* Her kat için süreçleri oluştur */
  for (int f = 0; f < cfg.num_floors; f++) {
    printf("--- Initializing Floor %d ---\n", f);
    fflush(stdout);

    /* Word-carrier süreçlerini oluştur */
    for (int w = 0; w < cfg.word_carriers; w++) {
      fflush(stdout);
      pid = fork();
      if (pid < 0) {
        perror("fork error (word-carrier)");
        signal_handler(SIGINT); /* Temizle ve çık */
        return 1;
      }
      if (pid == 0) {
        /* Child: word-carrier olarak çalış */
        log_msg(shm, "Word-carrier-process_%d initialized on floor %d\n",
                wc_display_id, f);
        sem_post(&shm->init_done_sem);
        sem_wait(&shm->start_sem);
        word_carrier_main(shm, wc_display_id, f);
        _exit(0);
      }
      register_child(pid);
      wc_display_id++;
    }

    /* Letter-carrier süreçlerini oluştur */
    for (int l = 0; l < cfg.letter_carriers; l++) {
      create_letter_carrier(shm, f, lc_display_id);
      lc_display_id++;
    }

    /* Sorting süreçlerini oluştur */
    for (int s = 0; s < cfg.sorting_procs; s++) {
      fflush(stdout);
      pid = fork();
      if (pid < 0) {
        perror("fork error (sorting)");
        signal_handler(SIGINT);
        return 1;
      }
      if (pid == 0) {
        /* Child: sorting process olarak çalış */
        log_msg(shm, "Sorting-process_%d initialized on floor %d\n",
                sp_display_id, f);
        sem_post(&shm->init_done_sem);
        sem_wait(&shm->start_sem);
        sorting_process_main(shm, sp_display_id, f);
        _exit(0);
      }
      register_child(pid);
      sp_display_id++;
    }

    /* Bu kattaki tüm süreçlerin (wc + lc + sp) başlatma mesajlarını bitirmesini
     * bekle */
    int expected = cfg.word_carriers + cfg.letter_carriers + cfg.sorting_procs;
    for (int i = 0; i < expected; i++) {
      sem_wait(&shm->init_done_sem);
    }
    printf("\n");
  }

  /* Delivery asansör sürecini oluştur */
  fflush(stdout);
  pid = fork();
  if (pid < 0) {
    perror("fork error (delivery elevator)");
    signal_handler(SIGINT);
    return 1;
  }
  if (pid == 0) {
    log_msg(shm, "Delivery elevator process started\n");
    sem_post(&shm->init_done_sem);
    sem_wait(&shm->start_sem);
    elevator_main(shm, 1); /* 1 = delivery asansörü */
    _exit(0);
  }
  register_child(pid);

  /* Reposition asansör sürecini oluştur */
  fflush(stdout);
  pid = fork();
  if (pid < 0) {
    perror("fork error (reposition elevator)");
    signal_handler(SIGINT);
    return 1;
  }
  if (pid == 0) {
    log_msg(shm, "Reposition elevator process started\n");
    sem_post(&shm->init_done_sem);
    sem_wait(&shm->start_sem);
    elevator_main(shm, 0); /* 0 = reposition asansörü */
    _exit(0);
  }
  register_child(pid);

  /* Asansörlerin başlatıldığını teyit et */
  sem_wait(&shm->init_done_sem);
  sem_wait(&shm->init_done_sem);

  printf("\n--------------------------------------------------\n\n");

  /* Tüm süreçlerin başlatma aşamasını tamamladık, çalışmaya başlayabilirler */
  shm->initial_phase_done = 1;
  for (int i = 0; i < child_count; i++) {
    sem_post(&shm->start_sem);
  }

  /* SİSTEM İZLEME DÖNGÜSÜ => Parent süreç tamamlanma ve boş kat durumunu izler
   * Meşgul bekleme (busy wait) yapılmaz - semafor ile beklenir */
  while (shm->system_running) {
    /* Tamamlanma semaforunda zamanlanmış bekleme (100ms) */
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    /* 100 milisaniye zaman aşımı ekle */
    ts.tv_nsec += 100000000L;
    if (ts.tv_nsec >= 1000000000L) {
      ts.tv_sec += 1;
      ts.tv_nsec -= 1000000000L;
    }
    sem_timedwait(&shm->completion_sem, &ts);

    /* Tüm kelimeler tamamlandı mı kontrol et */
    if (shm->total_completed >= shm->num_words) {
      /* Tüm iş bitti - sistemi durdur */
      shm->system_running = 0;
      break;
    }

    /* Her katı kontrol et: letter-carrier kalmamış mı? */
    for (int f = 0; f < shm->num_floors; f++) {
      pthread_mutex_lock(&shm->floor_mutex[f]);
      int count = shm->floors[f].letter_carrier_count;
      pthread_mutex_unlock(&shm->floor_mutex[f]);

      /* Eğer katta hiç letter-carrier yoksa yeni oluştur */
      if (count == 0) {
        /* Önce bu katta yapılacak iş var mı kontrol et */
        int has_work = 0;
        for (int w = 0; w < shm->num_words; w++) {
          if (shm->words[w].admitted && !shm->words[w].all_chars_picked &&
              shm->words[w].arrival_floor == f) {
            has_work = 1;
            break;
          }
        }
        if (has_work) {
          /* Başlangıçta belirtilen sayıda yeni letter-carrier oluştur */
          for (int l = 0; l < cfg.letter_carriers; l++) {
            create_letter_carrier(shm, f, lc_display_id);
            lc_display_id++;
          }
        }
      }
    }
  }

  /* SİSTEM KAPATMA AŞAMASI => Tüm süreçlere durma sinyali gönderilir ve beklenir */

  /* Tüm beklemeyi kaldır - tüm semaforlara bildirim gönder */
  sem_post(&shm->delivery_sem);
  sem_post(&shm->reposition_sem);
  sem_post(&shm->word_available_sem);
  for (int i = 0; i < shm->num_floors; i++) {
    /* Her kat için birden fazla post (birden fazla bekleyen olabilir) */
    for (int k = 0;
         k < (cfg.letter_carriers + cfg.sorting_procs + cfg.word_carriers) * 2;
         k++) {
      sem_post(&shm->letter_work_sem[i]);
      sem_post(&shm->sorting_work_sem[i]);
    }
    sem_post(&shm->word_available_sem);
  }
  /* Carrier semaforlarını uyandır */
  for (int i = 0; i < shm->total_carrier_count; i++) {
    if (shm->carriers[i].assigned) {
      sem_post(&shm->carriers[i].ready_sem);
    }
  }

  /* Tüm child süreçlerin sonlanmasını bekle */
  for (int i = 0; i < child_count; i++) {
    if (child_pids[i] > 0) {
      waitpid(child_pids[i], NULL, 0);
    }
  }

  /* Çıkış dosyasını oluştur */
  generate_output(shm, cfg.output_file);

  /* Özet bilgiyi yazdır */
  print_summary(shm);

  /* Senkronizasyon primitiflerini temizle */
  cleanup_sync(shm);

  /* Paylaşılan belleği serbest bırak */
  munmap(shm, sizeof(SharedData));

  return 0;
}
