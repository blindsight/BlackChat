#include "bc_server_queue.h"


SERVER_QUEUE_OBJ* init_queue(int max_size) {
  
    SERVER_QUEUE_OBJ* Q = (SERVER_QUEUE_OBJ *)malloc(sizeof(struct server_queue));
    
    if(Q == NULL)
      printf("Couldn't alloc");
    Q->capacity = max_size;  //if max size == -1 means unlimited size
    Q->size = 0;

    Q->head = (NODE_OBJ *)malloc(sizeof(struct node));
    Q->tail = (NODE_OBJ *)malloc(sizeof(struct node));

    Q->head->data = NULL;
    Q->head->next = NULL;
    Q->head->prev = NULL;

    return Q;

}

void enqueue(SERVER_QUEUE_OBJ *q, void *data) {

    if (!isFull(q)) {
      
      NODE_OBJ *temp = (NODE_OBJ *)malloc(sizeof(NODE_OBJ));

        if (q->head == NULL) {
            
            temp->data = data;
            temp->next = NULL;
	    temp->prev = NULL;

            q->head = temp;
            q->tail = temp;

            q->size++;

        }
        else {
            
            temp->data = data;
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
    q->tail->next = NULL;
    
    void* c = temp->data;
    
    free(temp);
    
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
