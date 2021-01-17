/**
 * @file client.c
 * @author Maximilian Hagn <11808237@student.tuwien.ac.at>
 * @date 12.01.2021
 *
 * @brief Client Program. Takes Port, Output File oder Directory and URL as input.
 * Sends a HTTP/1.1 Request to the host containing in the URL and receives the response.
 * The response is then printed to the specified output file path.
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "client.h"

/**
 * Pointer to name of program
 **/
static char *program_name;

/**
 * usage function.
 * @brief Usage of program is printed to stderr and program is exited with failure code.
 * @details global variables: program_name, contains the name of the program.
 **/
static void usage( void ) {
    fprintf( stderr, "Usage: %s [-p PORT] [-o FILE | -d DIR] URL\n", program_name );
    exit( EXIT_FAILURE );
}

/**
 * parse_url function. Extracts host and path from specified url
 * @brief This function receives the specified url and extracts the
 * part after the http:// and the first occurence of th chars ;/?:@=&
 * and the part after on of this chars.
 * @param url contains the url specified in the program options
 * @return Url Struct containing the host and path part
 **/
static struct Url parse_url( char *url ) {

    struct Url parsed_url;
    char ending_chars[] = ";/?:@=&";

    if ( strstr( url, "http://" )) {
        char *url_copy = malloc( strlen( url ) + 1 );
        strcpy( url_copy, url );
        url_copy += strlen( "http://" );

        char *url_path;
        url_path = strpbrk( url_copy, ending_chars );

        if ( url_path == NULL ) {
            parsed_url.host = malloc( strlen( url_copy ) + 1 );
            strcpy( parsed_url.host, url_copy );
            parsed_url.path = malloc( 2 );
            parsed_url.path = "/\0";
        } else {

            int path_length = strlen( url_path );
            parsed_url.path = malloc( path_length + 1 );
            parsed_url.path = url_path;

            int host_length = strlen( url_copy ) - strlen( parsed_url.path );
            parsed_url.host = malloc( host_length + 1 );
            strncpy( parsed_url.host, url_copy, host_length );

            parsed_url.host[ host_length + 1 ] = '\0';

        }


    } else {
        fprintf( stderr, "The given URL is invalid by this assignment specification" );
        exit( EXIT_FAILURE );
    }

    return parsed_url;
}


/**
 * Request function. Creates and Sends Request Body and prints Response Body.
 * @brief This function gets the parsed url and creates an request body. These
 * message is sent over sockets to the server. After receiving a response, the response
 * body is printed to specified file.
 * @param url contains the url specified in the program options
 * @param port the in the options specified port
 * @param output_file the request is print to this file
 * @return Exits the program EXIT_FAILURE if error occurs
 **/
static void request( char *url, char *port, FILE *output_file ) {

    struct Url parsed_url = parse_url( url );
    
    struct addrinfo hints, *ai;
    memset( &hints, 0, sizeof( hints ));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int getaddrinfo_error = getaddrinfo( parsed_url.host, port, &hints, &ai );
    if ( getaddrinfo_error != 0 ) {
        fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( getaddrinfo_error ));
        exit( EXIT_FAILURE );
    }

    int sockfd = socket( ai->ai_family, ai->ai_socktype, ai->ai_protocol );
    if ( sockfd < 0 ) {
        fprintf( stderr, "Couldn't create socket.." );
        exit( EXIT_FAILURE );
    }

    if ( connect( sockfd, ai->ai_addr, ai->ai_addrlen ) < 0 ) {
        fprintf( stderr, "Couldn't connect to Server." );
        exit( EXIT_FAILURE );
    }

    FILE *sockfile = fdopen( sockfd, "r+" );
    if ( sockfile == NULL ) {
        fprintf( stderr, "Couldn't open socket descriptor." );
        exit( EXIT_FAILURE );
    }

    char *request = malloc( strlen( parsed_url.path ) + strlen( parsed_url.host ) + 45 );
    sprintf( request, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", parsed_url.path, parsed_url.host );
    if ( fputs( request, sockfile ) == EOF ) {
        fprintf( stderr, "Couldn't store the request in socket file." );
        exit( EXIT_FAILURE );
    }

    if ( fflush( sockfile ) == EOF ) {
        fprintf( stderr, "Couldn't sent the request to server" );
        exit( EXIT_FAILURE );
    }

    int line_counter = 1;
    int is_response_body = 0;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while (( read = getline( &line, &len, sockfile )) != -1 ) {
        if ( line_counter == 1 ) {
            if ( !strstr( line, "HTTP/1.1" )) {

                fprintf( stderr, "Protocol error! \n" );
                exit( 2 );

            } else {

                char *line_copy = malloc( strlen( line ) + 1 );
                strcpy( line_copy, line );
                line_copy += strlen( "HTTP/1.1" );

                char *remaining_chars;
                int status_code = strtol( line_copy, &remaining_chars, 10 );

                if ( status_code != 200 ) {
                    fprintf( stderr, "Response: %s , No Success\n", line );
                    exit( 3 );
                }
            }
        } else {
            if ( strcmp( line, "\r\n" ) == 0 ) {
                is_response_body = 1;
            }
            if ( is_response_body == 1 ) {
                fprintf( output_file, "%s", line );
            }
        }
        line_counter++;
    }

    freeaddrinfo( ai );
    fclose( sockfile );

}

