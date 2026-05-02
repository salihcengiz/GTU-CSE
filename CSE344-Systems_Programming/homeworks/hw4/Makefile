CC       ?= gcc
CSTD      = -std=gnu11
WARN      = -Wall -Wextra -Wshadow -Wpointer-arith -Wwrite-strings
OPT       = -O2
DBG       = -g
PTHREAD   = -pthread

CFLAGS   ?= $(CSTD) $(WARN) $(OPT) $(DBG) $(PTHREAD) -D_GNU_SOURCE -D_POSIX_C_SOURCE=200809L
LDFLAGS  ?= $(PTHREAD)

LDLIBS   ?=

TARGET   = analyzer

SRCS = main.c shm.c reader.c dispatcher.c analyzer.c aggregator.c watchdog.c
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

.PHONY: all clean debug tsan valgrind help

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)

# .c -> .o derleme kuralı; -MMD ile header bağımlılıklarını üretir.
%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

debug: CFLAGS := $(CSTD) $(WARN) -O0 $(DBG) $(PTHREAD) -D_GNU_SOURCE -D_POSIX_C_SOURCE=200809L
debug: clean $(TARGET)

tsan: CFLAGS := $(CSTD) $(WARN) -O1 $(DBG) $(PTHREAD) -D_GNU_SOURCE -D_POSIX_C_SOURCE=200809L -fsanitize=thread -fno-omit-frame-pointer
tsan: LDFLAGS := $(PTHREAD) -fsanitize=thread
tsan: clean $(TARGET)

valgrind:
	@echo "Example: make && valgrind --leak-check=full --error-exitcode=1 \\"
	@echo "         ./$(TARGET) -c tests/basic.conf -f tests/filter.txt \\"
	@echo "         -k error -t 2 -w 2 -a 16 -b 16 -d 8 -o out.txt -O out.bin"

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET) *.o *.d out.txt out.bin out.bin.tmp

help:
	@echo "Targets:"
	@echo "  all      - release build (default)"
	@echo "  debug    - -O0 -g, sembol + minimum optimize"
	@echo "  tsan     - ThreadSanitizer ile derle (race detection)"
	@echo "  valgrind - komut örneğini yazdırır"
	@echo "  clean    - üretilen dosyaları sil"

-include $(DEPS)
