Client Application Technical Specs {#Technical_C}
------------------------------------
When the application is called in the command line it can have two optional arguments:
> i [ip address] -> IP address of the identity server\n Default: tejo.tecnico.ulisboa.pt\n
> p [port of address] -> Port of the identity server on that IP address\n Default: 59000\n

Program work flow (#client_workflow)
====================================
- Parse terminal arguments
- Get id server address to check if it exists:
- Initiates the program fundamental variables (See init_program())
    + Initialize sockets (\ref file_descriptors_client)
    + Initializes the timer implementation
    + Fetch servers from the identity server
    + Saves the servers in the list of message servers
    + Selects one of the servers to communicate with
- Enters a interactive loop where the program will run until it finds an error or is told to exit.
    + Sends all the file descriptors (fd) to a set, the set will be watched and each fd will be handled only when its ready.
    + If no server is selected, a specific error occurred or a selected server isn't answering we must get a new and valid one, only then the program proceeds. To better understand how we handled the matters with bad servers and functional ones see [Server Validity Check](\ref server_validity_check).
    + The program will wait in the select function until one of the [file descriptors](\ref file_descriptors_client) is ready to be read.
    + If the timer ticks the [test for server validity](\ref server_validity_check) is handled.
    + If an incoming udp message is received the incoming message is handled. See [udp handling](\ref udp_handle_client).
    + If the user sends input to console the command must be interpreted. See [user input handling](\ref user_input_client).
    + Strings are cleaned (filled with '\0'), and prompt is reprinted.
    + Loop restarts.
- Free all allocated memory and exit.

File descriptors{#file_descriptors_client}
=========================================
- STDIN: This file descriptor is used to read from the user and is only ready when the user send input to the terminal.
- UDP:
    + Binded fd: This socket is binded to a port in the system, so that the servers can communicate to a well known port and the select function can process the file descriptor
    + Outgoing fd: This socket is only used to ask for servers from the identity server
- TIMER: (Library: timer_fd.h) This timer is implemented via a file descriptor. When the timer is triggered the file descriptor is set as ready to read.

The file descriptors are handled with the select() function.

Server Validity Check {#server_validity_check}
============================================
###The if block
In order to explain better the work that is done in this block, there is a need to know how the server ban implementation works.
It's based in two major calls:
> ban_server(list, server) - Puts the server in a list of servers who are banned, and sets it's ban time to a specified time. If it already exists restore the ban time to the specified time.\n
> is_banned(list, server) - Checks if server is banned, returns true if it's banned and false if not. The ban time counter on that server is decreased.\n

If a server is not answering the server is banned and removed from the messages servers list. Then starts the look for a new server that is not banned, if we find one that is already banned it is removed from the message servers list. Until we find a non banned server or there aren't no servers left in the list.

On the end of the block one of two cases will happen:
- 1: We find a server and exit the block with that server.
- 2: The list is empty and new servers list is fetched from the identity server and the block restarts to check the fetched servers.

If the servers fetched are still the same after every fetch the banned servers will became unbanned eventually and can be used after that.
__________________________________________________________________
###The test
The program only uses the block after errors on sending outgoing communications to servers.\n
If there isn't a server selected to work.\n
Or if the server didn't passed the test in the specified time.

Now how does the program states that the server is not working.
After each user request to the server a test is performed. Based on the ask_for_messages() call, we can determine if the server is answering or not. 

So after a publish() is called to perform a user request a ask_for_messages() is also called with a parameter who sets the no need to print the answer when it comes. In the mean time the test is scheduled with a call to ask_server_test() and a internal flag is set.\n
The timer is responsible to check whether the server answered or not.

Every time the timer is triggered the function exec_server_test() is called and if ask_server_test() was called the "counter" inside exec_server_test() is incremented, if it happened two times the server is considered not_answering.

If a server answers with a valid response, the flag is reset and the next timer trigger will clean the test and the server is not marked as unresponsive.

To test after a user call to ask_for_messages() the same is done, only the parameter who states if the result is to print or not is set to print output.

Udp incoming messages treatment {#udp_handle_client}
=====================================================
The client is projected to only be able to receive one type of message from the messages servers. the messages carries one header: `'MESSAGES\n'` and can contain multiple lines after that containing info on each message.

First thing to do is allocate some space in memory to save our incoming communication, being sent via UDP, the message arrives all at once and the buffer needs to have size for the entire communication.\n
The size is calculated knowing the number of messages that were requested to the server. If the server sends more, some info may be lost.

After receiving the information, it is printed to the user. With a little aesthetic modification. 

User input interpretation {#user_input_client}
===============================================
The commands that the user can input are:
These are not case-sensitive and there are numeric shortcuts.
Command                       |Shortcut
:-----------------------------|:-------:
show\_servers                 | 1
publish __message__           | 2
show\_latest\_messages __n__  | 3
show\_selected\_server        | 4
exit                          | 9

To interpret the user request the function \code{C} scanf("%s%*[ ]%140[^\n]", a, b)  \endcode is called.\n
The first word until it finds a white-space is saved in 'a'.\n
Then we throw away the white-space.\n
And finally the next 140 characters, if not found new-line, are saved in 'b'.\n
For comparing the commands with the user_input strcasecmp() is used to compare without case sensitive characters.

There's protection to escape characters input, who prevent that messages are sent with possibly dangerous to program characters.

If the user inputs a message with more than 140 characters, the remaining is discarded.\n
The user is asked if he really wants to send the sliced message, if the operation in 'a' is publish.\n
See publish() to know more on how are messages sent.\n
See ask_for_messages() to see how the requests are made.

The show_servers command prints the list currently being used to select the server at work.\n
Exit command breaks out of the loop.