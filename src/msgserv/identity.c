#include "identity.h"

#define REQUEST "GET_SERVERS"
static char REG_MESSAGE[RESPONSE_SIZE];
struct addrinfo *id_server = NULL;

int init_tcp(server host) {
    int master_fd;
    struct sockaddr_in tcpaddr = {0, .sin_port = 0};
    struct timeval tv = {.tv_sec = 30, .tv_usec= 0};
    void (*old_handler)(int);   //interrupt handler

    master_fd = socket(AF_INET, SOCK_STREAM, 0);  //Create master socket
    if (master_fd == -1) {
        if (_VERBOSE_TEST) printf( KRED "error creating socket\n" KNRM );
        return -1;
    }
    setsockopt(master_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));

    tcpaddr.sin_family = AF_INET;
    tcpaddr.sin_addr.s_addr = inet_addr(get_ip_address(host));
    tcpaddr.sin_port = htons(get_tcp_port(host));

    if (0 != bind(master_fd, (struct sockaddr*)&tcpaddr,
                sizeof(tcpaddr))) { //Bind socket to the PORT_TCP defined

        if (_VERBOSE_TEST) printf(KRED "error bind failed\n" KNRM);
        return -1;
    }

    if (-1 == listen(master_fd, MAX_PENDING)) { //Specify MAX_PENDING connections to master socket
        if (_VERBOSE_TEST) printf(KRED "error on setting listen\n" KNRM);
        return -1;
    }

    if ((old_handler = signal(SIGPIPE,SIG_IGN)) == SIG_ERR) {
        if (_VERBOSE_TEST) printf(KRED "error protecting from SIGPIPE\n" KNRM);
        return -1;
    }

    return master_fd;

}

int init_udp(server host) {
    int u_fd;
    struct sockaddr_in udpaddr = {0 , .sin_port = 0};
    struct timeval tv = {.tv_sec = 30, .tv_usec= 0};

    u_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (u_fd==-1) {
        if (_VERBOSE_TEST) printf( KRED "error creating socket\n" KNRM);
        return -1;
    }

    setsockopt(u_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));

    udpaddr.sin_family = AF_INET;
    udpaddr.sin_addr.s_addr = inet_addr(get_ip_address(host));
    udpaddr.sin_port = htons(get_udp_port(host));

    if (0 != bind(u_fd, (struct sockaddr*)&udpaddr,
            sizeof(udpaddr))) {

        if (_VERBOSE_TEST) printf( KRED "error bind failed\n" KNRM);
        return -1;
    }

    return u_fd;
}

struct addrinfo *reg_server(int_fast16_t *fd, server host ,char *ip_name, char *udp_port) {
    struct addrinfo *id_server_info = get_server_address(ip_name, udp_port);
    if (!id_server_info) {
            return NULL;
    }
    struct timeval tv = {.tv_sec = 3, .tv_usec= 0};

    int nwritten;

    if (!id_server_info) return NULL;

    *fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (*fd <= 0) {
        if (_VERBOSE_TEST) printf(KRED "error creating udp socket for registry\n" KNRM);
        return NULL;
    }

    setsockopt(*fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));

    if (0 > sprintf( REG_MESSAGE, "%s %s;%s;%d;%d\n", JOIN_STRING, get_name(host),
                get_ip_address(host), get_udp_port(host), get_tcp_port(host))) return NULL;

    nwritten = sendto(*fd, REG_MESSAGE, strlen(REG_MESSAGE), 0,
            id_server_info->ai_addr, id_server_info->ai_addrlen);

    if (nwritten == -1) {
        if ( _VERBOSE_TEST ) fprintf(stderr, KYEL "unable to register\n" KNRM);
    }

    return id_server_info;
}

int update_reg(int fd, struct addrinfo* id_server_info) {
    int n = sendto(fd, REG_MESSAGE, strlen(REG_MESSAGE) + 1, 0,
            id_server_info->ai_addr, id_server_info->ai_addrlen);

    if (n == -1) {
        if (_VERBOSE_TEST) fprintf(stderr, KYEL "unable to update register\n" KNRM);
    }

    return 0;

}

int send_initial_comm(int processing_fd) {
    int status = 1;
    char to_send[STRING_SIZE];
    sprintf(to_send, "SGET_MESSAGES\n");
    if((unsigned int)send(processing_fd, to_send, strlen(to_send), 0) != strlen("SGET_MESSAGES\n")) {
        if (_VERBOSE_TEST) printf("error sending initial communication\n");
        status = 0;
    }

    return ((1 == status) ? processing_fd : (-1));
}

