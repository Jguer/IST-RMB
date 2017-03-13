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
