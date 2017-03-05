#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>

#include "utils.h"

void usage(char* name) {
    fprintf(stdout, "Example Usage: %s –n name –j ip -u upt –t tpt [-i siip] [-p sipt] [–m m] [–r r] \n", name);
    fprintf(stdout, "Arguments:\n"
            "\t-n\t\tserver name\n"
            "\t-j\t\tserver ip\n"
            "\t-u\t\tserver udp port\n"
            "\t-t\t\tserver tcp port\n"
            "\t-i\t\t[identity server ip (default:tejo.tecnico.ulisboa.pt)]\n"
            "\t-p\t\t[identity server port (default:59000)]\n"
            "\t-m\t\t[max server storage (default:200)]\n"
            "\t-r\t\t[register interval (default:10)]\n");
}

int main(int argc, char *argv[]) {
    int oc;
    char *name = NULL;
    char *ip = NULL;
    u_short udp_port = 0;
    u_short tcp_port = 0;

    char id_server_ip[STRING_SIZE] = "tejo.tecnico.ulisboa.pt";
    u_short id_server_port = 59000;

    int m = 200;
    int r = 10;

    srand(time(NULL));
    // Treat options
    while ((oc = getopt(argc, argv, "n:j:u:t:i:p:m:r:h")) != -1) { //Command-line args parsing, 'i' and 'p' args required for both
        switch (oc) {
            case 'n':
                name = (char *)malloc(strlen(optarg) + 1);
                strcpy(name, optarg); //optarg has the string corresponding to oc value
                break;
            case 'j':
                ip = (char *)malloc(strlen(optarg) + 1);
                strcpy(ip, optarg);
                break;
            case 'u':
                udp_port = (u_short)atoi(optarg);
                break;
            case 't':
                tcp_port = (u_short)atoi(optarg);
                break;
            case 'i':
                strcpy(id_server_ip, optarg);
                break;
            case 'p':
                id_server_port = (u_short)atoi(optarg);
                break;
            case 'm':
                m = atoi(optarg);
                break;
            case 'r':
                r = atoi(optarg);
                break;
            case 'h':
                usage(argv[0]);
                return EXIT_SUCCESS;
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

    fprintf(stdout, KBLU "Server Parameters:" KNRM " %s:%s:%d:%d\n", name, ip, udp_port, tcp_port);
    fprintf(stdout, KBLU "Identity Server:" KNRM " %s:%d\n", id_server_ip, id_server_port);

    char op[STRING_SIZE];
    char input_buffer[STRING_SIZE];

    // Interactive loop
    while (1) {
        memset( op, '\0', sizeof(char)*STRING_SIZE ); //Fill strings with string terminator (\0)
        memset( input_buffer, '\0', sizeof(char)*STRING_SIZE-1 );

        fprintf(stdout, KGRN "Prompt > " KNRM);
        scanf("%s%*[ ]%126[^\t\n]" , op, input_buffer); // Grab word, then throw away space and finally grab until \n

        //User options input: show_servers, exit, publish message, show_latest_messages n;

        if (strcmp("join", op) == 0) {
        } else if (strcmp("exit", op) == 0) {
            return EXIT_SUCCESS;
        } else if (strcmp("show_servers", op) == 0) {
        } else if (strcmp("show_messages", op) == 0) {
        } else {
            fprintf(stderr, KRED "%s is an unknown operation\n" KNRM, op);
        }
    }

    free(name);
    free(ip);
    return EXIT_SUCCESS;
}