/**
 * Program entry point.
 * @brief The program starts here. This function takes care about parameters
 * (calculates the output file, and other options ) and calls
 * the request function. After finishing allocated ressources are freed.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @details global variables: program_name
 * @return Exits the program with EXIT_SUCCESS or EXIT_FAILURE
 **/
int main( int argc, char *argv[] ) {
    program_name = argv[ 0 ];

    FILE *output_file = stdout;
    char *port = "80";
    char *directory_option = NULL;
    char *file_option = NULL;
    int o_counter = 0;
    int d_counter = 0;
    int current_option;

    while (( current_option = getopt( argc, argv, "p:o:d:" )) != -1 ) {
        switch ( current_option ) {
            case 'p':
                port = optarg;
                break;
            case 'o':
                o_counter += 1;
                file_option = optarg;
                break;
            case 'd':
                d_counter += 1;
                directory_option = optarg;
                break;
            case '?':
                usage( );
                break;
            default:
                usage( );
                break;
        }
    }

    int od_counter = o_counter + d_counter;
    if ( od_counter > 1 ) {
        usage( );
    }

    if (( argc - optind ) != 1 ) {
        usage( );
    }

    char *url = malloc( strlen( argv[ optind ] ));
    strcpy( url, argv[ optind ] );

    char *has_chars;
    int port_numeric = strtol( port, &has_chars, 10 );
    if ( strlen( has_chars ) > 0 || port_numeric < 0 ) {
        fprintf( stderr, "%s is an invalid port. Ports can't be negative and must be numeric!", port );
        exit( EXIT_FAILURE );
    }

    char *file_path = NULL;
    if ( directory_option != NULL ) {
        char *url_copy = malloc( strlen( url ));
        strcpy( url_copy, url );
        char *requested_file_name = strrchr( url_copy, '/' );

        char *directory_copy = malloc( strlen( directory_option ) + 1 );
        strcpy( directory_copy, directory_option );

        char *directory_file_name = strrchr( directory_copy, '/' );
        int directory_file_name_size = 0;

        if ( directory_file_name != NULL ) {
            directory_file_name_size = strlen( directory_file_name );
        }

        if ( directory_file_name_size == 1 ) {
            if ( requested_file_name != NULL ) {
                if ( strlen( requested_file_name ) > 1 ) {
                    strcat( directory_option, requested_file_name );
                } else {
                    strcat( directory_option, "index.html" );
                }
            }
        } else {
            strcat( directory_option, "/index.html" );
        }

        file_path = malloc( strlen( directory_option ) + 1 );
        strcpy( file_path, directory_option );
        fprintf( stderr, "%s", file_path );
        free( directory_copy );

    } else if ( file_option != NULL ) {
        file_path = malloc( strlen( file_option ) + 1 );
        strcpy( file_path, file_option );
    }

    if ( file_path != NULL ) {
        if ( !( output_file = fopen( file_path, "w" ))) {
            fprintf( stderr, "File %s couldn't be accessed. \n", file_path );
            exit( EXIT_FAILURE );
        }
    }

    request( url, port, output_file );

    if ( file_path != NULL ) {
        free( file_path );
    }

    if ( directory_option != NULL || file_option != NULL ) {
        fclose( output_file );
    }

    exit( EXIT_SUCCESS );
}
