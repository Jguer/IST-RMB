#include "message.h"

uint_fast8_t handle_sget_messages(int fd, matrix msg_matrix) {
    uint_fast8_t exit_code = 0;
    int_fast32_t nwritten = 0;

    char *response_buffer = (char *)malloc(STRING_SIZE * (get_capacity(msg_matrix) + 1));
    if (!response_buffer) {
        memory_error("unable to allocate response while sharing last message");
    }

    char* to_append = get_first_n_messages(msg_matrix, get_capacity(msg_matrix));
    int_fast32_t nbytes = 0;
    if (!to_append) {
        nbytes = snprintf(response_buffer, STRING_SIZE * (get_capacity(msg_matrix) + 1), "%s\n\n", SMESSAGE_CODE);
    } else {
        nbytes = snprintf(response_buffer, STRING_SIZE * (get_capacity(msg_matrix) + 1), "%s\n%s\n", SMESSAGE_CODE, to_append);
    }
    free(to_append);

    char *ptr = response_buffer;
    int_fast32_t nleft = nbytes;
    while (0 < nleft) {
        nwritten = write(fd,ptr + nwritten, nleft - nwritten);
        nleft = nleft - nwritten;
        if (0 >= nwritten) {//error
            exit_code = 1;
            break;
        }
    }

    free(response_buffer);
    return exit_code;
}

// cnt_array[0] must be of type char*
void send_to_server(item obj, void *cnt_array[]) {
    int_fast16_t nleft, nwritten = 0;
    server cur_server = (server) obj;
    int fd = get_fd(cur_server); //file descriptor/socket
    char *ptr = NULL;
    char *msg = (char *)cnt_array[0];

    ptr = msg;
    nleft = strlen(ptr);
    while (0 < nleft) {
        nwritten = write(fd,ptr + nwritten, nleft - nwritten);
        nleft = nleft - nwritten;
        if (-1 == nwritten) {
            if (_VERBOSE_TEST) printf("\nerror sending communication TCP\n");
            close(fd);
            set_fd(cur_server, -1);
            set_connected(cur_server, 0);
            return;
        } //error
    }
}

uint_fast8_t share_last_message(list servers_list, matrix msg_matrix) {
    uint_fast8_t exit_code = 0;
    char *response_buffer = NULL;

    if (_VERBOSE_TEST) printf(KCYN "\nSharing last message %s\n" KNRM, get_string(get_element(msg_matrix, get_size(msg_matrix) - 1)));

    response_buffer = (char *)alloca(2 * STRING_SIZE);
    if (NULL == response_buffer) {
        memory_error("unable to allocate response while sharing last message");
        return EXIT_FAILURE;
    }
    snprintf(response_buffer, STRING_SIZE * 2, "%s\n%d;%s",
            SMESSAGE_CODE, get_lc(get_element(msg_matrix, get_size(msg_matrix) - 1)),
            get_string(get_element(msg_matrix, get_size(msg_matrix) - 1)));

    for_each_element(servers_list, send_to_server, (void*[]){(void *)response_buffer});
    return exit_code;
}


uint_fast8_t handle_get_messages(int fd, struct sockaddr *address, int addrlen, matrix msg_matrix, char *input_buffer) {
    uint_fast8_t exit_code = 0;
    char *response_buffer;
    char *to_append;

    uint_fast32_t num = atoi(input_buffer);
    if (1 > num) {
        return 1;
    }

    num = get_capacity(msg_matrix) < num ? get_capacity(msg_matrix) : num;
    num = get_size(msg_matrix) < num ? get_size(msg_matrix) : num;

    to_append = get_first_n_messages(msg_matrix, num);

    if (NULL != to_append) {
        response_buffer = (char *)malloc(sizeof(char) * STRING_SIZE * (num + 1));
        snprintf(response_buffer, STRING_SIZE * (num + 1), "%s\n%s\n", "MESSAGES", to_append);

        int read_size = sendto(fd, response_buffer, strlen(response_buffer), 0,
                address, addrlen);

        free(to_append);
        if (-1 == read_size) {
            if (_VERBOSE_TEST) printf("\nerror sending communication UDP\n");
            exit_code = 1;
        }
        free(response_buffer);
    } else {
        int_fast8_t nwritten = sendto(fd, "MESSAGES\n", strlen("MESSAGES\n"), 0,
                address, addrlen);

        if (-1 == nwritten) {
            if (_VERBOSE_TEST) printf("\nerror sending communication UDP\n");
            exit_code = 1;
        }
    }

    return exit_code;
}

uint_fast8_t handle_publish(matrix msg_matrix, char *input_buffer) {
    add_element(msg_matrix, get_size(msg_matrix),
            (item)new_message(g_lc, input_buffer), free_message);
    g_lc++;
    return 2;
}

