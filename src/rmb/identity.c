#include "identity.h"

#define REQUEST "GET_SERVERS"

typedef struct _server {
    char    *name;
    char    *ip_addr;
    u_short udp_port;
    u_short tpc_port;
}server;

char *show_servers(char *server_ip, u_short server_port) {
    int i = 0;
    int fd = 0;
    ssize_t n = 0;
    struct hostent *host_ptr = NULL;
    struct sockaddr_in server_addr;
    socklen_t addr_len;
    char *response = (char *)malloc(RESPONSE_SIZE);
    char *return_string;

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
            return_string = (char *)malloc( (n+1) * sizeof(char));
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

