/**
 * @file mycompress.h
 * @author Maximilian Hagn <11808237@student.tuwien.ac.at>
 * @date 21.11.2020
 *
 * @brief contains structs for mycompress.c
 *
 **/

#ifndef MYCOMPRESS_H
#define MYCOMPRESS_H

// Holds information to files
struct Files {
    FILE * inputFile;
    FILE * outputFile;
};
typedef struct Files Files;

// Represents the counter for all chars and counter for written chars
struct Counter {
    int char_counter;
    int written_counter;
};
typedef struct Counter Counter;

#endif


