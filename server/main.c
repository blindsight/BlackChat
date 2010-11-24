#include "bc_client.h"
#include "bc_server_queue.h"
#include "bc_network.h"
#include <unistd.h>

SERVER_OBJ *bc_server;

void update_time(int sig){

  //TODO update each connected client with a new time
}

void cleanup(int sig){

  //TODO add cleanup code
  close(bc_server->server_socket);
  free(bc_server->connected_clients);
  free(bc_server->unconnected_clients);
  free(bc_server);
  exit(0);
  
}

void handle_messages(SERVER_OBJ *server, SERVER_QUEUE_OBJ *messages);

int main(int argc, char **argv) {
  
  signal(SIGTERM, cleanup);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGALRM, update_time); 
  
  bc_server = (SERVER_OBJ *)malloc(sizeof(SERVER_OBJ));
  memset(bc_server, 0, sizeof(SERVER_OBJ));
  
  SERVER_QUEUE_OBJ *messages = (SERVER_QUEUE_OBJ *)malloc(sizeof(SERVER_QUEUE_OBJ));
  memset(messages, 0, sizeof(SERVER_QUEUE_OBJ));
  
  messages = init_queue(-1);
  
  bc_server = init_network(messages);
  
  if(bc_server == NULL){
  
    perror("Unable to init server");
    exit(-1);
    
  }
  
  //Once here I need to start handling messages
  
  handle_messages(bc_server, messages);
  
    
}


