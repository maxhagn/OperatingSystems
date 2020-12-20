/**
 * @file cpair.h
 * @author Maximilian Hagn <11808237@student.tuwien.ac.at>
 * @date 20.12.2020
 *
 * @brief Contains structs for cpair.c
 *
 **/

#ifndef CPAIR_H
#define CPAIR_H

// Defines a single point, from indicates the x coordinate and to the y coordinate
struct Point {
    float from;
    float to;
};
typedef struct Point Point;

// Defines an array of points, length indicates the current size of the array
struct PointArray {
    int length;
    Point* content;
};
typedef struct PointArray PointArray;



#endif
