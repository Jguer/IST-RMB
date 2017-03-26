#pragma once
#include "utils.h"
#include "string.h"
#include "../utils/util_matrix.h"

#define MSG_WO_LC 0
#define MSG_W_LC 1

typedef struct _message *message;
extern uint_fast32_t g_lc;

// Gets
char    *get_string(message this);
int     get_lc(message this);
char    *get_first_n_messages(matrix msg_matrix, int n, int MODE);
// Sets
void    set_lc(message this, uint_fast32_t new_lc);
// Methods
message new_message(uint_fast32_t lc, char *src);
void    free_message(item got_item);
void    print_message(item got_item);
void    print_message_plain(item got_item);

