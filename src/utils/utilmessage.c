#include "utilmessage.h"

struct _message {
    int lc;
    char *content;
};

message *new_message(int lc, char *src) {
    message *new_msg = (message *)malloc(sizeof(message));
    new_msg->content = (char *)malloc(sizeof(char)*STRING_SIZE);

    new_msg->lc = lc;
    strncpy(new_msg->content, src, STRING_SIZE - 1);
    new_msg->content[STRING_SIZE - 1] = '\0';

    return new_msg;
}

char *get_string(message *this) {
    return this->content;
}

int get_lc(message *this) {
    return this->lc;
}

void set_lc(message *this, int new_lc) {
    this->lc = new_lc;
    return;
}

void free_message(item got_item) {
    message *this = (message *)got_item;

    free(this->content);
    free(this);
    return;
}

void print_message(item got_item) {
    message *this = (message *)got_item;

    fprintf(stdout,
            KBLU "LC:" RESET " %d "
            KBLU "Server IP:" RESET " %s ",
            this->lc, this->content);
    return;
}

char *get_first_n_messages(list *msg_list, int n) {
    node *aux_node = get_head(msg_list);
    if (aux_node == NULL) {
        return NULL;
    }
    char *to_return = (char*)malloc(sizeof(char) * STRING_SIZE * n);
    for (int i = 0; i < n; i++) {
        if (aux_node == NULL) {
            break;
        }
        strncat(to_return, get_string(get_node_item(aux_node)), STRING_SIZE * n);
        aux_node = get_next_node(aux_node);
    }
    return to_return;
}

list *parse_messages(char *buffer) {
    list *message_list = create_list();
    char msg[STRING_SIZE];
    char *separated_info;
    int  i = 0;

    separated_info = strtok(buffer, "\n"); //Gets the first info, stoping at newline
    separated_info = strtok(NULL, "\n");

    while ( separated_info != NULL ) { //Proceeds getting info and treating
        int sscanf_state = 0;
        sscanf_state = sscanf(separated_info, "%[^\n]", msg); //Separates info and saves it in variables

        if ( 1 != sscanf_state ) {
             if ( true == is_verbose() ) fprintf(stdout, KRED "error processing id server data. data is invalid or corrupt\n" KNRM);
             break;
        }

        push_item_to_list(message_list, (item)new_message(i, msg)); //Pushes to list

        separated_info = strtok(NULL, "\n");//Gets new info
        i++;
    }

    return message_list;
}