int connect_to_old_server(server old_server, bool is_comm_sent) {
    int processing_fd;
    int status = 1; //false
    struct timeval tv = {.tv_sec = 30, .tv_usec= 0};

    if (-2 == get_fd(old_server)) {
        // create new comunication
        processing_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (-1 == processing_fd) {
            if (_VERBOSE_TEST) printf(KRED "error creating socket\n" KNRM);
            status = -1; //fatal error
            return status;
        }
        setsockopt(processing_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));

        char portitoa[STRING_SIZE];
        if (0 > sprintf(portitoa, "%hu", get_tcp_port(old_server))) {
            if (_VERBOSE_TEST) printf(KRED "error converting u_short to string\n" KNRM);
            status = -1; //fatal error
            return status;
        }
        struct addrinfo *res = get_server_address_tcp( get_ip_address(old_server),
            portitoa);
        if (!res) {
            processing_fd = -1;
            status = 1; //false
        }
        else if (-1 == connect(processing_fd, res->ai_addr, res->ai_addrlen)) {

            if (_VERBOSE_TEST) printf( KYEL "cant connect to:%s:[%s]\n" KNRM, get_ip_address(old_server),
                portitoa); //Connect return Failure
            close(processing_fd);
            processing_fd = -1;
            status = 1; //false

        }                                       //Sends only a request for one of the initial servers
        else if (!is_comm_sent) {                            //Connect returns success
            //Send message like SGET_MESSAGES to request messages
            /* printf(KGRN "Sending new connection to:"KNRM"%s\n", get_name(old_server)); */
            processing_fd = send_initial_comm( processing_fd);
            if ( processing_fd == -1) status = 1; //false
            else{
                status = 0; //true
            }
        }

        freeaddrinfo(res);
        set_fd(old_server, processing_fd);
    }

    if(1 == status && true == is_comm_sent) status = 0; //if comm is already sent to one is good

    return status;
}

int join_to_old_servers(list msgservers_list , server host) {
    bool is_comm_sent = false;
    node aux_node = NULL;

    for (aux_node = get_head(msgservers_list); //Initialize comms with one new server
    aux_node != NULL;
    aux_node = get_next_node(aux_node)) {
        if (different_servers((server )get_node_item(aux_node), host)) {
            int status_check = connect_to_old_server((server)get_node_item(aux_node), is_comm_sent);
            if (0 == status_check) is_comm_sent = true;
            else if (1 == status_check) is_comm_sent = false;
            else return -1; //Fatal error
        }
    }

    if (!is_comm_sent) {
        printf(KYEL "\nNo connectable servers present: " KGRN "Wait mode\n" KNRM );
        fprintf(stdout, KGRN "\nPrompt > " KNRM);
        fflush(stdout);
    }

    return 0; //Success
}


int remove_bad_servers(list servers_list, server host, int max_fd, fd_set *rfds, void (*SET_FD)(int, fd_set *)) {
    if(servers_list != NULL) { //Add child sockets to the socket set
        node aux_node;
        if (NULL != (aux_node = get_head( servers_list ))) {

            if (-1 == get_fd((server )get_node_item(aux_node))) { //1 node erasement
                remove_head(servers_list, free_server);
            } else if (!different_servers((server )get_node_item(aux_node),host)) {
                remove_head(servers_list, free_server);
            }

            for (aux_node = get_head(servers_list);
            aux_node != NULL ;
            aux_node = get_next_node(aux_node)) {

                int processing_fd;
                node next_node;

                if (NULL != (next_node = get_next_node(aux_node))) {
                    if (-1 == get_fd((server )get_node_item(next_node))) {
                        //Delete next node
                        remove_next_node(servers_list, aux_node, free_server);
                    } else if ( 0 == different_servers((server )get_node_item(next_node),host)) {
                        remove_next_node(servers_list, aux_node, free_server);
                    }
                }

                processing_fd = get_fd((server )get_node_item(aux_node)); //file descriptor/socket

                //if the socket is valid then add to the read list
                if(0 < processing_fd) (*SET_FD)(processing_fd, rfds);

                //The highest file descriptor is saved for the select fnc
                max_fd = processing_fd > max_fd ? processing_fd: max_fd;
            }
        }
    }

    return max_fd;
}

