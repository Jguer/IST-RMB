Server Application Technical Specs {#Technical_S}
------------------------------------
When the application is called in the command line it can have various arguments:\n
Non-Optional\n
> n [name] -> Intended name for the server\n
> j [ip address] -> IP address for the server. Must be the ip of the machine\n
> u [UDP port] -> Port for udp incoming connections. Must be free port.\n
> t [TCP port] -> Port for tcp incoming connections. Must be free port.\n

Optional\n
> i [ip address] -> IP address of the identity server.\n Default: tejo.tecnico.ulisboa.pt\n
> p [port of address] -> Port of the identity server on that IP address.\n Default: 59000\n
> m [max. messages] -> Maximum number of messages that the server can save.\n Default: 200\n
> r [register interval] -> Time (in seconds) between registers to the id server.\n Default:10s\n

Program work flow (#server_workflow)
====================================
- Parse terminal arguments
- Get id server address to check if it exists:
- Initiates the program fundamental variables
    + Initialize sockets (\ref file_descriptors_server)
    + Initializes the timer implementation
- Enters a interactive loop where the program will run until it finds an error or is told to exit.
    + Sends all the file descriptors (fd) to a set, the set will be watched and each fd will be handled only when its ready (The communications only start after the user asks to join, and returns successful).
    + The program will wait in the select function until one of the [file descriptors](\ref file_descriptors_server) is ready to be read.
    + The registration to id server is refreshed, if the time as elapsed.
    + If a server tries to connect it is accepted and saved in a list. See [incoming request to connect](\ref incom_tcp_req)
    + If an incoming udp message is received the incoming message is handled. See [udp handling](\ref udp_handle_server).
    + If the user sends input to console the command must be interpreted. See [user input handling](\ref user_input_server).
        * User join command is where this program starts the main routines.
        * Join sends the first register to the id server. Gets the already present servers and verifies if this server is registered. Then proceeds to open communications to the already joined servers, where it ask for the messages to just one server (The first that answers the request).
        * The other options just print the current info of the messages and connected servers. Or exit.
    + If theres an incoming tcp message from a already connected server it is treated accordingly. See [tcp handling](\ref tcp_handle_server).
    + Reprints the prompt if its necessary.
    + Loop restarts.
- Free all allocated memory and exit.

File descriptors{#file_descriptors_server}
=========================================
- STDIN: This file descriptor is used to read from the user and is only ready when the user send input to the terminal.
- UDP:
    + Global fd: This socket is binded to a port in the system, so that the clients can communicate to a well known port and the select function can process the file descriptor, it is used just to receive and send information to clients.
    + Outgoing fd: This socket is only used to ask for servers from the identity server and register.
- TIMER: (Library: timer_fd.h) This timer is implemented via a file descriptor. When the timer is triggered the file descriptor is set as ready to read.
- TCP:
    + Listen fd: Listen_fd receives incoming connections who latter will be answered and communicate via another file descriptor (server_fd)
    + Server fd: There can be many file descriptors of this in the program. Any server who is connected tho this server will have it's own file descriptor, who is saved in their own server struct.
    + 
The file descriptors are handled with the select() function who blocks until one of the file descriptors signals that it is ready to read. This function control the work flow of the program.

Incoming requests to connect {#incom_tcp_req}
============================================
When a server tries to connect it is put on a queue of 5 servers capacity.\n
This routine accepts the connection, and saves that server info on a list dedicated to server struct items.
This items contain the server name, ip address, tcp port and the corresponding file descriptor.
Theres a udp port slot, who is unused. This structure is shared between client and server.

Every incoming server has is name set to Inbound Server.

UDP incoming messages treatment {#udp_handle_server}
=====================================================
The server is projected to only be able to receive two type of message from the clients. The messages can have one of two headers: `'PUBLISH message'` or `'GET_MESSAGES n'`.

First thing to do is allocate some space in memory to save our incoming communication, being sent via UDP, the message arrives all at once and the buffer needs to have size for the entire communication.\n
The size is the size of 'PUBLISH' plus the size of the whole message (140 char. max). All that is received with more size than the size who was allocated is lost.

After receiving the information, it is saved in a message struct, in the case of 'PUBLISH' being the header, the logical clock is set to the next logical clock. (eg. if LastMessageLC == 1 so NewMessageLC = 2)

If 'GET_MESSAGES n' is received, the last n messages are fetched from the matrix and sent to the client who made the request. If n is bigger than the number of messages present, only the present messages are sent to the user.

User input interpretation {#user_input_server}
===============================================
The commands that the user can input are:
These are not case-sensitive and there are numeric shortcuts.
Command                       |Shortcut
:-----------------------------|:-------:
join                          | 1
show\_servers                 | 2
show\_messages                | 3
exit                          | 9

Join command starts the communications to other servers and enables client communications.\n
The show_servers command prints the list currently being used to select the server at work.\n
The show_messages command prints the matrix currently being used to save messages.\n
Exit command breaks out of the loop.

TCP handling {#tcp_handle_server}
=====================================================
Server to server communications are based in two types of headers: `'SMESSAGES\n(message\n)\n'` or `'SGET_MESSAGES\n'`.

The parsing of information is made at the rate of the incoming bytes from the recv command, and split in '\n' sequences. 

The former is interpreted as a command to save the messages.
The later requests all the messages that this server has.

After receiving the information, it is saved in a message struct, in the case of 'SMESSAGES' being the header, the logical clock is set to the next logical clock of the MAX between LastMessageLC and IncomingMessageLC. (eg. if LastMessageLC == 20 and IncomingMessageLC == 5 so NewMessageLC = 21)

If 'SGET_MESSAGES' is received, the messages are fetched from the matrix and sent to the server who made the request.