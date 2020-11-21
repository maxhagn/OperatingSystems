# @file Makefile
# @author Maximilian Hagn <11808237@student.tuwien.ac.at>
# @date 21.11.2020
# @brief Makefile for MyCompress Program. Operations include all, mycompress and clean

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
