/**
 * @file client.h
 * @author Maximilian Hagn <11808237@student.tuwien.ac.at>
 * @date 12.01.2021
 *
 * @brief contains structs for client.c
 *
 **/

#ifndef CLIENT_H
#define CLIENT_H

// Defines the part of a specified url, host is the host part and path is the file path
struct Url {
    char *host;
    char *path;
};

#endif
