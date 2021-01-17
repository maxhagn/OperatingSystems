# @file Makefile
# @author Maximilian Hagn <11808237@student.tuwien.ac.at>
# @date 21.11.2020
# @brief Makefile for MyCompress Program. Operations include all, mycompress and clean

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g

.PHONY: all clean
all: client

client: client.o
	$(CC) -o client client.o

client.o: client.c
	$(CC) $(CFLAGS) $(DEFS) -c client.c

clean:
	rm -rf client client.o
