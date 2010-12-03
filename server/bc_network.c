#include "bc_network.h"

void fill_queue(SERVER_OBJ *server, SERVER_QUEUE_OBJ *messages);
void *listen_thread(void *args);
void *client_thread(void *args);

SERVER_OBJ *init_network(SERVER_QUEUE_OBJ *messages){
  
  struct sockaddr_in server_addr;

  SERVER_OBJ *server = (SERVER_OBJ *)malloc(sizeof(SERVER_OBJ));
  memset(server, 0, sizeof(SERVER_OBJ));
  
  server->server_socket = socket(AF_INET, SOCK_STREAM, 0);
  
  if(pthread_mutex_init(&mutex, NULL) != 0)
    return NULL; //Couldn't create mutex
    
  if(sem_init(&messages_sem, 0, 0) == -1)
    return NULL; //Couldn't init semaphore
  
  if(server->server_socket == -1)
    return NULL;  //Couldn't create socket
    
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr (SERVER_IP);
  server_addr.sin_port = htons (SERVER_PORT);
  
  if(bind (server->server_socket, (struct sockaddr *) &server_addr, sizeof (server_addr)) == -1)
    return NULL; //Couldn't bind addrress
    
    
  if(listen(server->server_socket, LISTEN_QUEUE) == -1)
    return NULL;  //Could't Listen for incoming connections
    
  pthread_mutex_lock(&mutex);  
  fill_queue(server, messages);
  pthread_mutex_unlock(&mutex);
  
  if(pthread_create(&server->listen_thread_id, NULL, listen_thread, server) == -1)
    return NULL; //Unable to spawn the thread to accept clients
    
    
  return server;
    
  
  
}

void fill_queue(SERVER_OBJ *server, SERVER_QUEUE_OBJ *messages){
  
  server->unconnected_clients = init_queue(10);
  server->connected_clients = init_queue(10);
 

  for(int i = 0; i < MAX_CONNECTIONS; i++){
    
    CLIENT_OBJ *client = (CLIENT_OBJ *)malloc(sizeof(CLIENT_OBJ));
    memset(client, 0, sizeof(CLIENT_OBJ));
    
    client->user_data = (UR_OBJ)malloc(sizeof(UR_OBJ));
    client->user_data->history = (HST_OBJ *)malloc(sizeof(HST_OBJ));
    client->user_data->im = (HST_OBJ *)malloc(sizeof(HST_OBJ));
    client->user_data->lurk = 0;
    client->user_data->uid = i;
    client->user_data->name = NULL;
    client->seconds_connected = 0;
    client->messages = messages;
    
    enqueue(server->unconnected_clients, client);
    
    client = NULL;
  }
}
void *listen_thread(void *args){
  
  SERVER_OBJ *server = (SERVER_OBJ *)args;
     
  for(;;){
    struct sockaddr_in client_addr;
    socklen_t client_len;
    int temp_client;
    bool is_queue_empty;
  
    client_len == sizeof(client_addr);
    
    temp_client = accept(server->server_socket, (struct sockaddr *)&client_addr, &client_len);
    
    pthread_mutex_lock(&mutex);
    is_queue_empty = isEmpty(server->unconnected_clients);
    pthread_mutex_unlock(&mutex);
    
    if(!is_queue_empty){
      
            
      CLIENT_OBJ *client = (CLIENT_OBJ *)dequeue(server->unconnected_clients);
      client->client_socket = temp_client;
      
      
      if(pthread_create(&client->client_thread_id, NULL, client_thread, client) == -1);
	//TODO handle thread creation error
      
      enqueue(server->connected_clients, client);
      pthread_mutex_unlock(&mutex);
      
      client = NULL;
      
      
      
    }
    else{
    
      //TODO send server Full message
      
      close(temp_client);
      
    }
  }

  
}

void *client_thread(void *args){
  
  CLIENT_OBJ *client = (CLIENT_OBJ *)args;
  
  pthread_mutex_lock(&mutex);
  
  if(!isEmpty(client->messages)){
  
    //TODO read message and put it in messages queue
    
  }
  else{
   
    //TODO read message put it in messages queue 
    sem_post(&messages_sem); //Lets the main thread know there are messages to process
    
  } 
  
  pthread_mutex_unlock(&mutex);
  
}
