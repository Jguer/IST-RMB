#include <time.h>
#include <errno.h>

#include "message.h"

void usage(char* name) {
    fprintf(stdout, "Example Usage: %s [-i siip] [-p sipt]\n", name);
    fprintf(stdout, "Arguments:\n"
            "\t-i\t\t[server ip]\n"
            "\t-p\t\t[server port]\n");
}

int main(int argc, char *argv[]) {
    char server_ip[STRING_SIZE] = "tejo.tecnico.ulisboa.pt";
    char server_port[STRING_SIZE] = "59000";

    srand(time(NULL));
    // Treat options
    int_fast8_t oc  = 0;
    while ((oc = getopt(argc, argv, "i:p:")) != -1) { //Command-line args parsing, 'i' and 'p' args required for both
        switch (oc) {
            case 'i':
                strncpy(server_ip, optarg, STRING_SIZE); //optarg has the string corresponding to oc value
                break;
            case 'p':
                strncpy(server_port, optarg, STRING_SIZE); //optarg has the string corresponding to oc value
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
    if (NULL == id_server) {
        fprintf(stderr, KRED "Unable to parse id server address from:\n %s:%s", server_ip, server_port);
        return EXIT_FAILURE;
    }

    char op[STRING_SIZE];
    char input_buffer[STRING_SIZE];

    uint_fast8_t exit_code = EXIT_SUCCESS;
    uint_fast8_t err = EXIT_SUCCESS;

    int_fast32_t fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct timeval timeout={3,0}; //set timeout for 3 seconds
    setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
    if (-1 == fd) {
        fprintf(stderr, KRED "Unable to create socket\n" KNRM);
        freeaddrinfo(id_server);
        return EXIT_FAILURE;
    }

    uint_fast8_t max_fd = fd; // Max fd number.
    uint_fast16_t msg_num = 0; // Number of messages asked
    int_fast32_t read_size = 0; // UDP receive size
    char *response_buffer = NULL;
    uint_fast16_t to_alloc = RESPONSE_SIZE;
    if (NULL == realloc(response_buffer, sizeof(char) * to_alloc)) {
        memory_error("Unable to malloc buffer\n");
    }

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);
    FD_SET(fd, &rfds);

    struct sockaddr_in server_addr = { 0 };
    socklen_t addr_len;

    list *msgservers_lst = fetch_servers(fd, id_server);
    server *sel_server = select_server(msgservers_lst);
    if (sel_server != NULL) {
        fprintf(stdout, KGRN "Prompt > " KNRM);
        fflush(stdout);
    }

    // Interactive loop
    while (true) {
        if (NULL == sel_server || err ) {
            fprintf(stderr, KYEL "No servers available. Sleeping 3s\n" KNRM);
            sleep(3);

            free_list(msgservers_lst, free_server);
            msgservers_lst = fetch_servers(fd, id_server);
            sel_server = select_server(msgservers_lst);
            if (sel_server != NULL) {
                fprintf(stdout, KGRN "Prompt > " KNRM);
                fflush(stdout);
            }
            continue;
        }

        int activity = select(max_fd + 1 , &rfds, NULL, NULL, NULL); //Select, threading function
        if(0 > activity){
            printf("error on select\n%d\n", errno);
            exit_code = EXIT_FAILURE;
            goto PROGRAM_EXIT;
        }

        if (FD_ISSET(fd, &rfds)) { //UDP receive
            addr_len = sizeof(server_addr);
            if (msg_num * RESPONSE_SIZE > to_alloc) {
                to_alloc = msg_num;
                if (NULL == realloc(response_buffer, sizeof(char) * to_alloc)) {
                    memory_error("Unable to realloc buffer\n");
                }
            }
            memset(response_buffer, '\0', sizeof(char) * to_alloc);

            read_size = recvfrom(fd, response_buffer, sizeof(response_buffer), 0,
                    (struct sockaddr *)&server_addr, &addr_len);

            if (-1 == read_size) {
                fprintf(stderr, KRED "Failed UPD receive from %s\n" KNRM, inet_ntoa(server_addr.sin_addr));
                goto UDP_END;
            }

            response_buffer[read_size] = '\0';
            puts(response_buffer);
UDP_END:
            fprintf(stdout, KGRN "Prompt > " KNRM);
            fflush(stdout);
        }

        if (FD_ISSET(STDIN_FILENO, &rfds)) { //Stdio input
            scanf("%s%*[ ]%140[^\t\n]" , op, input_buffer); // Grab word, then throw away space and finally grab until \n

            //User options input: show_servers, exit, publish message, show_latest_messages n;
            if (0 == strcmp("show_servers", op) || 0 == strcmp("0", op)) {
                print_list(msgservers_lst, print_server);
            } else if (0 == strcmp("publish", op) || 0 == strcmp("1", op)) {
                if (0 == strlen(input_buffer)) {
                    continue;
                }
                err = publish(fd, sel_server, input_buffer);
            } else if (0 == strcmp("show_latest_messages", op) || 0 == strcmp("2", op)) {
                msg_num = msg_num > (unsigned int)atoi(input_buffer) ? msg_num : (unsigned int)atoi(input_buffer);
                if (!msg_num) {
                    fprintf(stderr, KRED "%s is an invalid number\n" KNRM, input_buffer);
                    continue;
                }
                err = ask_for_messages(fd, sel_server, msg_num);
            } else if (0 == strcmp("exit", op) || 0 == strcmp("3", op)) {
                exit_code = EXIT_SUCCESS;
                goto PROGRAM_EXIT;
            } else {
                fprintf(stderr, KRED "%s is an unknown operation\n" KNRM, op);
            }

            memset(op, '\0', sizeof(char) * STRING_SIZE);
            memset(input_buffer, '\0', sizeof(char) * STRING_SIZE);

            fprintf(stdout, KGRN "Prompt > " KNRM);
            fflush(stdout);
        }
    }

PROGRAM_EXIT:
    freeaddrinfo(id_server);
    free(response_buffer);
    free_list(msgservers_lst, free_server);
    close(fd);
    return exit_code;
}
