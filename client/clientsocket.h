/*
 * Blackchat.
 */

#ifndef __CLIENT_SOCKET__
#define __CLIENT_SOCKET__


/* Creates a client socket and returns the file descriptor for it. */
int init_client(char *name);

/* initializes the client side user list */
void init_user_list(int client_id);

/* Closes a client socket.  You must pass in the file descriptor created by init_client */
void close_client(int client_id);

/* Received enter key from client. */
void write_out();

/* Read from server. */
void read_from_server(int client_id);


#endif

