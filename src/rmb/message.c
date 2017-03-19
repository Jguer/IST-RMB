#include "message.h"

#define PUBLISH "PUBLISH"
#define ASK "GET_MESSAGES"
#define SERVER_TEST 1

static uint_fast16_t _size_to_alloc = RESPONSE_SIZE;
static char *_response_buffer = NULL;
static bool _test_server = false;
static bool _last_test_server = false;
static bool _testing_with_results = true;

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

void rem_awol_server(list *server_list, server* awol_server){
    if(server_list != NULL){ //Add child sockets to the socket set
        node *aux_node;
        if ( NULL != (aux_node = get_head( server_list ) ) ){

            if ( 0 == comp_servers((server *)get_node_item(aux_node),awol_server) ){
                remove_first_node(server_list, free_server);
            }

            for ( aux_node = get_head(server_list);
            aux_node != NULL ;
            aux_node = get_next_node(aux_node)) {

                node *next_node;

                if ( NULL != ( next_node = get_next_node(aux_node) ) ){
                    if ( 0 == comp_servers((server *)get_node_item(next_node),awol_server) ){
                        remove_next_node(aux_node, next_node, free_server);
                        dec_size_list(server_list);
                    }
                }
            }
        }
    }
}

// publish sends a $(msg) with 140 characters max to $(sel_server).
int publish(int fd, server *sel_server, char *msg) {
    ssize_t n = 0;

    struct sockaddr_in server_addr = { 0 };
    socklen_t addr_len;
    char msg_to_send[RESPONSE_SIZE];

    sprintf(msg_to_send, "%s %s\n", PUBLISH, msg);

    server_addr.sin_family = AF_INET;
    if (1 != inet_aton(get_ip_address(sel_server), &server_addr.sin_addr)) {
        if (_VERBOSE_TEST) fprintf(stderr, KYEL "unable to convert %s to address\n" KNRM, get_ip_address(sel_server));
        return 1;
    }

    server_addr.sin_port = htons(get_udp_port(sel_server));
    addr_len = sizeof(server_addr);

    n = sendto(fd, msg_to_send, strlen(msg_to_send) + 1, 0,
            (struct sockaddr*)&server_addr, addr_len);

    if (-1 == n) {
        if (_VERBOSE_TEST) fprintf(stderr, KYEL "unable to send to %s\n" KNRM, inet_ntoa(server_addr.sin_addr));
        return 1;
    }

    return 0;
}

// ask_for_messages sends a UDP request to $(sel_server) for the last $(num) messages.
int ask_for_messages(int fd, server *sel_server, int num) {
    ssize_t n = 0;
    uint_fast16_t real_msg_num = ( 0 != num ? num : 1 ); //Exception for server test
    _testing_with_results = ( 0 != num ? true : false );

    socklen_t addr_len;
    struct sockaddr_in server_addr = { 0 };
    char msg_to_send[STRING_SIZE];
    sprintf(msg_to_send, "%s %d", ASK, real_msg_num);

    server_addr.sin_family = AF_INET;
    if (1 != inet_aton(get_ip_address(sel_server), &server_addr.sin_addr)) {
        if (_VERBOSE_TEST) fprintf(stderr, KYEL "unable to convert \"%s\" to address\n" KNRM, get_ip_address(sel_server));
        return 1;
    }

    server_addr.sin_port = htons(get_udp_port(sel_server));
    addr_len = sizeof(server_addr);

    n = sendto(fd, msg_to_send, strlen(msg_to_send) + 1, 0,
            (struct sockaddr*)&server_addr, addr_len);

    if (0 > n) {
        if (_VERBOSE_TEST) fprintf(stderr, KYEL "unable to send to %s\n" KNRM, inet_ntoa(server_addr.sin_addr));
        return 1;
    }
    return 0;
}


int handle_incoming_messages(int fd, uint num){
    struct sockaddr_in server_addr = {0};
    socklen_t addr_len = sizeof(server_addr);
    
    if ((num + 1) * RESPONSE_SIZE >= _size_to_alloc) {
        _size_to_alloc = (num + 1) * RESPONSE_SIZE;
        _response_buffer = (char*)realloc(_response_buffer, sizeof(char) * _size_to_alloc);
        if (NULL == _response_buffer) {
            memory_error("Unable to realloc buffer\n");
        }
    }
    memset(_response_buffer, '\0', sizeof(char) * _size_to_alloc);
    
    int_fast16_t read_size = recvfrom(fd, _response_buffer, sizeof(char)*_size_to_alloc, 0,
            (struct sockaddr *)&server_addr, &addr_len);

    if (-1 == read_size) {
        if (_VERBOSE_TEST) fprintf(stderr, KRED "Failed UPD receive from %s\n" KNRM, inet_ntoa(server_addr.sin_addr));
        return 1; //EXIT FAILLURE
    }

    if (_VERBOSE_TEST){  //Put raw incoming data
        puts(_response_buffer);
        fflush(stdout);
    }

    char op[RESPONSE_SIZE] = {'0'};

    sscanf(_response_buffer, "%s\n" , op);
    if(0 == strcmp(op, "MESSAGES")){
        if( true == _testing_with_results ){
            printf("Last %d messages:\n", num);
            printf("%s",&_response_buffer[9]);
            fflush(stdout);
            _test_server = false;
            return 2; //EXIT SUCCESS WITH PRINTED OUTPUT
        }
        _test_server = false;
        return 0; //EXIT SUCCESS
    }
    else{
        _test_server = true;
        return 0; //EXIT SUCCESS
    }

    _testing_with_results = false;
}


void free_incoming_messages(){

    if (NULL != _response_buffer){
        free(_response_buffer);
    }
    return;
}

void ask_server_test(){
    _test_server = true;
}

int exec_server_test(){
    if( _test_server && _last_test_server){
    _test_server = false;
    _last_test_server = false;
    return 1;
    }
    else if ( _test_server && !_last_test_server ){
        _last_test_server = true;
        return 2;
    }
    else{
        _test_server = false;
        _last_test_server = false;
        return 0;
    }
}
