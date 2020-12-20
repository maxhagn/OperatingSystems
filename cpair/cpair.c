/**
 * @file cpair.c
 * @author Maximilian Hagn <11808237@student.tuwien.ac.at>
 * @date 20.12.2020
 *
 * @brief The Cpair Module reads 2D Points from Stdin. If one Point is read, the program exit without output.
 * If two points are read, the program writes these two points to stdout. If more than two points are read the
 * program forks and passes each half to one of two child processes. The result is than compared and the two points
 * with the smallest distance are marked as closest pair.
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <math.h>
#include <sys/types.h>
#include <float.h>
#include "cpair.h"

/**
 * Pointer to name of program
 **/
static char *program_name;

/**
 * usage function.
 * @brief Usage of program is printed to stderr and program is exited with failure code
 * @details global variables: program_name, contains the name of the program
 **/
static void usage( void ) {
    fprintf( stderr, "Usage: %s\n", program_name );
    exit( EXIT_FAILURE );
}

/**
 * calc_distance function.
 * @brief The distance of two points is calculated and returned as a float value.
 * @param a - first point
 * @param b - second point
 * @return the distance of these two points in float.
 **/
static float calc_distance( Point a, Point b ) {
    float result = sqrt(( a.from - b.from ) *
                        ( a.from - b.from ) +
                        ( a.to - b.to ) *
                        ( a.to - b.to ));

    return result;
}

/**
 * read_input function.
 * @brief The input is read from a specified file and written to the given PointArray.
 * The input is written line by line und the points are than extracted and casted to float values.
 * @param * input - the file which is read from.
 * @param * result - the pointer to the result array.
 * @return integer 1 if successful, integer -1 if failure
 **/
static int read_input( FILE *input, PointArray *result ) {
    char *line = NULL;
    size_t len = 0;

    while ( getline( &line, &len, input ) != -1 ) {

        if ( strlen( line ) == 1 ) {
            free( line );
            return -1;
        }

        float from;
        float to;
        char *active_point = NULL;
        char *endptr = NULL;

        active_point = strtok( line, " " );
        for ( int i = 0; i < 2; i++ ) {
            if ( active_point != NULL) {

                if ( i == 0 ) {
                    from = strtof( active_point, &endptr );
                } else if ( i == 1 ) {
                    to = strtof( active_point, &endptr );
                }

                active_point = strtok(NULL, " " );
            } else {
                free( active_point );
                exit( EXIT_FAILURE );
            }
        }

        free( active_point );

        Point *new_data = realloc( result->content, (( result->length ) + 1 ) * sizeof( Point ));
        if ( new_data == NULL) {
            free( line );
            return -1;
        }

        result->length = ( result->length ) + 1;
        result->content = new_data;
        ( result->content )[ ( result->length ) - 1 ].from = from;
        ( result->content )[ ( result->length ) - 1 ].to = to;
    }

    free( line );
    return 1;
}

/**
 * fork_array function.
 * @brief If the input is greater than 2 the program is forked. This functionality is handeled by the
 * fork_array function. First the array is split into to halfs. Each half is redirected to one child process.
 * Furthermore, the results from the child function are compared and the smaller one is taken as result. Then all points
 * from the first child are compared to the other childs points. The smallest point is than redirected to the parent.
 * @param * point_array - a pointer to all read points.
 * @return integer 1 if successful, integer -1 if failure
 **/
