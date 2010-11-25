/* functions for client socket */
/* Josh Hartman */
/* itsthejash@gmail.com | hartman0331@live.missouristate.edu */
/* Black Group */
/* edited 11/24/10 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <wchar.h>
#include "client.h"
#include "clientsocket.h"
#include "../server/bcserver.h"
#include "../server/protocol.h"


/* This function is called whenever we (the client)
 * press the enter key. */
void write_out(int client_id)
{
    int type = 1;                    //set at 1 for testing (1: to transcript window)
    int by_sent;                    //bytes sent total!
    int from = client_id;           //being sent from.
    int to = 11;                    //set at 11 for testing (11: send to server)
    char *message = grab_text_from_client_typing_window();
    char buffer[2000];
    by_sent = 16 + sizeof(char)*strlen(message);        //total bytes 4*4 = 16 for the first 4 ints + string bytes
    sprintf(buffer,"%d%d%d%d%s", type, by_sent, from, to, message);     //combine to make string = [(type)(bytes sent)(from)(to)(message . . .)]
    if(write(client_id, buffer, by_sent) == -1) {
        write_to_transcript_window("Error: Couldn't write to server!\n");
       // write_to_transcript_window(buffer); 
    }

    clear_text_from_client_typing_window();
}

void read_from_server(int client_id)
{
    char servout[2000];
    fd_set servs;
    FD_ZERO(&servs);
    FD_SET(client_id, &servs);
    struct timeval timeout = {0, 75000};

    memset(servout, '\0', sizeof(servout));

    switch(select(client_id +1, &servs, 0, 0, &timeout))
    {
    case 0:
        break;
    case -1:
        write_to_transcript_window("Error: couldn't read from server!\n");
        break;
    default:
        if(read(client_id, servout, sizeof(servout)) > 0)
        {
            write_to_transcript_window(servout);
        }

    }
}


/* This function creates a socket for the client
 * then connects to the server and writes a short
 * message of "This is the client!" using the 
 * wchar_t wide character type. I also made the 
 * function return the file descriptor for the 
 * socket it creates. */
int init_client(void)
{
    int client;
    struct sockaddr_in address;
    char message[] = "This is the client!";

    client = socket(AF_INET, SOCK_STREAM, 0);   /* create the socket */
    if(client == -1)
    {
        perror("FAILED TO CREATE SOCKET!");
        exit(1);
    }

    memset (&address, 0, sizeof (address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr ("127.0.0.1");     /* server IP??? *///***********
    address.sin_port = htons (SERVER_PORT);       /* server port??? *///************

    if (connect (client, (struct sockaddr *) &address, sizeof (address)) == -1)     /* connect to the server */
    {
        perror("COULD NOT CONNECT TO SERVER!");
        exit(1);
    }

    if ( write(client, &message, sizeof(message)) == -1)
    {
        perror("COULD NOT WRITE TO SERVER!");
        exit(1);
    }
    return client;
}

/* This is a short function to close
 * the client socket and accepts the 
 * file descriptor as input. */
void close_client(int client_id)
{
    /* send the server our 'quit' message. */
    if( write(client_id, "exit", sizeof("exit")) == -1 )
    {
        perror("COULDN'T SEND EXIT MESSAGE!");
        exit(1);
    }

    if( close(client_id) == -1)      /* close the socket */
    {
        perror("COULD NOT CLOSE CLIENT!");
        exit(1);
     }
}


