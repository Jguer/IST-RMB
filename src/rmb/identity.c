#include "identity.h"

#define REQUEST "GET_SERVERS"

typedef struct _server {
    char    *name;
    char    *ip_addr;
    u_short udp_port;
    u_short tcp_port;
} server;

char *show_servers(char *server_ip, u_short server_port) {
    int i = 0;
    int fd = 0;
    ssize_t n = 0;
    struct hostent *host_ptr = NULL;
    struct sockaddr_in server_addr;
    socklen_t addr_len;
    char *return_string;
    char *response = (char *)malloc(RESPONSE_SIZE);
    if (response == NULL) {
        memory_error("failed to allocate error buffer");
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd==-1)
    {
        printf("error creating socket\n");
        exit(EXIT_FAILURE);
    }

    host_ptr = gethostbyname(server_ip);
    if(host_ptr == NULL){
        printf("error getting host by name\n");
        exit(EXIT_FAILURE);
    }

    memset((void*)&server_addr, (int)'\0',
            sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = ((struct in_addr *) (host_ptr->h_addr_list[0]))->s_addr;
    server_addr.sin_port = htons(server_port);
    addr_len = sizeof(server_addr);

    /* printf("Connected to: [%s:%hu]\n",inet_ntoa(serveraddr.sin_addr),ntohs(serveraddr.sin_port)); */

    while (i < 3) { // Try to send 3 times in case of disconnect
        n = sendto(fd, REQUEST, strlen(REQUEST) + 1, 0,
                (struct sockaddr*)&server_addr, addr_len);

        if (n == -1) {
            fprintf(stderr, KYEL "unable to send\n" KNRM);
            i++;
        } else {
            break;
        }
    }
    if (i == 3) {
        return NULL;
    }

    i = 0;
    while (i < 3) { // Try to receive 3 times in case of disconnect
        addr_len = sizeof(server_addr);
        n = recvfrom(fd, response, RESPONSE_SIZE, 0,
                (struct sockaddr*)&server_addr,
                &addr_len);

        if (n == -1) {
            fprintf(stderr, KYEL "unable to receive\n" KNRM);
            i++;
        } else {
            return_string = (char *)malloc((n+1) * sizeof(char));
            strcpy(return_string, response);
            break;
        }
    }

    if (i == 3) {
        return NULL;
    }

    close(fd);
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
list *parse_servers(char *id_serv_info){
    char *separated_info;
    char step_mem_name[STRING_SIZE]; //To define later
    char step_mem_ip_addr[STRING_SIZE];
    u_short step_mem_udp_port;
    u_short step_mem_tcp_port;
    list *msgserv_list = create_list();


    separated_info = strtok(id_serv_info, "\n"); //Gets the first info, stoping at newline
    separated_info = strtok(NULL, "\n");

    while ( separated_info != NULL ){ //Proceeds getting info and treating
        int sscanf_state = 0;

        sscanf_state = sscanf(separated_info, "%[^;];%[^;];%hu;%hu",step_mem_name, step_mem_ip_addr,
            &step_mem_udp_port, &step_mem_tcp_port);//Separates info and saves it in variables

        if ( 3 >= sscanf_state || EOF == sscanf_state ){
             fprintf(stdout, KRED "error processing id server data, data is invalid or corrupt" KNRM);
             return NULL;
        }

        push_item_to_list( msgserv_list, new_server(step_mem_name , 
        	step_mem_ip_addr, step_mem_udp_port, step_mem_tcp_port) ); //Pushes to list

        separated_info = strtok(NULL, "\n");//Gets new info
    }

    return msgserv_list;

}

list *fetch_servers(char *server_ip, u_short server_port){
	char *response;
	list *msgserv_lst;

	response = show_servers(server_ip, server_port); //Show server will return NULL on disconnection
    if (NULL != response){
        msgserv_lst = parse_servers(response);
        free(response);
    }
    if (NULL == msgserv_lst){
        printf( KRED "error, cannot reach identity server. now closing\n" KNRM);
        return NULL;
    }

   	return msgserv_lst;
}

/* Server Functions */

server *new_server(char *name, char *ip_addr, u_short udp_port, u_short tcp_port){
	server *pserver_to_node = NULL;
	
	pserver_to_node = (server *)malloc( sizeof(server) );
    if( NULL == pserver_to_node ) memory_error("Unable to reserve server struct memory");

   	pserver_to_node->name = (char *)malloc( sizeof(char)*(strlen(name)+1) );
   	if ( NULL == pserver_to_node->name) memory_error("Unable to reserve server name memory");
   	if ( NULL == memcpy(pserver_to_node->name, name, strlen(name)+1) ){
   		printf( KRED "error copying name to server struct" KNRM );
   		return NULL;
   	} 

	pserver_to_node->ip_addr = (char *)malloc( sizeof(char)*(strlen(ip_addr)+1) );
   	if(pserver_to_node->ip_addr == NULL) memory_error("Unable to reserve server ip address memory");
   	if ( NULL == memcpy(pserver_to_node->ip_addr, ip_addr, strlen(ip_addr)+1) ){
   		printf( KRED "error copying ip address to server struct" KNRM );
   		return NULL;
   	}

   	pserver_to_node->udp_port = udp_port;
   	pserver_to_node->tcp_port = tcp_port;

   	return pserver_to_node;
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

u_short get_tcp_port(server *this) {
    return this->tcp_port;
}

void print_server(item got_item) {
    server *this = (server *)got_item;

    fprintf(stdout,
            KYEL "Server name:" RESET " %s "
            KYEL "Server IP:" RESET " %s "
            KYEL "UDP Port:" RESET " %hu "
            KYEL "TCP Port:" RESET " %hu \n",
            this->name, this->ip_addr, this->udp_port, this->tcp_port);
    return;
}

void free_server(item got_item) {
    server *this = (server *)got_item;

    free(this->name);
    free(this->ip_addr);
    free(this);
    return;
}

