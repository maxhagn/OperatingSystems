#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

static char *program_name;

static void usage(void) {
    fprintf(stderr, "Usage: %s [-L int] [filename]\n", program_name);
    exit(EXIT_FAILURE);
}

/* COMPLICATED PARSE ARGS */
int main(int argc, char **argv) {

    program_name = argv[0];
    int current_option;
    int l_flag = 0;
    int l_value = 0;
    FILE * outfile;

    while ((current_option = getopt(argc, argv, "L:")) != -1) {
        switch ( current_option ) {

            case 'L':

                if ( l_flag ) {
                    fprintf(stderr,"Only one -L Option allowed\n");
                    usage();
                } else {
                    l_flag++;
                }


                long integer_value;
                char *rest;

                integer_value = strtol(optarg, &rest, 10);

                if ( *rest != '\0' ) {
                    fprintf(stdout, "we have a string inside the int" );
                }
                /*
                char string;
                int sscanf_error = sscanf(optarg, "%d%c", &l_value, &string);
                if ( sscanf_error != 1 ) {
                    fprintf(stdout, "No Integer given!");
                    exit(EXIT_FAILURE);
                }*/

                fprintf(stdout, "%ld", integer_value);

                break;

            case '?':

                usage();
        }
    }

    if ( argc > 1 ) {
        if ( l_flag >= 1 ) {
            if ( l_flag+2 < argc ) {
                outfile = fopen(argv[3], "w+");
            } else {
                outfile = fopen("standard.txt", "w+");
            }

        } else {
            outfile = fopen(argv[1], "w+");
        }


    } else {
        outfile = fopen("standard.txt", "w+");
    }

    fprintf(outfile, "%d", l_value);
/*

    FILE * infile = fopen("infile.txt", "r");
    char *line = NULL;
    char active;
    size_t len = 0;
    while ( (active = fgetc(infile)) != EOF ) {
        fprintf(stderr, "%c\n", active);
    }

    fclose(outfile);
    free(line);
    */
    exit(EXIT_SUCCESS);
    return 0;

}
