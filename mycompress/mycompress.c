/**
 * @file mycompress.c
 * @author Maximilian Hagn <11808237@student.tuwien.ac.at>
 * @date 21.11.2020
 *
 * @brief MyCompress Program. Reads inputs txt documents and compresses
 * the letters. Finally, it returns the count of all chars, the count of
 * all written chars and the compress ratio.
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "mycompress.h"

int charCounter;
int writeCounter;

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

    fprintf(stderr, "Usage: %s [-o outfile] [file...]\n", program_name);
    exit(EXIT_FAILURE);

}


/**
 * Compress function. Compresses input file to output file
 * @brief this functions iterates through all characters from the input
 * file and manages the occuourences
 * @param files contains current files
 * @param counter contains char count and written count
 **/
char compress(Files files, Counter * counter) {

    char active;
    char prev = EOF;
    int current_char_counter = 0;

    while ((active = fgetc(files.inputFile)) != EOF) {

        if (prev == EOF) {
            current_char_counter++;
        } else if (prev == active) {
            current_char_counter++;
        } else if (prev != active || active == EOF) {
            counter->char_counter += current_char_counter;
            counter->written_counter += 2;
            fprintf(files.outputFile, "%c%i", prev, current_char_counter);
            current_char_counter = 1;
        }
        prev = active;

    }

    counter->char_counter += current_char_counter;
    counter->written_counter += 2;
    fprintf(files.outputFile, "%c%i", prev, current_char_counter);

    fclose(files.inputFile);

    return 0;
}

/**
 * Program entry point.
 * @brief The program starts here. This function takes care about parameters and calls
 * the compress function for every input file
 * @param argc The argument counter.
 * @param argv The argument vector.
 **/
int main(int argc, char **argv) {

    int option;
    Counter counter;
    counter.char_counter = 0;
    counter.written_counter = 0;

    Files files;
    files.inputFile = stdin;
    files.outputFile = stdout;

    int o_flag = 0;
    int current_arg = 0;

    while ((option = getopt(argc, argv, "o:")) != -1) {


        switch (option) {

            case 'o':

                if (o_flag) {

                    printf("Only one -o Option allowed\n");
                    printUsageError();

                } else {

                    o_flag++;

                }

                printf("Writing compression to file '%s' ! \n", optarg);
                files.outputFile = fopen(optarg, "w");
                break;

            case '?':

                printUsageError();
        }
    }


    if (!o_flag) {

        printf("Writing compression to 'stdout' ! \n");
        current_arg = o_flag + 1;

    } else {

        current_arg = o_flag + 2;

    }

    if (o_flag < argc) {

        for (;
                current_arg < argc;
                current_arg++) {

            files.inputFile = fopen(argv[current_arg], "r");
            compress(files, &counter);
            fclose(files.inputFile);

        }

    } else {

        compress(files, &counter);

    }

    float ratio = ((float) counter.written_counter / counter.char_counter) * 100;
    fprintf(stderr,
            "\nRead: %7i characters\nWritten: %4i characters\nCompression ratio: %4.2f%%\n", counter.char_counter , counter.written_counter ,
            ratio);
    fclose(files.outputFile);
    exit(EXIT_SUCCESS);

    return 0;
}
