/* Copyright (C) 2010  BlackChat Group 
This file is part of BlackChat.

Ashes is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Ashes is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with BlackChat.  If not, see <http://www.gnu.org/licenses/>.
*/
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

/* Received enter key from client. ONLY WRITES TEXT FROM MAIN CHAT WINDOW!!!*/
void write_out(int client_id);

/*recieved yell! message from client*/
void write_yell(int client_id);

/* recieved vote from client */
void write_vote(int client_id);

/* recieved lurking key from client */
void write_lurk(int client_id);

/* Read from server. */
void read_from_server(int client_id);

/* write status to server for every character typed */
void write_status(int client_id);

/* request the user list from the server */
void write_deep_six(int client_id);

/* submit a deepsix vote on a user */
void submit_deep_six(int client_id, int vote_id);

#endif

