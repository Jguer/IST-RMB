#ifndef MESSAGEUTILH
#define MESSAGEUTILH

#include "utils.h"
#include "string.h"

typedef struct _message message;

message *new_message(int lc, char *src);
char    *get_string(message *this);
int     get_lc(message *this);
void    set_lc(message *this, int new_lc);
void    free_message(item got_item);
void    print_message(item got_item);
list    *parse_messages(char *buffer);
#endif
