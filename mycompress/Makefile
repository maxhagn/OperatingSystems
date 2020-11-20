CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g

.PHONY: all clean
all: mycompress

mycompress: mycompress.o
	$(CC) -o mycompress mycompress.o

mycompress.o: mycompress.c
	$(CC) $(CFLAGS) $(DEFS) -c mycompress.c

clean:
	rm -rf mycompress mycompress.o
