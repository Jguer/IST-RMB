#include "struct_server.h"

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

    if (/*-1 != this->fd*/ true ){
        fprintf(stdout,
            KYEL "Server name:" RESET " %s "
            KYEL "Server IP:" RESET " %s "
            KYEL "UDP Port:" RESET " %hu "
            KYEL "TCP Port:" RESET " %hu "
            "Conn: %d ",
            this->name, this->ip_addr, this->udp_port, this->tcp_port, this->fd);
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

    close(this->fd);
    free(this->name);
    free(this->ip_addr);
    free(this);
    return;
}
