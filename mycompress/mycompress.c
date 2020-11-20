#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

FILE *outputFile;
int charCounter;
int writeCounter;

void printUsageError() {

    fprintf(stderr, "Usage: mycompress [-o outfile] [file...]\n");
    exit(EXIT_FAILURE);

}

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

    } else if (oflag) {

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
            "\nRead: %7i characters\nWritten: %4i characters\nCompression ratio: %4.2f%%", charCounter, writeCounter,
            ratio);
    fclose(outputFile);
    exit(EXIT_SUCCESS);

    return 0;
}
