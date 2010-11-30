# BlackChat
# -- Server
#

CC = gcc
CFLAGS = -Wall -O3 -std=c99
LIBS = -lpthread
BUILD_DIR = ../bin
OBJS = bc_network.o bc_server_queue.o main.o


all: server

server: $(OBJS)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/server $(OBJS) $(LIBS)

clean:
	rm $(OBJS)
	rm $(BUILD_DIR)/server
