/*! \file message.h
 * \brief Outbound communication with message server.
 */

#include "../utils/utils.h"
#include "../utils/server.h"

/*! \fn server *select_server(list *server_list);
  \brief select_server returns a pointer to a random server in server_list.
  \param server_list List containing server information.
  */
server *select_server(list *server_list);

/*! \fn publish(int fd, server *sel_server, char *msg)
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

