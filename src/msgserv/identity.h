#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <signal.h>
#include "server.h"
#include "utils.h"

#define JOIN_STRING "REG"
#define MAX_PENDING 5

int init_tcp( server *host );
int init_udp( server *host );
struct addrinfo *reg_server( int *fd, server * host, char *ip_name, char *udp_port );
int update_reg( int fd, struct addrinfo* id_server_info );

