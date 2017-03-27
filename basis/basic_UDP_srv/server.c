#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define PORT 58000
extern int errno;

int main(){
    int fd;
    ssize_t n;
    struct hostent *hostptr;
    struct sockaddr_in serveraddr, clientaddr;
    struct in_addr *host_addr; 
    int addrlen;
    char msg[80], buffer[80];


    if(gethostname(buffer,128)==-1)
    	printf("error getting hostname: %s\n",strerror(errno));
    else
    	printf("Host name: %s\n", buffer);

    if((hostptr=gethostbyname(buffer))==NULL)
    	exit(EXIT_FAILURE);

    host_addr = (struct in_addr*)hostptr->h_addr_list[0];
    printf("Host address: %s\n",inet_ntoa(*host_addr));

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd==-1)
	{
		printf("error creating socket\n");
		exit(EXIT_FAILURE);
	}

    memset((void*)&serveraddr, (int)'\0',
            sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((u_short)PORT);

    bind(fd, (struct sockaddr*)&serveraddr,
            sizeof(serveraddr));
    addrlen = sizeof(clientaddr);

    while (1) {
        n = recvfrom(fd, buffer, sizeof(buffer), 0,
                (struct sockaddr*)&clientaddr,
                &addrlen);
        if (n == -1) {
            printf("receive error\n");
            exit(EXIT_FAILURE);
        }

        write(STDOUT_FILENO, "echo: ", 6);
        write(STDOUT_FILENO, buffer, n);
        buffer[n]='\0';

        hostptr=gethostbyaddr(&clientaddr.sin_addr,sizeof(clientaddr.sin_addr),AF_INET);
        if(hostptr==NULL)
        	printf("sent by [%s:%hu]\n",inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));
		else
			printf("sent by [%s:%hu]\n",hostptr->h_name,ntohs(clientaddr.sin_port));


        if (strncmp(buffer, "quit\n", n) == 0) {
            break;
        }

        if (strcpy(msg, buffer) == NULL) {
            printf("unable to copy buffer\n");
            exit(EXIT_FAILURE);
        }

        n = sendto(fd, msg, strlen(msg)+1, 0, 
                (struct sockaddr*)&clientaddr, addrlen);

        if (n == -1) {
            printf("send error\n");
            exit(EXIT_FAILURE);
        }
    }

    close(fd);
    exit(EXIT_SUCCESS);
}



