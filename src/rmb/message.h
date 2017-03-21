#pragma once
/*! \file rmb/message.h
 * \brief Outbound communication with message server.
 */

#include "../utils/utils.h"
#include "../utils/struct_server.h"

/*! \fn server *select_server(list server_list);
  \brief select_server returns a pointer to a random server in server_list.
  \param server_list List containing server information.
  */
server *select_server(list server_list);

/*!\fn void rem_awol_server(list server_list, server* awol_server);
  \brief removes from the list the server on awol_server
  \param server_list List containing server information.
  \param awol_server Server that is disconnected
*/
void rem_awol_server(list server_list, server* awol_server);

/*!\fn publish(int fd, server *sel_server, char *msg)
  \brief publish sends a msg with 140 characters max to sel_server.
  \param fd Descriptor to use in send.
  \param sel_server Randomly selected server.
  \param msg Message to publish
  */
int publish(int fd, server *sel_server, char *msg);

/*! \fn int ask_for_messages(int fd, server *sel_server, int num)
  \brief ask_for_messages sends a UDP request to sel_server for the last num messages.
  \param fd Descriptor to use in send.
  \param sel_server Randomly selected server.
  \param num Number of messages to get.
  */
int ask_for_messages(int fd, server *sel_server, int num);

/*! \fn int handle_incoming_messages(int fd, int msg_num, bool test_server, server *sel_server)
  \brief handle_incoming_messages receives all the messages and handles the content. Verifying the data.
  \param fd Descriptor to use in receive.
  \param num Number of messages to get.
*/
int handle_incoming_messages(int fd, uint num);

/*! \fn void free_incoming_messages()
  \brief free_incoming_messages must be run to clear the memory allocated by handle_incoming_messages
*/
void free_incoming_messages();

/*! \fn void ask_server_test()
  \brief ask_server_test marks the test to happen
*/
void ask_server_test();

/*! \fn int exec_server_test()
  \brief exec_server_test returns server state: 0 working, 1 not working, 2 still testing
*/
int exec_server_test();
