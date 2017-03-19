#include "identity.h"

int init_program(struct addrinfo *id_server, int_fast32_t *outgoing_fd,
	int_fast32_t *binded_fd, list **msgservers_lst, server **sel_server,
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