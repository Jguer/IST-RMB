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

int send_initial_comm( int processing_fd );
int connect_to_old_server( server *old_server, bool is_comm_sent );
int join_to_old_servers( list *servers_list, server *host );

int remove_bad_servers( list *servers_list, server *host, int max_fd, fd_set *rfds, void (*SET_FD)(int, fd_set *) );
