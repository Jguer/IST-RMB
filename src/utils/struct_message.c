#include "struct_message.h"

struct _message {
    uint_fast32_t lc;
    char *content;
};

int g_lc;

// Gets
char *get_string(message this) {
    return this->content;
}

int get_lc(message this) {
    return this->lc;
}

char *get_first_n_messages(matrix msg_matrix, int n) {
    char *to_return = (char*)malloc(sizeof(char) * STRING_SIZE * n);
    if (!to_return) {
        return to_return;
    }
    bzero(to_return, STRING_SIZE * n);
    int list_size = (int)get_size(msg_matrix);
    int i = list_size - n;

    if (i < 0) {
    	if (get_overflow(msg_matrix)){
			for (uint_fast32_t j = get_capacity(msg_matrix) + i; j < get_capacity(msg_matrix); j++) {
				if (!get_element(msg_matrix, j)) {
					break;
				}
				strncat(to_return, get_string(get_element(msg_matrix, j)), STRING_SIZE * n);
				strncat(to_return, "\n",STRING_SIZE * n);
			}
    	}

        for (i = 0; i < list_size; i++) {
			if (!get_element(msg_matrix, i)) {
				break;
			}
            strncat(to_return, get_string(get_element(msg_matrix, i)), STRING_SIZE * n);
            strncat(to_return, "\n",STRING_SIZE * n);
		}
    }
    else {
    	for (; i < list_size; i++) {
			if (!get_element(msg_matrix, i)) {
				break;
			}
			strncat(to_return, get_string(get_element(msg_matrix, i)), STRING_SIZE * n);
            strncat(to_return, "\n",STRING_SIZE * n);
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
message new_message(uint_fast32_t lc, char *src) {
    message new_msg = (message)malloc(sizeof(struct _message));
    new_msg->content = (char *)malloc(sizeof(char)*STRING_SIZE);

    new_msg->lc = lc;
    strncpy(new_msg->content, src, STRING_SIZE - 1);
    new_msg->content[STRING_SIZE - 1] = '\0';

    return new_msg;
}

matrix parse_messages(char *buffer, matrix msg_matrix) {
    char msg[STRING_SIZE];
    char lc[6];
    char *separated_info;
    int sscanf_state = 0;

    strtok(buffer, "\n"); //Gets the first info, stoping at newline
    separated_info = strtok(NULL, "\n");

    while (separated_info) { //Proceeds getting info and treating
        sscanf_state = sscanf(separated_info, "%[^;];%140[^\n]",lc, msg); //Separates info and saves it in variables

        if (1 != sscanf_state) {
             if (_VERBOSE_TEST) fprintf(stdout, KRED "error processing id server data. data is invalid or corrupt\n" KNRM);
        }

        add_element(msg_matrix, get_size(msg_matrix), (item)new_message(atoi(lc), msg), free_message);

        separated_info = strtok(NULL, "\n");//Gets new info
    }

    return msg_matrix;
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


