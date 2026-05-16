# HW6 Test Senaryoları

Bu klasör PDF'teki **4 zorunlu senaryoyu** ve PDF'te bahsedilen tüm protokol durumlarını test eden scriptleri içerir.

## Kullanım

Tüm scriptler `/work` (yani proje kök) dizininden çalışacak şekilde yazıldı.

Docker container'ı başlat:
```sh
./docker-run.sh
docker exec -it hw6 bash
cd /work
make           # ilk kez derle
```

### Her senaryo için adımlar
Her senaryonun kendi klasöründe **numaralı scriptler** var. Numaralandırma açma sırasını verir:
- `00_server.sh`  → 1. terminalde çalıştır
- `01_*.sh`, `02_*.sh`, ... → her biri için yeni bir `docker exec -it hw6 bash` terminali aç ve çalıştır

Her client scripti, PDF'te o terminal için listelenen komutları belirli aralıklarla otomatik gönderir, **sonunda `cat` ile bekler**. PDF'in dediği gibi sen Ctrl+C bastığında client `APPARATE` gönderip temiz çıkar — screenshot için ideal.

`SLEEP` değişkeniyle komutlar arası bekleme süresini ayarlayabilirsin (varsayılan: 1 saniye):
```sh
SLEEP=2 ./tests/scenario1/02_wizard_hermione.sh
```

## Senaryo özetleri

| Senaryo | -n | -t | Test ettiği şey |
|---|---|---|---|
| 1 | 4 | 30s | name_taken, NOT_ENROLLED, UNKNOWN command, UNAUTHORIZED (her iki yön), client Ctrl+C |
| 2 | 4 | 30s | Spellbook (BREW + CONSUME birikme/azalma), UNKNOWN_INGREDIENT, INSUFFICIENT_INGREDIENTS, INSPECT/SCROLL/ROSTER |
| 3 | 3 | 60s | HOGWARTS_FULL (kapasite aşımı), slot recovery (Ctrl+C → yer açılır → yeni client kabul) |
| 4 | 6 | 10s | Idle timeout (10s aktivitesizlikten sonra TIMEOUT DISCONNECT), eş zamanlı çoklu client, sunucu Ctrl+C → SERVER SHUTDOWN |

## Edge case scriptleri (`edge/`)
PDF'teki kalan protokol durumları:
- `edge/01_toolong.sh`            → ERR TOOLONG (>512 bayt satır)
- `edge/02_partial_read.sh`       → partial-read line buffer (bir satırı byte-byte gönder)
- `edge/03_unknown_ingredient.sh` → ERR UNKNOWN_INGREDIENT (BREW BUTTERBEER vs.)
- `edge/04_insufficient.sh`       → ERR INSUFFICIENT_INGREDIENTS
- `edge/05_hangup.sh`             → client TCP hangup (RST/FIN) — sunucu temiz disconnect
- `edge/06_signal_shutdown.sh`    → server Ctrl+C → connected client'lar SERVER SHUTDOWN alır
- `edge/run_all_edge.sh`          → tüm edge case'leri tek scriptte sırayla çalıştırır (otomatik)

## Önerilen screenshot sırası

Her senaryo için **uncut** screenshot:
1. Server terminalini ayrı pencere açtıktan sonra `00_server.sh` çalıştır.
2. Diğer terminalleri açıp sıra ile script numarasına göre başlat.
3. Her script kendi komutlarını gönderir, sonra `cat` ile bekler.
4. PDF'in son adımı **Ctrl+C** olan client'larda **sen** Ctrl+C bas; APPARATE → OK APPARATE → DISCONNECTED satırlarını gör.
5. Server terminalinde Ctrl+C bas; SHUTDOWN ve CLEANUP_DONE log satırlarını gör.
6. En son `cat server.log` ile log dosyasını da ekrana yansıt (screenshot 4. senaryoda zorunlu).