static int fork_array( PointArray *point_array ) {

    PointArray *smaller_than_arithmetic = ( PointArray * ) malloc( sizeof( PointArray ));
    smaller_than_arithmetic->length = 0;
    smaller_than_arithmetic->content = ( Point * ) malloc( sizeof( Point ));

    PointArray *larger_than_arithmetic = ( PointArray * ) malloc( sizeof( PointArray ));
    larger_than_arithmetic->length = 0;
    larger_than_arithmetic->content = ( Point * ) malloc( sizeof( Point ));

    float sum = 0;
    float arithmetic = 0;

    for ( int i = 0; i < point_array->length; i++ ) {
        sum = sum + ( point_array->content )[ i ].from;
    }

    arithmetic = sum * ( 1 / ( float ) point_array->length );

    for ( int i = 0; i < point_array->length; i++ ) {
        if (( point_array->content )[ i ].from <= arithmetic ) {

            Point *new_data = realloc( smaller_than_arithmetic->content,
                                       (( smaller_than_arithmetic->length ) + 1 ) * sizeof( Point ));
            if ( new_data == NULL) {
                return -1;
            }

            smaller_than_arithmetic->length = ( smaller_than_arithmetic->length ) + 1;
            smaller_than_arithmetic->content = new_data;
            ( smaller_than_arithmetic->content )[ ( smaller_than_arithmetic->length ) -
                                                  1 ].from = ( point_array->content )[ i ].from;
            ( smaller_than_arithmetic->content )[ ( smaller_than_arithmetic->length ) -
                                                  1 ].to = ( point_array->content )[ i ].to;

        } else {

            Point *new_data = realloc( larger_than_arithmetic->content,
                                       (( larger_than_arithmetic->length ) + 1 ) * sizeof( Point ));
            if ( new_data == NULL) {
                return -1;
            }

            larger_than_arithmetic->length = ( larger_than_arithmetic->length ) + 1;
            larger_than_arithmetic->content = new_data;
            ( larger_than_arithmetic->content )[ ( larger_than_arithmetic->length ) -
                                                 1 ].from = ( point_array->content )[ i ].from;
            ( larger_than_arithmetic->content )[ ( larger_than_arithmetic->length ) -
                                                 1 ].to = ( point_array->content )[ i ].to;


        }
    }


    int pipe_p_to_c1[2];
    if ( pipe( pipe_p_to_c1 ) == -1 ) {
        return -1;
    }

    int pipe_c1_to_p[2];
    if ( pipe( pipe_c1_to_p ) == -1 ) {
        return -1;
    }

    pid_t c1_id = fork( );
    if ( c1_id < 0 ) {
        return -1;
    } else if ( c1_id == 0 ) {

        if ( close( pipe_p_to_c1[ 1 ] ) == -1 ) {
            return -1;
        }

        if ( dup2( pipe_p_to_c1[ 0 ], STDIN_FILENO ) == -1 ) {
            return -1;
        }

        if ( close( pipe_p_to_c1[ 0 ] ) == -1 ) {
            return -1;
        }

        if ( close( pipe_c1_to_p[ 0 ] ) == -1 ) {
            return -1;
        }

        if ( dup2( pipe_c1_to_p[ 1 ], STDOUT_FILENO ) == -1 ) {
            return -1;
        }

        if ( close( pipe_c1_to_p[ 1 ] ) == -1 ) {
            return -1;
        }

        execlp( "./cpair", "cpair", NULL);
        return -1;

    }

    if ( close( pipe_p_to_c1[ 0 ] ) == -1 ) {
        return -1;
    }

    if ( close( pipe_c1_to_p[ 1 ] ) == -1 ) {
        return -1;
    }

    int pipe_p_to_c2[2];
    if ( pipe( pipe_p_to_c2 ) == -1 ) {
        return -1;
    }

    int pipe_c2_to_p[2];
    if ( pipe( pipe_c2_to_p ) == -1 ) {
        return -1;
    }

    pid_t c2_id = fork( );
    if ( c2_id < 0 ) {
        return -1;
    } else if ( c2_id == 0 ) {

        if ( close( pipe_p_to_c2[ 1 ] ) == -1 ) {
            return -1;
        }

        if ( dup2( pipe_p_to_c2[ 0 ], STDIN_FILENO ) == -1 ) {
            return -1;
        }

        if ( close( pipe_p_to_c2[ 0 ] ) == -1 ) {
            return -1;
        }

        if ( close( pipe_c2_to_p[ 0 ] ) == -1 ) {
            return -1;
        }

        if ( dup2( pipe_c2_to_p[ 1 ], STDOUT_FILENO ) == -1 ) {
            return -1;
        }

        if ( close( pipe_c2_to_p[ 1 ] ) == -1 ) {
            return -1;
        }

        execlp( "./cpair", "cpair", NULL);
        return -1;

    }

    if ( close( pipe_p_to_c2[ 0 ] ) == -1 ) {
        return -1;
    }

    if ( close( pipe_c2_to_p[ 1 ] ) == -1 ) {
        return -1;
    }


    FILE *file_p_to_c1 = fdopen( pipe_p_to_c1[ 1 ], "w" );
    if ( file_p_to_c1 != NULL) {

        for ( int i = 0; i < smaller_than_arithmetic->length; i++ ) {
            if ( fprintf( file_p_to_c1, "%f %f\n", ( smaller_than_arithmetic->content )[ i ].from,
                          ( smaller_than_arithmetic->content )[ i ].to ) < 0 ) {
                return -1;
            }
        }

        if ( fflush( file_p_to_c1 ) == EOF) {
            return -1;
        }


        if ( fclose( file_p_to_c1 ) == EOF) {
            return -1;
        }

    } else {
        return -1;
    }

    FILE *file_p_to_c2 = fdopen( pipe_p_to_c2[ 1 ], "w" );
    if ( file_p_to_c2 != NULL) {

        for ( int i = 0; i < larger_than_arithmetic->length; i++ ) {
            if ( fprintf( file_p_to_c2, "%f %f\n", ( larger_than_arithmetic->content )[ i ].from,
                          ( larger_than_arithmetic->content )[ i ].to ) < 0 ) {
                return -1;
            }
        }

        if ( fflush( file_p_to_c2 ) == EOF)
            return -1;

        if ( fclose( file_p_to_c2 ) == EOF)
            return -1;

    } else {
        return -1;
    }

    int status[2];
    int c1_error_code = waitpid( c1_id, &status[ 0 ], 0 );
    int c2_error_code = waitpid( c2_id, &status[ 1 ], 0 );
    if ( WEXITSTATUS( status[ 0 ] ) != EXIT_SUCCESS
         || WEXITSTATUS( status[ 1 ] ) != EXIT_SUCCESS
         || c1_error_code < 0
         || c2_error_code < 0 ) {
        return -1;
    }

    PointArray *c1_result = ( PointArray * ) malloc( sizeof( PointArray ));
    c1_result->length = 0;
    c1_result->content = ( Point * ) malloc( sizeof( Point ));
    FILE *c1_result_file = fdopen( pipe_c1_to_p[ 0 ], "r" );
    if ( read_input( c1_result_file, c1_result ) == -1 ) {
        free( c1_result->content );
        free( c1_result );
        exit( EXIT_FAILURE );
    }


    PointArray *c2_result = ( PointArray * ) malloc( sizeof( PointArray ));
    c2_result->length = 0;
    c2_result->content = ( Point * ) malloc( sizeof( Point ));
    FILE *c2_result_file = fdopen( pipe_c2_to_p[ 0 ], "r" );
    if ( read_input( c2_result_file, c2_result ) == -1 ) {
        free( c2_result->content );
        free( c2_result );
        exit( EXIT_FAILURE );
    }


    float distance_c1 = calc_distance(( c1_result->content )[ 0 ], ( c1_result->content )[ 1 ] );
    float distance_c2 = calc_distance(( c2_result->content )[ 0 ], ( c2_result->content )[ 1 ] );

    float min = FLT_MAX;
    Point result_points[2];

    if ( c1_result->length == 2 ) {
        min = distance_c1;
        result_points[ 0 ].from = ( c1_result->content )[ 0 ].from;
        result_points[ 0 ].to = ( c1_result->content )[ 0 ].to;
        result_points[ 1 ].from = ( c1_result->content )[ 1 ].from;
        result_points[ 1 ].to = ( c1_result->content )[ 1 ].to;
    }


    if ( distance_c2 < min ) {
        if ( c2_result->length == 2 ) {
            min = distance_c2;
            result_points[ 0 ].from = ( c2_result->content )[ 0 ].from;
            result_points[ 0 ].to = ( c2_result->content )[ 0 ].to;
            result_points[ 1 ].from = ( c2_result->content )[ 1 ].from;
            result_points[ 1 ].to = ( c2_result->content )[ 1 ].to;
        }
    }

    for ( int i = 0; i < c1_result->length; i++ ) {
        for ( int j = 0; j < c2_result->length; j++ ) {
            float distance = calc_distance(( c1_result->content )[ i ], ( c2_result->content )[ j ] );
            if ( distance < min ) {
                min = distance;
                result_points[ 0 ].from = ( c1_result->content )[ i ].from;
                result_points[ 0 ].to = ( c1_result->content )[ i ].to;
                result_points[ 1 ].from = ( c2_result->content )[ j ].from;
                result_points[ 1 ].to = ( c2_result->content )[ j ].to;
            }
        }
    }

    free( c1_result->content );
    free( c1_result );
    free( c2_result->content );
    free( c2_result );
    fclose( c1_result_file );
    fclose( c2_result_file );

    fprintf( stdout, "%f %f\n", result_points[ 0 ].from, result_points[ 0 ].to );
    fprintf( stdout, "%f %f\n", result_points[ 1 ].from, result_points[ 1 ].to );


    if ( fflush( stdout ) == EOF) {
        return -1;
    }

    if ( fclose( stdout ) == EOF) {
        return -1;
    }

    return 1;
}