uint_fast8_t handle_client_comms(int fd, matrix msg_matrix) {
    char buffer[STRING_SIZE] = {'\0'};
    char op[STRING_SIZE] = {'\0'};
    char input_buffer[STRING_SIZE] = {'\0'};
    uint_fast8_t err = 0;

    struct sockaddr_in receive_address = {0, .sin_port = 0};
    uint_fast16_t addrlen = sizeof(receive_address);

    int_fast16_t read_size = recvfrom(fd, buffer, sizeof(buffer), 0,
            (struct sockaddr *)&receive_address, (socklen_t*)&addrlen);

    if (-1 == read_size) {
        if (_VERBOSE_TEST) printf("\nUDP receive error\n");
        return 1;
    }

    sscanf(buffer, "%s%*[ ]%140[^\t\n]" , op, input_buffer); // Grab word, then throw away space and finally grab until \n
    input_buffer[strlen(input_buffer)]='\n';
    input_buffer[strlen(input_buffer) + 1] = '\0';

    if (_VERBOSE_TEST) puts(buffer);

    if (0 == strcmp("PUBLISH", op)) {
        err = handle_publish(msg_matrix, input_buffer);
    } else if (0 == strcmp("GET_MESSAGES", op)) {
        err = handle_get_messages(fd, (struct sockaddr *)&receive_address,
                addrlen, msg_matrix, input_buffer);
    }
    return err;
}

uint_fast8_t parse_messages(matrix msg_matrix) {
    char msg[STRING_SIZE];
    char lc_buffer[6];
    uint_fast32_t lc;
    char *separated_info;
    int sscanf_state = 0;

    //strtok(buffer, "\n"); //Gets the first info, stoping at newline
    separated_info = strtok(NULL, "\n");

    while (separated_info) { //Proceeds getting info and treating
        sscanf_state = sscanf(separated_info, "%[^;];%140[^\n]",lc_buffer, msg); //Separates info and saves it in variables

        if (2 != sscanf_state) {
             if (_VERBOSE_TEST) fprintf(stdout, KRED "error processing id server data. data is invalid or corrupt\n" KNRM);
             continue;
        }
        lc = atoi(lc_buffer);
        if (lc > g_lc) {
            g_lc = lc ++;
        }
        strncat(msg, "\n" ,STRING_SIZE - 1);
        add_element(msg_matrix, get_size(msg_matrix), (item)new_message(lc, msg), free_message);

        separated_info = strtok(NULL, "\n");//Gets new info
    }

    return 0;
}


// cnt_array[0] must be of type matrix and cnt_array[1] must be of type *fd_set
void server_treat_communications(item obj, void *cnt_array[]) {
    //Opt Args
    matrix msg_matrix = (matrix) cnt_array[0];
    fd_set *rfds = (fd_set *) cnt_array[1];

    server cur_server = (server)obj;

    int fd = get_fd(cur_server);
    int_fast32_t nread = 0;
    char op[STRING_SIZE];
    char *p = NULL;
    uint_fast8_t err = 0;

    char *buffer = (char *)alloca(STRING_SIZE * (get_capacity(msg_matrix) + 5));
    if (!buffer) {
        memory_error("tcp_fd_handle");
        return;
    }

    if (FD_ISSET(fd, rfds)) {
        bzero(buffer, STRING_SIZE * (get_capacity(msg_matrix) + 5));

        printf("Received Communication\n");
        while (nread != -1) {
            char micro_buffer[STRING_SIZE] = {'\0'};
            nread = recv(fd,micro_buffer,RESPONSE_SIZE - 1, MSG_DONTWAIT);
            if (0 == nread) {
                close(fd);
                set_fd(cur_server, -1 );
                set_connected(cur_server, 0);
                break;
            }
            strncat(buffer, micro_buffer, STRING_SIZE * (get_capacity(msg_matrix) + 5));
        }
        printf("Read %s\n", buffer);

        p = strtok(buffer, "\n");
        if (NULL != p) {
            strncpy(op, p, STRING_SIZE);
        } else {
            bzero(op, STRING_SIZE);
        }

        if (0 == strcmp("SGET_MESSAGES", op)) {
            err = handle_sget_messages(fd, msg_matrix);
            fflush(stdout);
            if (err) {
                close(fd);
                set_fd(cur_server, -1 );
                set_connected(cur_server, 0);
            }
            //Send all my messages

        } else if (0 == strcmp("SMESSAGES", op) ) {
            fflush(stdout);

            err = parse_messages(msg_matrix);
            if (err) {
                close(fd);
                set_fd(cur_server, -1 );
                set_connected(cur_server, 0);
            }
        }
    }
}
