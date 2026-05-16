#!/usr/bin/env bash
# Build image and start container for HW6.
set -e

IMG=hw6-hogwarts
NAME=hw6

echo "[*] Building image: $IMG"
docker build -t "$IMG" .

echo "[*] Removing old container (if any)"
docker rm -f "$NAME" 2>/dev/null || true

echo "[*] Starting container: $NAME"
docker run -dit --name "$NAME" -p 5000:5000 "$IMG"

cat <<EOF

[*] Ready.

Open a server terminal:
    docker exec -it $NAME bash
    ./hogwarts -p 5000 -s ingredients.txt -l server.log -n 4 -t 30

Open a wizard terminal (new window):
    docker exec -it $NAME bash
    ./wizard 127.0.0.1 5000 hermione

Open a professor terminal:
    docker exec -it $NAME bash
    ./professor 127.0.0.1 5000 dumbledore

Stop & clean:
    docker rm -f $NAME

EOF
