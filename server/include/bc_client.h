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
#ifndef BC_CLIENT_H_
#define BC_CLIENT_H_

#include <pthread.h>
#include <time.h>
#include "bc_server_queue.h"
#include "blackchat.h"

typedef struct client {

    int client_socket;
    unsigned int bytes_to;
    unsigned int bytes_from;
    time_t time_connected;
    time_t seconds_connected;
    bool is_connected;
    struct server *server;
    struct server_queue *messages;
    pthread_t client_thread_id;

    UR_OBJ user_data;

} CLIENT_OBJ;

#endif //BC_CLIENT_H_
