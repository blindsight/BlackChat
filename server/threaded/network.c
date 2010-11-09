/*
 * network.c
 *
 *  Created on: Nov 8, 2010
 *      Author: Tyler Reid
 */


#include "network.h"
#include "threads.h"


void kill_server(int sig){
	close(server);
	exit(0);
}

//This function will continually read from the client until the client closes then will
//close the client and add it's position to the queue so that the accept can add another client
void *client_thread(void *arg){

	int *client = (int *)arg;
	char* buf = malloc(1024);

	for(;;){
		ssize_t bytes_read;

		bytes_read = read(client, buf, 1024);

		if(bytes_read == 0){
			close(*client);
			//TODO remove this client from the array
			free(buf);
			pthread_exit(0);
		}

		write(client, buf, bytes_read);


	}

}

//This Function will wait for connection requests and will check if it has too many already
//If too many are connected it will connect send a is full message then disconnect
void *accept_connections_thread(void *arg){

	char too_many_connections[] = "Sorry The server is full try again later.";

	for(;;){
		struct sockaddr_in client_addr;
		socklen_t client_length;
		int client;

		client_length = sizeof(client_addr);

		client = accept(server, (struct sockaddr *) &client_addr, &client_length);
		if(client == -1){
			perror("Bad client socket");
			kill_server(0);
		}

		if(client_fd_index <= MAX_CONNECTIONS){
			client_fds[client_fd_index] = client;
			client_fd_index++;
			thread_client_spawn(client_thread, client);

		}
		else{
			write(&client, &too_many_connections, sizeof(too_many_connections));
			close(client);
		}



	}



}


void init_network(void){


	signal(SIGTERM, kill_server);
	signal(SIGPIPE, SIG_IGN);

	server = socket(AF_INET, SOCK_STREAM, 0);

	if(server == -1){
		perror("Error Creating sever socket.");
		kill_server(0);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	server_addr.sin_port = htons(SERVER_PORT);

	if(bind(server, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
		perror("Can't bind server socket");
		kill_server(0);
	}

	if(listen(server, 20) == -1){
		perror("Can't listen on server socket");
		kill_server(0);
	}

	printf("\n BCServer listening with socket: %d on port %d.\n", server, SERVER_PORT);

	//from here we spawn our accept thread
	threads_init(accept_connections_thread);


}
