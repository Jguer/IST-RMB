Reliable Message Board {#mainpage}
======================
Instituto Superior Técnico, Lisboa  
MEEC

This program was developed by two IST students for evaluation
in the course of RCI (*Internet and Computer Networks*).

The program is deployed with 2 applications, one is for the client side and other for the server side.
______________________________________________________________________________
##Client Side Application:
The client is capable of sending messages to a server and get the latest messages that the user requested.  
The defined terminal interface is:  
> publish __message__\n  
> show\_latest\_messages __n__\n  
> show\_servers\n  
> show\_selected\_server\n  
> exit\n

####publish
Sends to the server the content written in __message__ up to 140 characters
####show\_latest\_messages
Asks the server for the last __n__ messages and prints the response on the terminal
####show\_servers
Show the list of connect-able servers, servers that have been unresponsive only appear after some time
####show\_selected_server
Prints the info about the server to which the client is connected
####exit
Exits the application

Technical aspects of client application: See \ref Technical_C
_________________________________________________________________________
##Server side application
The server receives messages from clients and distributes them across the other servers. On request it gives the last
The defined terminal interface is:  
> join\n  
> show\_messages\n  
> show\_servers\n  
> exit\n

####join
Registers this server on the identity server, checks for other servers and connects to them.
####show\_messages
Prints all the messages received by this server
####show\_servers
Show the list of connected servers
####exit
Exits the application

Technical aspects of server application: See \ref Technical_S

Developed by:
----------------------
João Miguel Guerreiro 81248\n  
João Mário Domingos 81713\n