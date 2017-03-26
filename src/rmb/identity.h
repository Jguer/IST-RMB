#pragma once
/*! \file rmb/identity.h
 * \brief Initialization of the program and identity server communication
 */
#include <sys/timerfd.h>

#include "message.h"
#include "../utils/struct_server.h"
#include "../utils/struct_message.h"
#include "../utils/utils.h"

/*!\fn int init_program(struct addrinfo *id_server, int_fast32_t *outgoing_fd,
	int_fast32_t *binded_fd, list *msgservers_lst,	server *sel_server,
	struct itimerspec *new_timer, int_fast32_t *timer_fd)

	\brief Initiates the program variables

	\param id_server Identity Server
	\param outgoing_fd File descriptor for comunication with id_server
	\param binded_fd File decriptor for receiving info "MESSAGES"
	\param msgservers_lst List with the msgservers
	\param sel_server Selected server to operate (NULL if none)
	\param new_timer Timer spec for verifying the servers
	\param timer_fd	File descriptor to trigger on a schedule

*/
int init_program(struct addrinfo *id_server, int_fast32_t *outgoing_fd,
	int_fast32_t *binded_fd, list *msgservers_lst,	server *sel_server,
	struct itimerspec *new_timer, int_fast32_t *timer_fd);


/*!\fn list fetch_servers(int fd, struct addrinfo *id_server)

	\brief Grabs the servers from the identity server and returns them
	in a list of server structures.

	\param fd File descriptor for comunication with id_server
	\param id_server Identity Server IP address info
*/
list fetch_servers(int fd, struct addrinfo *id_server);