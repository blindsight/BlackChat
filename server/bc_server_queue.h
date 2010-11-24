
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
