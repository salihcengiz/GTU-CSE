FROM debian:12-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential gcc make valgrind netcat-openbsd procps \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work
COPY . /work

RUN make clean && make

CMD ["/bin/bash"]
