#include "../utils/struct_server.h"
#include "../utils/utils.h"
#include "../utils/struct_message.h"

/*! \fn handle_client_comms(int fd, int m, matrix msg_matrix)
	\brief handle_client_comms receives the comunications via udp from the client.
then interprets and sends back the requested info or saves the new message
	\param fd File descriptor for udp comms
	\param msg_matrix Structure to allocate messages
*/
uint_fast8_t handle_client_comms(int fd, matrix msg_matrix);

