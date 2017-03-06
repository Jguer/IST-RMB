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
#include <signal.h>
#define PORT 9000
#define WAIT_STACK 10
extern int errno;

int main(){
    int fd, newfd;
    int n,nw;
    struct hostent *hostptr;
    struct sockaddr_in serveraddr, clientaddr;
    struct in_addr *host_addr; 
    int clientlen;
    int addrlen;
    char *ptr, buffer[128];
    void (*old_handler)(int);   //interrupt handler


    if(gethostname(buffer,128)==-1)
    	printf("error getting hostname: %s\n",strerror(errno));
    else
    	printf("Host name: %s\n", buffer);

    if((hostptr=gethostbyname(buffer))==NULL)
    	exit(EXIT_FAILURE);

    host_addr = (struct in_addr*)hostptr->h_addr_list[0];
    printf("Host address: %s\n",inet_ntoa(*host_addr));

    fd = socket(AF_INET, SOCK_STREAM, 0);
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

    if ( -1 == listen(fd, WAIT_STACK) ) return EXIT_FAILURE; 

    if ( (old_handler=signal(SIGPIPE,SIG_IGN))==SIG_ERR ) {
        printf("error protecting from SIGPIPE\n");    
        return EXIT_FAILURE;
    }


    while(1){
        addrlen=sizeof(clientaddr);
        if ( (newfd=accept(fd,(struct sockaddr*)&clientaddr,&addrlen))==-1 ) {
            printf("error accepting connection\n");    
            return EXIT_FAILURE;
        }

        hostptr=gethostbyaddr(&clientaddr.sin_addr,sizeof(clientaddr.sin_addr),AF_INET);
        if(hostptr==NULL)
            printf("Connected to [%s:%hu]\n",inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));
        else
            printf("Connected to [%s:%hu]\n",hostptr->h_name,ntohs(clientaddr.sin_port));


        while ( (n=read(newfd,buffer,128))!=0 ) {
            
            if(n==-1){
                printf("error receiving info\n");    
                return EXIT_FAILURE;
            }

            ptr=&buffer[0];

            write(STDOUT_FILENO, "echo: ", 6);
            write(STDOUT_FILENO, buffer, n);
            buffer[n]='\0';

            while(n>0){
                if ( (nw=write(newfd,ptr,n)) <= 0 ){
                    printf("error sending info\n");    
                    return EXIT_FAILURE;
                }
                
                n-=nw; ptr+=nw;
            }
        }
        close(newfd);
    }

    close(fd);
    exit(EXIT_SUCCESS);
}



