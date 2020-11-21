/**
 * @file generator.c
 * @author Maximilian Hagn <11808237@student.tuwien.ac.at>
 * @date 21.11.2020
 *
 * @brief Generator module. Pushes continuously solutions
 * of the 3 color problem to the circular buffer.
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include "structs.h"

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

    fprintf(stderr, "Usage: %s EDGE1...\n", program_name);
    exit(EXIT_FAILURE);

}

/**
 * Returns Minimum
 * @brief returns the smaller number from params
 * @param a - first integer
 * @param b - second integer
 **/
int getMinimum( int a, int b ) {
    if ( a < b ) { return a; }
    else { return b; }
}

/**
 * Adds new edge to the array
 * @brief reallocate memory, adds new edge to edge array and updates length of array
 * @param *edges - pointer to the edges array
 * @param *edge - pointer to the new edge
 * @return integer 1 if successful, integer -1 if failure
 **/
static int addToArray(EdgeArray *edges, Edge *edge) {
    Edge **new_data = realloc(edges->content, (edges->length + 1) * sizeof(Edge));

    if (new_data == NULL) {
        return -1;
    }

    edges->length = (edges->length) + 1;
    edges->content = new_data;
    (edges->content)[(edges->length) - 1] = edge;

    return 1;
}

/**
 * generates a new solution
 * @brief adds a random coloring to the vertexes and deletes edges between vertexes with the same color
 * @param *edges - pointer to the given edges
 * @return Edge Array, array that holds the new solution
 **/
static EdgeArray generate(EdgeArray *edges) {
    int nodeColoring[edges->length];
    for (int i = 0; i < edges->length; i++) {
        int num = (rand() %(3)) + 1;
        nodeColoring[i] = num;
    }

    EdgeArray *returnEdges = (EdgeArray *) malloc(sizeof(EdgeArray));
    returnEdges->content = (Edge **) malloc(sizeof(Edge));
    returnEdges->length = 0;

    int NumberColorConflicts = 0;
    for (int i = 0;  i < edges->length;  i++) {

        if(nodeColoring[edges->content[i]->from] == nodeColoring[edges->content[i]->to]){
            if ( NumberColorConflicts < 8 ) {
                addToArray(returnEdges, edges->content[i]);
            }
            NumberColorConflicts++;
        }
    }

    return *returnEdges;
}

/**
 * handles new solutions
 * @brief opens all semaphores and shared memory, calls generate and adds solution to circular buffer
 * @param *edgeArray - pointer to given edges, forwarded to generate methode
 * @return integer 1 if success, integer -1 if failure
 **/
static int handleSolutions(EdgeArray *edgeArray) {

    int openErrCode = 1;
    int shmfd = shm_open("/shm", O_RDWR, 0600);
    if (shmfd == -1) { openErrCode = -1; }

    ShmObj *shmObj;
    shmObj = mmap(NULL, sizeof(*shmObj), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    if (shmObj == MAP_FAILED) {
        close(shmfd);
        shm_unlink("/shm");
        openErrCode = -1;
    }

    if (close(shmfd) == -1) {
        munmap(shmObj, sizeof(*shmObj));
        shm_unlink("/shm");
        openErrCode = -1;
    }

    sem_t *s_free = sem_open("/s_free", 0);
    if (s_free == SEM_FAILED) {
        munmap(shmObj, sizeof(*shmObj));
        shm_unlink("/shm");
        openErrCode = -1;
    }

    sem_t *s_used = sem_open("/s_used", 0);
    if (s_used == SEM_FAILED) {
        munmap(shmObj, sizeof(*shmObj));
        shm_unlink("/shm");
        sem_close(s_free);
        sem_unlink("/s_free");
        openErrCode = -1;
    }

    sem_t *s_write = sem_open("/s_write", 0);
    if (s_write == SEM_FAILED) {
        munmap(shmObj, sizeof(*shmObj));
        shm_unlink("/shm");
        sem_close(s_free);
        sem_unlink("/s_free");
        sem_close(s_used);
        sem_unlink("/s_used");
        openErrCode = -1;
    }

    if ( openErrCode != 1 ) {
        fprintf(stderr, "%s: Couldn't open semaphores or shared memory.\n", program_name);
        return openErrCode;
    }

    int exit_status = 1;

    while (1) {
        if (sem_wait(s_write) == -1) {
            if (errno == EINTR) {
                continue;
            }
            exit_status = -1;
            break;
        }

        if (sem_wait(s_free) == -1) {
            if (errno == EINTR) {
                continue;
            }
            exit_status = -1;
            break;
        }

        if (shmObj->terminate == 1) {
            fprintf(stderr, "%s: Terminating\n", program_name);
            sem_post(s_write);
            sem_post(s_free);
            break;
        }
        EdgeArray newSolution = generate(edgeArray);


        if (newSolution.length <= 8) {
            int solutionSize = getMinimum(8, newSolution.length);
            for (int i = 0; i < getMinimum(8, newSolution.length); i++) {
                shmObj->data[shmObj->writePos][i] = *(newSolution.content[i]);
            }

            for (int i = solutionSize; i < 8; i++) {
                Edge newItem;
                newItem.from = -1;
                newItem.to = -1;
                shmObj->data[shmObj->writePos][i] = newItem;
            }
        }

        shmObj->writePos = (shmObj->writePos + 1) % 50;
        sem_post(s_used);
        sem_post(s_write);
    }

    if (sem_close(s_free) == -1) {
        exit_status = -1;
    }
    if (sem_close(s_used) == -1) {
        exit_status = -1;
    }
    if (sem_close(s_write) == -1) {
        exit_status = -1;
    }

    if (sem_unlink("/s_free") == -1) {
        exit_status = -1;
    }
    if (sem_unlink("/s_used") == -1) {
        exit_status = -1;
    }
    if (sem_unlink("/s_write") == -1) {
        exit_status = -1;
    }

    if (munmap(shmObj, sizeof(*shmObj)) == -1) {
        exit_status = -1;
    }

    if (shm_unlink("/shm") == -1) {
        exit_status = -1;
    }

    return exit_status;

}

/**
 * Program entry point.
 * @brief The program starts here. This function takes care about input arguments.
 * All Edges are added to the EdgeArray. The EdgeArray is than passed to the handleSolutions Function.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS or EXIT_FAILURE
 **/
int main(int argc, char *argv[]) {
    program_name = argv[0];
    if (argc <= 1) { printUsageError(); }

    EdgeArray *edgeArray = (EdgeArray *) malloc(sizeof(EdgeArray));
    edgeArray->content = (Edge **) malloc(sizeof(Edge));
    edgeArray->length = 0;

    int current_arg;
    for (current_arg = 1; current_arg < argc; current_arg++) {
        Edge *new_edge = (Edge *) malloc(sizeof(Edge));

        int tmp_from, tmp_to, sscanf_err;
        sscanf_err = sscanf(argv[current_arg], "%d-%d", &tmp_from, &tmp_to);

        if (sscanf_err == 0) {
            fprintf (stderr, "%s: Given Edge couldn't be parsed.\n", program_name);
            free(edgeArray);
            exit(EXIT_FAILURE);
        }


        new_edge->from = tmp_from;
        new_edge->to = tmp_to;


        if (addToArray(edgeArray, new_edge) == -1) {
            free(new_edge);
            free(edgeArray);
            exit(EXIT_FAILURE);
        }
    }

    if (handleSolutions(edgeArray) == -1) {
        free(edgeArray);
        exit(EXIT_FAILURE);
    }

    free(edgeArray);
    exit(EXIT_SUCCESS);
}
