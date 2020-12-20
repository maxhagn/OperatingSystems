##
# Makefile for client.
#


# config

DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809 -D_XOPEN_SOURCE
override CFLAGS += -Wall -g -std=c99 -pedantic $(DEFS)
override LDFLAGS +=
override LIBS += 

# objects to build
OBJS = client.o


# rules

.PHONY : all clean

all: client

client: $(OBJS) $(LDOBJS)
	gcc $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f client $(OBJS)


# dependencies

client.o: client.c
