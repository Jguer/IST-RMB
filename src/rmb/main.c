#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>

#include "utils.h"
#include "server.h"
#include "message.h"
#include "utilmessage.h"

void usage(char* name) {
    fprintf(stdout, "Example Usage: %s [-i siip] [-p sipt]\n", name);
    fprintf(stdout, "Arguments:\n"
            "\t-i\t\t[server ip]\n"
            "\t-p\t\t[server port]\n");
}

int main(int argc, char *argv[]) {
    int oc;
    char server_ip[STRING_SIZE] = "tejo.tecnico.ulisboa.pt";
    /* u_short server_port = 59000; */
    char server_port[STRING_SIZE] = "59000";
    int err = -1;
    int fd = 0;
    int exit_code = EXIT_SUCCESS;
    char op[STRING_SIZE];
    char input_buffer[STRING_SIZE];

    server *sel_server   = NULL;
    list *msgservers_lst = NULL;
    list *message_list = NULL;

    srand(time(NULL));
    // Treat options
    while ((oc = getopt(argc, argv, "i:p:")) != -1) { //Command-line args parsing, 'i' and 'p' args required for both
        switch (oc) {
            case 'i':
                strcpy(server_ip, optarg); //optarg has the string corresponding to oc value
                break;
            case 'p':
                strcpy(server_port, optarg); //optarg has the string corresponding to oc value
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

    fprintf(stdout, KBLU "Identity Server:" KNRM " %s:%s\n", server_ip, server_port);
    struct addrinfo *id_server = get_server_address(server_ip, server_port);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == fd) {
        printf("error creating socket\n");
        exit_code = EXIT_FAILURE;
        goto PROGRAM_EXIT;
    }

    // Interactive loop
    while (true) {
        if (0 != err) {
            free_list(msgservers_lst, free_server);
            msgservers_lst = fetch_servers(fd, id_server);
            sel_server = select_server(msgservers_lst);
            if (NULL == sel_server) {
                fprintf(stderr, KGRN "No servers available\n" KNRM);
                sleep(5);
                continue;
            }
            if (-1 == err) {
                err = 0;
            }
        }

        if (0 == err) {
            memset( op, '\0', sizeof(char)*STRING_SIZE ); //Fill strings with string terminator (\0)
            memset( input_buffer, '\0', sizeof(char)*STRING_SIZE-1 );

            fprintf(stdout, KGRN "Prompt > " KNRM);
            scanf("%s%*[ ]%140[^\t\n]" , op, input_buffer);
            // Grab word, then throw away space and finally grab until \n
        }
        //User options input: show_servers, exit, publish message, show_latest_messages n;
        if (0 == strcmp("show_servers", op)) {
            print_list(msgservers_lst, print_server);
        } else if (0 == strcmp("exit", op) ) {
                goto PROGRAM_EXIT;
            return EXIT_SUCCESS;
        } else if (0 == strcmp("publish", op)) {
            if (0 == strlen(input_buffer)) {
                continue;
            }
            err = publish(fd, sel_server, input_buffer);
        } else if (0 == strcmp("show_latest_messages", op)) {
            free_list(message_list, free_message);
            message_list = get_latest_messages(fd, sel_server, atoi(input_buffer));
            print_list(message_list, print_message);
        } else {
            fprintf(stderr, KRED "%s is an unknown operation\n" KNRM, op);
        }
    }

PROGRAM_EXIT:
    freeaddrinfo(id_server);
    free_list(msgservers_lst, free_server);
    free_list(message_list, free_message);
    close(fd);
    return exit_code;
}


