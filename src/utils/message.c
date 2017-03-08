#include "message.h"

typedef struct _message {
    int lc;
    char *content;
} message;

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
