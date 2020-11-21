/**
 * @file generator.h
 * @author Maximilian Hagn <11808237@student.tuwien.ac.at>
 * @date 21.11.2020
 *
 * @brief Contains structs for generator.c
 *
 **/

#ifndef GENERATOR_H
#define GENERATOR_H

// Defines an array of edges, length indicates the current size of the array
struct EdgeArray {
    int length;
    Edge *content;
};
typedef struct EdgeArray EdgeArray;

#endif


