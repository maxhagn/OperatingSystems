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
 * @brief the program prints the signal and sets the global var quit to 1,
 * so the program can terminate.
 * @param signal, the signal that should be handed
 * @details global variables: quit, program_name
 **/
void handle_signal(int signal) {

    if ( signal == SIGTERM ) {
        fprintf(stderr, "\n%s Exiting due to signal SIGTERM\n", program_name);
    } else {
        fprintf(stderr, "\n%s Exiting due to signal SIGINT\n", program_name);
    }

    quit = 1;

}


/**
 * Program entry point.
 * @brief The program starts here. This function takes care about parameters.
 * Then it creates the shared memory and semaphores. After initialization the
 * program waits for solutions from the generators and handles the results.
 * global variables: program_name, quit
 * @param argc The argument counter.
 * @param argv The argument vector.
 **/
int main( int argc, char *argv[] ) {
    program_name = argv[0];

    if (argc != 1) {
        printUsageError();
    }

    int shmfd = shm_open(SHM, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (shmfd == -1) {
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shmfd, sizeof(ShmObj)) < 0) {
        close(shmfd);
        shm_unlink(SHM);
        exit(EXIT_FAILURE);
    }

    ShmObj *shm_obj;
    shm_obj = mmap(NULL, sizeof(*shm_obj), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    if (shm_obj == MAP_FAILED) {
        close(shmfd);
        shm_unlink(SHM);
        exit(EXIT_FAILURE);
    }

    shm_obj->writePos = 0;
    shm_obj->readPos = 0;

    if (close(shmfd) == -1) {
        munmap(shm_obj, sizeof(*shm_obj));
        shm_unlink(SHM);
        exit(EXIT_FAILURE);
    }

    sem_unlink(SEM_FREE);
    sem_unlink(SEM_USED);
    sem_unlink(SEM_WRITE);
    sem_t *s_free = sem_open(SEM_FREE, O_CREAT | O_EXCL, 0600, 50);
    if (s_free == SEM_FAILED) {
        munmap(shm_obj, sizeof(*shm_obj));
        shm_unlink(SHM);
        exit(EXIT_FAILURE);
    }

    sem_t *s_used = sem_open(SEM_USED, O_CREAT | O_EXCL, 0600, 0);
    if (s_used == SEM_FAILED) {
        munmap(shm_obj, sizeof(*shm_obj));
        shm_unlink(SHM);
        sem_close(s_free);
        sem_unlink(SEM_FREE);
        exit(EXIT_FAILURE);
    }

    sem_t *s_write = sem_open(SEM_WRITE, O_CREAT | O_EXCL, 0600, 1);
    if (s_write == SEM_FAILED) {
        munmap(shm_obj, sizeof(*shm_obj));
        shm_unlink(SHM);
        sem_close(s_free);
        sem_unlink(SEM_FREE);
        sem_close(s_used);
        sem_unlink(SEM_USED);
        exit(EXIT_FAILURE);
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    int exit_code = EXIT_SUCCESS;

    int best_solution_length = 100000;
    Edge best_solution[8];
    while (!quit) {

        if (sem_wait(s_used) == -1) {
            if (errno == EINTR) {
                continue;
            }
            exit_code = EXIT_FAILURE;
            break;
        }

        int edge_counter = 0;
        for (int i = 0; i < 8; i++) {
            if (!(shm_obj->data[shm_obj->readPos][i].from == -1
               || shm_obj->data[shm_obj->readPos][i].to == -1)) {
                edge_counter++;
            }
        }

        if (edge_counter == 0) {
            fprintf(stdout, "The graph is 3-colorable!\n");
            break;
        }

        else if (edge_counter < best_solution_length) {
            best_solution_length = edge_counter;
            fprintf(stdout, "Solution with %d edge(s):", edge_counter);

            for (int i = 0; i < 8; i++) {
                if (!(shm_obj->data[shm_obj->readPos][i].from == -1 ||
                      shm_obj->data[shm_obj->readPos][i].to == -1)) {

                    best_solution[i] = shm_obj->data[shm_obj->readPos][i];
                    fprintf(stdout, " %d-%d", best_solution[i].from, best_solution[i].to);

                }
            }

            fprintf(stdout, "\n");
        }



        shm_obj->readPos = (shm_obj->readPos + 1) % 50;
        sem_post(s_free);
    }

    shm_obj->terminate = 1;

    if (sem_close(s_free) == -1) {
        exit_code = EXIT_FAILURE;
    }
    if (sem_close(s_used) == -1) {
        exit_code = EXIT_FAILURE;
    }
    if (sem_close(s_write) == -1) {
        exit_code = EXIT_FAILURE;
    }

    if (sem_unlink(SEM_FREE) == -1) {
        exit_code = EXIT_FAILURE;
    }
    if (sem_unlink(SEM_USED) == -1) {
        exit_code = EXIT_FAILURE;
    }
    if (sem_unlink(SEM_WRITE) == -1) {
        exit_code = EXIT_FAILURE;
    }

    if (munmap(shm_obj, sizeof(*shm_obj)) == -1) {
        exit_code = EXIT_FAILURE;
    }

    if (shm_unlink(SHM) == -1) {
        exit_code = EXIT_FAILURE;
    }

    exit(exit_code);

}