int tcp_new_comm(list servers_list, int listen_fd, fd_set *rfds, int (*STAT_FD)(int, fd_set *)) {
    if ((*STAT_FD)(listen_fd, rfds)) { //if something happens on tcp_listen_fd create and allocate new socket
        struct timeval tv = {.tv_sec = 30, .tv_usec= 0};
        struct sockaddr_in newserv_info;
        int newserv_fd;

        int addrlen = sizeof(newserv_info);

        //Create new socket, new_fd
        if ((newserv_fd = accept(listen_fd, (struct sockaddr *)&newserv_info,
            (socklen_t*)&addrlen))<0 ) {

            if (_VERBOSE_TEST) printf("error accepting communication\n");
            return -1; //Fatal error
        }
        setsockopt(newserv_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));

        //add new socket to list of sockets
        server newserv = new_server("Inbound Server",inet_ntoa(newserv_info.sin_addr),0 , ntohs( newserv_info.sin_port ) );
        set_fd(newserv, newserv_fd);
        set_connected(newserv, 1);
        push_item_to_list(servers_list, newserv);
    }

    return 0;//everthing is good
}

// get_servers asks the identity server for the server list.
// Returns raw data.
char *get_servers(int fd) {
    ssize_t n = 0;
    char *return_string = NULL;
    char response[RESPONSE_SIZE] = {'\0'};

    n = sendto(fd, REQUEST, strlen(REQUEST) + 1, 0,
            id_server->ai_addr, id_server->ai_addrlen);

    if (0 > n) {
        if (_VERBOSE_TEST) fprintf(stderr, KYEL "unable to send\n" KNRM);
        return NULL;
    }

    time_t control_time = time(NULL);
    time_t request_time = time(NULL);
    
    while(true){
        n = recvfrom(fd, response, RESPONSE_SIZE, 0,
                id_server->ai_addr,
                &id_server->ai_addrlen);
    
        if (0 > n) {
            
            if (difftime(time(NULL), control_time) > 10){ //After ten seconds quit
                fprintf(stderr, KYEL "Identity Server doesn't answer\n" KNRM);
            } else if (difftime(time(NULL), request_time) > 2){ //Each three seconds send a new request
                request_time = time(NULL);
                if (0 != update_reg(fd, id_server)){
                    return NULL;
                }
                continue;
            } else if (_VERBOSE_TEST){ 
                fprintf(stderr, KYEL "unable to receive\n" KNRM);
            }
            return NULL;
        } else {
            return_string = (char *)malloc((n+1) * sizeof(char));
            strncpy(return_string, response, strlen(response) + 1);
            return return_string; //Dirty Pointer
        }
    }
}

uint_fast8_t parse_servers(int_fast16_t udp_register_fd, list msgsrv_list) {
	char *response;

	response = get_servers(udp_register_fd); //Show server will return NULL on disconnection
    if (!response) {
        return EXIT_FAILURE;
    }

    char *separated_info;
    char step_mem_name[STRING_SIZE]; //To define later
    char step_mem_ip_addr[STRING_SIZE];
    int  sscanf_state = 0;
    u_short step_mem_udp_port;
    u_short step_mem_tcp_port;

    separated_info = strtok(response, "\n"); //Gets the first info, stoping at newline
    if (0 != strcmp(separated_info, "SERVERS")){
        free(response);
        return EXIT_FAILURE;
    }
    separated_info = strtok(NULL, "\n");

    while (NULL != separated_info) { //Proceeds getting info and treating
        sscanf_state = sscanf(separated_info, "%[^;];%[^;];%hu;%hu",step_mem_name, step_mem_ip_addr,
            &step_mem_udp_port, &step_mem_tcp_port);//Separates info and saves it in variables

        if (4 != sscanf_state) {
             if (true == is_verbose()) fprintf(stdout, KRED "error processing id server data. data is invalid or corrupt\n" KNRM);
             continue;
        }

        server alloc_server = new_server(step_mem_name ,
            step_mem_ip_addr, step_mem_udp_port, step_mem_tcp_port);

        set_fd(alloc_server, -2);
        set_connected(alloc_server, 0);
        push_item_to_list(msgsrv_list, alloc_server); //Pushes to list

        separated_info = strtok(NULL, "\n");//Gets new info
    }

    free(response);
    return EXIT_SUCCESS;
}

uint_fast8_t handle_join(list msgsrv_list, int_fast16_t *udp_register_fd, server host, char *id_server_ip, char *id_server_port) {
    uint_fast8_t err = 0;

    id_server = reg_server(udp_register_fd, host, id_server_ip, id_server_port);
    if (id_server == NULL) {
        if (_VERBOSE_TEST) printf( KYEL "error registering on id_server\n" KNRM);
        return err = 1;
    }

    if (0 != parse_servers(*udp_register_fd, msgsrv_list)) {
        return err = 2;
    }

    if (0 != join_to_old_servers(msgsrv_list, host)) {
        return err = 3;
    }

    return EXIT_SUCCESS;
}
