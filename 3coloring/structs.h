/**
 * @file structs.c
 * @author Maximilian Hagn <11808237@student.tuwien.ac.at>
 * @date 21.11.2020
 *
 * @brief Contains all structs for generator.c and supervisor.c modules
 *
 **/

#ifndef STRUCTS_H
#define STRUCTS_H

// Representation of an Edge, with two integer values for vertex index
struct Edge {
    int from;
    int to;
};
typedef struct Edge Edge;

// Representation of an Shared Memory Object
// terminate indicates when functions should terminate
// write position is the current index of the write position, used by generators
// read position is the current pointer to the read position
// edge contains max. 50 solutions with eight edges each
struct ShmObj {
    int terminate;
    int writePos;
    int readPos;
    Edge data[50][8];
};
typedef struct ShmObj ShmObj;

// Defines an array of edges, length indicates the current size of the array
struct EdgeArray {
    int length;
    Edge **content;
};
typedef struct EdgeArray EdgeArray;

#endif


