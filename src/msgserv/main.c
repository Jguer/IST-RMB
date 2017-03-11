#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>
#include <time.h>
#include <netdb.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "identity.h"
#include "server.h"
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
    char id_server_port[STRING_SIZE] = "59000";

    int m = 200;
    int r = 10;

    fd_set rfds;

    int tcp_listen_fd; //TCP socket to accept connections
    int udp_global_fd; //UDP global socket
    int udp_register_fd; //UDP socket for id server comms
    int max_fd;        //Max fd number.

    char buffer[STRING_SIZE];
    char message[STRING_SIZE];
    int read_size;
    int err = 1;
    list *msgservers_lst = NULL;
    node *head = NULL;

    struct addrinfo * id_server;

    srand(time(NULL));
    // Treat options
    int flag_n = 0;
    int flag_j = 0;
    int flag_u = 0;
    int flag_t = 0;
    while ((oc = getopt(argc, argv, "n:j:u:t:i:p:m:r:h")) != -1) { //Command-line args parsing, 'i' and 'p' args required for both
        switch (oc) {
            case 'n':
                name = (char *)malloc(strlen(optarg) + 1);
                strcpy(name, optarg); //optarg has the string corresponding to oc value
                flag_n = 1;
                break;
            case 'j':
                ip = (char *)malloc(strlen(optarg) + 1);
                strcpy(ip, optarg);
                flag_j = 1;
                break;
            case 'u':
                udp_port = (u_short)atoi(optarg);
                flag_u = 1;
                break;
            case 't':
                tcp_port = (u_short)atoi(optarg);
                flag_t = 1;
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
            case '?':
            default:
                usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    if ( (flag_n && flag_j && flag_u && flag_t) != 1 ){
        printf("Required arguments not present\n");
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    fprintf(stdout, KBLU "Server Parameters:" KNRM " %s:%s:%d:%d\n", name, ip, udp_port, tcp_port);
    fprintf(stdout, KBLU "Identity Server:" KNRM " %s:%s\n", id_server_ip, id_server_port);

    server* host = new_server(name, ip, udp_port, tcp_port); //host parameters

    udp_global_fd = init_udp( host ); //Initiates UDP connection
    tcp_listen_fd = init_tcp( host ); //Initiates TCP connection

    if ( 0 >= udp_global_fd || 0 >= tcp_listen_fd ){
        fprintf(stdout, KYEL "Cannot initializate UDP and/or TCP connections\n" KGRN
         "Check the ip address and ports\n" KNRM) ;
        return EXIT_FAILURE;
    }

    while(1){
        int addrlen;

        //Clear the set
        FD_ZERO(&rfds);

        //Add tcp_listen_fd, udp_global_fd and stdio to the socket set
        FD_SET(0, &rfds);
        FD_SET(udp_global_fd, &rfds);
        FD_SET(tcp_listen_fd, &rfds);
        
        max_fd = tcp_listen_fd; //The master is the first socket
        if(udp_global_fd > max_fd)   max_fd = udp_global_fd;


        if(msgservers_lst != NULL){ //Add child sockets to the socket set
            node *aux_node;
            for ( aux_node = get_head(msgservers_lst); 
                    aux_node != NULL ; 
                    aux_node = get_next_node(aux_node)) {

                int processing_fd;
                node *next_node;
                server * next_server;

                if( NULL != ( next_node = get_next_node(aux_node) ) ){
                    if( NULL != ( next_server = (server *)get_node_item(next_node) ) ){
                        if( -1 == get_fd(next_server) ){
                            //Delete next node
                            remove_next_node(aux_node, next_node, free_server);
                        }
                    }
                }

                processing_fd = get_fd((server *)get_node_item(aux_node)); //file descriptor/socket

                //if the socket is valid then add to the read list
                if(processing_fd > 0)  FD_SET(processing_fd, &rfds);

                //The highest file descriptor is saved for the select fnc
                if(processing_fd > max_fd) max_fd = processing_fd;
            }
        }

        //wait for one of the descriptors is ready
        int activity;
        activity = select( max_fd + 1 , &rfds, NULL, NULL, NULL); //Select, threading function
        if(activity < 0){
            printf("error on select\n%d\n", errno);
            return EXIT_FAILURE;
        }

        
        if (FD_ISSET(tcp_listen_fd, &rfds)){ //if something happens on tcp_listen_fd create and allocate new socket

            struct sockaddr_in tcp_newserv_info;
            int tcp_newserv_fd;

            addrlen = sizeof( tcp_newserv_info );

            //Create new socket, new_fd
            if ( (tcp_newserv_fd = accept(tcp_listen_fd, (struct sockaddr *)&tcp_newserv_info,
                (socklen_t*)&addrlen))<0 ){

                printf("error accepting communication\n");
                return EXIT_FAILURE;
            }

            //Send message like SGET_MESSAGES to request messages
            strcpy(message, "SGET_MESSAGES\n");
            if( send(tcp_newserv_fd, message, strlen(message), 0) != strlen(message) ) {

                printf("error sending communication\n");
                return EXIT_FAILURE;
            }

            //add new socket to list of sockets
            server * tcp_newserv = new_server( NULL ,inet_ntoa(tcp_newserv_info.sin_addr), 0, ntohs(tcp_newserv_info.sin_port) );
            set_fd(tcp_newserv, tcp_newserv_fd);
            push_item_to_list( msgservers_lst, tcp_newserv);        

        }

        //if something happened on other socket we must process it
        if ( FD_ISSET(0, &rfds) ){ //Stdio input

            read_size = read( 0, buffer, STRING_SIZE);
            if ( 0 == read_size )
            {
                printf("error reading from stdio\n");
                return EXIT_FAILURE;
            }

            buffer[read_size-1] = '\0'; //switches \n to \0

                //User options input: show_servers, exit, publish message, show_latest_messages n;
            if (strcmp("join", buffer) == 0) {

                //Register on idServer
                id_server = reg_server( &udp_register_fd, host, id_server_ip, id_server_port);
                if(id_server == NULL){
                    printf( KYEL "error registing on id_server\n" KNRM);
                }

                //Get the message servers list
                msgservers_lst = fetch_servers(udp_register_fd, id_server);
                head = get_head(msgservers_lst);
                if (NULL == head) {
                    printf( KRED "error fetching servers, information not present or invalid\n" KNRM);
                }

                //Start connections with the new listed servers
                    //Protect from connecting to our host
                    //create one socket for each
                    //set_fd in server list
                    //getadrrinfo
                    //connect
                    //write SGET_MESSAGES

            } else if (strcmp("exit", buffer) == 0) {
                return EXIT_SUCCESS;
            } else if (strcmp("show_servers", buffer) == 0) {
                if (msgservers_lst != NULL) print_list(msgservers_lst, print_server);
                else printf("No registred servers\n");
            } else if (strcmp("show_messages", buffer) == 0) {
            } else {
                fprintf(stderr, KRED "%s is an unknown operation\n" KNRM, buffer);
            }
        }

        if ( FD_ISSET(udp_global_fd, &rfds) ){ //UDP communications handling

            struct sockaddr_in udpaddr;

            addrlen = sizeof(udpaddr);

            read_size = recvfrom(udp_global_fd, buffer, sizeof(buffer), 0,
                            (struct sockaddr *)&udpaddr, (socklen_t*)&addrlen);

            if (read_size == -1) {
                printf("udp receive error\n");
                return EXIT_FAILURE;
            }

            buffer[read_size]='\0';
            printf( "UDP -> echoing: %s to %s:%hu\n", buffer, inet_ntoa(udpaddr.sin_addr),
                ntohs(udpaddr.sin_port) );

            read_size = sendto(udp_global_fd, buffer, strlen(buffer)+1, 0, 
                    (struct sockaddr*)&udpaddr, addrlen);

            if (read_size == -1) {
                printf("send error\n");
                return EXIT_FAILURE;
            }
        }

        if(msgservers_lst != NULL ){ //TCP sockets already connected handling
            
            node *aux_node;
            for ( aux_node = get_head(msgservers_lst); 
                    aux_node != NULL ; 
                    aux_node = get_next_node(aux_node)) {

                int processing_fd;

                processing_fd = get_fd( (server *)get_node_item(aux_node) ); //file descriptor/socket

                if ( FD_ISSET(processing_fd , &rfds) ){

                    read_size = read( processing_fd, buffer, STRING_SIZE );

                    if ( 0 == read_size ){
                        
                        //The server disconnected, put fd equal to -1 (FD_INVALID)
                        close(processing_fd);
                        set_fd( (server *)get_node_item(aux_node), -1 );   
                        
                    }
                    else{

                        //Echo back the message that came in / INPLEMENT DATA TREATMENT
                        buffer[read_size] = '\0';
                        if( send(processing_fd, buffer, strlen(buffer), 0) != strlen(buffer) ) {
                            printf("error sending communication\n");
                            return EXIT_FAILURE;
                        }
                    }
                }
            }
        }
    }
}


