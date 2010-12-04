#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "blackchat.h"

char *cmd_type[] = {"text","window","vote","userlist","error"};

char *users[] = {"timothy","josh","henry","tyler","david", "mary","nancy","tanya","austin"};

void process_command(char *input, char *output);

	char line[1024 * 4];

int main() {
	FILE * tests;
	int failed = 0;
	int passed = 0;
	int test_num = 0;
	int line_num = 0;
	int pro_text = 1;
	char *result = (char *)malloc(sizeof(line));

	tests = fopen("tests", "r");

	while(fgets(line,sizeof(line),tests)) {
		
		line_num++;

		if(!pro_text) { //result needs to be checked
			line[strlen(line)-1]='\0'; //remove the new line
			int compare_value = strcmp(result, line);

			printf("Test %d:",test_num);
			if(compare_value == 0) {
				printf("PASSED\n");
				passed++;
			} else {
				printf("FAILED %d LINE %d result: '%s' expected '%s'\n",compare_value, line_num,result,line);
				failed++;
			}

			result[0] = '\0';
			line[0]='\0';
			pro_text = 1;
			continue;
		}

		process_command(line, result);
		pro_text = 0;	
		
		test_num++;
	}


	fclose(tests);
	
	char *input = (char *)malloc(sizeof(line));
	
	result[0]='\0';
	input[0]='\0';
	
	create_text_message(TEXT_MAIN_CHAT, 345, "this is a normal test\n", input);
	
	process_command(input, result);
	printf("test: %s %s\n", input, result);
	
	result[0]='\0';
	input[0]='\0';
	
	WIN_OBJ window = (WIN_OBJ)malloc(sizeof(struct window_obj));
	
	window->type = TYP_INPUT;
	window->wid = 45;
	window->x = 456;
	window->y = 4520;
	window->z = 45;
	window->h = 563;
	window->w = 3985;
	
	create_window_message(window, input);
	process_command(input, result);
	printf("test: %s %s\n", input, result);
		
	free(window);
	
	result[0]='\0';
	input[0]='\0';
	create_vote_message(34, 456, input);
	process_command(input, result);
	printf("test: %s %s\n", input, result);
	
	result[0]='\0';
	
//	create_first_user(int user_list_type, int from_uid, UR_OBJ user, char *result);
//	create_next_user(UR_OBJ user, char *result);
	
	free(result);
	
	printf("Passed %d, Failed %d\n",passed, failed);
	return 0;
}

void process_command(char *input, char *output) {
	char *message = (char *)malloc(sizeof(line));
	WIN_OBJ window;

	int type_result;
	int user = -1;
	int from_user = -1;
	int type = -1;
	int offset = 0;
	int user_num = 0;
	int i = 0;
	UR_OBJ user_list[20];
	UR_OBJ curr_user;
	
	//should there be a function to get all this information?
	type_result = get_type_from_message(input);
	user = get_user_from_message(input);
	
	//printf("type result %d\n",type_result);
	switch(type_result) {
		case CMD_TEXT:
			get_text_from_message(input,message);
			message[strlen(message)-1] = '\0'; //remove the newline 
			type = get_text_type_from_message(input);

			if(type == TEXT_IM) {
				from_user = get_from_user_from_message(input);
				sprintf(output,"cmd:%s(%d) text:%d uid:%d fuid:%d content:%s",cmd_type[type_result], type_result, type, user, from_user, message);
			} else {
				sprintf(output,"cmd:%s(%d) text:%d uid:%d content:%s",cmd_type[type_result], type_result, type, user, message);
			}

		break;
		case	CMD_WINDOW:
			window = (WIN_OBJ)malloc(sizeof(struct window_obj));
			
			get_window_from_message(input, window);
			
			sprintf(output,"cmd:%s(%d) type:%d wid:%d x:%d y:%d z:%d w:%d h:%d",cmd_type[type_result], type_result, window->type, window->wid, window->x, window->y, window->z, window->w, window->h);
		
			free(window);
		break;
		case	CMD_VOTE:
			from_user = get_voted_for_uid_from_message(input);
			type = get_vote_type_from_message(input);
			sprintf(output,"cmd:%s(%d) type:%d uid:%d vuid:%d",cmd_type[type_result], type_result, type, user, from_user);
		break;
		case	CMD_USERLIST:
			type = get_userlist_type_from_message(input);
			
			if(type != USER_LIST_REQUEST) {
				char temp_buff[1024];
				sprintf(output,"cmd:%s(%d) type:%d uid:%d",cmd_type[type_result], type_result, type, user);
				user_list[user_num] = (UR_OBJ)malloc(sizeof(struct user_obj));

				offset = get_first_user(input, user_list[user_num]);	
			
			//	user_num++;
				
				do {	
					user_num++;
					user_list[user_num] = (UR_OBJ)malloc(sizeof(struct user_obj));
				}
				while((offset = get_next_user(offset,input, user_list[user_num])) > 0);
			
				//printf("user num %d\n",user_num);
				for(i=0; i<user_num; i++) {
					curr_user = user_list[i];
					
					sprintf(temp_buff," offset:%d uid:%d name:%s", (int)strlen(curr_user->name)+3,curr_user->uid, curr_user->name);
					strcat(output, temp_buff);
					
					free(curr_user); //we can get rid of the user since they aren't needed anymore
				}
			
			} else {
				sprintf(output,"cmd:%s(%d) type:%d uid:%d",cmd_type[type_result], type_result, type, user);
			}
		break;
		case	CMD_ERROR:
			type = get_error_type_from_message(input);
			sprintf(output,"cmd:%s(%d) type:%d",cmd_type[type_result], type_result, type);
		break;
		default:
			sprintf(output,"invalid command [%d]",type_result);
		break;
	}

	free(message);
	return;
}
