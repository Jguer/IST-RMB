#include "identity.h"

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