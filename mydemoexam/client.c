#include "client.h"

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"

/* Given */
char *program_name = "<not yet set>";
char *shmp = MAP_FAILED;
int shmfd;
sem_t *sem_request = SEM_FAILED;
sem_t *sem_response = SEM_FAILED;
sem_t *sem_client = SEM_FAILED;
char SHM_NAME[256] = "/osue_shm_";
char SEM_NAME_REQUEST[256] = "/osue_request_";
char SEM_NAME_CLIENT[256] = "/osue_client_";
char SEM_NAME_RESPONSE[256] = "/osue_response_";
void initialize_names(void) {
    strcat(SHM_NAME, getlogin());
    strcat(SEM_NAME_REQUEST, getlogin());
    strcat(SEM_NAME_CLIENT, getlogin());
    strcat(SEM_NAME_RESPONSE, getlogin());
}

void parse_arguments(int argc, char *argv[], args_t *args) {
    program_name = argv[0];
    int p_flag = 0;
    int current_option;

    while ((current_option = getopt(argc, argv, "p:")) != -1) {
        switch (current_option) {

            case 'p':

                if (p_flag) {
                    usage("./client -p Password");
                    exit(EXIT_FAILURE);
                } else {
                    args->password = optarg;
                    p_flag++;
                }



                break;

            case '?':
                usage("./client -p Password");
                exit(EXIT_FAILURE);
        }
    }

    if( p_flag == 0 || argc > 3 ) {
        usage("./client -p Password");
        exit(EXIT_FAILURE);
    }

}

void allocate_resources(void) {
    shmfd = shm_open(SHM_NAME, O_RDWR, PERMISSIONS);

    if (shmfd == -1) {
        error_exit("shm open failed");
    }

    shmp = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    sem_request = sem_open(SEM_NAME_REQUEST, 0);
    sem_response = sem_open(SEM_NAME_RESPONSE, 0);
    sem_client = sem_open(SEM_NAME_CLIENT, 0);

    if (sem_request == SEM_FAILED || sem_response == SEM_FAILED || sem_client == SEM_FAILED || shmp == MAP_FAILED) {
        close(shmfd);
        shm_unlink(SHM_NAME);
        munmap(shmp, SHM_SIZE);
        shm_unlink(SEM_NAME_REQUEST);
        shm_unlink(SEM_NAME_RESPONSE);
        shm_unlink(SEM_NAME_CLIENT);
        error_exit("initialization failed");
    }

}

void process_password(const char *password, char *hash) {
    while (1) {
        if (sem_wait(sem_client) == -1) {
            if (errno == EINTR) {
                continue;
            }
            error_exit("error waiting for sem client");
            break;
        }

        strcpy(shmp, password);
        sem_post(sem_request);

        if (sem_wait(sem_response) == -1) {
            if (errno == EINTR) {
                continue;
            }
           error_exit("error waiting for sem client");
            break;
        }

        strcpy(hash, shmp);
        sem_post(sem_client);
        break;

    }
}

/* Given */
int main(int argc, char *argv[]) {
    args_t args;
    program_name = argv[0];
    char hash[SHM_SIZE] = {0};
    initialize_names();
    parse_arguments(argc, argv, &args);
    allocate_resources();
    process_password(args.password, hash);
    printf("Hash: %s\n", hash);
    print_message("detach shared memory");
    free_resources();
    return 0;
}
