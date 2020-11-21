/**
 * @file supervisor.c
 * @author Maximilian Hagn <11808237@student.tuwien.ac.at>
 * @date 21.11.2020
 *
 * @brief Supervisor module. Reads continuously solutions
 * from the circular Buffer.
 *
 **/

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
#include "structs.h"

/**
 * indicates if supervisor should terminate
 **/
volatile sig_atomic_t quit = 0;

/**
 * Pointer to name of program
 **/
static char *program_name;

/**
 * printUsageError function.
 * @brief Usage of program is printed to stderr and program is exited with failure code
 * @details global variables: program_name, contains the name of the program
 **/
void printUsageError() {

    fprintf(stderr, "Usage: %s\n", program_name);
    exit(EXIT_FAILURE);

}

/**
 * handles SIGTERM and SIGINT signals.
 * @brief The program starts here. This function takes care about input arguments.
 * All Edges are added to the EdgeArray. The EdgeArray is than passed to the handleSolutions Function.
 * @param signal, the signal that should be handed
 * @details global variables: quit, indicated if program should exit
 **/
void handle_signal(int signal) {

    fprintf(stderr, "\nExiting due to %s\n", (signal == SIGTERM) ? "SIGTERM" : "SIGINT");
    quit = 1;

}


/**
 * Program entry point.
 * @brief The program starts here. This function takes care about parameters.
 * Then it created the shared memory and semaphores. After initialization the
 * program waits for solutions from the generators and handles the results.
 * global variables: program_name, quit
 * @param argc The argument counter.
 * @param argv The argument vector.
 **/
int main( int argc, char *argv[] ) {
    if (argc != 1) {
        printUsageError();
    }

    /* erzeugt ein shared memory object */
    int shmfd = shm_open("/shm", O_RDWR | O_CREAT | O_EXCL, 0600);
    if (shmfd == -1) {
        exit(EXIT_FAILURE);
    }

    /* change size to size of an shmobj struct */
    if (ftruncate(shmfd, sizeof(ShmObj)) < 0) {
        close(shmfd);
        shm_unlink("/shm");
        exit(EXIT_FAILURE);
    }

    ShmObj *shmObj;
    shmObj = mmap(NULL, sizeof(*shmObj), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    if (shmObj == MAP_FAILED) {
        close(shmfd);
        shm_unlink("/shm");
        exit(EXIT_FAILURE);
    }

    shmObj->writePos = 0;
    shmObj->readPos = 0;

    if (close(shmfd) == -1) {
        munmap(shmObj, sizeof(*shmObj));
        shm_unlink("/shm");
        exit(EXIT_FAILURE);
    }

    sem_unlink("/s_free");
    sem_unlink("/s_used");
    sem_unlink("/s_write");
    sem_t *s_free = sem_open("/s_free", O_CREAT | O_EXCL, 0600, 50);
    if (s_free == SEM_FAILED) {
        munmap(shmObj, sizeof(*shmObj));
        shm_unlink("/shm");
        exit(EXIT_FAILURE);
    }

    sem_t *s_used = sem_open("/s_used", O_CREAT | O_EXCL, 0600, 0);
    if (s_used == SEM_FAILED) {
        munmap(shmObj, sizeof(*shmObj));
        shm_unlink("/shm");
        sem_close(s_free);
        sem_unlink("/s_free");
        exit(EXIT_FAILURE);
    }

    sem_t *s_write = sem_open("/s_write", O_CREAT | O_EXCL, 0600, 1);
    if (s_write == SEM_FAILED) {
        munmap(shmObj, sizeof(*shmObj));
        shm_unlink("/shm");
        sem_close(s_free);
        sem_unlink("/s_free");
        sem_close(s_used);
        sem_unlink("/s_used");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa = {
            .sa_handler = &handle_signal
    };

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    int exit_status = EXIT_SUCCESS;

    int bestSolutionCount = 100000;
    Edge bestSolution[8];
    while (!quit) {

        if (sem_wait(s_used) == -1) {
            if (errno == EINTR) {
                continue;
            }
            exit_status = EXIT_FAILURE;
            break;
        }

        int edgeCounter = 0;
        for (int i = 0; i < 8; i++) {
            if (!(shmObj->data[shmObj->readPos][i].from == -1
               || shmObj->data[shmObj->readPos][i].to == -1)) {
                edgeCounter++;
            }
        }

        if (edgeCounter == 0) {
            fprintf(stdout, "The graph is 3-colorable \n");
            break;
        }

        else if (edgeCounter < bestSolutionCount) {
            bestSolutionCount = edgeCounter;
            fprintf(stdout, "Solution with %d edge(s):", edgeCounter);

            for (int i = 0; i < 8; i++) {
                if (!(shmObj->data[shmObj->readPos][i].from == -1 ||
                      shmObj->data[shmObj->readPos][i].to == -1)) {

                    bestSolution[i] = shmObj->data[shmObj->readPos][i];
                    fprintf(stdout, " %d-%d", bestSolution[i].from, bestSolution[i].to);

                }
            }

            fprintf(stdout, "\n");

        }

        shmObj->readPos = (shmObj->readPos + 1) % 50;
        sem_post(s_free);
    }

    // terminate and unmap operations
    shmObj->terminate = 1;

    if (sem_close(s_free) == -1) {
        exit_status = EXIT_FAILURE;
    }
    if (sem_close(s_used) == -1) {
        exit_status = EXIT_FAILURE;
    }
    if (sem_close(s_write) == -1) {
        exit_status = EXIT_FAILURE;
    }

    if (sem_unlink("/s_free") == -1) {
        exit_status = EXIT_FAILURE;
    }
    if (sem_unlink("/s_used") == -1) {
        exit_status = EXIT_FAILURE;
    }
    if (sem_unlink("/s_write") == -1) {
        exit_status = EXIT_FAILURE;
    }

    if (munmap(shmObj, sizeof(*shmObj)) == -1) {
        exit_status = EXIT_FAILURE;
    }

    if (shm_unlink("/shm") == -1) {
        exit_status = EXIT_FAILURE;
    }

    exit(exit_status);

}
