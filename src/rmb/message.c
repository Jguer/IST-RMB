#include "message.h"

#define PUBLISH "PUBLISH "
server *select_server(list *server_list) {
    int r = rand() % get_list_size(server_list);
    int i = 0;
    node *head = get_head(server_list);

    for (i = 0; i < r; i ++) {
        head = get_next_node(head);
    }

    return (server *)get_node_item(head);
}

void publish(list *server_list, char *msg) {
    bool success = false;
    u_short i = 0;
    int fd = 0;
    ssize_t n = 0;
    struct sockaddr_in server_addr;
    socklen_t addr_len;
    server *sel_server;
    char *msg_to_send = (char *)malloc(RESPONSE_SIZE);
    if (msg_to_send == NULL) {
        memory_error("failed to allocate error buffer");
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd==-1)
    {
        printf("error creating socket\n");
        exit(EXIT_FAILURE);
    }

    msg_to_send[0] = '\0';
    strcat(msg_to_send, PUBLISH);
    strcat(msg_to_send, msg);

    while (success != true) {
        sel_server = select_server(server_list);

        memset((void*)&server_addr, (int)'\0',
                sizeof(server_addr));

        server_addr.sin_family = AF_INET;
        if (inet_aton(get_ip_address(sel_server), &server_addr.sin_addr) != 1) {
            fprintf(stderr, KYEL "unable to convert %s to address\n" KNRM, get_ip_address(sel_server));
            i++;
            continue;
        }

        server_addr.sin_port = htons(get_udp_port(sel_server ));
        addr_len = sizeof(server_addr);

        printf("Connected to: [%s:%hu]\n",inet_ntoa(server_addr.sin_addr),ntohs(server_addr.sin_port));

        n = sendto(fd, msg_to_send, strlen(msg_to_send) + 1, 0,
                (struct sockaddr*)&server_addr, addr_len);

        if (n != -1) {
            success = true;
        } else {
            fprintf(stderr, KYEL "unable to send to %s\n" KNRM, inet_ntoa(server_addr.sin_addr));
            i++;
            if (i > get_list_size(server_list) + 3) {
                break;
            }
        }
    }

    close(fd);
    free(msg_to_send);
    return;
}
