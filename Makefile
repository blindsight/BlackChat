# BlackChat
#

CC = gcc
CFLAGS = -Wall -O3
LIBS = -lncurses
BUILD_DIR = bin

CLIENT_SRC_DIR = client
CLIENT_OBJECTS = $(CLIENT_SRC_DIR)/client.o $(CLIENT_SRC_DIR)/clientsocket.o

SERVER_SRC_DIR = server
SERVER_OBJECTS = $(SERVER_SRC_DIR)/bcserver.o


all: client bcserver

client: $(CLIENT_OBJECTS)
	$(CC) -o $(BUILD_DIR)/client $(CLIENT_OBJECTS) $(LIBS)

bcserver: $(SERVER_OBJECTS)
	$(CC) -o $(BUILD_DIR)/bcserver $(SERVER_OBJECTS) $(LIBS)

clean:
	rm $(SERVER_OBJECTS)
	rm $(CLIENT_OBJECTS)
	rm $(BUILD_DIR)/bcserver
	rm $(BUILD_DIR)/client
