#include "identity.h"

static char REG_MESSAGE[RESPONSE_SIZE];

int init_tcp(server *host){
	int master_fd;
	struct sockaddr_in tcpaddr;
	void (*old_handler)(int);   //interrupt handler

	master_fd = socket(AF_INET, SOCK_STREAM, 0);  //Create master socket
    if (master_fd==-1)
	{
		if ( _VERBOSE_TEST ) printf( KRED "error creating socket\n" KNRM );
		return -1;
	}

    memset((void*)&tcpaddr, (int)'\0',
            sizeof(tcpaddr));

    tcpaddr.sin_family = AF_INET;
    tcpaddr.sin_addr.s_addr = inet_addr(get_ip_address(host));
    tcpaddr.sin_port = htons(get_tcp_port(host));

    if ( 0 != bind(master_fd, (struct sockaddr*)&tcpaddr,
            sizeof(tcpaddr)) ){ //Bind socket to the PORT_TCP defined

        if ( _VERBOSE_TEST ) printf(KRED "error bind failed\n" KNRM);
        return -1;
    }

    if ( -1 == listen(master_fd, MAX_PENDING) ){ //Specify MAX_PENDING connections to master socket
    	if ( _VERBOSE_TEST ) printf(KRED "error on setting listen\n" KNRM);
    	return -1;
    }

    if ( (old_handler=signal(SIGPIPE,SIG_IGN))==SIG_ERR ) {

        if ( _VERBOSE_TEST ) printf(KRED "error protecting from SIGPIPE\n" KNRM);
        return -1;
    }

    return master_fd;

}

int init_udp(server *host){
    int u_fd;
    struct sockaddr_in udpaddr;

    u_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (u_fd==-1){

        if ( _VERBOSE_TEST ) printf( KRED "error creating socket\n" KNRM);
        return -1;
    }

    memset((void*)&udpaddr, (int)'\0',
            sizeof(udpaddr));

    udpaddr.sin_family = AF_INET;
    udpaddr.sin_addr.s_addr = inet_addr(get_ip_address(host));
    udpaddr.sin_port = htons(get_udp_port(host));

    if ( 0 != bind(u_fd, (struct sockaddr*)&udpaddr,
            sizeof(udpaddr)) ){

        if ( _VERBOSE_TEST ) printf( KRED "error bind failed\n" KNRM);
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
        if ( _VERBOSE_TEST ) printf(KRED "error creating udp socket for registry\n" KNRM);
        return NULL;
    }
    *fd = local_fd;

    if ( 0 > sprintf( REG_MESSAGE, "%s %s;%s;%d;%d\n", JOIN_STRING, get_name(host),
                get_ip_address(host), get_udp_port(host), get_tcp_port(host) ) ) return NULL;

    n = sendto(local_fd, REG_MESSAGE, strlen(REG_MESSAGE) + 1, 0,
            id_server_info->ai_addr, id_server_info->ai_addrlen);

    if (n == -1) {
        if ( _VERBOSE_TEST ) fprintf(stderr, KYEL "unable to register\n" KNRM);
    }

    return id_server_info;
}

int update_reg(int fd, struct addrinfo* id_server_info) {
    int n = sendto(fd, REG_MESSAGE, strlen(REG_MESSAGE) + 1, 0,
            id_server_info->ai_addr, id_server_info->ai_addrlen);

    if (n == -1) {
        if ( _VERBOSE_TEST ) fprintf(stderr, KYEL "unable to update register\n" KNRM);
    }

    return 0;

}

