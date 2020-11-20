CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g

.PHONY: all clean
all: supervisor generator

generator: generator.o circularBuffer.o
	$(CC) -o generator generator.o circularBuffer.o

generator.o: generator.c
	$(CC) $(CFLAGS) $(DEFS) -c generator.c

supervisor: supervisor.o circularBuffer.o
	$(CC) -o generator supervisor.o circularBuffer.o

supervisor.o: supervisor.c
	$(CC) $(CFLAGS) $(DEFS) -c supervisor.c

circularBuffer.o: circularBuffer.c
	$(CC) $(CFLAGS) $(DEFS) -c circularBuffer.c

clean:
	rm -rf *.o
	rm -rf supervisor
	rm -rf generator
