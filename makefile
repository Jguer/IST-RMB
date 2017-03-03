CC = gcc
CFLAGS = -Wall -Wextra -ansi -g --pedantic
CLIENT = rmb
SERVER = msgserv

.PHONY: default all clean

all: client server

client: $(wildcard src/rmb/*.c)
	$(CC) $(CFLAGS) $(wildcard src/rmb/*.c) -o bin/$(CLIENT)

server: $(wildcard src/msgserv/*.c)
	$(CC) $(CFLAGS) $(wildcard src/msgserv/*.c) -o bin/$(SERVER)

clean:
	rm $(wildcard bin/*)
