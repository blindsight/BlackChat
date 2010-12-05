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
#include "bc_network.h"
#include "blackchat.h"

UR_OBJ user_list[20];       //list of all users currently on server
UR_OBJ curr_user;           //this client
int uid = -1;               //user id for this client

/* This function is called whenever we (the client)
 * press the enter key. */
void write_out(int client_id)
{
    char *buffer = (char *)malloc(4096);
    char *status = (char *)malloc(4096);
    create_main_chat_message(uid,grab_text_from_client_typing_window(),buffer);
    create_status_message(uid,grab_text_from_client_typing_window(),status);

    if(write(client_id, buffer, strlen(buffer)*sizeof(char)) == -1) {
        write_to_transcript_window("Error: Couldn't write to server main chat message!\n");
       // write_to_transcript_window(buffer); 
    }
    if(write(client_id, status, strlen(status)*sizeof(char)) == -1) {
        write_to_transcript_window("Error: Couldn't write to server status message!\n");
       // write_to_transcript_window(buffer); 
    }
    clear_user_window_text(1);       //TODO: add window number
    clear_text_from_client_typing_window();
    free(buffer);
    free(status);
}

void write_yell(int client_id)
{
//TODO: there seems to be a discrepency between who actually keeps the yell message
}

void write_im(int client_id, int to_user)
{
    char *message = (char *)malloc(4096);
    //create_im_message(uid, #to_user, ...usertext ..., message);
    
    if(write(client_id, message, strlen(message)*sizeof(char)) == -1) {
        write_to_transcript_window("Error: Couldn't write to server for yell message!\n");
       // write_to_transcript_window(buffer); 
    }
    free(message);
   //TODO: finish IM message, clear text being written from user window 
}

void write_vote(int client_id)
{   
    char *vote_message = (char *)malloc(4096);
    //create_vote_message(uid, #for_user, vote_message);
    
    if(write(client_id, vote_message, strlen(vote_message)*sizeof(char)) == -1) {
        write_to_transcript_window("Error: Couldn't write to server for vote message!\n");
       // write_to_transcript_window(buffer); 
    }
    free(vote_message);
//TODO: clear transcript for vote?
}

void write_lurk(int client_id)
{
    char *lurk_result = (char *)malloc(4096);
    create_user_lurking(uid,lurk_result);

    if(write(client_id, lurk_result, strlen(lurk_result)*sizeof(char)) == -1) {
        write_to_transcript_window("Error: Couldn't write to server that client is lurking!\n");
       // write_to_transcript_window(buffer); 
    }
    free(lurk_result);
}

void write_window_session(int client_id)
{
    WIN_OBJ curr_window = (WIN_OBJ)malloc(sizeof(struct window_obj));

    free(curr_window);
    //TODO: finish this
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
    int user_lurk = -1;

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
                   // set_window_user_name( #win_number, char name);
                   // TODO: get the window numbers and name.
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
                char *vote_response = (char *)malloc(200);

                switch(vote_type)
                {
                case VOTE_ACCEPTED:
                    sprintf(vote_response, "%d vote accepted", user);
                    write_to_transcript_window(vote_response);
                break;
                case VOTE_NOT_ACCEPTED:
                    sprintf(vote_response, "%d vote NOT accepted", user);
                    write_to_transcript_window(vote_response);
                }
                free(vote_response);
            break;
            case CMD_LURK:
                user_lurk = get_user_lurking(servout);
                char *lurk_message = (char *)malloc(4096);

                sprintf(lurk_message, "user with id: %d is currently lurking", user_lurk);
                write_to_transcript_window(lurk_message);
            break;
            case CMD_USERLIST:
                ul_type = get_userlist_type_from_message(servout);
                switch(ul_type)
                {
                    case USER_LIST_CURRENT || USER_LIST_SIGN_OFF:       //This means we are getting the current user list from the server.
                        user_list[user_num] = (UR_OBJ)malloc(sizeof(struct user_obj));
                        offset = get_first_user(servout, user_list[user_num]);

                        do
                        {
                            user_num++;
                            user_list[user_num] = (UR_OBJ)malloc(sizeof(struct user_obj));
                        }
                        while((offset = get_next_user(offset,servout,user_list[user_num])) > 0);
                    break;
                    case USER_LIST_RECEIVE_UID:
                        uid = get_user_from_message(servout);
                        curr_user->uid = uid;
                    break;
                }  //TODO: user list sign off if it is actually needed.
            break;
            case CMD_ERROR:
                err_type = get_error_type_from_message(servout);

              /*  if(err_type == ERROR_CHAT_FULL)
                {
                    write_to_transcript_window("Sorry chat is full.");
                }*/
            break;
            default:
                write_to_transcript_window("INVALID CMD_TYPE FROM SERVER!");
            break;
            }

         }
    }
free(output);
free(text);
free(servout);
}


/* This function creates a socket for the client
 * then connects to the server and writes a short
 * message of "This is the client!" using the 
 * wchar_t wide character type. I also made the 
 * function return the file descriptor for the 
 * socket it creates. */
int init_client(char *name)
{
    int client;
    struct sockaddr_in address;
    char *message = (char *)malloc(4096);
    strcpy(curr_user->name, name);
    create_user_name_message(name,message);

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

    if ( write(client, message, strlen(message)*sizeof(char)) == -1)
    {
        perror("COULD NOT WRITE TO SERVER!");
        exit(1);
    }
    return client;
}

/* This is a function that is only used once in 
 * the client to get the initial user list when
 * the client connects to the server. */
void init_user_list(int client_id)
{
    char *temp = (char *)malloc(4096);
    request_user_list(curr_user, temp);
    if( write(client_id, temp, strlen(temp)*sizeof(char)) == -1)
    {
        perror("COULDN'T SEND MESSAGE!");
        exit(1);
    }
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


