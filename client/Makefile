# BlackChat
#

.PHONY: clean

CC = gcc
CFLAGS = -Wall -O3
LIBS = -lncurses ../bin/lib/libbcprotocol.so.1.0.1
BUILD_DIR = ../bin
OBJS = logger.o client.o clientsocket.o



all: client

client: $(OBJS)
	$(CC) $(CFLAGS) -I../protocol/src -o $(BUILD_DIR)/client $(OBJS) $(LIBS)

clean:
	rm $(OBJS)
	rm $(BUILD_DIR)/client
