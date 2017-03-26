#include "identity.h"

#define REQUEST "GET_SERVERS"

int init_program(struct addrinfo *id_server, int_fast32_t *outgoing_fd,
	int_fast32_t *binded_fd, list *msgservers_lst, server *sel_server,
	struct itimerspec *new_timer, int_fast32_t *timer_fd)
	{

    *outgoing_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == *outgoing_fd) {
        fprintf(stderr, KRED "Unable to create socket\n" KNRM);
        return 1;
    }

    *binded_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == *binded_fd) {
        fprintf(stderr, KRED "Unable to create socket\n" KNRM);
        return 1;
    }


    *msgservers_lst = fetch_servers(*outgoing_fd, id_server);
    *sel_server = select_server(*msgservers_lst);

    //Bind socket to any port on the client ip
    struct sockaddr_in serveraddr;
    memset((void*)&serveraddr,(int)'\0',
    sizeof(serveraddr));
    serveraddr.sin_family= AF_INET;
    serveraddr.sin_addr.s_addr= htonl(INADDR_ANY);
    serveraddr.sin_port= htons((u_short)0); //INPORT_ANY

    if (-1 == bind(*binded_fd,(struct sockaddr*)&serveraddr,sizeof(serveraddr))){
        fprintf(stderr, KRED "Unable to create socket\n" KNRM);
        return 1; //EXIT FAILURE
    }

    /* Start Timer */
    *timer_fd = timerfd_create (CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (*timer_fd == -1) {
        printf(KRED "Unable to create timer.\n" KNRM);
        return 1; //EXIT FAILURE
    }

    int err_tm= timerfd_settime (*timer_fd, 0, new_timer, NULL);
    if (-1 == err_tm) {
        printf(KRED "Unable to set timer\n" KNRM);
        return 1; //EXIT FAILURE
    }

    return 0; //EXIT SUCCESS
}

// get_servers asks the identity server for the server list.
// Returns raw data.
char *get_servers(int fd, struct addrinfo *id_server) {
    ssize_t n = 0;
    char *return_string = NULL;
    char response[RESPONSE_SIZE] = {'\0'};

    n = sendto(fd, REQUEST, strlen(REQUEST) + 1, 0,
            id_server->ai_addr, id_server->ai_addrlen);

    if (0 > n) {
        if ( true == is_verbose() ) fprintf(stderr, KYEL "unable to send\n" KNRM);
        fprintf(stderr, KYEL "unable to send\n" KNRM);
        return NULL;
    }

    n = recvfrom(fd, response, RESPONSE_SIZE, 0,
            id_server->ai_addr,
            &id_server->ai_addrlen);

    if (0 > n) {
        if ( true == is_verbose() ) fprintf(stderr, KYEL "unable to receive\n" KNRM);
        return NULL;
    } else {
        return_string = (char *)malloc((n+1) * sizeof(char));
        strncpy(return_string, response, strlen(response) + 1);
    }

    return return_string; //Dirty Pointer
}

list parse_servers(char *id_serv_info) {
    char *separated_info;
    char step_mem_name[STRING_SIZE]; //To define later
    char step_mem_ip_addr[STRING_SIZE];
    int  sscanf_state = 0;
    u_short step_mem_udp_port;
    u_short step_mem_tcp_port;
    list msgserv_list = create_list();

    strtok(id_serv_info, "\n"); //Gets the first info, stoping at newline
    separated_info = strtok(NULL, "\n");

    while (NULL != separated_info) { //Proceeds getting info and treating

        sscanf_state = sscanf(separated_info, "%[^;];%[^;];%hu;%hu",step_mem_name, step_mem_ip_addr,
            &step_mem_udp_port, &step_mem_tcp_port);//Separates info and saves it in variables

        if (4 != sscanf_state) {
             if ( true == is_verbose() ) fprintf(stdout, KRED "error processing id server data. data is invalid or corrupt\n" KNRM);
             return msgserv_list;
        }

        server alloc_server = new_server(step_mem_name ,
            step_mem_ip_addr, step_mem_udp_port, step_mem_tcp_port);

        set_fd(alloc_server, -2);
        set_connected(alloc_server, 0);
        push_item_to_list( msgserv_list, alloc_server ); //Pushes to list

        separated_info = strtok(NULL, "\n");//Gets new info
    }

    return msgserv_list;
}

// fetch_servers returns a list parsed from the response of (get_servers).
list fetch_servers(int fd, struct addrinfo *id_server) {
    char *response;
    list msgserv_lst = NULL;

    response = get_servers(fd, id_server); //Show server will return NULL on disconnection
    if (NULL != response){
        msgserv_lst = parse_servers(response);
        free(response);
    }

    return msgserv_lst;
}
