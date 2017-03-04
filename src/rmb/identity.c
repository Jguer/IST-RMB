#include "identity.h"

#define REQUEST "GET_SERVERS"
char *show_servers(char *server_ip, u_short server_port) {
    int i = 0;
    int fd = 0;
    ssize_t n = 0;
    struct hostent *hostptr = NULL;
    struct sockaddr_in serveraddr;
    socklen_t addrlen;
    char *response = (char *)malloc(RESPONSE_SIZE);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd==-1)
    {
        printf("error creating socket\n");
        exit(EXIT_FAILURE);
    }

    hostptr = gethostbyname(server_ip);
    if(hostptr==NULL){
        printf("error getting host by name\n");
        exit(EXIT_FAILURE);
    }

    memset((void*)&serveraddr, (int)'\0',
            sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = ((struct in_addr *) (hostptr->h_addr_list[0]))->s_addr;
    serveraddr.sin_port = htons(server_port);
    addrlen = sizeof(serveraddr);

    /* printf("Connected to: [%s:%hu]\n",inet_ntoa(serveraddr.sin_addr),ntohs(serveraddr.sin_port)); */

    while (i < 3) { // Try to send 3 times in case of disconnect
        n = sendto(fd, REQUEST, strlen(REQUEST) + 1, 0,
                (struct sockaddr*)&serveraddr, addrlen);

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
        addrlen = sizeof(serveraddr);
        n = recvfrom(fd, response, RESPONSE_SIZE, 0,
                (struct sockaddr*)&serveraddr,
                &addrlen);

        if (n == -1) {
            fprintf(stderr, KYEL "unable to receive\n" KNRM);
            i++;
        } else {
            break;
        }
    }

    if (i == 3) {
        return NULL;
    }

    close(fd);
    return response; //Dirty Pointer
}

