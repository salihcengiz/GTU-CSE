# CSE344 - Sistem Programlama - Ödev 3
# Makefile: Sadece derleme yapar, çalıştırmaz (ödev kuralı)

CC = gcc
CFLAGS = -Wall -pedantic -std=c11 -D_GNU_SOURCE
LDFLAGS = -lrt -lpthread

TARGET = hw3

SRCS = hw3.c word_carrier.c letter_carrier.c sorting.c elevator.c
OBJS = $(SRCS:.c=.o)

# Varsayılan hedef: tüm kaynak dosyaları derle ve bağla
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c common.h
	$(CC) $(CFLAGS) -c $< -o $@

# Temizlik: derlenmiş dosyaları sil
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean