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
    
  server->num_users_connected = 0;
  server->user_idle_time_max = 1800; //30 mins
  //server->clients = (CLIENT_OBJ *)malloc(sizeof(CLIENT_OBJ) * 11); //Make room for 10 clients
  memset(server->clients, 0, sizeof(CLIENT_OBJ));
    
  pthread_mutex_lock(&mutex);  
  fill_queue(server, messages);
  pthread_mutex_unlock(&mutex);
  
  if(pthread_create(&server->listen_thread_id, NULL, listen_thread, server) == -1)
    return NULL; //Unable to spawn the thread to accept clients
    
    
  return server;
    
  
  
}

void fill_queue(SERVER_OBJ *server, SERVER_QUEUE_OBJ *messages){
  
  for(int i = 0; i < MAX_CONNECTIONS; i++){
    
    CLIENT_OBJ *client = (CLIENT_OBJ *)malloc(sizeof(CLIENT_OBJ));
    memset(client, 0, sizeof(CLIENT_OBJ));
    
    client->bytes_from = 0;
    client->bytes_to = 0;
    client->user_data = (UR_OBJ)malloc(sizeof(UR_OBJ));
    client->user_data->history = (HST_OBJ)malloc(sizeof(HST_OBJ));
    client->user_data->im = (HST_OBJ)malloc(sizeof(HST_OBJ));
    client->user_data->history->next = NULL;
    client->user_data->im->next = NULL;
    client->user_data->lurk = 0;
    client->user_data->uid = i + 1;
    client->user_data->vote = -1;
    snprintf(client->user_data->name, sizeof(client->user_data->name - 1), "User%d", i);
    client->seconds_connected = 0;
    client->messages = messages;
    client->is_connected = false;
    
    server->clients[i + 1] = client;
        
    client = NULL;
  }
}
void *listen_thread(void *args){
  
  SERVER_OBJ *server = (SERVER_OBJ *)args;
     
  for(;;){
    struct sockaddr_in client_addr;
    socklen_t client_len;
    int temp_client;
    int num_connections;
  
    client_len = sizeof(client_addr);
    
    temp_client = accept(server->server_socket, (struct sockaddr *)&client_addr, &client_len);
    
    pthread_mutex_lock(&mutex);
    num_connections = server->num_users_connected;
    pthread_mutex_unlock(&mutex);
    
    if(num_connections < MAX_CONNECTIONS){
            
      for(int i = 1; i < MAX_CONNECTIONS + 1; i++){
      
	pthread_mutex_lock(&mutex);
	if(!server->clients[i]->is_connected){
	
	  server->clients[i]->client_socket = temp_client;
	  
	  if(pthread_create(&server->clients[i]->client_thread_id, NULL, client_thread, server->clients[i]) == -1)
	    //TODO handle error
	  server->clients[i]->is_connected = true;
	  
	}
	
	pthread_mutex_unlock(&mutex);
      }      
      
      server->num_users_connected += 1; 
      
    }
    else{
      char *buff = (char *)malloc(1024);
      //Uncomment once function is implemented
      //create_error_message(ERROR_CHAT_FULL, buff);
      
      write(temp_client, buff, 1024);      
      close(temp_client);      
    }
  }
return NULL;
}

void *client_thread(void *args){
  
  CLIENT_OBJ *client = (CLIENT_OBJ *)args;
  char *buff = (char *)malloc(1024 * 8);  //Read in 8k at a time should be more than enough
  int bytes_read;
  bool messages_empty;
  
  for(;;){
    pthread_testcancel();
    
    pthread_mutex_lock(&mutex);
    messages_empty = isEmpty(client->messages);
    pthread_mutex_lock(&mutex);
    
    if(!messages_empty){
      
      bytes_read = read(client->client_socket, buff, 1024*8);
      client->bytes_from += bytes_read;
      
      if(bytes_read == 0){
	
	break;
      }
      pthread_mutex_lock(&mutex);
      enqueue(client->messages, buff);
      pthread_mutex_unlock(&mutex);
    }
    else{
   
      bytes_read = read(client->client_socket, buff, 1024*8);
      client->bytes_from += bytes_read;
      
      if(bytes_read == 0)
	break;
      pthread_mutex_lock(&mutex);
      enqueue(client->messages, buff);
      pthread_mutex_unlock(&mutex);
      
      sem_post(&messages_sem); //Lets the main thread know there are messages to process
    
    } 
    
  pthread_mutex_lock(&mutex);
  client->bytes_from = 0;
  client->bytes_to = 0;
  client->is_connected = false;
  client->seconds_connected = 0;
  close(client->client_socket);
  pthread_mutex_unlock(&mutex);
 
  pthread_exit(0);
  
  }
return NULL;
}

void broadcast_all(CLIENT_OBJ* clients[], char* message){
  
  int bytes_written;
  
  for(int i = 1; i < MAX_CONNECTIONS + 1; i++){
  
    
    bytes_written = write(clients[i]->client_socket, message, 1024 * 8);
    
    pthread_mutex_lock(&mutex);
    clients[i]->bytes_to =+ bytes_written;
    pthread_mutex_unlock(&mutex);
    
  }

}

void broadcast_client(CLIENT_OBJ* client, char* message){
  
 int bytes_written;
 
 bytes_written = write(client->client_socket, message, 1024 * 8);
 
 pthread_mutex_lock(&mutex);
 client->bytes_to += bytes_written;
 pthread_mutex_unlock(&mutex);

}




