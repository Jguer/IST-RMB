#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include "utils.h"
#include "identity.h"

void usage(char* name) {
    fprintf(stdout, "Example Usage: %s [-i siip] [-p sipt]\n", name);
    fprintf(stdout, "Arguments:\n"
            "\t-i\t\t[server ip]\n"
            "\t-p\t\t[server port]\n");
}

int main(int argc, char *argv[]) {
    int oc;
    char server_ip[STRING_SIZE] = "tejo.tecnico.ulisboa.pt";
    u_short server_port = 59000;
    list *msgservers_lst;

    // Treat options
    while ((oc = getopt(argc, argv, "i:p:")) != -1) { //Command-line args parsing, 'i' and 'p' args required for both
        switch (oc) {
            case 'i':
                strcpy(server_ip, optarg); //optarg has the string corresponding to oc value
                break;
            case 'p':
                server_port = (u_short)atoi(optarg);
                break;
            case ':':
                /* missing option argument */
                fprintf(stderr, "%s: option '-%c' requires an argument\n",
                        argv[0], optopt);
                break;
            case '?': //Left blank on purpose, help option
            default:
                usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    fprintf(stdout, KBLU "Identity Server:" KNRM " %s:%d\n", server_ip, server_port);

    char op[STRING_SIZE];
    char input_buffer[STRING_SIZE];

    msgservers_lst = fetch_servers(server_ip, server_port);
    if ( NULL == get_head(msgservers_lst) ){
        printf( KRED "error fetching servers, information not present or invalid\n" KNRM);
        return EXIT_FAILURE;
    }

    // Interactive loop
    while (1) {
        memset( op, '\0', sizeof(char)*STRING_SIZE ); //Fill strings with string terminator (\0)
        memset( input_buffer, '\0', sizeof(char)*STRING_SIZE-1 );

        fprintf(stdout, KGRN "Prompt > " KNRM);
        scanf("%s%*[ ]%126[^\t\n]" , op, input_buffer); // Grab word, then throw away space and finally grab until \n

        //User options input: show_servers, exit, publish message, show_latest_messages n;

        if (strcmp("show_servers", op) == 0) {
            print_list(msgservers_lst, print_server);
        } else if (strcmp("exit", op) == 0) {
            return EXIT_SUCCESS;
        } else if (strcmp("publish", op) == 0) {
        } else if (strcmp("show_latest_messages", op) == 0) {
        } else {
            fprintf(stderr, KRED "%s is an unknown operation\n" KNRM, op);
        }
    }

    free_list(msgservers_lst, free_server());
    return EXIT_SUCCESS;
}


