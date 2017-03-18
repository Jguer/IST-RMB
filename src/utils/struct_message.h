#include "utils.h"
#include "string.h"
#include "../utils/util_matrix.h"

typedef struct _message *message;

// Gets
char    *get_string(message this);
int     get_lc(message this);
char    *get_first_n_messages(matrix msg_matrix, int n);
// Sets
void    set_lc(message this, uint_fast32_t new_lc);
// Methods
message new_message(uint_fast32_t lc, char *src);
matrix  parse_messages(char *buffer, uint_fast32_t m);
void    free_message(item got_item);
void    print_message(item got_item);
