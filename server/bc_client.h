

#ifndef BC_CLIENT_H_
#define BC_CLIENT_H_

#include <pthread.h>
#include "bc_server_queue.h"

typedef struct client {

    int client_socket;
    int client_id;
    char *client_name;
    int bytes_to;
    int bytes_from;
    int seconds_connected;
    struct server_queue *messages;
    pthread_t client_thread_id;

    //TODO add protocol USR_OBJ

} CLIENT_OBJ;

#endif //BC_CLIENT_H_
