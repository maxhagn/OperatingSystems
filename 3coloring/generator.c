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
#include "generator.h"

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
static int addToArray(EdgeArray *edges, Edge edge) {
    Edge *new_array_length = realloc(edges->content, (edges->length + 1) * sizeof(Edge));

    if (new_array_length == NULL) {
        return -1;
    }

    edges->length = (edges->length) + 1;
    edges->content = new_array_length;
    (edges->content)[(edges->length) - 1] = edge;

    return 1;
}

/**
 * generates a new solution and adds the edges to the circular buffer
 * @brief adds a random coloring to the vertexes and deletes edges between vertexes with the same color.
 * As long as the solution is smaller than eight, the edges are added to the circular buffer.
 * @param *edges - pointer to the given edges
 * @param *shm_obj - pointer to the mapped shared memory object
 * @return Edge Array, array that holds the new solution
 **/
static int generate(EdgeArray *edges, ShmObj *shm_obj) {
    int colored_nodes[edges->length];
    for (int i = 0; i < edges->length; i++) {
        int num = (rand() %(3)) + 1;
        colored_nodes[i] = num;
    }

    EdgeArray *solution_edges = (EdgeArray *) malloc(sizeof(EdgeArray));
    solution_edges->content = (Edge *) malloc(sizeof(Edge));
    solution_edges->length = 0;

    int NumberColorConflicts = 0;
    for (int i = 0;  i < edges->length;  i++) {

        if(colored_nodes[edges->content[i].from] == colored_nodes[edges->content[i].to]){
            if ( NumberColorConflicts < 8 ) {
                addToArray(solution_edges, edges->content[i]);
            }
            NumberColorConflicts++;
        }
    }

    if (solution_edges->length <= 8) {
        int solution_length = getMinimum(8, solution_edges->length);
        for (int i = 0; i < getMinimum(8, solution_edges->length); i++) {
            shm_obj->data[shm_obj->writePos][i] = solution_edges->content[i];
        }

        for (int i = solution_length; i < 8; i++) {
            Edge new_edge;

            new_edge.from = -1;
            new_edge.to = -1;

            shm_obj->data[shm_obj->writePos][i] = new_edge;
        }
    }

    free(solution_edges->content);
    free(solution_edges);
    return 1;
}

/**
 * handles new solutions
 * @brief opens all semaphores and shared memory, calls generate and adds solution to circular buffer
 * @param edge_array - pointer to given edges, forwarded to generate methode
 * @return integer 1 if success, integer -1 if failure
 **/
static int handleSolutions(EdgeArray *edge_array) {

    int openErrCode = 1;
    int shmfd = shm_open(SHM, O_RDWR, 0600);
    if (shmfd == -1) { openErrCode = -1; }

    ShmObj *shm_obj;
    shm_obj = mmap(NULL, sizeof(*shm_obj), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    if (shm_obj == MAP_FAILED) {
        close(shmfd);
        shm_unlink(SHM);
        openErrCode = -1;
    }

    if (close(shmfd) == -1) {
        munmap(shm_obj, sizeof(*shm_obj));
        shm_unlink(SHM);
        openErrCode = -1;
    }

    sem_t *s_free = sem_open(SEM_FREE, 0);
    if (s_free == SEM_FAILED) {
        munmap(shm_obj, sizeof(*shm_obj));
        shm_unlink(SHM);
        openErrCode = -1;
    }

    sem_t *s_used = sem_open(SEM_USED, 0);
    if (s_used == SEM_FAILED) {
        munmap(shm_obj, sizeof(*shm_obj));
        shm_unlink(SHM);
        sem_close(s_free);
        sem_unlink(SEM_FREE);
        openErrCode = -1;
    }

    sem_t *s_write = sem_open(SEM_WRITE, 0);
    if (s_write == SEM_FAILED) {
        munmap(shm_obj, sizeof(*shm_obj));
        shm_unlink(SHM);
        sem_close(s_free);
        sem_unlink(SEM_FREE);
        sem_close(s_used);
        sem_unlink(SEM_USED);
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

        if (shm_obj->terminate == 1) {
            fprintf(stderr, "%s: Supervisor terminates %s.\n", program_name, program_name);
            sem_post(s_write);
            sem_post(s_free);
            break;
        }

        if ( generate(edge_array, shm_obj) == -1) {
            exit_status = -1;
            break;
        }

        shm_obj->writePos = (shm_obj->writePos + 1) % 50;
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

    if (sem_unlink(SEM_FREE) == -1) {
        exit_status = -1;
    }
    if (sem_unlink(SEM_USED) == -1) {
        exit_status = -1;
    }
    if (sem_unlink(SEM_WRITE) == -1) {
        exit_status = -1;
    }

    if (munmap(shm_obj, sizeof(*shm_obj)) == -1) {
        exit_status = -1;
    }

    if (shm_unlink(SHM) == -1) {
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

    EdgeArray *edge_array = (EdgeArray *) malloc(sizeof(EdgeArray));
    edge_array->content = (Edge *) malloc(sizeof(Edge));
    edge_array->length = 0;

    int current_arg;
    for (current_arg = 1; current_arg < argc; current_arg++) {

        Edge new_edge;

        int tmp_from, tmp_to, sscanf_err;
        sscanf_err = sscanf(argv[current_arg], "%d-%d", &tmp_from, &tmp_to);

        if (sscanf_err == 0) {
            fprintf (stderr, "%s: Given Edge couldn't be parsed.\n", program_name);
            free(edge_array);
            exit(EXIT_FAILURE);
        }

        new_edge.from = tmp_from;
        new_edge.to = tmp_to;


        if (addToArray(edge_array, new_edge) == -1) {
            free(edge_array);
            exit(EXIT_FAILURE);
        }
    }

    if (handleSolutions(edge_array) == -1) {
        free(edge_array);
        exit(EXIT_FAILURE);
    }

    free(edge_array);
    exit(EXIT_SUCCESS);
}
