#include "message.h"

uint_fast8_t handle_get_messages(int fd, struct sockaddr *address, int addrlen, matrix msg_matrix, char *input_buffer, int m) {
    uint_fast32_t to_alloc = 0;
    uint_fast8_t exit_code = 0;
    char *response_buffer;
    char *to_append;

    uint_fast32_t num = atoi(input_buffer);
    if (num < 1) {
        return 1;
    }

    num = (uint_fast32_t)m < num ? (uint_fast32_t)m : num;
    num = get_size(msg_matrix) < num ? get_size(msg_matrix) : num;

    response_buffer = (char *)malloc(sizeof(char) * STRING_SIZE * (num + 1));
    to_append = get_first_n_messages(msg_matrix, num);

    if (NULL != to_append) {
        char * message_code = "MESSAGES";
        message_code[strlen(message_code)] = '\n';
        snprintf(response_buffer, STRING_SIZE * to_alloc, "%s\n%s", message_code, to_append);
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

uint_fast8_t handle_publish(matrix msg_matrix, char *input_buffer) {
    add_element(msg_matrix, get_size(msg_matrix), (item)new_message(get_size(msg_matrix), input_buffer), free_message);
    return 0;
}

uint_fast8_t handle_client_comms(int fd, int m, matrix msg_matrix) {
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
        err = handle_publish(msg_matrix, input_buffer);
    } else if (0 == strcmp("GET_MESSAGES", op)) {
        err = handle_get_messages(fd, (struct sockaddr *)&receive_address,
                addrlen, msg_matrix, input_buffer, m);
    }
    return err;
}

