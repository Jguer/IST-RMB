#include "message.h"

uint_fast8_t handle_sget_messages(int fd, matrix msg_matrix) {
    uint_fast8_t exit_code = 0;
    int_fast32_t nwritten = 0;

    char *response_buffer = (char *)malloc(STRING_SIZE * (get_capacity(msg_matrix) + 1));
    if (!response_buffer) {
        memory_error("unable to allocate response while sharing last message");
    }

    char* to_append = get_first_n_messages(msg_matrix, get_capacity(msg_matrix));
    int_fast32_t nbytes = snprintf(response_buffer, STRING_SIZE * (get_capacity(msg_matrix) + 1), "%s\n%s", SMESSAGE_CODE, to_append);
    free(to_append);

    char *ptr = response_buffer;
    int_fast32_t nleft = nbytes;
    while(0 < nleft) {
        nwritten = write(fd,ptr + nwritten, nleft - nwritten);
        nleft = nleft - nwritten;
        if(nwritten <= 0) {//error
            exit_code = 1;
            break;
        }
    }

    free(response_buffer);
    return exit_code;
}

uint_fast8_t share_last_message(list servers_list, matrix msg_matrix) {
    node aux_node;
    int_fast16_t processing_fd;
    uint_fast8_t exit_code = 0;
    char *response_buffer, *ptr = NULL;
    int_fast16_t nleft, nwritten = 0;

    printf(KCYN "\n Sharing last message %s \n" KNRM, get_string(get_element(msg_matrix, get_size(msg_matrix) - 1)));

    response_buffer = (char *)alloca(2 * STRING_SIZE);
    if (NULL == response_buffer) {
        memory_error("unable to allocate response while sharing last message");
        return EXIT_FAILURE;
    }
    snprintf(response_buffer, STRING_SIZE * 2, "%s\n%d;%s",
            SMESSAGE_CODE, get_lc(get_element(msg_matrix, get_size(msg_matrix) - 1)),
            get_string(get_element(msg_matrix, get_size(msg_matrix) - 1)));

    if (NULL != servers_list) { //TCP sockets already connected handling
        for (aux_node = get_head(servers_list);
                aux_node != NULL ;
                aux_node = get_next_node(aux_node)) {

            processing_fd = get_fd((server )get_node_item(aux_node)); //file descriptor/socket

            ptr = response_buffer;
            nleft = strlen(ptr);
            while(0 < nleft) {
                nwritten = write(processing_fd,ptr + nwritten ,nleft - nwritten);
                nleft = nleft - nwritten;
                if(-1 == nwritten) {
                    if ( _VERBOSE_TEST ) printf("error sending communication TCP\n");
                    close(processing_fd);
                    set_fd((server )get_node_item(aux_node), -1 );
                    set_connected((server )get_node_item(aux_node), 0);
                    return 1;
                } //error
            }
        }
    }

    return exit_code;
}


