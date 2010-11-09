/*
 * network.h
 *
 *  Created on: Nov 8, 2010
 *      Author: Tyler Reid
 */

#ifndef NETWORK_H_
#define NETWORK_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define SERVER_PORT 11380
#define SERVER_IP "127.0.0.1"
#define MAX_CONNECTIONS 10

int server = 0;
struct sockaddr_in server_addr;
int *client_fds = (int)malloc(10 * sizeof(int));
int client_fd_index = 0;

void kill_server(int sig);
void init_network(void);


#endif /* NETWORK_H_ */
