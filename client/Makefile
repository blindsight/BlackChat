# BlackChat
#

.PHONY: clean

CC = gcc
CFLAGS = -Wall -g -O3 -I../protocol/include -Iinclude -I../server/include -I../ncurses/include
# LIBS = ../bin/lib/libncursesw.a ../bin/lib/libbcprotocol.a
LIBS = -lncurses ../bin/lib/libbcprotocol.a
BUILD_DIR = ../bin
OBJS = logger.o client.o clientsocket.o



all: client

client: $(OBJS)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/client $(OBJS) $(LIBS)

clean:
	rm $(OBJS)
	rm $(BUILD_DIR)/client
	rm client.log
