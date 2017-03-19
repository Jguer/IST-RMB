#include "../utils/struct_server.h"
#include "../utils/utils.h"
#include "../utils/struct_message.h"

#define MESSAGE_CODE "MESSAGES"
#define SMESSAGE_CODE "SMESSAGES"

uint_fast8_t handle_client_comms(int fd, int m, matrix msg_matrix);
uint_fast8_t share_last_message(list *servers_list, matrix msg_matrix);
uint_fast8_t handle_publish(matrix msg_matrix, char *input_buffer);
uint_fast8_t handle_sget_messages(int fd, matrix msg_matrix);
uint_fast8_t tcp_fd_handle(list *servers_list,matrix msg_matrix, fd_set *rfds, int (*STAT_FD)(int, fd_set *));
uint_fast8_t parse_messages(char *buffer, matrix msg_matrix);

