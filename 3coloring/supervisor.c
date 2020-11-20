#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>

#include <unistd.h>
#include <sys/types.h>

#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <errno.h>

#include "generator.h"
#include "shared.h"


void printUsageError() {

    fprintf(stderr, "Usage: supervisor\n");
    exit(EXIT_FAILURE);

}

int main( int argc, char *argv[] ) {

    if (argc != 1) {
        printUsageError();
    }

}