/**
 * Program entry point.
 * @brief The program starts here. This function creates the storage for all read points. After
 * reading the input, the function decides if the program exits, if the the input is written to stdout
 * or if the program is forked.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS or EXIT_FAILURE
 **/
int main( int argc, char **argv ) {

    program_name = argv[ 0 ];

    if ( argc > 1 ) {
        usage( );
    }

    PointArray *point_array = ( PointArray * ) malloc( sizeof( PointArray ));
    point_array->length = 0;
    point_array->content = ( Point * ) malloc( sizeof( Point ));

    if ( read_input( stdin, point_array ) == -1 ) {
        free( point_array->content );
        free( point_array );
        exit( EXIT_FAILURE );
    }

    if ( point_array->length <= 1 ) {
        free( point_array->content );
        free( point_array );
        exit( EXIT_SUCCESS );
    }

    if ( point_array->length == 2 ) {
        for ( int i = 0; i < 2; i++ ) {
            fprintf( stdout, "%f %f\n", ( point_array->content )[ i ].from, ( point_array->content )[ i ].to );
        }

        free( point_array->content );
        free( point_array );
        exit( EXIT_SUCCESS );
    }

    if ( point_array->length > 2 ) {
        if ( fork_array( point_array ) == -1 ) {
            free( point_array->content );
            free( point_array );
            exit( EXIT_FAILURE );
        }
    }

    free( point_array->content );
    free( point_array );
    exit( EXIT_SUCCESS );

}
