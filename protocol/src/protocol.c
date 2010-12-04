#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include "blackchat.h"


//General type functions
int get_type_from_message(const char *message) {
	//TODO: force it to only give back CMD_*
	char result[2];
	
 	strncpy(result, message,2);
	result[2]='\0';
	
	return atoi(result);
}

//text type functions
int get_text_type_from_message(const char *message) {
	char result[2];  
	
 	strncpy(result, message+2,2);
	result[2]='\0';

	return atoi(result);
}

void get_text_from_message(char *message, char *result) {
	
	int type = get_text_type_from_message(message);
	
	if(type!=TEXT_IM) {
		result = strcpy(result,message+7);
	} else { //we have to remove the from user id as well
		result = strcpy(result,message+10);
	}
}

void create_text_message(int text_type, int uid, char *message, char *result) {
	sprintf(result,"%02d%02d%03d%s",CMD_TEXT, text_type, uid, message);
}

//windows type functions
int get_window_type_from_message(const char *message) {
	char result[2];
	
 	strncpy(result, message+2,2);
	
	result[2]='\0';
	//printf("type result %s\0",result);
	
	return atoi(result);
}

int get_window_x_from_message(const char *message) {
	char result[4];
	
	strncpy(result, message+7,4);
	result[4]='\0';
	//printf("x result %s\0",result);
	return atoi(result);
}

int get_window_y_from_message(const char *message) {
	char result[4];
	
	strncpy(result, message+11,4);
	result[4]='\0';
	return atoi(result);
}

int get_window_z_from_message(const char *message) {
	char result[2];
	
	strncpy(result, message+15,2);
	result[2]='\0';
	return atoi(result);
}

int get_window_w_from_message(const char *message) {
	char result[4];
	
	strncpy(result, message+17,4);
	result[4]='\0';
	return atoi(result);
}

int get_window_h_from_message(const char *message) {
	char result[4];
	
	strncpy(result, message+21,4);
	result[4]='\0';
	return atoi(result);
}


int get_window_id_from_message(const char *message) {
	char result[3]; 
	
 	strncpy(result, message+4,3);
	result[4]='\0';
	return atoi(result);
}

void get_window_from_message(const char *message, WIN_OBJ window) {
	if(get_type_from_message(message) == CMD_WINDOW) {
	
		window->wid = get_window_id_from_message(message);
		window->type = get_window_type_from_message(message);
		window->x = get_window_x_from_message(message);
		window->y = get_window_y_from_message(message);
		window->z = get_window_z_from_message(message);
		window->w = get_window_w_from_message(message);
		window->h = get_window_h_from_message(message);
	
	} else {
		window=NULL;
	}
}


void create_window_message(WIN_OBJ win, char *result) {	
	sprintf(result,"%02d%02d%03d%04d%04d%02d%04d%04d", CMD_WINDOW, win->type, win->wid, win->x, win->y, win->z, win->w, win->h);
}

int get_vote_type_from_message(const char *message) {
	char result[2];
	
 	strncpy(result, message+2,2);
	
	result[2]='\0';
	//printf("type result %s\0",result);
	
	return atoi(result);
}

int get_voted_for_uid_from_message(const char *message) {
	char result[3];
	
 	strncpy(result, message+7,3);
	
	result[3]='\0';
	//printf("type result %s\0",result);
	
	return atoi(result);	
}

void create_vote_message(int vote_type, int uid, int uid_vote, char *result) {
	if(vote_type == VOTE) {
		sprintf(result,"%02d%02d%03d%03d",CMD_VOTE, vote_type, uid, uid_vote);
	} else {
		sprintf(result,"%02d%02d%03d%03d",CMD_VOTE, vote_type, uid, uid_vote);
	}
}

int get_userlist_type_from_message(const char *message) {
	char result[2];

	strncpy(result, message+2,2);

	result[2]='\0';
	//printf("type result %s\0",result);

	return atoi(result);
}

int get_user_from_message(const char *message) {
	char result[3]; 
	
 	strncpy(result, message+4,3);
	result[4]='\0';
	return atoi(result);
}

int get_from_user_from_message(const char *message) {
	char result[3]; 
	
 	strncpy(result, message+7,3);
	result[3]='\0';
	return atoi(result);
}

int get_error_type_from_message(const char *message) {
	char result[2]; 
	
 	strncpy(result, message+2,2);
	result[2]='\0';
	return atoi(result);	
}

int get_first_user(const char *message, UR_OBJ user)  {
	char result[3];
	int index = 7;
	int offset = 0;
	int name_len = 0;
	char uid[3];
	
//	printf("result: %s mess %s m7: %s\n", result, message, message+7);
	strncpy(result, message+7, 3);
//	printf("result: %s mess %s m7: %s\n", result, message, message+7);

	offset = atoi(result);
	
	if(offset > 3 ) {
		name_len = offset - 3;
	} else {
		name_len = 3 - offset;
	}
	
	strncpy(uid, message+10, 3);
	
	user->uid = atoi(uid);
	strncpy(user->name,message+13, name_len);
	
	index = index + 3 + offset;
	
//	printf("in:%d off:%d uid:%s user:%s len:%d", index, offset, uid, user->name, name_len);
	return index;
}

int get_next_user(int offset, const char *message, UR_OBJ user) {
	//TODO: refactor this code
	char result[3];
	int index = 7;
	int name_len = 0;
	int user_offset = 0;
	char uid[UID_LEN];
	
	strncpy(result, message+offset, 3);
	result[3]='\0';
	user_offset = atoi(result);
	
	if(user_offset == 0) {
		return 0;
	}
	
	if(user_offset > 3 ) {
		name_len = user_offset - 3;
	} else {
		name_len = 3 - user_offset;
	}
	
	strncpy(uid, message+offset+3, UID_LEN);
	uid[UID_LEN]='\0';
	
	user->uid = atoi(uid);
	strncpy(user->name,message+offset+3+UID_LEN, name_len);
	
	index = offset + 3 + UID_LEN + name_len;
	
	return index;
}
<<<<<<< HEAD
=======

void create_first_user(int user_list_type, int from_uid, UR_OBJ user, char *result) {
	int offset = strlen(user->name) + UID_LEN;
	
	sprintf(result,"%02d%02d%03d%03d%03d%s", CMD_USERLIST, user_list_type, from_uid, offset, user->uid, user->name);
}

void create_next_user(UR_OBJ user, char *result) {
	int offset = strlen(user->name) + UID_LEN;
	sprintf(result,"%s%03d%03d%s", result, offset, user->uid, user->name);	
}
>>>>>>> d20c2cd94c2565541a18ec8f14338be92975f74b
