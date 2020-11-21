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

FILE *outputFile;
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
 * printUsageError function.
 * @brief Usage of program is printed to stderr and program is exited with failure code
 * @details global variables: program_name, contains the name of the program
 **/
int checkOccurrence(char prev, char active) {
    static int counter = 0;

    if (prev == EOF) {
        counter++;
    } else if (prev == active) {
        counter++;
    } else if (prev != active || active == EOF) {
        charCounter += counter;
        writeCounter += 2;
        fprintf(outputFile, "%c%i", prev, counter);
        counter = 1;
    }

    return counter;
}

/**
 * printUsageError function.
 * @brief Usage of program is printed to stderr and program is exited with failure code
 * @details global variables: program_name, contains the name of the program
 **/
char getSingleChars(FILE *inputFile) {

    char active;
    char prev = EOF;

    while ((active = fgetc(inputFile)) != EOF) {

        checkOccurrence(prev, active);
        prev = active;

    }
    checkOccurrence(prev, EOF);

    fclose(inputFile);

    return 0;
}

/**
 * printUsageError function.
 * @brief Usage of program is printed to stderr and program is exited with failure code
 * @details global variables: program_name, contains the name of the program
 **/
int main(int argc, char **argv) {

    int option;
    outputFile = stdout;
    FILE *inputFile = stdin;
    int oflag = 0;
    int optionCounter = 0;

    while ((option = getopt(argc, argv, "o:")) != -1) {


        switch (option) {

            case 'o':

                if (oflag) {

                    printf("Only one -o Option allowed\n");
                    printUsageError();

                } else {

                    oflag++;

                }

                printf("Writing compression to file '%s' ! \n", optarg);
                outputFile = fopen(optarg, "w");
                break;

            case '?':

                printUsageError();
        }
    }


    if (!oflag) {

        printf("Writing compression to 'stdout' ! \n");
        optionCounter = oflag + 1;

    } else {

        optionCounter = oflag + 2;

    }

    if (oflag < argc) {

        for (;
                optionCounter < argc;
                optionCounter++) {

            inputFile = fopen(argv[optionCounter], "r");
            getSingleChars(inputFile);
            fclose(inputFile);

        }

    } else {

        getSingleChars(inputFile);

    }

    float ratio = ((float) writeCounter / charCounter) * 100;
    fprintf(stderr,
            "\nRead: %7i characters\nWritten: %4i characters\nCompression ratio: %4.2f%%\n", charCounter, writeCounter,
            ratio);
    fclose(outputFile);
    exit(EXIT_SUCCESS);

    return 0;
}
