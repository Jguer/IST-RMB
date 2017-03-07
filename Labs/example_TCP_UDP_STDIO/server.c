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
#define PORT_TCP 9000
#define PORT_UDP 59000
#define MAX_PENDING 5
#define MAX_SOCKETS 30
#define SIZE_BUFFER 1024
#define SIZE_MESSAGE 1024
extern int errno;

int main(){
    int fd, u_fd, master_fd, max_fd, new_fd; //File descriptors
    int n; //For cycles counter
    int read_size; //size of the read string
    int activity; 
    struct hostent *hostptr;
    struct sockaddr_in tcpaddr, udpaddr, commaddr;
    struct in_addr *host_addr;
    int clientlen;
    int addrlen;
    char *ptr, buffer[SIZE_BUFFER], message[SIZE_MESSAGE];
    fd_set rfds; //File descriptor set, read list
    struct fd_handler{

    	int fd;
    	struct sockaddr_in info;
     	
    } child_socket[MAX_SOCKETS];
	void (*old_handler)(int);   //interrupt handler

	/******************************************************
		Initializations
	*/

    for ( n=0; n<MAX_SOCKETS; n++ ) { //Make sure of the initialization
    	child_socket[n].fd = 0;
    }   

    if(gethostname(buffer,128)==-1)
    	printf("error getting hostname: %s\n",strerror(errno));
    else
    	printf("Host name: %s\n", buffer);

    if((hostptr=gethostbyname(buffer))==NULL) exit(EXIT_FAILURE);

    host_addr = (struct in_addr*)hostptr->h_addr_list[0];
    printf("Host address: %s\n",inet_ntoa(*host_addr));

    /*********************************************************
		TCP initialization:
			Master Socket;
			TCP address declaration;
			Bind;
			Listen;
			Protect from SIGPIPE termination.		
    */

    master_fd = socket(AF_INET, SOCK_STREAM, 0);  //Create master socket
    if (master_fd==-1)
	{
		printf("error creating socket\n");
		return EXIT_FAILURE;
	}

    memset((void*)&tcpaddr, (int)'\0',
            sizeof(tcpaddr));

    tcpaddr.sin_family = AF_INET;
    tcpaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    tcpaddr.sin_port = htons((u_short)PORT_TCP);

    if ( 0 != bind(master_fd, (struct sockaddr*)&tcpaddr,
            sizeof(tcpaddr)) ){ //Bind socket to the PORT_TCP defined
 
        printf("error bind failed");
        return EXIT_FAILURE;
    }


    if ( -1 == listen(master_fd, MAX_PENDING) ){ //Specify MAX_PENDING connections to master socket
    	printf("error on setting listen\n");
    	return EXIT_FAILURE;
    }

    if ( (old_handler=signal(SIGPIPE,SIG_IGN))==SIG_ERR ) {
        
        printf("error protecting from SIGPIPE\n");    
        return EXIT_FAILURE;
    }

    /*******************************************************
		UDP initialization:
			Unique client Socket;
			UDP address declaration;
			Bind;			
    */

    u_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (u_fd==-1){

		printf("error creating socket\n");
		exit(EXIT_FAILURE);
	}

	memset((void*)&udpaddr, (int)'\0',
            sizeof(udpaddr));

    udpaddr.sin_family = AF_INET;
    udpaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    udpaddr.sin_port = htons((u_short)PORT_UDP);

    if ( 0 != bind(u_fd, (struct sockaddr*)&udpaddr,
            sizeof(udpaddr)) ){

        printf("error bind failed");
        return EXIT_FAILURE;
    }
    /********************************************************
		Interactive threading loop
    */
 
    addrlen = sizeof(commaddr);

    while(1){
    	//Clear the set
    	FD_ZERO(&rfds);

    	//Add master_fd, u_fd and stdio to the socket set
    	FD_SET(u_fd, &rfds);
    	FD_SET(master_fd, &rfds);
    	FD_SET(0, &rfds);

    	max_fd = master_fd; //The master is the first socket
    	if(u_fd > max_fd)	max_fd = u_fd;

    	//Add child sockets to the socket set
    	for ( n = 0; n < MAX_SOCKETS ; n++) {

    		fd = child_socket[n].fd; //file descriptor/socket

    		//if the socket is valid then add to the read list
    		if(fd > 0)	FD_SET(fd, &rfds);

    		//The highest file descriptor is saved for the select fnc
    		if(fd > max_fd)	max_fd = fd;
    	}

    	//wait for one of the descriptors is ready, writen to
    	activity = select( max_fd + 1 , &rfds, NULL, NULL, NULL);

    	if(activity < 0){
    		printf("error on select\n");
    		return EXIT_FAILURE;
    	}

    	//if something happens on master_fd create and allocate new socket
    	if (FD_ISSET(master_fd, &rfds)){

    		//Create new socket, new_fd
	    	if ( (new_fd = accept(master_fd, (struct sockaddr *)&commaddr,
	    		(socklen_t*)&addrlen))<0 ){

	    		printf("error accepting communication\n");
	    		return EXIT_FAILURE;
	    	}

	   		//Send message like SGET_MESSAGES to request messages
	   		strcpy(message, "SGET_MESSAGES\n");
	   		if( send(new_fd, message, strlen(message), 0) != strlen(message) ) {

                printf("error sending communication\n");
	    		return EXIT_FAILURE;
            }

            //add new socket to array of sockets
            for (n = 0; n < MAX_SOCKETS; n++) 
            {

                //if position is empty
                if( child_socket[n].fd == 0 )
                {
                    child_socket[n].fd = new_fd;
                    child_socket[n].info = commaddr;  
                    printf("New child socket on %s:%hu connected\n",
        				inet_ntoa(child_socket[n].info.sin_addr), ntohs(child_socket[n].info.sin_port) );          
                    break;
                }
            }
        }

        //if something happened on other socket we must process it
    	if ( FD_ISSET(0, &rfds) ){ //Stdio input

    		read_size = read( fd, buffer, SIZE_BUFFER);
    		if ( 0 == read_size )
    		{
    			printf("error reading from stdio\n");
    			return EXIT_FAILURE;
    		}

    		buffer[read_size-1] = '\0'; //switches \n to \0

    		//User options input: exit;
		    if (strcmp("exit", buffer) == 0) {
		       	return EXIT_FAILURE;
		    } else {
        		fprintf(stderr, "%s is an unknown operation\n", buffer);
    		}
    	}

    	if ( FD_ISSET(u_fd, &rfds) ){ //UDP communications handling

    		addrlen = sizeof(udpaddr);

    		read_size = recvfrom(u_fd, buffer, sizeof(buffer), 0,
            				(struct sockaddr*)&udpaddr, &addrlen);

	       	if (read_size == -1) {
	            printf("udp receive error\n");
	            return EXIT_FAILURE;
	        }

	        buffer[read_size]='\0';
	        printf( "UDP -> echoing: %s to %s:%hu\n", buffer, inet_ntoa(udpaddr.sin_addr),
	        	ntohs(udpaddr.sin_port) );

	        read_size = sendto(u_fd, buffer, strlen(buffer)+1, 0, 
	                (struct sockaddr*)&udpaddr, addrlen);

	        if (read_size == -1) {
	            printf("send error\n");
	            return EXIT_FAILURE;
	        }
    	}


        for ( n=0 ; n<MAX_SOCKETS; n++){

        	fd = child_socket[n].fd;


        	if ( FD_ISSET(fd , &rfds) ){

        		read_size = read( fd, buffer, SIZE_BUFFER );

        		if ( 0 == read_size ){
        			
        			//The server disconnected, remove from the fd list and close fd
        			close(fd);
        			child_socket[n].fd = 0;
        			printf("Child socket on %s:%hu disconnected\n",
        				inet_ntoa(child_socket[n].info.sin_addr), ntohs(child_socket[n].info.sin_port) );
        		}
        		else{

        			//Echo back the message that came in
        			buffer[read_size] = '\0';
        			printf("TCP -> echoing: %s to %s:%hu\n", buffer, inet_ntoa(child_socket[n].info.sin_addr),
        				ntohs(child_socket[n].info.sin_port) );
        			if( send(fd, buffer, strlen(buffer), 0) != strlen(buffer) ) {

		                printf("error sending communication\n");
	    				return EXIT_FAILURE;
            		}
        		}
        	}
        }
    }

    close(master_fd);
    exit(EXIT_SUCCESS);
}



