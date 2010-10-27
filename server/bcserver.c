#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "bcserver.h"


int server = 0;
int port;


void cleanup (int sig)
{
    close (server);
    exit (0);
}



void die (const char *msg)
{
    perror (msg);
    cleanup (0);
}



void handle_clients (void)
{
    struct sockaddr_in client_addr;
    socklen_t client_len;
    int client;
    char ch[1024];
    client_len = sizeof(client_addr);
    client = accept(server, (struct sockaddr *) &client_addr, &client_len);
    if(client == -1) die("server: bad client socket");

    for (;;)
    {
      //  int client;
       // char ch;

       // client_len = sizeof (client_addr);
        //client = accept (server, (struct sockaddr *) &client_addr, &client_len);
        //if (client == -1) die ("server: bad client socket");

        read (client, ch, 1024);
        //write (client, &ch, 1024);
        if(strcmp(ch, "exit") == 0)
            close (client);
        else
            write(client, ch, 1024);
    }
}



int main (int argc, char *argv[])
{

    struct sockaddr_in server_addr;

    signal (SIGTERM, cleanup);
    signal (SIGPIPE, SIG_IGN);
    
    if(argc == 2)
        port = argv[1];
    else
        port = SERVER_PORT;

    server = socket (AF_INET, SOCK_STREAM, 0);
    if (server == -1) die ("can't create server socket");

    memset (&server_addr, 0, sizeof (server_addr));  /* IMPORTANT! */
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr (SERVER_IP);
    server_addr.sin_port = htons (port);
    if (bind (server, (struct sockaddr *) &server_addr, sizeof (server_addr)) == -1)
        die ("can't bind server socket");

   
    if (listen (server, 5) == -1)
        die ("can't listen on server socket");

   
    printf ("\nServer %d (socket %d) listening on port %d.\n",
            getpid (), server, SERVER_PORT);

    handle_clients ();

    return 0;  
}
