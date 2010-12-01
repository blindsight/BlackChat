#include <strings.h>
#include <stdlib.h>
#include "blackchat.h"

int get_type_from_message(const char *message) {
	//TODO: force it to only give back CMD_*
	char result[1024]; 
	
 	strncpy(result, message,2);
	
	return atoi(result);
}

int get_text_type_from_message(const char *message) {
	char result[1024];  
	
 	strncpy(result, message+2,2);
	result[2] = '\0';

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

int get_user_from_message(const char *message) {
	char result[1024]; 
	
 	strncpy(result, message+4,3);
	
	return atoi(result);
}

int get_from_user_from_message(const char *message) {
	char result[1024]; 
	
 	strncpy(result, message+7,3);
	
	return atoi(result);
}
