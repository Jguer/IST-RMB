#include <time.h>
#include <errno.h>
#include <sys/timerfd.h>

#include "message.h"

void usage(char* name) {
    fprintf(stdout, "Example Usage: %s [-i siip] [-p sipt] %s \n", name, _VERBOSE_OPT_SHOW);
    fprintf(stdout, "Arguments:\n"
            "\t-i\t\t[server ip]\n"
            "\t-p\t\t[server port]\n"
            "%s", _VERBOSE_OPT_INFO);
}

int main(int argc, char *argv[]) {
    char server_ip[STRING_SIZE] = "tejo.tecnico.ulisboa.pt";
    char server_port[STRING_SIZE] = "59000";

    srand(time(NULL));
    // Treat options
    int_fast8_t oc  = 0;
    while ((oc = getopt(argc, argv, "i:p:v")) != -1) { //Command-line args parsing, 'i' and 'p' args required for both
        switch (oc) {
            case 'i':
                strncpy(server_ip, optarg, STRING_SIZE); //optarg has the string corresponding to oc value
                break;
            case 'p':
                strncpy(server_port, optarg, STRING_SIZE); //optarg has the string corresponding to oc value
                break;
            case ':':
                /* missing option argument */
                fprintf(stderr, "%s: option '-%c' requires an argument\n",
                        argv[0], optopt);
                break;
            case 'v':
                _VERBOSE_OPT_CHECK;
            case '?': //Left blank on purpose, help option
            default:
                usage(argv[0]);
                return EXIT_FAILURE;
        }
    }


    fprintf(stdout, KBLU "Identity Server:" KNRM " %s:%s\n", server_ip, server_port);
    struct addrinfo *id_server = get_server_address(server_ip, server_port);
    if (NULL == id_server) {
        fprintf(stderr, KRED "Unable to parse id server address from:\n %s:%s", server_ip, server_port);
        return EXIT_FAILURE;
    }

    char op[STRING_SIZE];
    char input_buffer[STRING_SIZE];

    uint_fast8_t exit_code = EXIT_SUCCESS;
    uint_fast8_t err = EXIT_SUCCESS;

    int_fast32_t outgoing_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == outgoing_fd) {
        fprintf(stderr, KRED "Unable to create socket\n" KNRM);
        freeaddrinfo(id_server);
        return EXIT_FAILURE;
    }

    int_fast32_t binded_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == outgoing_fd) {
        fprintf(stderr, KRED "Unable to create socket\n" KNRM);
        freeaddrinfo(id_server);
        return EXIT_FAILURE;
    }

    int_fast32_t timer_fd = -1;

    struct sockaddr_in serveraddr;
    memset((void*)&serveraddr,(int)'\0',
    sizeof(serveraddr));
    serveraddr.sin_family= AF_INET;
    serveraddr.sin_addr.s_addr= htonl(INADDR_ANY);
    serveraddr.sin_port= htons((u_short)0);    

    if (-1 == bind(binded_fd,(struct sockaddr*)&serveraddr,sizeof(serveraddr))){
        fprintf(stderr, KRED "Unable to create socket\n" KNRM);
        freeaddrinfo(id_server);
        goto PROGRAM_EXIT;
    }

    uint_fast8_t max_fd = binded_fd; // Max fd number.
    uint_fast16_t msg_num = 0; // Number of messages asked

    fd_set rfds;

    list *msgservers_lst = fetch_servers(outgoing_fd, id_server);
    server *sel_server = select_server(msgservers_lst);
    if (sel_server != NULL) {
        fprintf(stdout, KGRN "Prompt > " KNRM);
        fflush(stdout);
    }
    
    struct itimerspec new_timer = {
        {SERVER_TEST_TIME_SEC,SERVER_TEST_TIME_nSEC},   //Interval of time
        {SERVER_TEST_TIME_SEC,SERVER_TEST_TIME_nSEC}    //Stop time
        };
    
    /* Start Timer */
    timer_fd = timerfd_create (CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timer_fd == -1) {
        printf(KRED "Unable to create timer.\n" KNRM);
    }

    int err_tm= timerfd_settime (timer_fd, 0, &new_timer, NULL);
    if (-1 == err_tm) {
        printf(KRED "Unable to set timer\n" KNRM);
        exit_code = EXIT_FAILURE;
        goto PROGRAM_EXIT;
    }

    bool server_not_answering = false;
    server *old_server = NULL;
    uint_fast8_t ban_counter = 0;

    // Interactive loop
    while (true) {
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(binded_fd, &rfds);
        FD_SET(timer_fd, &rfds);

        max_fd = binded_fd > max_fd ? binded_fd : max_fd;
        max_fd = timer_fd > max_fd ? timer_fd : max_fd;

        if (NULL == sel_server || err || server_not_answering) {
            fprintf(stderr, KYEL "Searching\n" KNRM);
            sleep(3);

            if (SERVER_BAN_TIME < ban_counter){
                if (sel_server != NULL) free_server(sel_server);
                sel_server = NULL;
                if (old_server != NULL) free_server(old_server);
                old_server = NULL;
                server_not_answering = false;
                ban_counter = 0;
                continue;
            }

            if (server_not_answering){
            	if(sel_server != NULL) old_server = copy_server(old_server, sel_server);

                if (old_server == NULL){
                    goto PROGRAM_EXIT;
                }
            }

            free_list(msgservers_lst, free_server);
            msgservers_lst = fetch_servers(outgoing_fd, id_server);
            if (server_not_answering){
                rem_awol_server(msgservers_lst, old_server);
            }
            sel_server = select_server(msgservers_lst);
            if (sel_server != NULL) {
                fprintf(stderr, KGRN "Connected to new server\n" KNRM);
                fprintf(stdout, KGRN "Prompt > " KNRM);
                fflush(stdout);
                server_not_answering = false;
                ban_counter = 0;
                if(old_server != NULL) free_server(old_server);
            }else{
                fprintf(stderr, KYEL "No servers available..." KNRM);
                fflush(stdout);
            }

            ban_counter ++;
            continue;
        }

        int activity = select(max_fd + 1 , &rfds, NULL, NULL, NULL); //Select, threading function
        if(0 > activity){
            printf("error on select\n%d\n", errno);
            exit_code = EXIT_FAILURE;
            goto PROGRAM_EXIT;
        }
        
        if (FD_ISSET(timer_fd, &rfds)) { //if the timer is triggered
            uint_fast8_t server_test_status = exec_server_test();
            
            if (1 == server_test_status){
                printf(KYEL "Server not answering\n" KNRM);
                fflush(stdout);
                server_not_answering = true;
            }

            timerfd_settime (timer_fd, 0, &new_timer, NULL);
            continue;
        }

        if (FD_ISSET(binded_fd, &rfds)) { //UDP receive
            if (2 == handle_incoming_messages(binded_fd, msg_num)){
                fprintf(stdout, KGRN "Prompt > " KNRM);
                fflush(stdout);    
            }        
        }

        if (FD_ISSET(STDIN_FILENO, &rfds)) { //Stdio input
            scanf("%s%*[ ]%140[^\t\n]" , op, input_buffer); // Grab word, then throw away space and finally grab until \n

            //User options input: show_servers, exit, publish message, show_latest_messages n;
            if (0 == strcasecmp("show_servers", op) || 0 == strcmp("0", op)) {
                print_list(msgservers_lst, print_server);
            } else if (0 == strcasecmp("publish", op) || 0 == strcmp("1", op)) {
                if (0 == strlen(input_buffer)) {
                    continue;
                }
                err = publish(binded_fd, sel_server, input_buffer);
                err = ask_for_messages(binded_fd, sel_server, 0);
                ask_server_test();

            } else if (0 == strcasecmp("show_latest_messages", op) || 0 == strcmp("2", op)) {
                msg_num = atoi(input_buffer);
                if( 0 < msg_num){
                    err = ask_for_messages(binded_fd, sel_server, msg_num);
                    ask_server_test();
                }
                else{
                    printf(KRED "%s is invalid value, must be positive\n" KNRM, input_buffer);
                }
            } else if (0 == strcasecmp("exit", op) || 0 == strcmp("3", op)) {
                exit_code = EXIT_SUCCESS;
                goto PROGRAM_EXIT;
            } else {
                fprintf(stderr, KRED "%s is an unknown operation\n" KNRM, op);
            }

            memset(op, '\0', sizeof(char) * STRING_SIZE);
            memset(input_buffer, '\0', sizeof(char) * STRING_SIZE);

            fprintf(stdout, KGRN "Prompt > " KNRM);
            fflush(stdout);
        }
    }

PROGRAM_EXIT:
    freeaddrinfo(id_server);
    free_incoming_messages();
    free_list(msgservers_lst, free_server);
    close(outgoing_fd);
    close(binded_fd);
    return exit_code;
}
