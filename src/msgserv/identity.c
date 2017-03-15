#include "identity.h"

static char REG_MESSAGE[RESPONSE_SIZE];

int init_tcp(server *host){
	int master_fd;
	struct sockaddr_in tcpaddr;
	void (*old_handler)(int);   //interrupt handler

	master_fd = socket(AF_INET, SOCK_STREAM, 0);  //Create master socket
    if (master_fd==-1)
	{
		printf( KRED "error creating socket\n" KNRM );
		return -1;
	}

    memset((void*)&tcpaddr, (int)'\0',
            sizeof(tcpaddr));

    tcpaddr.sin_family = AF_INET;
    tcpaddr.sin_addr.s_addr = inet_addr(get_ip_address(host));
    tcpaddr.sin_port = htons(get_tcp_port(host));

    if ( 0 != bind(master_fd, (struct sockaddr*)&tcpaddr,
            sizeof(tcpaddr)) ){ //Bind socket to the PORT_TCP defined

        printf(KRED "error bind failed\n" KNRM);
        return -1;
    }

    if ( -1 == listen(master_fd, MAX_PENDING) ){ //Specify MAX_PENDING connections to master socket
    	printf(KRED "error on setting listen\n" KNRM);
    	return -1;
    }

    if ( (old_handler=signal(SIGPIPE,SIG_IGN))==SIG_ERR ) {

        printf(KRED "error protecting from SIGPIPE\n" KNRM);
        return -1;
    }

    return master_fd;

}

int init_udp(server *host){
    int u_fd;
    struct sockaddr_in udpaddr;

    u_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (u_fd==-1){

        printf( KRED "error creating socket\n" KNRM);
        return -1;
    }

    memset((void*)&udpaddr, (int)'\0',
            sizeof(udpaddr));

    udpaddr.sin_family = AF_INET;
    udpaddr.sin_addr.s_addr = inet_addr(get_ip_address(host));
    udpaddr.sin_port = htons(get_udp_port(host));

    if ( 0 != bind(u_fd, (struct sockaddr*)&udpaddr,
            sizeof(udpaddr)) ){

        printf( KRED "error bind failed\n" KNRM);
        return -1;
    }

    return u_fd;
}

struct addrinfo *reg_server(int *fd, server *host ,char *ip_name, char *udp_port) {
    struct addrinfo *id_server_info = get_server_address(ip_name, udp_port);
    int n;
    int local_fd;

    if(id_server_info == NULL) return NULL;

    local_fd = socket( AF_INET, SOCK_DGRAM, 0);
    if(local_fd <= 0){
        printf(KRED "error creating udp socket for registry\n" KNRM);
        return NULL;
    }
    *fd = local_fd;

    if ( 0 > sprintf( REG_MESSAGE, "%s %s;%s;%d;%d\n", JOIN_STRING, get_name(host),
                get_ip_address(host), get_udp_port(host), get_tcp_port(host) ) ) return NULL;

    n = sendto(local_fd, REG_MESSAGE, strlen(REG_MESSAGE) + 1, 0,
            id_server_info->ai_addr, id_server_info->ai_addrlen);

    if (n == -1) {
        fprintf(stderr, KYEL "unable to register\n" KNRM);
    }

    return id_server_info;
}

int update_reg(int fd, struct addrinfo* id_server_info) {
    int n = sendto(fd, REG_MESSAGE, strlen(REG_MESSAGE) + 1, 0,
            id_server_info->ai_addr, id_server_info->ai_addrlen);

    if (n == -1) {
        fprintf(stderr, KYEL "unable to update register\n" KNRM);
    }

    return 0;

}

int send_initial_comm( int processing_fd ){
    
    int status = 1;
    if( (unsigned int)send(processing_fd, "SGET_MESSAGES\n", strlen("SGET_MESSAGES\n"), 0) != strlen("SGET_MESSAGES\n") ) {

        printf("error sending initial communication\n");
        status = 0;
    }

    return ( status ? processing_fd : (-1) );
}

int connect_to_old_server( server *old_server, bool is_comm_sent ){
    
    int processing_fd;
    int status = 1; //false


    if ( -2 == get_fd( old_server ) ){
        // create new comunication
        processing_fd = socket(AF_INET, SOCK_STREAM, 0); 
        if ( -1 == processing_fd ){
            printf( KRED "error creating socket\n" KNRM );
            status = -1; //fatal error
            return status;
        }

        char portitoa[STRING_SIZE];
        if ( 0 > sprintf(portitoa, "%hu",get_tcp_port( old_server )) ){
            printf(KRED "error converting u_short to string\n" KNRM);
            status = -1; //fatal error
            return status;
        }
        struct addrinfo *res = get_server_address_tcp( get_ip_address(old_server), 
            portitoa);
        if( res == NULL ){
            processing_fd = -1;
            status = 1; //false
        }
        else if ( -1 == connect(processing_fd, res->ai_addr, res->ai_addrlen) ) {

            printf( KYEL "cant connect to:%s:[%s]\n" KNRM, get_ip_address(old_server),
                portitoa); //Connect return Failure
            close(processing_fd);
            processing_fd = -1;
            status = 1; //false

        }                                       //Sends only a request for one of the initial servers
        else if ( false == is_comm_sent ) {                            //Connect returns success

            //Send message like SGET_MESSAGES to request messages
            printf( "Sending new connection to:%s\n", get_name( old_server ) );
            processing_fd = send_initial_comm( processing_fd );
            if ( processing_fd == -1) status = 1; //false
            else{
                status = 0; //true
            }
        }

        set_fd(old_server, processing_fd);
    }

    if( 1 == status && true == is_comm_sent ) status = 0; //if comm is already sent to one is good

    return status;

}

int join_to_old_servers( list *msgservers_list , server *host ){
    
    bool is_comm_sent = false;
    node *aux_node = NULL;
    
    for ( aux_node = get_head(msgservers_list); //Initialize comms with one new server
    aux_node != NULL;
    aux_node = get_next_node(aux_node)) {

        if ( 0 != comp_servers( (server *)get_node_item(aux_node), host ) ){
        
            int status_check = connect_to_old_server( (server *)get_node_item(aux_node), is_comm_sent );
            if (0 == status_check) is_comm_sent = true;
            else if (1 == status_check) is_comm_sent = false;
            else{
                return -1; //Fatal error
            }
        }
    }

    if (false == is_comm_sent)
    {
        printf(KYEL "No connectable servers present: " KGRN "Wait mode\n" KNRM );
    }

    return 0; //Success
}