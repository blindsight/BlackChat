#include "bc_client.h"
#include "bc_server_queue.h"
#include "bc_network.h"
#include <unistd.h>



SERVER_OBJ *bc_server;

void update_time(int sig){
    char *buff = (char *)malloc(256);
    char *lurk_text = (char *)malloc(16);
    char *to_client = (char *)malloc(256);
    

    printf("UpdateTime\n");

    for(int i = 1; i <= 10; i++){
      memset(buff, '\0', 256);
      memset(lurk_text, '\0', 16);
      memset(to_client, '\0', 256);
      if(bc_server->clients[i]->is_connected){
	bc_server->clients[i]->seconds_connected = time(NULL) - bc_server->clients[i]->time_connected;
        if(bc_server->clients[i]->user_data->lurk)
            strcpy(lurk_text, "True");
        else
            strcpy(lurk_text, "False");
         
      sprintf(buff, "Bytes sent: %d | Bytes received: %d\nSeconds Connected: %d | Lurking: %s\nUser name: %s", bc_server->clients[i]->bytes_from, bc_server->clients[i]->bytes_to,
              bc_server->clients[i]->seconds_connected, lurk_text, bc_server->clients[i]->user_data->name);
    
      create_yell_message(i, buff, to_client);
      printf("Status Update message: %s\n", to_client);
      broadcast_all(bc_server->clients, to_client);
      }     
		     
    }

    free(buff);
    free(lurk_text);
    free(to_client);
    //alarm(1);
}

void cleanup(int sig){

  #ifdef DEBUG
    printf("cleanup");
#endif
  //TODO add cleanup code
  close(bc_server->server_socket);
  free(bc_server->clients);
//  free(bc_server->connected_clients);
//  free(bc_server->unconnected_clients);
  free(bc_server);
  exit(0);
  
}

void handle_messages(SERVER_OBJ *server, SERVER_QUEUE_OBJ *messages);
int save_user_window(WIN_OBJ window, int user);
int get_user_window(WIN_OBJ window, int user);
void disconnect_user(int uid);

int main(int argc, char **argv) {
  signal(SIGTERM, cleanup);
  signal(SIGPIPE, SIG_IGN);
  //signal(SIGALRM, update_time); 
  #ifdef DEBUG
    printf("start main");
#endif


  
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
 //   printf("before handle messages\n");
   // alarm(1);
 for(;;){
   //  if(bc_server->num_users_connected > 0)
     //   update_time(0);
  handle_messages(bc_server, messages);
  update_time(0);
 }
  printf("After handle messages\n");
  
  exit(0);
  
    
}

