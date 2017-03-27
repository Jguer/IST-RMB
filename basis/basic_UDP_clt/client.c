#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define PORT 58000

int main(){
	int fd;
	ssize_t n;
	struct hostent *hostptr;
	struct sockaddr_in serveraddr, clientaddr;
	int addrlen;
	char msg[80], buffer[80];

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd==-1)
	{
		printf("error creating socket\n");
		exit(EXIT_FAILURE);
	}

	hostptr = gethostbyname("JOAOMARIOLAPTOP");
	if(hostptr==NULL){
		printf("error getting host by name\n");
		exit(EXIT_FAILURE);
	}

	memset((void*)&serveraddr, (int)'\0',
			sizeof(serveraddr));

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = ((struct in_addr *) (hostptr->h_addr_list[0]))->s_addr;
	serveraddr.sin_port = htons((u_short)PORT);
	addrlen = sizeof(serveraddr);

	printf("Connected to: [%s:%hu]\n",inet_ntoa(serveraddr.sin_addr),ntohs(serveraddr.sin_port));
	
	

	while(1){
		if(fgets(msg,79,stdin) == NULL)
		{
			printf("read error\n");
			exit(EXIT_FAILURE);
		}
	
		if (strncmp(msg, "quit\n", n) == 0) {
			break;
		}
	
		n = sendto(fd, msg, strlen(msg)+1, 0, 
				(struct sockaddr*)&serveraddr, addrlen);
	
		if (n == -1) {
			printf("send error\n");
			exit(EXIT_FAILURE);
		}
	
		addrlen = sizeof(serveraddr);
		n = recvfrom(fd, buffer, sizeof(buffer), 0,
				(struct sockaddr*)&serveraddr,
				&addrlen);
		if (n == -1) {
			printf("receive error\n");
			exit(EXIT_FAILURE);
		}
	
		write(STDOUT_FILENO, "response: ", 10);
		write(STDOUT_FILENO, buffer, n);

		hostptr=gethostbyaddr(&serveraddr.sin_addr,sizeof(serveraddr.sin_addr),AF_INET);
        if(hostptr==NULL)
        	printf("sent by [%s:%hu]\n",inet_ntoa(serveraddr.sin_addr),ntohs(serveraddr.sin_port));
		else
			printf("sent by [%s:%hu]\n",hostptr->h_name,ntohs(serveraddr.sin_port));
	
	}

	close(fd);
	exit(EXIT_SUCCESS);
}



