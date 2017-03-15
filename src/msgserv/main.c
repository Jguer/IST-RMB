#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "identity.h"
#include "server.h"
#include "utils.h"
#include "utilmessage.h"
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

    char buffer[STRING_SIZE];
    int read_size;
    int err = 1;
    list *message_lst = NULL;
    list *msgservers_lst = NULL;

    struct addrinfo * id_server;

    srand(time(NULL));
    // Treat options
    while ((oc = getopt(argc, argv, "n:j:u:t:i:p:m:r:h:v")) != -1) { //Command-line args parsing, 'i' and 'p' args required for both
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
                strcpy(id_server_port, optarg);
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
            case 'v':
                _VERBOSE_OPT_CHECK;
            case '?':
            default:
                usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    if ((NULL == name) || (NULL == ip) || (0 == udp_port) || (0 == tcp_port)){
        printf("Required arguments not present\n");
        usage(argv[0]);
        return EXIT_FAILURE;
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
        return EXIT_FAILURE;
    }

    fprintf(stdout, KBLU "Server Parameters:" KNRM " %s:%s:%d:%d\n"
            KBLU "Identity Server:" KNRM " %s:%s\n"
            KGRN "Prompt > " KNRM
            ,name, ip, udp_port, tcp_port, id_server_ip, id_server_port);
    fflush(stdout);

    // Processing Loop
    while(true) {
        int addrlen;

        //Clear the set
        FD_ZERO(&rfds);

        //Add tcp_listen_fd, udp_global_fd and stdio to the socket set
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(timer_fd, &rfds);
        FD_SET(udp_global_fd, &rfds);
        FD_SET(tcp_listen_fd, &rfds);

        max_fd = tcp_listen_fd > udp_global_fd ? tcp_listen_fd : udp_global_fd;
        max_fd = timer_fd > tcp_listen_fd ? timer_fd : tcp_listen_fd;         

      
        //Removes the bad servers and sets the good in fd_set rfds.
        max_fd = remove_bad_servers( msgservers_lst, host, max_fd, &rfds, put_fd_set); 

        //wait for one of the descriptors is ready
        int activity = select( max_fd + 1 , &rfds, NULL, NULL, NULL); //Select, threading function
        if(0 > activity){
            if ( _VERBOSE_TEST ) printf("error on select\n%d\n", errno);
            return EXIT_FAILURE;
        }

        if (FD_ISSET(timer_fd, &rfds)) { //if the timer is triggered
            update_reg(udp_register_fd, id_server);
            err = timerfd_settime (timer_fd, 0, &new_timer, NULL);
        }


        if ( 0 != tcp_new_comm(msgservers_lst, tcp_listen_fd, &rfds, is_fd_set ) ){ //Starts connection for incoming requests
            return EXIT_FAILURE;
        }

        //if something happened on other socket we must process it
        if (FD_ISSET(STDIN_FILENO, &rfds)) { //Stdio input
            read_size = read( 0, buffer, STRING_SIZE);
            if ( 0 == read_size )
            {
                if ( _VERBOSE_TEST ) printf("error reading from stdio\n");
                return EXIT_FAILURE;
            }

            buffer[read_size-1] = '\0'; //switches \n to \0

            //User options input: show_servers, exit, publish message, show_latest_messages n;
            //User options input: show_servers, exit, publish message, show_latest_messages n;
            if ( strcmp("join", buffer) == 0 ) {
                if (false == is_join_complete ){
                    //Register on idServer
                    id_server = reg_server( &udp_register_fd, host, id_server_ip, id_server_port);
                    if(id_server == NULL){
                        if ( _VERBOSE_TEST ) printf( KYEL "error registing on id_server\n" KNRM);
                        return EXIT_FAILURE;
                    }

                    err = timerfd_settime (timer_fd, 0, &new_timer, NULL);
                    if (-1 == err){
                        if ( _VERBOSE_TEST ) printf(KRED "Unable to set timer\n" KNRM);
                        return EXIT_FAILURE;
                    }

                    //Get the message servers list
                    msgservers_lst = fetch_servers(udp_register_fd, id_server);
                    node *head = get_head(msgservers_lst);
                    if (NULL == head) {
                        if ( _VERBOSE_TEST ) printf( KRED "error fetching servers, information not present or invalid\n" KNRM);
/*>>>>>>>>>>>>>>>>>>*/  return EXIT_FAILURE; //Change with try again option
                    }

                    if ( 0 != join_to_old_servers(msgservers_lst, host) ){
                        return EXIT_FAILURE;
                    }

                    is_join_complete = true;
                }
                else {
                    printf(KGRN "Already joined!\n" KNRM);
                }

            } else if (strcmp("exit", buffer) == 0) {
                return EXIT_SUCCESS;
            } else if (strcmp("show_servers", buffer) == 0) {
                if (msgservers_lst != NULL && 0 != get_list_size(msgservers_lst)) print_list(msgservers_lst, print_server);
                else printf("No registred servers\n");
            } else if (strcmp("show_messages", buffer) == 0) {
                print_list(message_lst, print_message);
            } else {
                fprintf(stderr, KRED "%s is an unknown operation\n" KNRM, buffer);
            }

            fprintf(stdout, KGRN "Prompt > " KNRM);
            fflush(stdout);
        }

        if (FD_ISSET(udp_global_fd, &rfds)){ //UDP communications handling
            struct sockaddr_in udpaddr;

            addrlen = sizeof(udpaddr);

            read_size = recvfrom(udp_global_fd, buffer, sizeof(buffer), 0,
                    (struct sockaddr *)&udpaddr, (socklen_t*)&addrlen);

            if (-1 == read_size) {
                if ( _VERBOSE_TEST ) printf("udp receive error\n");
                return EXIT_FAILURE;
            }

            buffer[read_size]='\0';
            if ( _VERBOSE_TEST ) printf( "UDP -> echoing: %s to %s:%hu\n", buffer, inet_ntoa(udpaddr.sin_addr),
                ntohs(udpaddr.sin_port) );

            read_size = sendto(udp_global_fd, buffer, strlen(buffer)+1, 0,
                (struct sockaddr*)&udpaddr, addrlen);

            if (-1 == read_size) {
                if ( _VERBOSE_TEST ) printf("error sending communication UDP\n");
                return EXIT_FAILURE;
            }
        }

        tcp_fd_handle( msgservers_lst, NULL, &rfds, is_fd_set ); //TCP already started comunications handling
    }
}

