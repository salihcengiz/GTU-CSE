# HW6 - Magic Academy - Test Paketi

Bu zip, CSE344 HW6 ödevini PDF'teki 4 zorunlu senaryo + ek protokol
durumlarına karşı test eden scriptleri içerir.

## İçerik

    tests/            Test scriptleri (lib.sh + scenario1-4 + edge)
    ingredients.txt   Testlerin beklediği ingredient seti (GEREKLI)
    docker/           Debian 12 ortamı (OPSIYONEL - aşağıya bakın)

## Kurulum

1. Bu zip'in içindeki `tests/` klasörünü ve `ingredients.txt` dosyasını
   kendi proje KÖK dizininize kopyalayın (hogwarts.c'nin yanına).

2. Projeniz şu binary isimlerini üretmeli:  `hogwarts`, `wizard`, `professor`

3. Çıplak Debian 12'de bir kez:
       sudo apt install -y netcat-openbsd procps

4. Derleyin:
       make

## Çalıştırma

Her senaryo birden fazla terminal gerektirir. Numaralar açma sırasıdır:

    # Terminal 1 (server):
    ./tests/scenario1/00_server.sh
    # Terminal 2, 3, ... (her biri yeni terminal):
    ./tests/scenario1/01_wizard_hermione.sh
    ./tests/scenario1/02_professor_dumbledore.sh
    ...

Detaylı senaryo açıklamaları için `tests/README.md` dosyasına bakın.

Edge case'lerin otomatik toplu çalıştırması:
    ./tests/edge/run_all_edge.sh

## Docker (opsiyonel)

macOS/Windows kullanıyorsanız veya temiz bir Debian 12 ortamı istiyorsanız:

1. `docker/Dockerfile` ve `docker/docker-run.sh` dosyalarını proje KÖK
   dizinine taşıyın (Dockerfile `COPY . /work` yaptığı için kök gerekir).
2. `./docker-run.sh` çalıştırın.
3. `docker exec -it hw6 bash` ile container'a girin, `cd /work`, `make`.

netcat-openbsd ve procps container imajında zaten kuruludur.
