# BlackChat
#

CC = gcc
CFLAGS = -Wall -O3 -ansi
LIBS = -lncurses
BUILD_DIR = bin

CLIENT_SRC_DIR = client
CLIENT_OBJECTS = main.o

all: client

client: $(CLIENT_SRC_DIR)/$(CLIENT_OBJECTS)
	$(CC) -o $(BUILD_DIR)/client $(CLIENT_SRC_DIR)/$(CLIENT_OBJECTS) $(LIBS)

clean:
	rm $(CLIENT_SRC_DIR)/$(CLIENT_OBJECTS)
	rm $(BUILD_DIR)/client
