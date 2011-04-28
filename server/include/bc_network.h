/* Copyright (C) 2010  BlackChat Group 
This file is part of BlackChat.

Ashes is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Ashes is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with BlackChat.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef BC_NETWORK_H_
#define BC_NETWORK_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdbool.h>
#include "bc_server_queue.h"
#include "bc_client.h"
#include "blackchat.h"

#define SERVER_PORT 11380
#define SERVER_IP "127.0.0.1"
#define MAX_CONNECTIONS 10
#define LISTEN_QUEUE 20
#define MAX_IDLE_TIME 1800


pthread_mutex_t mutex;
sem_t messages_sem;

typedef struct server{
  
  int server_socket;
  int user_idle_time_max;
  int num_users_connected;
  
  
  //char *yell_messages[26]; 
  pthread_t listen_thread_id;
  
  
  
  //SERVER_QUEUE_OBJ *connected_clients;
  //SERVER_QUEUE_OBJ *unconnected_clients;
  
  CLIENT_OBJ* clients[11];
    
} SERVER_OBJ;


SERVER_OBJ *init_network(SERVER_QUEUE_OBJ *messages);
void broadcast_all(CLIENT_OBJ *clients[], char *message); 
void broadcast_client(CLIENT_OBJ *client, char *message); 

void *client_thread(void *args);

#endif //BC_NETWORK_H_
