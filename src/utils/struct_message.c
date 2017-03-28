#include "struct_message.h"

struct _message {
    uint_fast32_t lc;
    char *content;
};

uint_fast32_t g_lc;

// Gets
char *get_string(message this) {
    return this->content;
}

int get_lc(message this) {
    return this->lc;
}

char *get_first_n_messages(matrix msg_matrix, int n, int MODE){
    int list_size = (int)get_size(msg_matrix);
    if (list_size == 0){
        return NULL;
    }
    char *to_return = (char*)malloc(sizeof(char) * STRING_SIZE * n);
    if (!to_return) {
        return NULL;
    }
    bzero(to_return, STRING_SIZE * n);
    int i = list_size - n;

    if (i < 0) {
    	if (get_overflow(msg_matrix)){
			for (uint_fast32_t j = get_capacity(msg_matrix) + i; j < get_capacity(msg_matrix); j++) {
				if (!get_element(msg_matrix, j)) {
					break;
				}
				char to_append[STRING_SIZE * 2] = {'\0'};
                if (MSG_W_LC == MODE){
                    snprintf(to_append, STRING_SIZE * 2 ,"%d;%s\n",get_lc(get_element(msg_matrix, j)),
                    get_string(get_element(msg_matrix, j)) );
                } else {
                    snprintf(to_append, STRING_SIZE * 2 ,"%s\n",
                    get_string(get_element(msg_matrix, j)) );
                }
                strncat(to_return, to_append, STRING_SIZE * n);
			}
    	}

        for (i = 0; i < list_size; i++) {
			if (!get_element(msg_matrix, i)) {
				break;
			}
			char to_append[STRING_SIZE * 2] = {'\0'};
			if (MSG_W_LC == MODE){
                snprintf(to_append, STRING_SIZE * 2 ,"%d;%s\n",get_lc(get_element(msg_matrix, i)),
                get_string(get_element(msg_matrix, i)) );
            } else {
                snprintf(to_append, STRING_SIZE * 2 ,"%s\n",
                get_string(get_element(msg_matrix, i)) );
            }
			strncat(to_return, to_append, STRING_SIZE * n);
		}
    }
    else {
    	for (; i < list_size; i++) {
			if (!get_element(msg_matrix, i)) {
				break;
			}
			char to_append[STRING_SIZE * 2] = {'\0'};
			if (MSG_W_LC == MODE){
                snprintf(to_append, STRING_SIZE * 2 ,"%d;%s\n",get_lc(get_element(msg_matrix, i)),
                get_string(get_element(msg_matrix, i)) );
            } else {
                snprintf(to_append, STRING_SIZE * 2 ,"%s\n",
                get_string(get_element(msg_matrix, i)) );
            }
			strncat(to_return, to_append, STRING_SIZE * n);
		}
    }

    return to_return;
}

// Sets
void set_lc(message this, uint_fast32_t new_lc) {
    this->lc = new_lc;
    return;
}

// Methods
message new_message(char *src) {
    message new_msg = (message)malloc(sizeof(struct _message));
    new_msg->content = (char *)malloc(sizeof(char)*STRING_SIZE);

    new_msg->lc = g_lc;
    g_lc ++;
    strncpy(new_msg->content, src, STRING_SIZE - 1);
    new_msg->content[STRING_SIZE - 1] = '\0';

    return new_msg;
}


void free_message(item got_item) {
    if (!got_item) {
        return;
    }
    message this = (message)got_item;

    free(this->content);
    free(this);
    return;
}

void print_message_plain(item got_item) {
    if (!got_item) {
        return;
    }
    message this = (message)got_item;

    fprintf(stdout,
            "LC: %zu Message: %s\n",
            this->lc, this->content);
    return;
}

void print_message(item got_item) {
    if (!got_item) {
        return;
    }
    message this = (message)got_item;

    fprintf(stdout,
            KBLU "LC:" RESET " %zu "
            KBLU "Message:" RESET " %s\n",
            this->lc, this->content);
    return;
}


