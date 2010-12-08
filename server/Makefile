# BlackChat
# -- Server
#

CC = gcc
CFLAGS = -Wall -O3 -std=c99 -I../protocol/include -Iinclude
LIBS = -lpthread ../bin/lib/libbcprotocol.a
BUILD_DIR = ../bin
OBJS = bc_network.o bc_server_queue.o main.o

.PHONY: clean

all: server

server: $(OBJS)
	$(CC) $(CFLAGS)  -o $(BUILD_DIR)/server $(OBJS) $(LIBS)

clean:
	rm $(OBJS)
	rm $(BUILD_DIR)/server
