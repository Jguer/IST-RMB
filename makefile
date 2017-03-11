CC = gcc
CFLAGS = -Wall -Wextra -g --pedantic
CLIENT = rmb
SERVER = msgserv
UTILS_DIR = src/utils

.PHONY: default all clean

all: client server

client: $(wildcard src/rmb/*.c)
	$(CC) $(CFLAGS) -I$(UTILS_DIR) $(wildcard wildcard src/utils/*.c) $(wildcard src/rmb/*.c) -o bin/$(CLIENT)

server: $(wildcard src/msgserv/*.c)
	$(CC) $(CFLAGS) -I$(UTILS_DIR) $(wildcard wildcard src/utils/*.c) $(wildcard src/msgserv/*.c) -o bin/$(SERVER)

clean:
	rm $(wildcard bin/*)