int send_initial_comm( int processing_fd ){
    
    int status = 1;
    if( (unsigned int)send(processing_fd, "SGET_MESSAGES\n", strlen("SGET_MESSAGES\n"), 0) != strlen("SGET_MESSAGES\n") ) {

        if ( _VERBOSE_TEST ) printf("error sending initial communication\n");
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
            if ( _VERBOSE_TEST ) printf( KRED "error creating socket\n" KNRM );
            status = -1; //fatal error
            return status;
        }

        char portitoa[STRING_SIZE];
        if ( 0 > sprintf(portitoa, "%hu",get_tcp_port( old_server )) ){
            if ( _VERBOSE_TEST ) printf(KRED "error converting u_short to string\n" KNRM);
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

            if ( _VERBOSE_TEST ) printf( KYEL "cant connect to:%s:[%s]\n" KNRM, get_ip_address(old_server),
                portitoa); //Connect return Failure
            close(processing_fd);
            processing_fd = -1;
            status = 1; //false

        }                                       //Sends only a request for one of the initial servers
        else if ( false == is_comm_sent ) {                            //Connect returns success

            //Send message like SGET_MESSAGES to request messages
            printf(KGRN "Sending new connection to:"KNRM"%s\n", get_name( old_server ) );
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


int remove_bad_servers( list *servers_list, server *host, int max_fd, fd_set *rfds, void (*SET_FD)(int, fd_set *) ){

    if(servers_list != NULL){ //Add child sockets to the socket set
        node *aux_node;
        if ( NULL != (aux_node = get_head( servers_list ) ) ){

            if ( -1 == get_fd((server *)get_node_item(aux_node)) ){ //1 node erasement
                
                remove_first_node(servers_list, free_server);
            }else if ( 0 == comp_servers((server *)get_node_item(aux_node),host) ){

                remove_first_node(servers_list, free_server);
            }              
            
            for ( aux_node = get_head(servers_list);
            aux_node != NULL ;
            aux_node = get_next_node(aux_node)) {

                int processing_fd;
                node *next_node;

                if ( NULL != ( next_node = get_next_node(aux_node) ) ){     
                    if ( -1 == get_fd( (server *)get_node_item(next_node) ) ){
                        //Delete next node
                        remove_next_node(aux_node, next_node, free_server);
                        dec_size_list(servers_list);
                    }else if ( 0 == comp_servers((server *)get_node_item(next_node),host) ){
                        remove_next_node(aux_node, next_node, free_server);
                        dec_size_list(servers_list);
                    }
                }

                processing_fd = get_fd((server *)get_node_item(aux_node)); //file descriptor/socket

                //if the socket is valid then add to the read list
                if(processing_fd > 0)  (*SET_FD)(processing_fd, rfds);

                //The highest file descriptor is saved for the select fnc
                if(processing_fd > max_fd) max_fd = processing_fd;
            }
        }
    }

    return max_fd;
}


void tcp_fd_handle( list *servers_list,list *messages_list, fd_set *rfds, int (*STAT_FD)(int, fd_set *)){

    char buffer[RESPONSE_SIZE] = "";
    char input_buffer[RESPONSE_SIZE];
    char op[STRING_SIZE];

    if(NULL != servers_list){ //TCP sockets already connected handling
        node *aux_node;
        for ( aux_node = get_head(servers_list);
                aux_node != NULL ;
                aux_node = get_next_node(aux_node)) {

            int processing_fd;

            processing_fd = get_fd( (server *)get_node_item(aux_node) ); //file descriptor/socket

            if ( (*STAT_FD)(processing_fd , rfds) ){

                int read_size = read( processing_fd, buffer, STRING_SIZE );

                if ( 0 == read_size ){

                    //The server disconnected, put fd equal to -1 (FD_INVALID)
                    close(processing_fd);
                    set_fd( (server *)get_node_item(aux_node), -1 );
                    set_connected((server *)get_node_item(aux_node), 0);

                }
                else{
                    //Echo back the message that came in 
                    // INPLEMENT DATA TREATMENT
                    memset((void*)&op, (int)'\0', //Makes the program safer
                        sizeof(op));

                    memset((void*)&input_buffer, (int)'\0',
                        sizeof(input_buffer));

                    buffer[read_size] = '\0';
                    sscanf(buffer, "%20[^\n]\n%511c" , op , input_buffer); //Grab op, then grab everything else to input_buffer

                    if (0 == strcmp("SGET_MESSAGES", op)) {
                        printf( KGRN "JUST To know he sent %s" KNRM, op);
                        fflush( stdout );
                        //Send all my messages

                    } else if (0 == strcmp("SMESSAGES", op) ) {
                        printf("JUST To know he sent %s, with %s", op, input_buffer);
                        fflush( stdout );
                        //save all messages from him, which are valid
                    } else {
                        if ( _VERBOSE_TEST ) fprintf(stderr, KRED "%s is an unknown operation\n" KNRM, op);
                    }


                /*  if ( _VERBOSE_TEST ) printf( "TCP -> echoing: %s to %s:%hu\n", buffer, get_ip_address( (server *)get_node_item(aux_node) ),
                        get_tcp_port((server *)get_node_item(aux_node) ) );

                    if( (unsigned int)send(processing_fd, buffer, strlen(buffer), 0) != strlen(buffer) ) {

                        if ( _VERBOSE_TEST ) printf("error sending communication TCP\n");
                        close(processing_fd);
                        set_fd( (server *)get_node_item(aux_node), -1 );
                        set_connected((server *)get_node_item(aux_node), 0);                            
                    }
                    */
                }
            }
        }
    }
}



int tcp_new_comm( list *servers_list, int listen_fd, fd_set *rfds, int (*STAT_FD)(int, fd_set *) ){


    if ( (*STAT_FD)( listen_fd, rfds ) ) { //if something happens on tcp_listen_fd create and allocate new socket

        struct sockaddr_in newserv_info;
        int newserv_fd;

        int addrlen = sizeof( newserv_info );

        //Create new socket, new_fd
        if ( (newserv_fd = accept(listen_fd, (struct sockaddr *)&newserv_info,
            (socklen_t*)&addrlen))<0 ){

            if ( _VERBOSE_TEST ) printf("error accepting communication\n");
            return -1; //Fatal error
        }

        //add new socket to list of sockets
        server * newserv = new_server( "MSG Server",inet_ntoa( newserv_info.sin_addr ), 0, ntohs( newserv_info.sin_port ) );
        set_fd( newserv, newserv_fd );
        set_connected( newserv, 1 );
        push_item_to_list( servers_list, newserv );
    }

    return 0;//everthing is good

}
