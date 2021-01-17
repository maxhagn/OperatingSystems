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
    char* tmp_host = NULL;
    char* tmp_path = NULL;
    char ending_chars[] = ";/?:@=&";

    if ( strstr( url, "http://" )) {
        size_t len = strlen( url );
        char *url_short = (char *)malloc( len + 1);
        memcpy( url_short, url, len );
        url_short[ len ] = '\0';
        url_short += strlen( "http://" );

        char *url_path;
        url_path = strpbrk( url_short, ending_chars );
        if ( url_path == NULL ) {

            tmp_host = malloc( strlen( url_short ) + 1 );
            memset(tmp_host, 0, strlen( url_short ) + 1);
            memcpy( tmp_host, url_short, strlen( url_short ) );

            tmp_path = malloc( strlen("/") + 1 );
            memset(tmp_path, 0, 2);
            char *slash = "/";
            memcpy( tmp_path, slash, strlen( slash ) );

        } else {

            int path_length = strlen( url_path );
            tmp_path = malloc(path_length + 1);
            memset(tmp_path, 0, path_length + 1);
            memcpy(tmp_path, url_path, path_length);

            int host_length = strlen( url_short ) - strlen( tmp_path );
            tmp_host = malloc(host_length + 1);
            memset(tmp_host, 0, host_length + 1);
            memcpy(tmp_host, url_short, host_length );


        }

        free(url_short - strlen("http://"));
    } else {
        free( url );
        fprintf( stderr, "The given URL is invalid by this assignment specification" );
        exit( EXIT_FAILURE );
    }

    parsed_url.host = malloc(strlen(tmp_host) + 1);
    strcpy(parsed_url.host, tmp_host);
    parsed_url.path = malloc(strlen(tmp_path) + 1);
    strcpy(parsed_url.path, tmp_path);



    if ( tmp_path != NULL ) {
        free(tmp_path);
    }

    if ( tmp_host != NULL ) {
        free(tmp_host);
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

    free(request);

    if ( fflush( sockfile ) == EOF ) {
        fprintf( stderr, "Couldn't sent the request to server" );
        exit( EXIT_FAILURE );
    }

    int line_counter = 1;
    int is_response_body = 0;
    char *line = NULL;
    size_t len = 0;

    while (( getline( &line, &len, sockfile )) != -1 ) {
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
                free(line_copy - strlen("HTTP/1.1"));

                if ( status_code != 200 ) {
                    free( url );
                    free(line);
                    line = NULL;
                    free(parsed_url.host);
                    free(parsed_url.path);
                    freeaddrinfo( ai );
                    fclose( sockfile );
                    fprintf( stderr, "Response: %sNo Success\n", line );
                    exit( 3 );
                }



            }
        } else {
        if ( is_response_body == 1 ) {
                        fprintf( output_file, "%s", line );
                    }
            if ( strcmp( line, "\r\n" ) == 0 ) {
                is_response_body = 1;
            }

        }
        line_counter++;
    }

    free(line);
    free(parsed_url.host);
    free(parsed_url.path);
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

    char *url = malloc( strlen( argv[ optind ] ) + 1);
    memset( url, 0, strlen( argv[ optind ]) + 1);
    memcpy( url, argv[ optind ], strlen( argv[ optind ]));

    char *remaining_chars;
    int port_numeric = strtol( port, &remaining_chars, 10 );
    if ( strlen( remaining_chars ) > 0 || port_numeric < 0 ) {
        fprintf( stderr, "%s is an invalid port. Ports can't be negative and must be numeric!", port );
        free( url );
        exit( EXIT_FAILURE );
    }

    char *file_path = NULL;
    if ( directory_option != NULL ) {
        size_t url_length = strlen( url ) + 1;
        char *url_copy = malloc( url_length );
        memset(url_copy, 0, url_length);
        memcpy( url_copy, url, url_length );

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

        free(url_copy);
        file_path = malloc( strlen( directory_option ) + 1 );
        strcpy( file_path, directory_option );
        free( directory_copy );

    } else if ( file_option != NULL ) {
        file_path = malloc( strlen( file_option ) + 1 );
        strcpy( file_path, file_option );
    }

    if ( file_path != NULL ) {
        if ( !( output_file = fopen( file_path, "w" ))) {
            fprintf( stderr, "File %s couldn't be accessed. \n", file_path );
            free( url );
            exit( EXIT_FAILURE );
        }
    }

    request( url, port, output_file );

    if ( url != NULL ) {
        free( url );
    }

    if ( file_path != NULL ) {
        free( file_path );
    }

    if ( directory_option != NULL || file_option != NULL ) {
        fclose( output_file );
    }

    exit( EXIT_SUCCESS );
}
