#include <time.h>
#include <errno.h>
#include <sys/timerfd.h>

#include "message.h"
#include "identity.h"
#include "ban.h"

/*! \fn usage(char *name)
    
    \brief Prints the application terminal usage
    \param name Name of the app
*/
//_Verbose_OPT_* are debug only variables
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
                strncpy(server_port, optarg, STRING_SIZE);
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

    //Prints information to user
    fprintf(stdout, KBLU "Identity Server:" KNRM " %s:%s\n", server_ip, server_port);
    //Fetches the address of the id_server
    struct addrinfo *id_server = get_server_address(server_ip, server_port);
    if (!id_server) {
        fprintf(stderr, KRED "Unable to parse id server address from:\n %s:%s", server_ip, server_port);
        return EXIT_FAILURE;
    }

    //Program variables declaring
    int_fast32_t outgoing_fd = -1, binded_fd = -1, timer_fd = -1;
    uint_fast8_t exit_code = EXIT_SUCCESS, err = EXIT_SUCCESS, max_fd = -1;
    uint_fast32_t msg_num = 0;

    list msgservers_lst = NULL;
    server sel_server;

    struct itimerspec new_timer = {
        {SERVER_TEST_TIME_SEC,SERVER_TEST_TIME_nSEC},   //Interval of time
        {SERVER_TEST_TIME_SEC,SERVER_TEST_TIME_nSEC}    //Stop time
        };

    // Program variables initialization 
    if (1 == init_program(id_server, &outgoing_fd,
                &binded_fd, &msgservers_lst, &sel_server,
                &new_timer, &timer_fd)){
        exit_code = EXIT_FAILURE;
        goto PROGRAM_EXIT;
    }

    //Loop only variables, unnecessary to declare above the initialization of major variables
    fd_set rfds;
    bool server_not_answering = false;
    char op[STRING_SIZE], input_buffer[STRING_SIZE];
    list banservers_lst = create_list();

    if (sel_server != NULL) { //Prints the prompt
        fprintf(stdout, KGRN "Prompt > " KNRM);
        fflush(stdout);
    }

    // Interactive loop (Cycles for the rest of the program)
    while (true) {
        FD_ZERO(&rfds); //Add file descriptors to the Set (select() MACROS)
        FD_SET(STDIN_FILENO, &rfds); //fd is always 0
        FD_SET(binded_fd, &rfds);
        FD_SET(timer_fd, &rfds);

        //Calculates the maximum file descriptor index
        max_fd = binded_fd > max_fd ? binded_fd : max_fd;
        max_fd = timer_fd > max_fd ? timer_fd : max_fd;

        //Active server choice
        if (NULL == sel_server || err || server_not_answering) {
            fprintf(stderr, KYEL "Searching\n" KNRM);
            sleep(3); //Waits for 3 seconds (To let the id_server info refresh)

            if (server_not_answering) { //Ban the server if it's not answering the requests
            	if(sel_server != NULL){
                    ban_server(banservers_lst, sel_server);
                }
            }

            if (server_not_answering) { //Removes the server from the servers list
                rem_awol_server(msgservers_lst, sel_server);
            }
            
            sel_server = select_server(msgservers_lst); //Chooses one servers from the list
            //Every time we test a server for his banned status a ban time counter decreases
            while (sel_server != NULL && is_banned(banservers_lst, sel_server)){
                //While we don't have a server or the ones we have are still banned
                //Remove the banned servers from the message servers list
                rem_awol_server(msgservers_lst, sel_server);
                //Try again with the next
                sel_server = select_server(msgservers_lst);
            }

            if (sel_server != NULL) { //When it finds a server not yet banned
                fprintf(stderr, KGRN "Connected to new server\n" KNRM);
                fprintf(stdout, KGRN "Prompt > " KNRM);
                fflush(stdout);
                server_not_answering = false;
            } else { //Our list is empty. Go and fetch a new list
                fprintf(stderr, KYEL "No servers available..." KNRM);
                fflush(stdout);
                free_list(msgservers_lst, free_server); //Get new servers if the list is all run
                msgservers_lst = fetch_servers(outgoing_fd, id_server);
                //After getting the list repeat the servers check on the new servers.
                sel_server = NULL;
                server_not_answering = false;
            }
            continue;
        }

        int activity = select(max_fd + 1 , &rfds, NULL, NULL, NULL); //Select, manages the file descriptors
        if (0 > activity) {
            printf("error on select\n%d\n", errno);
            exit_code = EXIT_FAILURE;
            goto PROGRAM_EXIT;
        }
        //Select changes the status of a fd on a fd_set, if it's ready to read we can process it

        //First fd to check: TIMER ( fd implementation that triggers like an incoming message, but on schedule)
        if (FD_ISSET(timer_fd, &rfds)) { //if the timer is triggered
            //Test if the server already answered to the last test.
            uint_fast8_t server_test_status = exec_server_test();
            if (1 == server_test_status) {
                printf(KYEL "Server not answering\n" KNRM);
                fflush(stdout);
                //If it didn't mark the server as not answering
                server_not_answering = true;
            }
            //Set up the next trigger
            timerfd_settime (timer_fd, 0, &new_timer, NULL);
            continue;
        }
        //Second fd to check: UDP, handles the incomming messages
        if (FD_ISSET(binded_fd, &rfds)) {
            if (2 == handle_incoming_messages(binded_fd, msg_num)) {
                //Info was printed, re-print the prompt
                fprintf(stdout, KGRN "Prompt > " KNRM);
                fflush(stdout);
            }
        }

        //Third fd to check: USER_INPUT (Using stdio with is fd (0))
        if (FD_ISSET(STDIN_FILENO, &rfds)) { //Stdio input
            scanf("%s%*[ ]%140[^\t\n]" , op, input_buffer); // Grab word, then throw away space and finally grab until \n

            //User options input: show_servers, exit, publish message, show_latest_messages n;
            if (0 == strcasecmp("show_servers", op) || 0 == strcmp("0", op)) {
                //Prints the current reliable and untested servers list
                print_list(msgservers_lst, print_server);
            } else if (0 == strcasecmp("publish", op) || 0 == strcmp("1", op)) {
                if (0 == strlen(input_buffer)) {
                    //User input invalid
                    fprintf(stdout, KGRN "Prompt > " KNRM);
                    fflush(stdout);
                    continue;
                }
                //Publish the user message $(input_buffer) to the server $(sel_server)
                err = publish(binded_fd, sel_server, input_buffer);
                if (err) {
                    fprintf(stderr, KRED "Publish error\n" KNRM);
                }
                //Make a test to the server (Just tests answering, not content) the zero msg_num configures the test
                err = ask_for_messages(binded_fd, sel_server, 0);
                if (err) {
                    fprintf(stderr, KRED "Ask for messages error\n" KNRM);
                }
                ask_server_test(); //Say that a test was made

            } else if (0 == strcasecmp("show_latest_messages", op) || 0 == strcmp("2", op)) {
                int msg_num_test = atoi(input_buffer);
                if( 0 < msg_num_test) { //Requests the last $(msg_num_test) messages to the server
                    msg_num = msg_num_test;
                    err = ask_for_messages(binded_fd, sel_server, msg_num); //Requests messages
                    ask_server_test(); //Say that we still need to get an answer
                }
                else {
                    //Only positive (understanded as zero is not a positive)
                    printf(KRED "%s is invalid value, must be positive\n" KNRM, input_buffer);
                }
            }else if (0 == strcasecmp("show_selected_server", op) || 0 == strcmp("s", op)) {
                //Added option, prints the server being currently used
                print_server(sel_server);
            }else if (0 == strcasecmp("exit", op) || 0 == strcmp("3", op)) {
                //Kills the program
                exit_code = EXIT_SUCCESS;
                goto PROGRAM_EXIT;
            } else {
                fprintf(stderr, KRED "%s is an unknown operation\n" KNRM, op);
            }
            //Sets the strings to zero (bzero = memset with the '\0' arg)
            bzero(op, STRING_SIZE);
            bzero(input_buffer, STRING_SIZE);

            //Reprints the prompt
            fprintf(stdout, KGRN "Prompt > " KNRM);
            fflush(stdout);
        }
    }

PROGRAM_EXIT: //Cleaning routine
    freeaddrinfo(id_server);
    free_incoming_messages();
    free_list(msgservers_lst, free_server);
    if(banservers_lst) free_list(banservers_lst, free_server);
    close(outgoing_fd);
    close(binded_fd);
    return exit_code;
}
