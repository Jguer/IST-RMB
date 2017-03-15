#include "message.h"

#define PUBLISH "PUBLISH"
#define ASK "GET_MESSAGES"

// select_server returns a pointer to a random server in $(server_list).
server *select_server(list *server_list) {
    node *head = get_head(server_list);
    if (NULL == head) {
        return NULL;
    }

    uint_fast16_t r = rand() % get_list_size(server_list);

    for (uint_fast16_t i = 0; i < r; i ++) {
        head = get_next_node(head);
    }

    return (server *)get_node_item(head);
}

// publish sends a $(msg) with 140 characters max to $(sel_server).
int publish(int fd, server *sel_server, char *msg) {
    ssize_t n = 0;

    struct sockaddr_in server_addr = { 0 };
    socklen_t addr_len;
    char msg_to_send[STRING_SIZE];

    sprintf(msg_to_send, "%s %s", PUBLISH, msg);

    server_addr.sin_family = AF_INET;
    if (1 != inet_aton(get_ip_address(sel_server), &server_addr.sin_addr)) {
        fprintf(stderr, KYEL "unable to convert %s to address\n" KNRM, get_ip_address(sel_server));
        return 1;
    }

    server_addr.sin_port = htons(get_udp_port(sel_server));
    addr_len = sizeof(server_addr);

    n = sendto(fd, msg_to_send, strlen(msg_to_send) + 1, 0,
            (struct sockaddr*)&server_addr, addr_len);

    if (-1 == n) {
        fprintf(stderr, KYEL "unable to send to %s\n" KNRM, inet_ntoa(server_addr.sin_addr));
        return 1;
    }

    return 0;
}

// ask_for_messages sends a UDP request to $(sel_server) for the last $(num) messages.
int ask_for_messages(int fd, server *sel_server, int num) {
    ssize_t n = 0;

    socklen_t addr_len;
    struct    sockaddr_in server_addr = { 0 };
    char msg_to_send[STRING_SIZE];

    sprintf(msg_to_send, "%s %d", ASK, num);

    server_addr.sin_family = AF_INET;
    if (1 != inet_aton(get_ip_address(sel_server), &server_addr.sin_addr)) {
        fprintf(stderr, KYEL "unable to convert \"%s\" to address\n" KNRM, get_ip_address(sel_server));
        return 1;
    }

    server_addr.sin_port = htons(get_udp_port(sel_server));
    addr_len = sizeof(server_addr);

    n = sendto(fd, msg_to_send, strlen(msg_to_send) + 1, 0,
            (struct sockaddr*)&server_addr, addr_len);

    if (0 > n) {
        fprintf(stderr, KYEL "unable to send to %s\n" KNRM, inet_ntoa(server_addr.sin_addr));
        return 1;
    }
    return 0;
}