void handle_messages(SERVER_OBJ* server, SERVER_QUEUE_OBJ* messages){
  
  int cmd_type;
  int text_type;
//  int window_type;
//  int vote_type;
  int user_type;
  int error_type;
  char *message;
//  char *message_data;

  //update_time(0);
    
  if(isEmpty(messages)){
      printf("Waiting on semaphore main thread\n");
	sem_wait(&messages_sem);
      printf("Done waiting on semaphore\n");  
  }

  printf("Before extracting message\n");
  pthread_mutex_lock(&mutex);
  message = dequeue(messages);	 
  pthread_mutex_unlock(&mutex);
  //printf("Message extracted: %s\n", message);
  
  cmd_type = get_type_from_message(message);

 // printf("CMD_TYPE: %d\n", cmd_type);
  
  switch(cmd_type) {
  
    case CMD_TEXT:
	text_type = get_text_type_from_message(message);
       // printf("TEXT_TYPE: %d\n", text_type);
	switch(text_type) {
	
	  case TEXT_MAIN_CHAT:
	  {
	    char *buff = (char *)malloc(1024 * 8);
            char *message_to_server = (char *)malloc(1024 * 8);
            memset(buff, '\0', 1024 * 8);
            memset(message_to_server, '\0', 1024 * 8);
            get_text_from_message(message, buff);

            //printf("Contents of Buff: %s\n", buff);

            int user = get_user_from_message(message);

            sprintf(message_to_server, "%s says: %s", server->clients[user]->user_data->name, buff);  
            
            //printf("Message to Server: %s\n", message_to_server);
	    
	   // HST_OBJ temp = server->clients[user]->user_data->history;
	    
	    
	    //server->clients[user]->user_data->history->from = NULL;
	    //server->clients[user]->user_data->history->next = temp;
	    
            server->clients[user]->seconds_connected = time(NULL) - server->clients[user]->time_connected;
	    
	    create_text_message(TEXT_MAIN_CHAT, user, message_to_server, buff);

            //printf("Sending to client: %s\n", buff);
	    
	    broadcast_all(server->clients, buff);
	    
	    free(buff);	    
	  }    
	    break;
	  case TEXT_YELL:
	  {
	    int user = get_user_from_message(message);
	    char *result = (char *)malloc(1024 * 8);
	    char *buff = (char *)malloc(1024 * 8);
	    get_text_from_message(message,result);
	
	    create_yell_message(user, result, buff);
	    
	    broadcast_all(server->clients, buff);
            
            free(result);
	    free(buff);
	   }
	    break;
	  case TEXT_STATUS:
	  {
	   
	    int user = get_user_from_message(message);
            char *buff = (char *)malloc(1024 * 8);
            char *to_client = (char *)malloc(512);
            char *temp = (char *)malloc(32);
            int len_of_str;

            get_text_from_message(message, buff);
            
            len_of_str = strlen(buff);

            if(len_of_str > 20){

                strncpy(temp, buff+len_of_str-20, 20);
                strncpy(buff, temp, 21);
            
            }


            sprintf(to_client, "%s\n%s", server->clients[user]->user_data->name, buff);

            
            memset(buff, '\0', strlen(to_client));
            create_status_message(user, to_client, buff);

           // printf("Status message sent: %s\n", buff);

            broadcast_all(server->clients, buff);

            free(buff);
            free(temp);
            free(to_client);

	    
	    
	}
	    break;
	  case TEXT_IM:
	  {
	    int user = get_from_user_from_message(message);
	    int to_user = get_user_from_message(message);
	    HST_OBJ temp = server->clients[user]->user_data->im;
	    char *buff = (char *)malloc(1024 * 8);
	    
	    get_text_from_message(message, server->clients[user]->user_data->im->line);
	    //server->clients[user]->user_data->im->from = NULL;
	    //server->clients[user]->user_data->im->next = temp;
	    //TODO update time
	    
	    create_im_message(user, to_user, server->clients[user]->user_data->im->line, buff);
	    
	    broadcast_client(server->clients[to_user], buff);
	    
	    free(buff);
	    
	  }
	    break;
	  default:
	  {
//	    int user = get_from_user_from_message(message);
	//    char *temp = "Invalid Message Sent to Server!!\n";
	    //TODO send error
	    //create_text_message();
	  }
	    break;
	  
	}
      break;
    case CMD_WINDOW:
    {
      WIN_OBJ window = (WIN_OBJ)malloc(sizeof(struct window_obj));

      get_window_from_message(message, window);

      save_user_window(window, get_user_from_message(message));

	free(window);
    } 
      break;
    case CMD_VOTE:
    {
      int voted_user = get_voted_for_uid_from_message(message);
      int user = get_user_from_message(message);
      int num_votes = 0;
      //bool is_vote_done = false;
      char *buff = (char *)malloc(1024 * 8);
      
      if(server->clients[user]->user_data->vote != voted_user){
	
	server->clients[user]->user_data->vote = voted_user;
	
	respond_vote_message(VOTE_ACCEPTED, user, voted_user, buff);
	
	broadcast_client(server->clients[user], buff);
	
      }
      else{
      
	respond_vote_message(VOTE_NOT_ACCEPTED, user, voted_user, buff);
	broadcast_client(server->clients[user], buff);
	
      }
      
      for(int i = 1; i < MAX_CONNECTIONS + 1; i++){
      
	pthread_mutex_lock(&mutex);
	if(server->clients[i]->is_connected && server->clients[i]->user_data->vote != -1)
	  num_votes++;
	
	pthread_mutex_unlock(&mutex);
	
      }
      
      if(num_votes == server->num_users_connected){
	int votes[] = {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int user_voted_off = 0;
	
	for(int i = 1; i < MAX_CONNECTIONS; i++){
	
	  pthread_mutex_lock(&mutex);
	  if(server->clients[i]->is_connected && server->clients[i]->user_data->vote != -1){	  
	    votes[server->clients[i]->user_data->vote]++;	    
	  }	  
	  pthread_mutex_unlock(&mutex);
	  
	}
	
	for(int i = 0; i < 11; i++){
	
	  if(votes[user_voted_off] > votes[i])
	    user_voted_off = i;
	  
	}
	
	disconnect_user(user_voted_off);
        server->num_users_connected -= 1;
        
	
	for(int i = 1; i <= MAX_CONNECTIONS; i++){
	
	  pthread_mutex_lock(&mutex);
	  server->clients[i]->user_data->vote = -1;
	  pthread_mutex_unlock(&mutex);
	  
	}
	
      }
      free(buff);
    }
      
      break;
    case CMD_USERLIST:
      user_type = get_userlist_type_from_message(message);
      
      switch(user_type){
      
	case USER_LIST_REQUEST:
	  //TODO loop through user array and send to client
	  break;
	case USER_LIST_SIGN_OFF:
	  //Prefer not to use this and would rather detect a read of 0 bytes from client to disconnect
	  break;
	default:
	  //TODO send ERROR_UNKNOWN_MSG
	  break;
	
      }
      break;
    case CMD_LURK:
      {
          char *message_to_server = (char *)malloc(1024 * 8);
          char *buff = (char *)malloc(1024 * 8);

          int user = get_user_lurking(message);

          if(server->clients[user]->user_data->lurk){
              sprintf(message_to_server, "%s is no longer lurking!", server->clients[user]->user_data->name);
              server->clients[user]->user_data->lurk = 0;
          }
          else
          {
              sprintf(message_to_server, "%s is Lurking!!!", server->clients[user]->user_data->name);
              server->clients[user]->user_data->lurk = 1;

          }

          printf("Lurk message: %s\n", message_to_server);

          create_text_message(TEXT_MAIN_CHAT, user, message_to_server, buff);

          broadcast_all(server->clients, buff);
          free(message_to_server);
          free(buff);
      }
      break;
    case CMD_ERROR:
	error_type = get_error_type_from_message(message);
	
	switch(error_type){
	
	  default:
	    //TODO send ERROR_UNKNOWN_MSG
	    break;
	  
	}
      break;
    default:
      //TODO send ERROR_UNKNOWN_MSG
    break;
  }

}


int save_user_window(WIN_OBJ window, int user){
  
  char *file_to_open = (char *)malloc(1024);
  FILE *file;
  int written;
  int win[] = { window->h, window->w, window->x, window->y, window->z, window->type, window->wid};
  sprintf(file_to_open, "./saved_sessions/%d/%d/%d", user, window->type, window->wid);
  
  file = fopen(file_to_open, "wb");
  if(file == NULL)
    return -1;
    
  written = fwrite(win, sizeof(int), 7, file);
  
  if(written != 7)
    return -1;
  
  fclose(file);
  free(file_to_open);
  
  return 0;

}

int get_user_window(WIN_OBJ window, int user){  
    
  char *file_to_open = (char *)malloc(1024);
  FILE *file;
  int read;
  int win[] = { 0, 0, 0, 0, 0, 0, 0};
  sprintf(file_to_open, "./saved_sessions/%d/%d/%d", user, window->type, window->wid);
  
  file = fopen(file_to_open, "rb");
  if(file == NULL)
    return -1;
    
  read = fread(win, sizeof(int), 7, file);
  
  if(read != 7)
    return -1;
  
  window->h = win[0];
  window->w = win[1];
  window->x = win[2];
  window->y = win[3];
  window->z = win[4];
  window->type = win[5];
  window->wid = win[6];
  
  fclose(file);
  free(file_to_open);
  
  return 0;

}


void disconnect_user(int uid){
  CLIENT_OBJ *temp = bc_server->clients[uid];
  pthread_mutex_lock(&mutex);
  
  pthread_cancel(temp->client_thread_id);
  temp->bytes_from = 0;
  temp->bytes_to = 0;
  temp->is_connected = false;
  close(temp->client_socket);
  pthread_mutex_unlock(&mutex);
}



