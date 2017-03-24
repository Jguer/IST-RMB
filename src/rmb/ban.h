#pragma once
#include "../utils/struct_server.h"
/*! \fn ban_server(list *list_of_banned, server *server_to_ban)
	\brief ban_server Puts the server to ban in a list, if it's not in there already, if it is reset the counter
	\param list_of_banned Structure of type list that saves the servers data
	\param server_to_ban Server to check if is on the list, if its not put
*/
void ban_server(list list_of_banned, server server_to_ban);

/*! \fn bool is_banned(list *list_of_banned, server *server_to_check)
	\brief ban_server Checks if the server is banned, if it is return true, else false; 
	\param list_of_banned Structure of type list that has the servers data
	\param server_to_ban Server to check if is on the list and still banned
*/
bool is_banned(list list_of_banned, server server_to_check);