
/*protocol defines */
/* basic protocol CMD_<define>:however the CMD differs */
/* the command is in the first byte */
#define	CMD_TEXT		0
#define	CMD_WINDOW		1
#define	CMD_VOTE		2
#define CMD_USERLIST		3
#define CMD_ERROR		4
#define CMD_LURK		5

// text defines
#define TEXT_MAIN_CHAT	1	// TEXT DEFINE<0-99>:UID<0-256>:TEXT LEN<2048>:TEXT 
#define TEXT_YELL	2
#define TEXT_STATUS	4	// TEXT DEFINE:UID:TEXT LEN:TEXT 
#define TEXT_IM		8	// TEXT DEFINE:FROM UID:TO UID:TEXT LEN:TEXT 


/* window defines */
#define TYP_STATUS	0
#define TYP_HELP	1
#define TYP_MAIN	2
#define TYP_IM		3
#define TYP_INPUT	4
#define TYP_DISPLAY	5

// vote defines
#define VOTE			0
#define VOTE_ACCEPTED		1
#define VOTE_NOT_ACCEPTED	2

// user list defines
#define USER_LIST_REQUEST	0
#define USER_LIST_USER_NAME	1
#define USER_LIST_RECEIVE_UID	3
#define USER_LIST_CURRENT	4
#define USER_LIST_SIGN_OFF	5

//errors
#define	ERROR_GENERAL	0
#define ERROR_CHAT_FULL	1
#define ERROR_NAME_LONG	2
#define ERROR_CHAT_DOWN	3

#define USER_NAME_LEN	20	//this is code points not chars
				/* which means any char that uses this will need
				to times this by 4 for the max size */
				
#define UID_LEN	3

typedef struct protocol_command {

} PRO_CMD;

typedef struct window_obj {
	int wid;
	int type;
	int x, y, z, w, h;
} *WIN_OBJ;

typedef struct history_obj {
	char* line;
	int time;
	struct user_obj *from;
	struct history_obj *next;
} *HST_OBJ;

typedef struct user_obj {
	int lurk;
	int vote; //id of user to vote off
	HST_OBJ *history;
	int uid;
	char name[USER_NAME_LEN*4];	//alpha num + foreign chars
	HST_OBJ *im;
} *UR_OBJ;

//general functions			
int get_type_from_message(const char *message);
int get_user_from_message(const char *message);

//text
int get_text_type_from_message(const char *message);
void get_text_from_message(char *message, char *result);
void create_text_message(int text_type, int uid, char *message, char *result);
void create_main_chat_message(int uid, char *message, char *result);
void create_im_message(int uid, int to_uid, char *message, char *result);
void create_status_message(int uid, char *message, char *result);
void create_yell_message(int uid, char *message, char *result);
int get_from_user_from_message(const char *message);

//window
int get_window_type_from_message(const char *message);
int get_window_x_from_message(const char *message);
int get_window_y_from_message(const char *message);
int get_window_z_from_message(const char *message);
int get_window_w_from_message(const char *message);
int get_window_h_from_message(const char *message);
int get_window_id_from_message(const char *message);
void get_window_from_message(const char *message, WIN_OBJ window);
void create_window_message(WIN_OBJ win, char *result);

//user list functions
int get_userlist_type_from_message(const char *message);
int get_next_user(int offset, const char *message, UR_OBJ user);
int get_first_user(const char *message, UR_OBJ user);
void request_user_list(UR_OBJ user, char *result);
void create_first_user(int user_list_type, int from_uid, UR_OBJ user, char *result);
void create_next_user(UR_OBJ user, char *result);
void get_user_name_message(const char *message, char *username);
void create_uid_message(int uid, char *message);
void create_user_name_message(char *username, char *result);

//voting functions
int get_vote_type_from_message(const char *message);
int get_voted_for_uid_from_message(const char *message);
void create_vote_message(int uid, int uid_vote, char *result);
void respond_vote_message(int vote_type, int uid, int uid_vote, char *result);

//error functions
int get_error_type_from_message(const char *message);
void create_error_message(int error_type, char *result);

//lurking functions
void create_user_lurking(int uid, char *result);
int get_user_lurking(const char *message);

