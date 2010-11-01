/*
 * Blackchat.
 */

#ifndef __CLIENT_SOCKET__
#define __CLIENT_SOCKET__


/* Creates a client socket and returns the file descriptor for it. */
int init_client(void);

/* Closes a client socket.  You must pass in the file descriptor created by init_client */
void close_client(int client_id);

#endif

