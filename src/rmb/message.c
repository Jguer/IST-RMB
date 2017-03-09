#include "message.h"

#define PUBLISH "PUBLISH "
#define ASK "GET_MESSAGES "
server *select_server(list *server_list) {
    node *head = get_head(server_list);
    if (NULL == head) {
        return NULL;
    }

    int r = rand() % get_list_size(server_list);
    int i = 0;

    for (i = 0; i < r; i ++) {
        head = get_next_node(head);
    }

    return (server *)get_node_item(head);
}

int publish(int fd, server *sel_server, char *msg) {
    ssize_t n = 0;
    struct sockaddr_in server_addr;
    socklen_t addr_len;

    char *msg_to_send = (char *)malloc(RESPONSE_SIZE);
    if (NULL == msg_to_send) {
        memory_error("failed to allocate error buffer");
    }

    msg_to_send[0] = '\0';
    strcat(msg_to_send, PUBLISH);
    strcat(msg_to_send, msg);

    memset((void*)&server_addr, (int)'\0',
            sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    if (1 != inet_aton(get_ip_address(sel_server), &server_addr.sin_addr)) {
        fprintf(stderr, KYEL "unable to convert %s to address\n" KNRM, get_ip_address(sel_server));
    }

    server_addr.sin_port = htons(get_udp_port(sel_server ));
    addr_len = sizeof(server_addr);

    n = sendto(fd, msg_to_send, strlen(msg_to_send) + 1, 0,
            (struct sockaddr*)&server_addr, addr_len);

    if (-1 == n) {
        fprintf(stderr, KYEL "unable to send to %s\n" KNRM, inet_ntoa(server_addr.sin_addr));
        return 1;
    }

    close(fd);
    free(msg_to_send);
    return 0;
}

list *get_latest_messages(int num) {
    ssize_t n = 0;
    struct sockaddr_in server_addr;
    socklen_t addr_len;

    char *msg_to_send = (char *)malloc(RESPONSE_SIZE);
    if (NULL == msg_to_send) {
        memory_error("failed to allocate error buffer");
    }

    msg_to_send[0] = '\0';
    strcat(msg_to_send, ASK);
    strcat(msg_to_send, );

    memset((void*)&server_addr, (int)'\0',
            sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    if (1 != inet_aton(get_ip_address(sel_server), &server_addr.sin_addr)) {
        fprintf(stderr, KYEL "unable to convert %s to address\n" KNRM, get_ip_address(sel_server));
    }

    server_addr.sin_port = htons(get_udp_port(sel_server ));
    addr_len = sizeof(server_addr);

    n = sendto(fd, msg_to_send, strlen(msg_to_send) + 1, 0,
            (struct sockaddr*)&server_addr, addr_len);

    if (-1 == n) {
        fprintf(stderr, KYEL "unable to send to %s\n" KNRM, inet_ntoa(server_addr.sin_addr));
        return 1;
    }

    close(fd);
    free(msg_to_send);
    return NULL;
}
