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
#include <signal.h>
#include <string.h>
#include <getopt.h>
#include "mycompress.h"

/**
 * Pointer to name of program
 **/
static char *program_name;

/**
 * indicates if supervisor should terminate
 **/
volatile sig_atomic_t quit = 0;

/**
 * printUsageError function.
 * @brief Usage of program is printed to stderr and program is exited with failure code.
 * @details global variables: program_name, contains the name of the program.
 **/
void printUsageError() {

    fprintf(stderr, "Usage: %s [-o outfile] [file...]\n", program_name);
    exit(EXIT_FAILURE);

}

/**
 * handles SIGINT signal
 * @brief the program prints the signal and sets the global var quit to 1,
 * so the program can terminate.
 * @param signal, the signal that should be handed
 * @details global variables: quit
 **/
void handle_signal(int signal) {

    if ( signal == SIGINT ) {
        quit= 1;
    }

}

/**
 * Compress function. Compresses input file to output file.
 * @brief this functions iterates through all characters from the input
 * file and manages the occurrences.
 * @param files contains current files
 * @param counter contains char count and written count
 * @details global variables: quit
 * @return 1 if success and -1 if failure
 **/
char compress(Files files, Counter * counter) {

    char active_char;
    char prev = EOF;
    int current_char_counter = 0;

    while (!quit) {
        active_char = fgetc(files.inputFile);
        if ( active_char == EOF ) {
            break;
        }

        if (prev == EOF) {
            current_char_counter++;
        } else if (prev == active_char) {
            current_char_counter++;
        } else if ( prev != active_char ) {
            counter->char_counter += current_char_counter;
            counter->written_counter += 2;
            fprintf(files.outputFile, "%c%i", prev, current_char_counter);
            current_char_counter = 1;
        } else {
            return -1;
        }

        prev = active_char;

    }

    counter->char_counter += current_char_counter;
    counter->written_counter += 2;

    fprintf(files.outputFile, "%c%i", prev, current_char_counter);
    fclose(files.inputFile);

    return 1;
}

/**
 * Program entry point.
 * @brief The program starts here. This function takes care about parameters and calls
 * the compress function for every given input file. After compression of all given files
 * this functions prints the statistics to stderr.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @details global variables: program_name
 * @return Exits the program with EXIT_SUCCESS or EXIT_FAILURE
 **/
int main(int argc, char **argv) {

    program_name = argv[0];
    
    Counter counter;
    counter.char_counter = 0;
    counter.written_counter = 0;

    Files files;
    files.inputFile = stdin;
    files.outputFile = stdout;

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    int current_option;
    int o_flag = 0;
    int current_arg = 0;

    while ((current_option = getopt(argc, argv, "o:")) != -1) {


        switch (current_option) {

            case 'o':

                if (o_flag) {

                    fprintf(stderr,"Only one -o Option allowed\n");
                    printUsageError();

                } else {

                    o_flag++;

                }

                fprintf(stdout, "Writing compression to file '%s' ! \n", optarg);
                files.outputFile = fopen(optarg, "w");
                break;

            case '?':

                printUsageError();
        }
    }


    if (!o_flag) {

        fprintf(stdout, "Writing compression to 'stdout' ! \n");
        current_arg = o_flag + 1;

    } else {

        current_arg = o_flag + 2;

    }

    if (current_arg < argc) {

        for (;
                current_arg < argc;
                current_arg++) {

            files.inputFile = fopen(argv[current_arg], "r");
            if (compress(files, &counter) != 1) {
                fprintf(stderr, "%s: File couldn't be compressed", program_name);
		exit(EXIT_FAILURE);
	    }
        }

    } else {

      fprintf(stdout, "Reading from 'stdin' ! \n");
      if (compress(files, &counter) != 1) {
	fprintf(stderr, "%s: File couldn't be compressed", program_name);
	exit(EXIT_FAILURE);
      }

    }

    float ratio = 0;
    ratio = ((float) counter.written_counter / counter.char_counter) * 100;
    fprintf(stderr,
            "\nRead: %7i characters\nWritten: %4i characters\nCompression ratio: %4.2f%%\n", counter.char_counter , counter.written_counter ,
            ratio);
    fclose(files.outputFile);
    exit(EXIT_SUCCESS);

    return 0;
}
