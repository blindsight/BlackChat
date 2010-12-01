#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "blackchat.h"

char *cmd_type[] = {"text","window","vote","userlist"};

char *users[] = {"timothy","josh","henry","tyler","david", "mary","nancy","tanya","austin"};

char line[1024 * 4];

void process_command(char *input, char *output);

int main() {
	FILE * tests;

	int failed = 0;
	int passed = 0;
	int test_num = 0;
	char *result = (char *)malloc(sizeof(line));

	tests = fopen("tests", "r");

	while(fgets(line,sizeof(line),tests)) {
		if(line[0]==';') { printf("skip comment %s\n", line); continue; //comment
		}
		if(result[0]!='\0' && test_num>0) { //result needs to be checked

			printf("Test %d: ",test_num);
			if(strcmp(line, result) == 0) {
				printf("PASSED\n");
				passed++;
			} else {
				printf("FAILED %d text: '%s' expected '%s'\n",strcmp(line, result),result,line);
				failed++;
			}

			result[0] = '\0';
			continue;
		}

		process_command(line, result);

		test_num++;
	}


	fclose(tests);
	free(result);

	printf("Passed %d, Failed %d\n",passed, failed);
	return 0;
}

void process_command(char *input, char *output) {
	char *result = malloc(sizeof(line));

	output[0]='\0'; //reset output
	int type_result;
	int user = -1;
	int from_user = -1;
	int text_type = -1;
	
	//should there be a function to get all this information?
	type_result = get_type_from_message(input);
	user = get_user_from_message(input);
	
	switch(type_result) {
		case CMD_TEXT:

			get_text_from_message(input,result);
			text_type = get_text_type_from_message(input);

			if(text_type == TEXT_IM) {
				from_user = get_from_user_from_message(input);
				sprintf(output,"cmd:%s(%d) text:%d uid:%d fuid:%d content:%s",cmd_type[type_result], type_result, text_type, user, from_user, result);
			} else {
				sprintf(output,"cmd:%s(%d) text:%d uid:%d content:%s",cmd_type[type_result], type_result, text_type, user, result);
			}

		break;
		case	CMD_WINDOW:
		case	CMD_VOTE:
		case	CMD_USERLIST:
		default:
		 // /dev/null it
		break;
	}

	free(result);
	return;
}
