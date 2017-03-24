#include "ban.h"

/*
	Private implementation
*/
server is_server_in_list(list list_of_banned, server server_to_check){
	if(list_of_banned) { //Add child sockets to the socket set
        if ((get_head(list_of_banned))) {
            if (!different_servers((server)get_node_item(get_head(list_of_banned)), server_to_check)) {
                return (server)get_node_item(get_head(list_of_banned));
            }

            node next_node = get_next_node(get_head(list_of_banned));
            if (!next_node) {
                return NULL;
            }

            for (node aux_node = get_head(list_of_banned);
            aux_node != NULL && next_node != NULL;
            aux_node = next_node, next_node = get_next_node(next_node)) {
                if (!different_servers((server )get_node_item(next_node), server_to_check)) {
                    return (server )get_node_item(next_node);
                }
            }
        }
    }
    return NULL;
}

void reduce_ban(server server_to_update){
	set_fd(server_to_update, (get_fd(server_to_update) - 1));
}

void set_ban_time(server server_to_update, int ban_time){
	set_fd(server_to_update, ban_time);
}

int get_ban_time(server server_to_check){
	return get_fd(server_to_check);
}

/*
	Public use
*/

void ban_server(list list_of_banned, server server_to_ban){
	server mem_server = copy_server(NULL, server_to_ban);
    server server_to_update = NULL;
    set_ban_time(mem_server, SERVER_BAN_TIME);
    if (NULL == (server_to_update = is_server_in_list(list_of_banned, mem_server))) push_item_to_list(list_of_banned, mem_server);
    else if (0 > get_ban_time(server_to_update)){
    	set_ban_time(server_to_update, SERVER_BAN_TIME);
    }
}

bool is_banned(list list_of_banned, server server_to_check){
    server server_to_update = NULL;
	if (NULL != (server_to_update = is_server_in_list(list_of_banned, server_to_check))){
		reduce_ban(server_to_update);
		if (get_ban_time(server_to_update) < 0){
			return false;
		}
		else return true;
	}
	return false;
}