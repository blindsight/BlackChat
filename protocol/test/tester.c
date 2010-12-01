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
	free(result);

	printf("Passed %d, Failed %d\n",passed, failed);
	return 0;
}

void process_command(char *input, char *output) {
	char *message = (char *)malloc(sizeof(line));

	int type_result;
	int user = -1;
	int from_user = -1;
	int type = -1;
	int x, y, z, w, h;
	
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
		
			x = get_window_x_from_message(input);
			y = get_window_y_from_message(input);
			z = get_window_z_from_message(input);
			w = get_window_w_from_message(input);
			h = get_window_h_from_message(input);
		
			type = get_window_type_from_message(input);
			sprintf(output,"cmd:%s(%d) type:%d uid:%d x:%d y:%d z:%d w:%d h:%d",cmd_type[type_result], type_result, type, user, x, y, z, w, h);
		break;
		case	CMD_VOTE:
			from_user = get_voted_for_uid_from_message(input);
			type = get_vote_type_from_message(input);
			sprintf(output,"cmd:%s(%d) type:%d uid:%d vuid:%d",cmd_type[type_result], type_result, type, user, from_user);
		break;
		case	CMD_USERLIST:
			type = get_userlist_type_from_message(input);
			sprintf(output,"cmd:%s(%d) type:%d uid:%d",cmd_type[type_result], type_result, type, user);
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
