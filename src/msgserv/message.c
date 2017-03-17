#include "message.h"

char *get_first_n_messages(list *msg_list, int n) {
    node *aux_node = get_head(msg_list);

    if (aux_node == NULL) {
        return NULL;
    }

    char *to_return = (char*)malloc(sizeof(char) * STRING_SIZE * n);
    if (!to_return) {
        return to_return;
    }

    for (int i = 0; i < n; i++) {
        if (aux_node == NULL) {
            break;
        }
        strncat(to_return, get_string(get_node_item(aux_node)), STRING_SIZE * n);
        aux_node = get_next_node(aux_node);
    }
    return to_return;
}

uint_fast8_t handle_get_messages(int fd, struct sockaddr *address, int addrlen, list *msg_list, char *input_buffer, int m) {
    uint_fast32_t to_alloc = 0;
    uint_fast8_t exit_code = 0;
    char *response_buffer;
    char *to_append;

    uint_fast32_t num = atoi(input_buffer);
    if (num < 1) {
        return 1;
    }

    num = (uint_fast32_t)m < num ? (uint_fast32_t)m : num;
    num = get_list_size(msg_list) < num ? get_list_size(msg_list) : num;

    response_buffer = (char *)malloc(sizeof(char) * STRING_SIZE * (num + 1));
    to_append = get_first_n_messages(msg_list, num);

    if (NULL != to_append) {
        snprintf(response_buffer, STRING_SIZE * to_alloc, "%s\n%s", "MESSAGES", to_append);
        int read_size = sendto(fd, response_buffer, strlen(response_buffer) + 1, 0,
                address, addrlen);

        free(to_append);
        if (-1 == read_size) {
            if (_VERBOSE_TEST) printf("error sending communication UDP\n");
            exit_code = 1;
        }
    }

    free(response_buffer);
    return exit_code;
}

uint_fast8_t handle_publish(list *msg_list, char *input_buffer) {
    push_item_to_list(msg_list, new_message(get_list_size(msg_list), input_buffer));
    return 0;
}

uint_fast8_t handle_client_comms(int fd, int m, list *msg_list) {
    char buffer[STRING_SIZE];
    char op[STRING_SIZE];
    char input_buffer[STRING_SIZE];
    uint_fast8_t err = 0;

    struct sockaddr_in receive_address = { 0 };
    uint_fast16_t addrlen = sizeof(receive_address);

    memset(buffer, '\0', sizeof(char) * STRING_SIZE);

    int_fast16_t read_size = recvfrom(fd, buffer, sizeof(buffer), 0,
            (struct sockaddr *)&receive_address, (socklen_t*)&addrlen);

    if (-1 == read_size) {
        if ( _VERBOSE_TEST ) printf("udp receive error\n");
        return 1;
    }

    buffer[read_size]='\0';
    sscanf(buffer, "%s%*[ ]%140[^\t\n]" , op, input_buffer); // Grab word, then throw away space and finally grab until \n
    fprintf(stdout, KBLU "\n %s Received from %s\n" KBLU, buffer, inet_ntoa(receive_address.sin_addr));

    if (0 == strcmp("PUBLISH", op)) {
        err = handle_publish(msg_list, input_buffer);
    } else if (0 == strcmp("GET_MESSAGES", op)) {
        err = handle_get_messages(fd, (struct sockaddr *)&receive_address,
                addrlen, msg_list, input_buffer, m);
    }
    return err;
}

