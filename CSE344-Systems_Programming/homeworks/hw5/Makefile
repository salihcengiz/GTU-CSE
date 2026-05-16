CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -pthread -O2
LDFLAGS = -pthread

TARGET  = cargoGTU
OBJS    = main.o parser.o priority_queue.o courier.o signal_thread.o stats.o log.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

main.o: main.c cargo.h parser.h priority_queue.h courier.h signal_thread.h stats.h
	$(CC) $(CFLAGS) -c $< -o $@

parser.o: parser.c parser.h cargo.h
	$(CC) $(CFLAGS) -c $< -o $@

priority_queue.o: priority_queue.c priority_queue.h cargo.h
	$(CC) $(CFLAGS) -c $< -o $@

courier.o: courier.c courier.h cargo.h priority_queue.h
	$(CC) $(CFLAGS) -c $< -o $@

signal_thread.o: signal_thread.c signal_thread.h cargo.h priority_queue.h
	$(CC) $(CFLAGS) -c $< -o $@

stats.o: stats.c stats.h cargo.h
	$(CC) $(CFLAGS) -c $< -o $@

log.o: log.c cargo.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
