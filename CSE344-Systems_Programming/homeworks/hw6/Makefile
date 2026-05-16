CC = gcc
CFLAGS = -Wall -Wextra -std=c11

all: hogwarts wizard professor

hogwarts: hogwarts.c
	$(CC) $(CFLAGS) -o hogwarts hogwarts.c

wizard: wizard.c
	$(CC) $(CFLAGS) -o wizard wizard.c

professor: professor.c
	$(CC) $(CFLAGS) -o professor professor.c

clean:
	rm -f hogwarts wizard professor *.o
