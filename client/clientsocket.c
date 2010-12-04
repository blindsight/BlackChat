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
#include "../server/bc_network.h"
//#include "../server/protocol.h"
//#include "../protocol/src/blackchat.h"

UR_OBJ user_list[20];       //list of all users currently on server
UR_OBJ curr_user;           //this client

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
    WIN_OBJ window;

    int cmd_type = -1;
    int user = -1;
    int text_type = -1;
    int from_user = -1;
    int vote_type = -1;
    int ul_type = -1;
    int user_num = 0;
    int offset = 0;
    int err_type = -1;

    char *output = (char *)malloc(4096);
    char *text = (char *)malloc(4096);
    char *servout = (char *)malloc(4096);

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
            cmd_type = get_type_from_message(servout);
            user = get_user_from_message(servout);

            switch(cmd_type)
            {
            case CMD_TEXT:
                get_text_from_message(servout, text);
                text[strlen(text)-1] = '\0';
                text_type = get_text_type_from_message(servout);
                if(text_type == TEXT_IM)        //send to IM window
                {
                    from_user = get_from_user_from_message(servout);
                    //TODO: write to IM window
                }
                else                            //send to transcript window
                {
                    sprintf(output, "%d says: %s", user, text);
                    write_to_transcript_window(output);
                }
            break;
            case CMD_WINDOW:
                window = (WIN_OBJ)malloc(sizeof(struct window_obj));

                get_window_from_message(servout, window);
                //TODO: wrtie out window somehow?
                free(window);
            break;
            case CMD_VOTE:
                from_user = get_voted_for_uid_from_message(servout);
                vote_type = get_vote_type_from_message(servout);
                //TODO: possibly print out who user voted for?

            break;
            case CMD_USERLIST:
                ul_type = get_userlist_type_from_message(servout);
                if(ul_type == USER_LIST_CURRENT)        //This means we are getting the current user list from the server.
                {
                    user_list[user_num] = (UR_OBJ)malloc(sizeof(struct user_obj));
                    offset = get_first_user(servout, user_list[user_num]);

                    do
                    {
                        user_num++;
                        user_list[user_num] = (UR_OBJ)malloc(sizeof(struct user_obj));
                    }
                    while((offset = get_next_user(offset,servout,user_list[user_num])) > 0);
                }
                //TODO: user list sign off if it is actually needed.
            break;
            case CMD_ERROR:
                err_type = get_error_type_from_message(servout);
                //TODO: process error?
            break;
            default:
                write_to_transcript_window("INVALID CMD_TYPE FROM SERVER!");
            break;
            }

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


