CC = gcc
CFLAGS = -march=native -std=gnu11 -Wall -Wextra -g -Wpedantic -Wshadow -Wstrict-overflow -fno-strict-aliasing
CFLAGS_RELEASE = -march=native -std=gnu11 -O2
CLIENT = rmb
SERVER = msgserv
UTILS_DIR = src/utils

.PHONY: default all clean

all: client server

release:
	$(CC) $(CFLAGS_RELEASE) -I$(UTILS_DIR) $(wildcard wildcard src/utils/*.c) $(wildcard src/rmb/*.c) -o bin/$(CLIENT)
	$(CC) $(CFLAGS_RELEASE) -I$(UTILS_DIR) $(wildcard wildcard src/utils/*.c) $(wildcard src/msgserv/*.c) -o bin/$(SERVER)

client: $(wildcard src/rmb/*.c)
	$(CC) $(CFLAGS) -I$(UTILS_DIR) $(wildcard wildcard src/utils/*.c) $(wildcard src/rmb/*.c) -o bin/$(CLIENT)

server: $(wildcard src/msgserv/*.c)
	$(CC) $(CFLAGS) -I$(UTILS_DIR) $(wildcard wildcard src/utils/*.c) $(wildcard src/msgserv/*.c) -o bin/$(SERVER)

clean:
	rm $(wildcard bin/*)
