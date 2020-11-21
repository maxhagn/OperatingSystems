# @file Makefile
# @author Maximilian Hagn <11808237@student.tuwien.ac.at>
# @date 21.11.2020
# @brief Makefile for 3color Program. Operations include all, generator, supervisor and clean

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g

.PHONY: all clean
all: supervisor generator

generator: generator.o
	$(CC) -o generator generator.o

supervisor: supervisor.o
	$(CC) -o supervisor supervisor.o

generator.o: generator.c
	$(CC) $(CFLAGS) $(DEFS) -c generator.c



supervisor.o: supervisor.c
	$(CC) $(CFLAGS) $(DEFS) -c supervisor.c

clean:
	rm -rf *.o
	rm -rf supervisor
	rm -rf generator
