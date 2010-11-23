/*
 * threads.c
 *
 *  Created on: Nov 2, 2010
 *      Author: Tyler Reid
 */
#include "threads.h"
//This function takes a function pointer and
//Initializes the listen thread
void thread_init(void (*listen)(void *arg)){
	//create Listen thread using supplied function
	if(pthread_create(&listener, NULL, listen, NULL) != 0){
		perror("Could not create Listener Thread!");
		exit(-1);
	}
}//end thread_init

//This function will be called every time we get a new connection
//If we are below our cap we will create a new thread for the client
//If we are at our maximum then we connect to the client and gracefully reject them
void thread_client_spawn(void (*client)(void *arg)){
	pthread_t client_thread;
	if(pthread_create(&client_thread, NULL, client, NULL) != 0){
		perror("could not spawn client thread!");
		exit(-1);
	}

	clients[client_index] = client_thread;
	if(client_index <= 10) client_index++;
}//end thread_client_spawn



