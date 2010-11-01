# BlackChat
#

CC = gcc
CFLAGS = -Wall -O3
LIBS = -lncurses
BUILD_DIR = bin

CLIENT_SRC_DIR = client
CLIENT_OBJECTS = $(CLIENT_SRC_DIR)/main.o $(CLIENT_SRC_DIR)/clientsocket.o

all: client

client: $(CLIENT_OBJECTS)
	$(CC) -o $(BUILD_DIR)/client $(CLIENT_OBJECTS) $(LIBS)

clean:
	rm $(CLIENT_OBJECTS)
	rm $(BUILD_DIR)/client