uint_fast8_t handle_get_messages(int fd, struct sockaddr *address, int addrlen, matrix msg_matrix, char *input_buffer) {
    uint_fast8_t exit_code = 0;
    char *response_buffer;
    char *to_append;

    uint_fast32_t num = atoi(input_buffer);
    if (num < 1) {
        return 1;
    }

    num = get_capacity(msg_matrix) < num ? get_capacity(msg_matrix) : num;
    num = get_size(msg_matrix) < num ? get_size(msg_matrix) : num;

    to_append = get_first_n_messages(msg_matrix, num);

    if (NULL != to_append) {
        response_buffer = (char *)malloc(sizeof(char) * STRING_SIZE * (num + 1));
        snprintf(response_buffer, STRING_SIZE * (num + 1), "%s\n%s", "MESSAGES", to_append);

        int read_size = sendto(fd, response_buffer, strlen(response_buffer), 0,
                address, addrlen);

        free(to_append);
        if (-1 == read_size) {
            if (_VERBOSE_TEST) printf("error sending communication UDP\n");
            exit_code = 1;
        }
        free(response_buffer);
    }
    else{
        int_fast8_t nwritten = sendto(fd, "MESSAGES\n", strlen("MESSAGES\n"), 0,
                address, addrlen);

        if (-1 == nwritten) {
            if (_VERBOSE_TEST) printf("error sending communication UDP\n");
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
    char buffer[STRING_SIZE];
    char op[STRING_SIZE];
    char input_buffer[STRING_SIZE];
    uint_fast8_t err = 0;

    struct sockaddr_in receive_address = { 0, .sin_port = 0 };
    uint_fast16_t addrlen = sizeof(receive_address);

    memset(buffer, '\0', sizeof(char) * STRING_SIZE);
    memset(op, '\0', sizeof(char) * STRING_SIZE);
    memset(input_buffer, '\0', sizeof(char) * STRING_SIZE);

    int_fast16_t read_size = recvfrom(fd, buffer, sizeof(buffer), 0,
            (struct sockaddr *)&receive_address, (socklen_t*)&addrlen);

    if (-1 == read_size) {
        if (_VERBOSE_TEST) printf("udp receive error\n");
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
        strncat(msg, "\n" ,STRING_SIZE);
        add_element(msg_matrix, get_size(msg_matrix), (item)new_message(lc, msg), free_message);

        separated_info = strtok(NULL, "\n");//Gets new info
    }

    return 0;
}

uint_fast8_t tcp_fd_handle(list servers_list, matrix msg_matrix, fd_set *rfds, int (*STAT_FD)(int, fd_set *)) {
    char *buffer = NULL;
    char op[STRING_SIZE];
    char *p = NULL;
    int nread = 0;
    uint_fast8_t err = 0;

    buffer = (char *)malloc(sizeof(char) * STRING_SIZE * (get_capacity(msg_matrix) + 5));
    if (!buffer) {
        memory_error("tcp_fd_handle");
        return 1;
    }

    if (NULL != servers_list) { //TCP sockets already connected handling
        node aux_node;
        for (aux_node = get_head(servers_list);
                aux_node != NULL ;
                aux_node = get_next_node(aux_node)) {

            int processing_fd;

            processing_fd = get_fd((server )get_node_item(aux_node)); //file descriptor/socket

            if ((*STAT_FD)(processing_fd, rfds)) {

            	bzero(buffer,sizeof(char) * STRING_SIZE * (get_capacity(msg_matrix) + 5));

                while (nread != -1) {
                	char micro_buffer[STRING_SIZE] = {'\0'};
                	nread = recv(processing_fd,micro_buffer,RESPONSE_SIZE - 1, MSG_DONTWAIT);
                    if (0 == nread) {
                        close(processing_fd);
                        set_fd( (server )get_node_item(aux_node), -1 );
                        set_connected((server )get_node_item(aux_node), 0);
                        break;
                    }
					printf("JUST To know he sent %s\n", micro_buffer);
					strncat(buffer, micro_buffer, STRING_SIZE * (get_capacity(msg_matrix) + 5));
                }

                p = strtok(buffer, "\n");
                if( NULL != p )strncpy(op, p, STRING_SIZE);
                else{
                	bzero(op, STRING_SIZE);
                }

                if (0 == strcmp("SGET_MESSAGES", op)) {
                    printf( KGRN "JUST To know he sent %s\n" KNRM, op);
                    err = handle_sget_messages(processing_fd, msg_matrix);
                    fflush( stdout );
                    if (err) {
                        close(processing_fd);
                        set_fd( (server )get_node_item(aux_node), -1 );
                        set_connected((server )get_node_item(aux_node), 0);
                    }
                    //Send all my messages

                } else if (0 == strcmp("SMESSAGES", op) ) {
                    printf("JUST To know he sent %s\n", op);
                    fflush(stdout);

                    err = parse_messages(msg_matrix);
                    if (err) {
                        close(processing_fd);
                        set_fd((server)get_node_item(aux_node), -1 );
                        set_connected((server)get_node_item(aux_node), 0);
                    }
                }
            }
        }
    }

    free(buffer);
    return 0;
}
