#include <errno.h>
#include <sys/timerfd.h>

#include "identity.h"
#include "message.h"

void usage(char* name) {
    fprintf(stdout, "Example Usage: %s –n name –j ip -u upt –t tpt [-i siip] [-p sipt] [–m m] [–r r] %s \n", name, _VERBOSE_OPT_SHOW );
    fprintf(stdout, "Arguments:\n"
            "\t-n\t\tserver name\n"
            "\t-j\t\tserver ip\n"
            "\t-u\t\tserver udp port\n"
            "\t-t\t\tserver tcp port\n"
            "\t-i\t\t[identity server ip (default:tejo.tecnico.ulisboa.pt)]\n"
            "\t-p\t\t[identity server port (default:59000)]\n"
            "\t-m\t\t[max server storage (default:200)]\n"
            "\t-r\t\t[register interval (default:10)]\n"
            "%s", _VERBOSE_OPT_INFO);
}

void put_fd_set(int fd, fd_set *rfds){
    FD_SET(fd, rfds);
    return;
}
int is_fd_set(int fd, fd_set *rfds){
    return FD_ISSET(fd, rfds);
}

int main(int argc, char *argv[]) {
    int oc;
    char *name = NULL;
    char *ip = NULL;
    u_short udp_port = 0;
    u_short tcp_port = 0;

    char id_server_ip[STRING_SIZE] = "tejo.tecnico.ulisboa.pt";
    char id_server_port[STRING_SIZE] = "59000";

    int m = 200;
    int r = 10;
    bool is_join_complete = false;

    fd_set rfds;

    int tcp_listen_fd;   //TCP socket to accept connections
    int udp_global_fd;   //UDP global socket
    int udp_register_fd; //UDP socket for id server comms
    int timer_fd;        //Timer socket for reregister
    int max_fd;          //Max fd number.
    uint_fast8_t exit_code = EXIT_SUCCESS;

    char buffer[STRING_SIZE];

    int read_size;
    int err = 1;
    list *msgservers_lst = NULL;
    struct addrinfo *id_server;

    srand(time(NULL));
    // Treat options
    while ((oc = getopt(argc, argv, "n:j:u:t:i:p:m:r:h:v")) != -1) { //Command-line args parsing, 'i' and 'p' args required for both
        switch (oc) {
            case 'n':
                name = (char *)malloc(strlen(optarg) + 1);
                strncpy(name, optarg, strlen(optarg) + 1); //optarg has the string corresponding to oc value
                break;
            case 'j':
                ip = (char *)malloc(strlen(optarg) + 1);
                strncpy(ip, optarg, strlen(optarg) + 1);
                break;
            case 'u':
                udp_port = (u_short)atoi(optarg);
                break;
            case 't':
                tcp_port = (u_short)atoi(optarg);
                break;
            case 'i':
                strncpy(id_server_ip, optarg, strlen(optarg) + 1);
                break;
            case 'p':
                strncpy(id_server_port, optarg, strlen(optarg) + 1);
                break;
            case 'm':
                m = atoi(optarg);
                break;
            case 'r':
                r = atoi(optarg);
                break;
            case 'h':
                usage(argv[0]);
                exit_code = EXIT_FAILURE;
                goto PROGRAM_EXIT;
                break;
            case ':':
                /* missing option argument */
                fprintf(stderr, "%s: option '-%c' requires an argument\n",
                        argv[0], optopt);
                break;
            case 'v':
                _VERBOSE_OPT_CHECK;
            case '?':
            default:
                usage(argv[0]);
                exit_code = EXIT_FAILURE;
                goto PROGRAM_EXIT;
        }
    }

    if ((NULL == name) || (NULL == ip) || (0 == udp_port) || (0 == tcp_port)){
        printf("Required arguments not present\n");
        usage(argv[0]);
        exit_code = EXIT_FAILURE;
        goto PROGRAM_EXIT;
    }


    struct itimerspec new_timer = {{r,0}, {r,0}};
    server* host = new_server(name, ip, udp_port, tcp_port); //host parameters

    /* Start Timer */
    timer_fd = timerfd_create (CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timer_fd == -1) {
        printf(KRED "Unable to create timer.\n" KNRM);
    }

    udp_global_fd = init_udp(host); //Initiates UDP connection
    tcp_listen_fd = init_tcp(host); //Initiates TCP connection

    if ( 0 >= udp_global_fd || 0 >= tcp_listen_fd ){
        fprintf(stdout, KYEL "Cannot initializate UDP and/or TCP connections\n" KGRN
                "Check the ip address and ports\n" KNRM) ;
        exit_code = EXIT_FAILURE;
        goto PROGRAM_EXIT;
    }
    matrix msg_matrix = create_matrix(m);

    fprintf(stdout, KBLU "Server Parameters:" KNRM " %s:%s:%d:%d\n"
            KBLU "Identity Server:" KNRM " %s:%s\n"
            KGRN "Prompt > " KNRM
            ,name, ip, udp_port, tcp_port, id_server_ip, id_server_port);
    fflush(stdout);

    // Processing Loop
    while(true) {
        //Clear the set
        FD_ZERO(&rfds);

        //Add tcp_listen_fd, udp_global_fd and stdio to the socket set
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(timer_fd, &rfds);
        FD_SET(udp_global_fd, &rfds);
        FD_SET(tcp_listen_fd, &rfds);

        max_fd = tcp_listen_fd > udp_global_fd ? tcp_listen_fd : udp_global_fd;
        max_fd = timer_fd > max_fd ? timer_fd : max_fd;


        //Removes the bad servers and sets the good in fd_set rfds.
        max_fd = remove_bad_servers(msgservers_lst, host, max_fd, &rfds, put_fd_set);

        //wait for one of the descriptors is ready
        int activity = select(max_fd + 1 , &rfds, NULL, NULL, NULL); //Select, threading function
        if(0 > activity){
            if ( _VERBOSE_TEST ) printf("error on select\n%d\n", errno);
            exit_code = EXIT_FAILURE;
            goto PROGRAM_EXIT;
        }

        if (FD_ISSET(timer_fd, &rfds)) { //if the timer is triggered
            update_reg(udp_register_fd, id_server);
            timerfd_settime (timer_fd, 0, &new_timer, NULL);
        }


        if ( 0 != tcp_new_comm(msgservers_lst, tcp_listen_fd, &rfds, is_fd_set)) { //Starts connection for incoming requests
            exit_code = EXIT_FAILURE;
            goto PROGRAM_EXIT;
        }

        //if something happened on other socket we must process it
        if (FD_ISSET(STDIN_FILENO, &rfds)) { //Stdio input
            read_size = read( 0, buffer, STRING_SIZE);
            if ( 0 == read_size ) {
                if ( _VERBOSE_TEST ) printf("error reading from stdio\n");
                exit_code = EXIT_FAILURE;
                goto PROGRAM_EXIT;
            }

            buffer[read_size-1] = '\0'; //switches \n to \0

            //User options input: show_servers, exit, publish message, show_latest_messages n;
            //User options input: show_servers, exit, publish message, show_latest_messages n;
            if (strcasecmp("join", buffer) == 0 || 0 == strcmp("0", buffer)) {
                if (false == is_join_complete) {
                    //Register on idServer
                    id_server = reg_server(&udp_register_fd, host, id_server_ip, id_server_port);
                    if(id_server == NULL) {
                        if ( _VERBOSE_TEST ) printf( KYEL "error registing on id_server\n" KNRM);
                        exit_code = EXIT_FAILURE;
                        goto PROGRAM_EXIT;
                    }

                    err = timerfd_settime (timer_fd, 0, &new_timer, NULL);
                    if (-1 == err) {
                        if ( _VERBOSE_TEST ) printf(KRED "Unable to set timer\n" KNRM);
                        exit_code = EXIT_FAILURE;
                        goto PROGRAM_EXIT;
                    }

                    //Get the message servers list
                    msgservers_lst = fetch_servers(udp_register_fd, id_server);
                    node *head = get_head(msgservers_lst);
                    if (NULL == head) {
                        if ( _VERBOSE_TEST ) printf( KRED "error fetching servers, information not present or invalid\n" KNRM);
                        return EXIT_FAILURE;
                    }

                    if (0 != join_to_old_servers(msgservers_lst, host)) {
                        exit_code = EXIT_FAILURE;
                        goto PROGRAM_EXIT;
                    }

                    is_join_complete = true;
                }
                else {
                    printf(KGRN "Already joined!\n" KNRM);
                }
            } else if (0 == strcasecmp("show_servers", buffer) || 0 == strcmp("1", buffer)) {
                if (msgservers_lst != NULL && 0 != get_list_size(msgservers_lst)) print_list(msgservers_lst, print_server);
                else printf("No registred servers\n");
            } else if (0 == strcasecmp("show_messages", buffer) || 0 == strcmp("2", buffer)) {
                print_matrix(msg_matrix, print_message);
            } else if (0 == strcasecmp("exit", buffer) || 0 == strcmp("3", buffer)) {
                return EXIT_SUCCESS;
            } else {
                fprintf(stderr, KRED "%s is an unknown operation\n" KNRM, buffer);
            }

            fprintf(stdout, KGRN "Prompt > " KNRM);
            fflush(stdout);
        }

        if (FD_ISSET(udp_global_fd, &rfds)){ //UDP communications handling
            handle_client_comms(udp_global_fd, m, msg_matrix);
        }
        tcp_fd_handle( msgservers_lst, msg_matrix, &rfds, is_fd_set ); //TCP already started comunications handling
    }

PROGRAM_EXIT:
    free(name);
    free(ip);
    return exit_code;
}

