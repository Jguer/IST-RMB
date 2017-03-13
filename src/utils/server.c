#include "server.h"

#define REQUEST "GET_SERVERS"

struct _server {
    char    *name;
    char    *ip_addr;
    u_short udp_port;
    u_short tcp_port;
    bool    connected;
    int     fd;
};

struct addrinfo *get_server_address(char *server_ip, char *server_port) {
    struct addrinfo hints = { .ai_socktype = SOCK_DGRAM, .ai_family=AF_INET };
    struct addrinfo *result;
    int err;

    err = getaddrinfo(server_ip, server_port, &hints, &result);
    if(0 != err){
        perror("getaddrinfo");
        printf("getaddrinfo : %s \n",gai_strerror(err));
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
        perror("getaddrinfo");
        printf("getaddrinfo : %s \n",gai_strerror(err));
        return NULL;
    }

    return result;
}

char *get_servers(int fd, struct addrinfo *id_server) {
    struct timeval timeout = {3,0}; //set timeout for 2 seconds
    ssize_t n = 0;

    char *return_string = NULL;
    char *response = (char *)malloc(RESPONSE_SIZE);
    memset(response, '\0', RESPONSE_SIZE);

    if (NULL == response) {
        memory_error("failed to allocate error buffer");
    }

    n = sendto(fd, REQUEST, strlen(REQUEST) + 1, 0,
            id_server->ai_addr, id_server->ai_addrlen);

    if (0 > n) {
        fprintf(stderr, KYEL "unable to send\n" KNRM);
    }

    setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));

    n = recvfrom(fd, response, RESPONSE_SIZE, 0,
            id_server->ai_addr,
            &id_server->ai_addrlen);

    if (0 > n) {
        fprintf(stderr, KYEL "unable to receive\n" KNRM);
    } else {
        return_string = (char *)malloc((n+1) * sizeof(char));
        strcpy(return_string, response);
    }

    free(response);
    return return_string; //Dirty Pointer
}

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
list *parse_servers(char *id_serv_info) {
    char    *separated_info;
    char    step_mem_name[STRING_SIZE]; //To define later
    char    step_mem_ip_addr[STRING_SIZE];
    u_short step_mem_udp_port;
    u_short step_mem_tcp_port;
    list *msgserv_list = create_list();

    separated_info = strtok(id_serv_info, "\n"); //Gets the first info, stoping at newline
    separated_info = strtok(NULL, "\n");

    while (NULL != separated_info){ //Proceeds getting info and treating
        int sscanf_state = 0;

        sscanf_state = sscanf(separated_info, "%[^;];%[^;];%hu;%hu",step_mem_name, step_mem_ip_addr,
            &step_mem_udp_port, &step_mem_tcp_port);//Separates info and saves it in variables

        if (4 != sscanf_state) {
             fprintf(stdout, KRED "error processing id server data. data is invalid or corrupt\n" KNRM);
             return msgserv_list;
        }

        server *alloc_server = new_server(step_mem_name ,
            step_mem_ip_addr, step_mem_udp_port, step_mem_tcp_port);

        set_fd(alloc_server, -2);
        set_connected(alloc_server, 0);
        push_item_to_list( msgserv_list, alloc_server ); //Pushes to list

        separated_info = strtok(NULL, "\n");//Gets new info
    }

    return msgserv_list;
}

list *fetch_servers(int fd, struct addrinfo *id_server) {
	char *response;
	list *msgserv_lst;

	response = get_servers(fd, id_server); //Show server will return NULL on disconnection
    if (NULL != response){
        msgserv_lst = parse_servers(response);
    }

    free(response);
   	return msgserv_lst;
}

/* Server Functions */
server *new_server(char *name, char *ip_addr, u_short udp_port, u_short tcp_port) {
	server *pserver_to_node = NULL;

	pserver_to_node = (server *)malloc(sizeof(server));
    if( NULL == pserver_to_node ) memory_error("Unable to reserve server struct memory");

   	if(name != NULL){
        pserver_to_node->name = (char *)malloc( sizeof(char)*(strlen(name)+1) );
       	if ( NULL == pserver_to_node->name) memory_error("Unable to reserve server name memory");
       	if ( NULL == memcpy(pserver_to_node->name, name, strlen(name)+1) ){
       		printf( KRED "error copying name to server struct" KNRM );
       		return NULL;
       	}
    }

	pserver_to_node->ip_addr = (char *)malloc( sizeof(char)*(strlen(ip_addr)+1) );
   	if(pserver_to_node->ip_addr == NULL) memory_error("Unable to reserve server ip address memory");
   	if ( NULL == memcpy(pserver_to_node->ip_addr, ip_addr, strlen(ip_addr)+1) ){
   		printf( KRED "error copying ip address to server struct" KNRM );
   		return NULL;
   	}

   	pserver_to_node->udp_port  = udp_port;
   	pserver_to_node->tcp_port  = tcp_port;
    pserver_to_node->connected = false;
    pserver_to_node->fd = -1;

   	return pserver_to_node;
}

int comp_servers(server *serv1, server *serv2){
    if ( 0 == strcmp(serv1->name,serv2->name) 
        && 0 == strcmp(serv1->ip_addr,serv2->ip_addr) 
        && serv1->udp_port == serv2->udp_port 
        && serv1->tcp_port == serv2->tcp_port ){
        return 0;
    }

    return 1;
}

char *get_name(server *this) {
    return this->name;
}

char *get_ip_address(server *this) {
    return this->ip_addr;
}

u_short get_udp_port(server *this) {
    return this->udp_port;
}

bool get_connected(server *this) {
    return this->connected;
}

void set_connected(server *this, bool connected) {
    this->connected = connected;
    return;
}

u_short get_tcp_port(server *this) {
    return this->tcp_port;
}

int get_fd(server *this){
    return this->fd;
}

void set_fd(server *this, int fd){
    this->fd = fd;
    return;
}

void print_server(item got_item) {
    server *this = (server *)got_item;

    fprintf(stdout,
            KYEL "Server name:" RESET " %s "
            KYEL "Server IP:" RESET " %s "
            KYEL "UDP Port:" RESET " %hu "
            KYEL "TCP Port:" RESET " %hu "
            KYEL "Connected:" RESET " %d "
            KYEL "fd:" RESET " %d ", 
            this->name, this->ip_addr, this->udp_port, this->tcp_port, this->connected, this->fd);
    return;
}

void free_server(item got_item) {
    server *this = (server *)got_item;

    free(this->name);
    free(this->ip_addr);
    free(this);
    return;
}

