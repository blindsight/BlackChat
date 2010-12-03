

#ifndef BC_CLIENT_H_
#define BC_CLIENT_H_

#include <pthread.h>
#include "bc_server_queue.h"
#include "../protocol/blackchat.h" 

typedef struct client {

    int client_socket;
    //int client_id; in user object
    //char *client_name; in user object
    unsigned int bytes_to;
    unsigned int bytes_from;
    unsigned int seconds_connected;
    struct server_queue *messages;
    pthread_t client_thread_id;

    UR_OBJ user_data;

} CLIENT_OBJ;

#endif //BC_CLIENT_H_
