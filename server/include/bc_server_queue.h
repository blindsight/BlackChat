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
#ifndef BC_SERVER_QUEUE_H_
#define BC_SERVER_QUEUE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "bc_client.h"

typedef struct node {

    void *data;

    struct node *next;
    struct node *prev;


} NODE_OBJ;

typedef struct server_queue {

    int size;
    int capacity;

    NODE_OBJ *head;
    NODE_OBJ *tail;

} SERVER_QUEUE_OBJ;

SERVER_QUEUE_OBJ *init_queue(int max_size);

void enqueue(SERVER_QUEUE_OBJ *q, void *data);
void* dequeue(SERVER_QUEUE_OBJ *q);
void* peek(SERVER_QUEUE_OBJ *q);
bool isFull(SERVER_QUEUE_OBJ *q);
bool isEmpty(SERVER_QUEUE_OBJ *q);
int size(SERVER_QUEUE_OBJ *q);
int capacity(SERVER_QUEUE_OBJ *q);

#endif //BC_SERVER_QUEUE_H_
