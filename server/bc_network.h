

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

#define SERVER_PORT 11380
#define SERVER_IP "127.0.0.1"
#define MAX_CONNECTIONS 10
#define LISTEN_QUEUE 20


pthread_mutex_t mutex;
sem_t messages_sem;

typedef struct server{
  
  int server_socket;
  int num_clients;
  pthread_t listen_thread_id;
  
  SERVER_QUEUE_OBJ *connected_clients;
  SERVER_QUEUE_OBJ *unconnected_clients;
    
} SERVER_OBJ;


SERVER_OBJ *init_network(SERVER_QUEUE_OBJ *messages);
void broadcast_all(SERVER_QUEUE_OBJ *clients); //TODO add message
void broadcast_client(CLIENT_OBJ *client); //TODO add message


#endif //BC_NETWORK_H_