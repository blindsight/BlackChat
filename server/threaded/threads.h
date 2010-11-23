/*
 * threads.h
 *
 *  Created on: Nov 2, 2010
 *      Author: Tyler Reid
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#ifndef THREADS_H_
#define THREADS_H_

pthread_t *client_threads = malloc(10 * sizeof(pthread_t));
int client_index = 0;


void threads_init(void (*listen)(void *arg));
void thread_client_spawn(void (*client)(void *arg), int client_fd);


#endif /* THREADS_H_ */
