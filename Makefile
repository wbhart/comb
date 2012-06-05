CC=gcc
CFLAGS=-O2

all: combinator.o calc.c
	$(CC) $(CFLAGS) calc.c combinator.o -o calc

combinator.o: combinator.c combinator.h
	$(CC) $(CFLAGS) -c combinator.c -o combinator.o

clean:
	rm -f combinator.o calc
