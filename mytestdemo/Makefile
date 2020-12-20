CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g

.PHONY: all clean
all: mytest

mytest: mytest.o
	$(CC) -o mytest mytest.o

mytest.o: mytest.c
	$(CC) $(CFLAGS) $(DEFS) -c mytest.c

clean:
	rm -rf mytest mytest.o
