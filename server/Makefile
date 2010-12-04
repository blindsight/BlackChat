# BlackChat
# -- Server
#

CC = gcc
CFLAGS = -Wall -g -std=c99
LIBS = -lpthread ../bin/lib/libbcprotocol.so.1.0.1
BUILD_DIR = ../bin
OBJS = bc_network.o bc_server_queue.o main.o

.PHONY: clean

all: server

server: $(OBJS)
	$(CC) $(CFLAGS) -I../protocol/src -o $(BUILD_DIR)/server $(OBJS) $(LIBS)

clean:
	rm $(OBJS)
	rm $(BUILD_DIR)/server
