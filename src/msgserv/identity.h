#ifndef IDENTITYH
#define IDENTITYH
	
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <signal.h>
#include "server.h"
#include "utils.h"


#define MAX_PENDING 5

int init_tcp( server *host );
int init_udp( server *host );


#endif