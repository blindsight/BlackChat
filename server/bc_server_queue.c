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
#include "bc_server_queue.h"


SERVER_QUEUE_OBJ* init_queue(int max_size) {
  
    SERVER_QUEUE_OBJ* Q = (SERVER_QUEUE_OBJ *)malloc(sizeof(struct server_queue));
    
    if(Q == NULL)
      printf("Couldn't alloc");
    Q->capacity = max_size;  //if max size == -1 means unlimited size
    Q->size = 0;

    Q->head = (NODE_OBJ *)malloc(sizeof(struct node));
    Q->tail = Q->head;

    Q->head->data = NULL;
    Q->head->next = NULL;
    Q->head->prev = NULL;

    return Q;

}

void enqueue(SERVER_QUEUE_OBJ *q, void *data) {

    if (!isFull(q)) {

      void* real_data = (void*)malloc(1024 * 8);
      memcpy(real_data, data, 1024 * 8);

      NODE_OBJ *temp = (NODE_OBJ *)malloc(sizeof(NODE_OBJ));


        if (q->head == NULL || q->size == 0) {
            
            temp->data = real_data;
            temp->next = NULL;
	    temp->prev = NULL;

            q->head = temp;
            q->tail = temp;

            q->size++;

        }
        else {
            
            
            temp->data = real_data;
            temp->next = q->head;
	    temp->prev = NULL;
	    
	    q->head->prev = temp;

            q->head = temp;

            q->size++;
        }            


    }
}

void* dequeue(SERVER_QUEUE_OBJ* q){
  
  if(!isEmpty(q)){
    
    NODE_OBJ* temp = q->tail;
    q->tail = temp->prev;
    if(q->tail == NULL)
        q->tail = (struct node *)malloc(sizeof(struct node));
    q->tail->next = NULL;
    
    void* c = temp->data;
    
   // free(temp);
    
    q->size--;
    
    return c;
  }
  
  return NULL; //If the queue was empty return NULL
  
}

void* peek(SERVER_QUEUE_OBJ* q){

  if(!isEmpty(q))
    return q->tail->data;
  
  return NULL; //NULL if queue was empty
  
}


bool isFull(SERVER_QUEUE_OBJ* q){

  
  if(q->capacity == -1 || q->size < q->capacity)
    return false;
  else
    return true;
}

bool isEmpty(SERVER_QUEUE_OBJ* q){
  
  if(q->size == 0)
    return true;
  else
    return false;
  
}

int size(SERVER_QUEUE_OBJ* q){

  return q->size;
}

int capacity(SERVER_QUEUE_OBJ* q){

  return q->capacity;
}
