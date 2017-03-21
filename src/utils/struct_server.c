#include "struct_server.h"

#define REQUEST "GET_SERVERS"

struct _server {
    char    *name;
    char    *ip_addr;
    u_short udp_port;
    u_short tcp_port;
    bool    connected;
    int     fd;
};

// GETS {{{
char *get_name(server this) {
    return this->name;
}

char *get_ip_address(server this) {
    return this->ip_addr;
}

u_short get_udp_port(server this) {
    return this->udp_port;
}

u_short get_tcp_port(server this) {
    return this->tcp_port;
}

bool get_connected(server this) {
    return this->connected;
}

int get_fd(server this) {
    return this->fd;
}

struct addrinfo *get_server_address(char *server_ip, char *server_port) {
    struct addrinfo hints = { .ai_socktype = SOCK_DGRAM, .ai_family=AF_INET };
    struct addrinfo *result;
    int err;

    err = getaddrinfo(server_ip, server_port, &hints, &result);
    if(0 != err){
        if ( true == is_verbose() ) perror("getaddrinfo");
        if ( true == is_verbose() ) printf("getaddrinfo : %s \n",gai_strerror(err));
        return NULL;
    }

    return result;
}

struct addrinfo *get_server_address_tcp(char *server_ip, char *server_port) {
    struct addrinfo hints = { .ai_socktype = SOCK_STREAM, .ai_family=AF_INET };
    struct addrinfo *result;
    int err;

    err = getaddrinfo(server_ip, server_port, &hints, &result);
    if(err != 0){
        if ( true == is_verbose() ) perror("getaddrinfo");
        if ( true == is_verbose() ) printf("getaddrinfo : %s \n",gai_strerror(err));
        return NULL;
    }

    return result;
}
// }}}

/*
    Parses the information given by the id server
    Info comes in structured like:
        name;ip;upt;tpt\n
    Info comes out in a linked list with struct item like:
        char    *name;
        char    *ip_addr;
        u_short udp_port;
        u_short tpc_port;

    Returns pointer to the head of the list.
    Returns NULL on failure.
*/

/* Server Functions */
server new_server(char *name, char *ip_addr, u_short udp_port, u_short tcp_port) {
	server pserver_to_node = NULL;

	pserver_to_node = (server)malloc(sizeof(struct _server));
    if(!pserver_to_node ) {
        memory_error("Unable to reserve server struct memory");
    }
   	if(name) {
        pserver_to_node->name = (char *)malloc(STRING_SIZE);
        if (!pserver_to_node->name) {
            memory_error("Unable to reserve server name memory");
        }

        if (!strncpy(pserver_to_node->name, name, STRING_SIZE)){
       		if ( true == is_verbose() ) printf( KRED "error copying name to server struct" KNRM );
       		return NULL;
       	}
    }

	pserver_to_node->ip_addr = (char *)malloc(sizeof(char)*(strlen(ip_addr)+1));
   	if(pserver_to_node->ip_addr == NULL) memory_error("Unable to reserve server ip address memory");
   	if ( NULL == memcpy(pserver_to_node->ip_addr, ip_addr, strlen(ip_addr)+1)) {
   		if ( true == is_verbose() ) printf( KRED "error copying ip address to server struct" KNRM );
   		return NULL;
   	}

   	pserver_to_node->udp_port  = udp_port;
   	pserver_to_node->tcp_port  = tcp_port;
    pserver_to_node->connected = false;
    pserver_to_node->fd = -1;

   	return pserver_to_node;
}

int different_servers(server serv1, server serv2) {
    if (!strcmp(serv1->ip_addr,serv2->ip_addr)
        && serv1->udp_port == serv2->udp_port
        && serv1->tcp_port == serv2->tcp_port) {
        return 0;
    }
    return 1;
}

server copy_server(server serv1, server serv2) {
    server serv_new = NULL;

    if(!serv1){
        serv_new = new_server(serv2->name, serv2->ip_addr, serv2->udp_port, serv2->tcp_port);
    } else {
        if (!strncpy(serv1->name, serv2->name, strlen(serv2->name)+1)){
            if ( true == is_verbose() ) printf( KRED "error copying name to server struct" KNRM );
            return NULL; //EXIT_FAILURE
        }
        if (!strncpy(serv1->ip_addr, serv2->ip_addr, strlen(serv2->ip_addr)+1) ){
            if ( true == is_verbose() ) printf( KRED "error copying ip address to server struct" KNRM );
            return NULL; //EXIT_FAILURE
        }
        serv1->udp_port = serv2->udp_port;
        serv1->tcp_port = serv2->tcp_port;
        serv_new = serv1;
    }

    return serv_new; //EXIT
}


void set_connected(server this, bool connected) {
    this->connected = connected;
    return;
}


void set_fd(server this, int fd){
    this->fd = fd;
    return;
}

void print_server(item got_item) {
    server this = (server)got_item;

    if (-1 != this->fd ){
        fprintf(stdout,
            KYEL "Server name:" RESET " %s "
            KYEL "Server IP:" RESET " %s "
            KYEL "UDP Port:" RESET " %hu "
            KYEL "TCP Port:" RESET " %hu ",
            this->name, this->ip_addr, this->udp_port, this->tcp_port);
    }
    else{
        fprintf(stdout, KCYN "Server to remove: invalid" KNRM);
    }
    return;
}

void free_server(item got_item) {
    server this = (server)got_item;
    if (this == NULL) {
        return;
    }

    free(this->name);
    free(this->ip_addr);
    free(this);
    return;
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
