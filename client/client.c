/*
 * BlackChat
 *
 * Nice curses tutorials:
 * http://snap.nlc.dcccd.edu/learn/fuller3/chap6/chap6.html
 */


#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <ncurses.h>
#include "include/clientsocket.h"
#include "include/client.h"
#include "include/logger.h"


#define OTHER_WINDOW_BUFFER_SIZE			256
#define MAX_MESSAGE_LENGTH                              128


#define MAX_ROWS					1
#define MAX_COLUMNS 					80
#define MAX_CHARS_IN_CLIENT_TYPING_WINDOW 	MAX_ROWS * MAX_COLUMNS

#define TRANSCRIPT_MAX_ROWS				23
#define TRANSCRIPT_MAX_COLUMNS				40
#define MAX_CHARS_IN_TRANSCRIPT_WINDOW		TRANSCRIPT_MAX_ROWS * TRANSCRIPT_MAX_COLUMNS



char client_buffer[1024];
int  client_current_line = 0;
int  client_cursor_position = 0;

char *transcript_buffer;
int   transcript_buffer_size = 256;
int   transcript_current_line = 0;
char *f_transcript_buffer;
int   f_transcript_buffer_size = 256;


user_stats user_info[10];
char yell_messages[26][MAX_MESSAGE_LENGTH];

char status_win_text[128];

char client_user_name[MAX_USER_NAME_LENGTH];

WINDOW *transcript_window;
WINDOW *fullscreen_transcript_window;
WINDOW *client_chat_window;
WINDOW *lurk_win;
WINDOW *yell_win;
WINDOW *deepsix_win;
WINDOW *deepsix_win;
WINDOW *im_win;
WINDOW *status_win;
WINDOW *info_win;

typedef struct other_window_t
{
	WINDOW *window;
	char buffer[OTHER_WINDOW_BUFFER_SIZE];
} other_window;


other_window *other_chat_windows;
int user_is_scrolling = 0;
int gaudy_mode_on     = 0;
int transcript_maxed  = 0;


/* Get the size of the current terminal window. */
static void get_terminal_size(int *x, int *y)
{
	struct winsize ws;
	if(ioctl(0, TIOCGWINSZ, &ws) != 0) {
		fprintf(stderr, "ERROR TIOCGWINSZ:%s\n", strerror(errno));
		exit(1);
	}

	*x = ws.ws_col;
	*y = ws.ws_row;
}

/* Re-Draw all our windows. */
static void refresh_other_windows();
/* -------------------------- */
static void refresh_all_windows(int is_lurking)
{  
        wrefresh(status_win);
        wrefresh(info_win);

        if(!is_lurking) {
                wrefresh(client_chat_window);

                if(!transcript_maxed) {
                        refresh_other_windows();
                        wrefresh(transcript_window);
                } else {
                        wrefresh(fullscreen_transcript_window);
                }
	}
}

/* Get number of lines in buffer. */
static int str_get_num_lines(char *str, int max_columns)
{
        int i;
        int num_lines = 0;

        for(i = 0; i < strlen(str); i++) {
                if( str[i] == '\n' || 
		   (i % max_columns) == 0) {
			num_lines ++;
		}
        }

        return num_lines;
}


/* Page the specified window down by "line" number of lines. */
void window_page_down(WINDOW *win, int *line, int max_columns, char *buffer)
{
        int i = 0;
        int num_bold_chars = 0;
        int start = 0;
        int using_bold = 0;
        int found_num_lines = 0;
        int actual_number_of_lines = str_get_num_lines(buffer, max_columns);


	/* Make sure the user didn't pass to large of a value. */
	if((*line) >= actual_number_of_lines) {
                (*line) = actual_number_of_lines - 1;
                return;
        }


	/* Figure out where to start printing from. */
        for(i = 0; i < strlen(buffer); i ++) {
                /* --------- */
                if(buffer[i] == 2) {
                    if(!using_bold) {
                        num_bold_chars ++;
                    }
                    using_bold = 1;
                /* --------- */
                } else if(buffer[i] == 3) {
                    num_bold_chars ++;
                    using_bold = 0;
                }
 

                if( (i % max_columns) == 0 ) found_num_lines ++;
                if(found_num_lines >= (*line)) {
                        break;
                }
        }


        wclear(win);
        start = i + num_bold_chars;


	/* Now write our buffer to the window. */
	for(i = start; i <= strlen(buffer); i ++) {
		/* Check if we found the start of a gaudy character. */
		if( buffer[i] == 2 ) {
			using_bold = 1;
		} else if(buffer[i] == 3) {
			using_bold = 0;
		} else {
			/* Write the character to the window. */
			if(using_bold) {
				wattron(win, A_BOLD);
				wprintw(win, "%c", buffer[i]);
                        } else {
				wattroff(win, A_BOLD);
				wprintw(win, "%c", buffer[i]);
                        }
		}
	}


	/* Disable bold-ing just in case the user forgot to disable it. */
	wattroff(win, A_BOLD);
}

/* Page the specified window up by "line" number of lines. */
void window_page_up(WINDOW *win, int *line, int max_columns, char *buffer)
{
	int i = 0;
        int num_bold_chars = 0;
        int found_num_lines = 0;
        int start = 0;
        int using_bold = 0;


	/* Make sure we didn't get passed to small of a value. */
	if((*line) < 0) {
            (*line) = 0;
            return;
        }


	/* Start counting the number of lines and figure out where the (*line) number
	 * we want to print from starts. */
        for(i = 0; i < strlen(buffer); i ++) {
                /* --------- */
                if(buffer[i] == 2) {
                    if(!using_bold) {
                        num_bold_chars ++;
                    }
                    using_bold = 1;
                /* --------- */
                } else if(buffer[i] == 3) {
                    num_bold_chars ++;
                    using_bold = 0;
                }
  
                if( (i % max_columns) == 0 ) found_num_lines ++;
                if(found_num_lines >= (*line)) {
                        break;
                }
        }


        wclear(win);
        start = i + num_bold_chars;


	/* Now write our buffer to the window. */
	for(i = start; i <= strlen(buffer); i ++) {
		/* Check if we found the start of a gaudy character. */
		if( buffer[i] == 2 ) {
			using_bold = 1;
		} else if(buffer[i] == 3) {
			using_bold = 0;
		} else {
			/* Write the character to the window. */
			if(using_bold) {
				wattron(win, A_BOLD);
				wprintw(win, "%c", buffer[i]);
                        } else {
				wattroff(win, A_BOLD);
				wprintw(win, "%c", buffer[i]);
                        }
		}
	}


	/* Disable bold-ing just in case the user forgot to disable it. */
	wattroff(win, A_BOLD);
}



/* Print out the current buffer into the specified window.
 *
 * win		 ~ The ncurses window to print to.
 * max_chars	 ~ The max number of characters that can appear in the windows view at any given time.
 * max_columns   ~ The max number of characters that can be in a given column.
 * buffer	 ~ The character buffer to write to the window.
 * curr_line	 ~ The current line the user is at in the window.
 */
static void print_buffer_to_window(WINDOW *win, int max_chars, int max_columns, char *buffer, int *curr_line)
{
	int buf_len        = strlen(buffer);
        int num_bold_chars = 0;
	int using_bold     = 0;
	int i;


	/* Clear the chat window for writing. */
	wclear(win);

	/* Here, we need to figure out how much text is already in our chat window.
	 * The reason is because every time we want to add a new character to our 
	 * window, we don't want it to scroll to the top.  We want our window to stay
	 * were it's at. */
	int num_chars_off_screen =  buf_len - max_chars;
	(*curr_line) = 0;
	while(num_chars_off_screen > 0) {
		(*curr_line) ++;
		num_chars_off_screen -= max_columns;
	}


        /* Figure out how many bold chars are above the current line where printing. */
        for(i = 0; i < (*curr_line)*max_columns; i ++) {
                /* --------- */
                if(buffer[i] == 2) {
                    num_bold_chars ++;
                    using_bold = 1;
                } else if(buffer[i] == 3) {
                    num_bold_chars ++;
                    using_bold = 0;
                }
        }



        /* Reset vars. */
        i = i + num_bold_chars;

	/* Now write our buffer to the window. */
	for(; i <= buf_len; i ++) {
		/* Check if we found the start of a gaudy character. */
		if( buffer[i] == 2 ) {
			using_bold = 1;
		} else if(buffer[i] == 3) {
			using_bold = 0;
		} else {
			/* Write the character to the window. */
			if(using_bold) {
				wattron(win, A_BOLD);
				wprintw(win, "%c", buffer[i]);
                        } else {
				wattroff(win, A_BOLD);
				wprintw(win, "%c", buffer[i]);
                        }
		}
	}


	/* Disable bold-ing just in case the user forgot to disable it. */
	wattroff(win, A_BOLD);
}

/* Print out the current client chat-typing window. */
static void print_client_chat_buffer()
{
	int i = 0;
	int xPos = 0;
	int yPos = 0;

	
	/* Make sure the cursor position is valid. */
	if(client_cursor_position > strlen(client_buffer)) {
		client_cursor_position = strlen(client_buffer);
	} else if(client_cursor_position < 0) {
		client_cursor_position = 0;
	}


	/* Print the text to the client window. */
	print_buffer_to_window( client_chat_window, 
				MAX_CHARS_IN_CLIENT_TYPING_WINDOW,
			/*	MAX_COLUMNS,	*/
				1,
				client_buffer,
				&client_current_line );


	/* Get the position the cursor in the client window. */
	for(i = 0; i < strlen(client_buffer); i ++) {
		if (client_buffer[i] != 2 && 
		    client_buffer[i] != 3 ) {
			xPos++;
		}
	}


	/* Position the cursor for realz. */
	wmove(client_chat_window, yPos, xPos);
}

/* Print out the current client chat-typing window. */
static void print_transcript_chat_buffer()
{
	print_buffer_to_window( transcript_window, 
				TRANSCRIPT_MAX_ROWS * TRANSCRIPT_MAX_COLUMNS,
				TRANSCRIPT_MAX_COLUMNS,
				transcript_buffer,
				&transcript_current_line );
				
	print_buffer_to_window( fullscreen_transcript_window, 
				TRANSCRIPT_MAX_ROWS * (TRANSCRIPT_MAX_COLUMNS * 2),
				TRANSCRIPT_MAX_COLUMNS * 2,
				f_transcript_buffer,
				&transcript_current_line );
}

/* Delete the last word in the specified buffer. */
static void delete_last_word_in_buffer(char *buffer)
{
	int word_end      = strlen(buffer);
	int word_start    = word_end;
	int did_find_word = 0;
	int i;


	/* Starting from the index at the end of the string, start moving
	 * backwards until we come to something other than a space, then
	 * continue moving until we hit a space again. */
	for(i = word_end; i >= 0; i --) {
		/* Check if we found something other than a space. */
		if(buffer[i] != ' ' && buffer[i] != '\n' && buffer[i] != '\0') {
			did_find_word = 1;
		/* If where here, that means we found a space. */
		} else {
			/* We finally found the end of our word... */
			if(did_find_word == 1) {
				word_start = i;
				break;
			}
		}
	}


	/* Check if we actually went to the beginning of out string.
	 * ie. The last word is the first word. */
	if(word_start == word_end)	word_start = 0;


	/* Zero out from the beginnnig of where our word starts from. */
	for(i = word_start; i <= word_end; i ++) {
		buffer[i] = '\0';
	}
}

/* Initialze other windows. */
static void init_other_windows()
{
	int i;
	int xPos = 40;
	int yPos = 4;
        int lastColor = 1;
	other_chat_windows = (other_window*)malloc( sizeof(other_window) * 10 );

	for(i = 0; i < 10; i ++) {	
		other_chat_windows[i].window = newwin(2,20,yPos,xPos);
		memset(other_chat_windows[i].buffer,   '\0', sizeof(other_chat_windows[i].buffer));

		/* Move window to new location. */
		if(i % 2 == 0) {
			if(lastColor == 1) 
                            wcolor_set(other_chat_windows[i].window, 1, NULL);
                        else
                            wcolor_set(other_chat_windows[i].window, 2, NULL);
                        
                        mvprintw(yPos+2, xPos, "--------------------");
			yPos ++;
			xPos = 60;
                        lastColor ++;
		} else {
                        if(lastColor == 1) 
                            wcolor_set(other_chat_windows[i].window, 1, NULL);
                        else
                            wcolor_set(other_chat_windows[i].window, 2, NULL);

                        mvprintw(yPos-1, xPos, "--------------------");
			yPos += 2;
			xPos = 40;
		}


                /* This ensures zig-zagging of colors. */
                if(lastColor > 1) lastColor = 0;
	}
}

/* Free our other windows. */
static void free_other_windows()
{
        int i;
        for(i = 0; i < 10; i ++) {
                delwin(other_chat_windows[i].window);
        }
	free(other_chat_windows);
}


/* Draw other windows. */
static void refresh_other_windows()
{
        char textToPrint[OTHER_WINDOW_BUFFER_SIZE];
        int using_bold = 0;
	int i, j;



        /* Print the text. */
	for(i = 0; i < 10; i ++) {
                
                /* Make sure we only print the end of the buffer AND/OR print white space so the
                 * color continues to the end of the window. */
                memset(textToPrint, '\0', sizeof(other_chat_windows[i].buffer));
                strcpy(textToPrint, other_chat_windows[i].buffer);
             

                /* Clear the window. */
                wclear(other_chat_windows[i].window);



                /* Now write our buffer to the window. */
                for(j = 0; j < strlen(textToPrint); j ++) {
                        /* Check if we found the start of a gaudy character. */
                        if( textToPrint[j] == 2 ) {
                                using_bold = 1;
                        } else if(textToPrint[j] == 3) {
                                using_bold = 0;
                        } else {
                                /* Write the character to the window. */
                                if(using_bold) {
                                        wattron(other_chat_windows[i].window, A_BOLD);
                                        wprintw(other_chat_windows[i].window, "%c", textToPrint[j]);
                                } else {
                                        wattroff(other_chat_windows[i].window, A_BOLD);
                                        wprintw(other_chat_windows[i].window, "%c", textToPrint[j]);
                                }
                        }
                }


                /* Disable bold-ing just in case the user forgot to disable it. */
                wattroff(other_chat_windows[i].window, A_BOLD);


                
                
                /* Redraw the window. */
//                wprintw(other_chat_windows[i].window, textToPrint);
		wrefresh(other_chat_windows[i].window);
	}
}


/* Draw user names that we can IM. */
static void draw_im_window()
{
	int i;
	wclear(im_win);

	/* Show user names in im window. */
	for(i = 0; i < 10; i ++) {
		if(user_info[i].name[0] != '\0') {
			wprintw(im_win, "%d | %s\n", i, user_info[i].name);
		} else {
			wprintw(im_win, "%d | ------------\n", i);
		}
	}
	wprintw(im_win, "\nPress number associated with the user to send them an IM.\n");
	wprintw(im_win, "Press any other key to exit.");
	

	/* redraw the im window. */
	wrefresh(im_win);
}


/* Draw user names that we can deepsix. */
static void draw_deepsix_window()
{
	int i;
	wclear(deepsix_win);

	/* Show user names in deepsix window. */
	for(i = 0; i < 10; i ++) {
		if(user_info[i].name[0] != '\0' && user_info[i].canDeepSix == 1) {
			wprintw(deepsix_win, "%d | %s\n", i, user_info[i].name);
		} else {
			wprintw(deepsix_win, "%d | ------------\n", i);
		}
	}
	wprintw(deepsix_win, "\nPress any other key to exit.");
	

	/* redraw the deepsix window. */
//	redrawwin(deepsix_win);
	wrefresh(deepsix_win);
}


/* Delete the specified number of characters behind the current cursor position. */
#if 0
static void delete_num_chars_behind_cursor(int ch)
{
	
}
#endif



/*
 =================================================================
 =================================================================
 =================================================================
 */

/* Set a yell message. */
void set_yell_message(int index, char *message)
{
        int i;
        if(index < 0 || index > 25) {
                log_writeln("WARNING: Can't set yell message because index is not between 0 and 25!");
                return;
        }
        if(strlen(message) >= MAX_MESSAGE_LENGTH) {
                log_writeln("WARNING: Can't set yell message because message length is greater than MAX_MESSAGE_LENGTH!");
                return;
        }


        memset(yell_messages[index], '\0', MAX_MESSAGE_LENGTH * sizeof(char));
        strcpy(yell_messages[index], message);
        wclear(yell_win);

        /* Print what to say to yell window. */
        for(i = 0; i < 26; i ++) {
                if( yell_messages[i][0] != '\0' ) {
                        wprintw(yell_win, "%c | %s\n", i+97, yell_messages[i]);
                }
        }
        wprintw(yell_win, "\nHit any other key to exit.");
}

/* Returns the clients name. */
char *get_client_name()
{
    return client_user_name;
}


/* Append text to the specified user window. */
void append_text_to_window(int num, char *text)
{
        num--;
        if(num < 0 || num > 9) {
            log_writeln("Warning: Can't append text to window because uid/window = [%d] is not valid.", num);
            return;
        }

   //     write_to_transcript_window(text);
    //    write_to_transcript_window("---------------------");
        strcat(other_chat_windows[num].buffer, text);
}

/* Clear the text from the specified user window. */
void clear_user_window_text(int num)
{
        num--;
        if(num < 0 || num > 9) {
            log_writeln("Warning: Can't clear window because uid/window = [%d] is not valid.", num);
            return;
        }

        memset(other_chat_windows[num].buffer, '\0', sizeof(other_chat_windows[num].buffer));
}

/* Visually removes the user from the chat */
void remove_user_from_window(int num)
{
	if(num < 0 || num > 9) {
		log_writeln("WARNING: Cannot remove user at index[%d] because it is not a valid index.  (valid index are 0-9).", num);
		return;
	}
	
	
	wclear(other_chat_windows[num].window);
	memset(user_info[num].name,            '\0', sizeof(user_info[num].name));
	memset(other_chat_windows[num].buffer, '\0', sizeof(other_chat_windows[num].buffer));
}


/* Get the text inside our chat window. */
char *grab_text_from_client_typing_window(void)
{
	return client_buffer;
}

/* Clear the text from our chat window. */
void clear_text_from_client_typing_window(void)
{
	client_cursor_position = 0;
	client_current_line = 0;
	wclear(client_chat_window);
	memset(client_buffer, '\0', sizeof(client_buffer));
}

/* Put text in status window. */
void set_text_in_status_window(char *text)
{
        strcpy(status_win_text,text);
}


/* This is a special strlen function for use in the function below it.
 * We need a special strlen to pick up the escape characters ^B and ^C. 
 * NOTE: This function basically returns how many of the "special" chars
 *       appear in the string. */
static int s_strlen(char *str)
{
	int index  = 0;
	int length = 0;
	for(;;) {
		if( str[index] == '\0' )  break;
		if( str[index] == 2 || 
		    str[index] == 3 ) {
			length ++;
		}

		index ++;
	}

	return length;
}

/* Append the line to the transcript window. */
static char *write_to_ts_win(char *str, int max_col, int max_row, WINDOW *win, char *buffer, int *line, int *buffer_size)
{
	int   size = sizeof(char) * (strlen(str)+(max_col*4));
	char *text = (char*)malloc(size);
	int   i;


	/* Copy over string. */
	memset(text, '\0', size);
	strcpy(text, str);

	/* Pad out the string so it takes up the whole line. */
	while( strlen(text) % max_col != 0 ) {
		strcat(text, " ");
	}
	/* ---- */
	for(i = 0; i < s_strlen(text); i ++) {
		strcat(text, " ");
	}


        /* Deal with buffer over run for the transcript window. */
	while( (strlen(buffer) + strlen(text)) > *buffer_size ) {
		int i = 0;
		char *temp_buffer = (char*)malloc( sizeof(char) * (*buffer_size*2) );

		/* Copy over old values. */
		for(i = 0; i < strlen(buffer); i++) {
			temp_buffer[i] = buffer[i];
		}

		free(buffer);		/* Free old buffer. */
		*buffer_size *= 2;
		buffer = temp_buffer;
	}


        /* Append text to transcript window. */
	strcat(buffer, text);
	if(user_is_scrolling == 0) {
		print_buffer_to_window(win, max_col*max_row, max_col, buffer, line);
	} else {
		window_page_up(win, line, max_col, buffer);
	}

//	log_writeln(text);
	free(text);
	return buffer;
}

/* add some text to our transcript window. */
void write_to_transcript_window(char *str)
{
	int length = strlen(str);
	int i, j;
	int found_nl_index = 0;


	/*************************************************/
	for(i = 0; i <= length; i ++) {
		/* We found a new line. */
		if(str[i] == '\n' || str[i] =='\0') {
			char *s = (char*)malloc( sizeof(char) * (i+2) );
			int s_index = 0;

			/* Copy over the string we found so far. */
			for(j = found_nl_index; j < i; j ++) {
				s[s_index] = str[j];
				s_index ++;
			}

			s[s_index] = '\0';

			/* Append the text to our transcript window. */
			transcript_buffer   = write_to_ts_win(s, TRANSCRIPT_MAX_COLUMNS,   TRANSCRIPT_MAX_ROWS, transcript_window, 	        transcript_buffer, &transcript_current_line,   &transcript_buffer_size);
	//		f_transcript_buffer = write_to_ts_win(s, TRANSCRIPT_MAX_COLUMNS*2, TRANSCRIPT_MAX_ROWS, fullscreen_transcript_window, f_transcript_buffer, &transcript_current_line, &f_transcript_buffer_size);
			found_nl_index = i+1;   /* Increment our new line index. */

			free(s);
		}
	}
}


/**************************************************************************/
/* This is called 5 seconds after we try to scroll the transcript window. */
void scroll_ended_handler(int x)
{
    user_is_scrolling = 0;
    print_transcript_chat_buffer();
    wrefresh(transcript_window);
}
/**************************************************************************/



int main(int argc, char* argv[])
{
	int is_running = 1;
	int x_terminal_size, y_terminal_size;
        int is_lurking = 0;
        int is_yelling = 0;
        int in_deepsix = 0;
        int sending_im = 0;
        int i;
        int client_id;


        memset(client_user_name, '\0', MAX_USER_NAME_LENGTH);
        if(argc <= 1) {
                strcpy(client_user_name, "guest");
        } else {
                strncpy(client_user_name, argv[1], MAX_USER_NAME_LENGTH);
        }

	/* josh-note:
			These need to be uncommented for the client connection to work. */
	client_id = init_client(client_user_name);         /*          create a client. */
  //      init_user_list(client_id);                    /*  init the client size user list. */

        fd_set servs;
        FD_ZERO(&servs);
        FD_SET(client_id, &servs);
        FD_SET(0, &servs);


	log_init();
	log_writeln(" --------------------------- ");
	log_writeln(" > Starting BlackChat");

        signal(SIGALRM, scroll_ended_handler);

        for(i = 0; i < 26; i ++) {   /* set our message to null */
                memset(yell_messages[i], '\0', MAX_MESSAGE_LENGTH * sizeof(char)); 
        }

	transcript_buffer   = (char*)malloc(sizeof(char)*transcript_buffer_size);
	f_transcript_buffer = (char*)malloc(sizeof(char)*f_transcript_buffer_size);
    memset(client_buffer, '\0', sizeof(client_buffer));
	memset(transcript_buffer, '\0', sizeof(transcript_buffer));

	get_terminal_size(&x_terminal_size, &y_terminal_size);
	log_writeln(" > ... detecting current terminal size xy:(%d,%d)", x_terminal_size, y_terminal_size);

	log_writeln(" > ... initializing ncurses screen in raw mode");
	initscr();
	start_color();
	init_pair(0, COLOR_WHITE,   COLOR_BLACK);
	init_pair(1, COLOR_GREEN,   COLOR_BLACK);
	init_pair(2, COLOR_YELLOW,  COLOR_BLACK);
        init_pair(3, COLOR_CYAN,    COLOR_BLACK);
        init_pair(4, COLOR_MAGENTA, COLOR_BLACK);
	raw();
	keypad(stdscr, TRUE);
	noecho();

	color_set(0, NULL);
	
	log_writeln(" > ... creating transcript and client window");
	transcript_window  				= newwin(TRANSCRIPT_MAX_ROWS,TRANSCRIPT_MAX_COLUMNS,   0,0);
	fullscreen_transcript_window	= newwin(TRANSCRIPT_MAX_ROWS,TRANSCRIPT_MAX_COLUMNS*2, 0,0);
	client_chat_window 				= newwin(MAX_ROWS,MAX_COLUMNS,24,0);
        lurk_win           			= newwin(MAX_ROWS,MAX_COLUMNS,24,0);
        yell_win           			= newwin(23,40,0,0);
        deepsix_win        			= newwin(23,40,0,0);
        im_win						= newwin(TRANSCRIPT_MAX_ROWS,TRANSCRIPT_MAX_COLUMNS,   0,0);
        status_win					= newwin(3,40,21,40);
        info_win					= newwin(3,40,0,40);
   //     box(yell_win, '|', '-');

        set_yell_message(0, "Hello Everyone");
        set_yell_message(1, "Bye");
        set_yell_message(2, "I agree");
        set_yell_message(3, "I disagree");
        set_yell_message(4, "I'm tired");
        set_yell_message(5, "I like tacos");
        set_yell_message(6, "This is never going to get done");
        set_yell_message(7, "partial credit?");
        set_yell_message(8, "sorry we let you down Dr. Shade");
        set_yell_message(9, "Dr. Shade is awesome.");
        set_yell_message(10,"#1@^&*$%*#$");
        set_yell_message(11, "HOORAY FOR REDBULL!!!!!");


        wcolor_set(lurk_win,           4, NULL);
        wcolor_set(transcript_window,  3, NULL);
        wcolor_set(fullscreen_transcript_window, 3, NULL);
        wcolor_set(client_chat_window, 4, NULL);
        wcolor_set(yell_win,           2, NULL);
        wprintw(lurk_win, "Lurking... Use CTRL-L to unLurk or CTRL-Q to quit.");
	log_writeln(" > ... creating other 9 windows");
	init_other_windows();

	log_writeln(" > ... [beginning transcript]");
	write_to_transcript_window("***************************************");
	write_to_transcript_window("******** Wecome to BlackChat! *********");
	write_to_transcript_window("***************************************");
        
//        set_text_in_status_window("asdkjf aklsjdf\n23874823974\niouqrweioruweir");



        refresh_all_windows(is_lurking);
        print_client_chat_buffer();


        while(is_running) {
            fd_set testfds = servs;

            switch(select(client_id+1, &testfds, 0, 0, NULL)) {
            case 0:
                refresh_all_windows(is_lurking);
                break;
            case -1:
                switch(errno) {
                    case EINTR: continue;
                }
                perror("Error on select! (client)");
                is_running = 0;
                break;

            default:
                if( FD_ISSET(0, &testfds) )
                {
                    int ch = getch();
    /*		char buf[512];          //get chars

                    sprintf(buf, "key pressed: '%c'  int value: %d\n", ch, ch);
                    write_to_transcript_window(buf);
      */                                      //end get char

                /* Check if were in "Lurk" mode. */
                if(is_lurking) {
                        switch(ch) {
                                case 12: /* lurk-off */
                                        write_lurk(client_id);
                                        is_lurking = 0;
                                        print_client_chat_buffer();
                                        break;
                                case 17: /* quit */
                                        is_running = 0;
                                        break;
                                default:
                                        wrefresh(lurk_win);
                        }
                }
                /* Check if were in IM mode. */
                else if(sending_im) {
                            /*
                             TODO: Display list of users (like deepsix) to send IM to.
                                    All IM's will be displayed on the main transcript with some type of "marker"
                                    indicating that this was an IM.
                             */
                            if(ch >= 48 && ch <= 57) {
                                            /* josh-note:
                                                    Have josh send IM based on "ch" */
                            }
                
                
                            /* quit */
                        if(ch == 17) {
                                    is_running = 0;
                        }
                        
                        /* exit IM */
                        sending_im = 0;
                        window_page_up(transcript_window,            &transcript_current_line, TRANSCRIPT_MAX_COLUMNS,   transcript_buffer);
                        window_page_up(fullscreen_transcript_window, &transcript_current_line, TRANSCRIPT_MAX_COLUMNS*2, f_transcript_buffer);
                }
                /* Check if were in deepsix mode. */
                else if(in_deepsix) {
                            /* kick user */
                            if(ch >= 48 && ch <= 57) {
                                            /* josh-note:
                                                    Have josh make a "kick_user(ch-48)" command. 
                                                    Also, have josh keep track of who user voted for and display message on transcript as to how user voted
                                                    and/or if they already voted for the user. */		
                            }
                
                            /* quit */
                        if(ch == 17) {
                                    is_running = 0;
                        }
                        
                        /* exit deepsix */
                        in_deepsix = 0;
                        window_page_up(transcript_window,            &transcript_current_line, TRANSCRIPT_MAX_COLUMNS,   transcript_buffer);
                        window_page_up(fullscreen_transcript_window, &transcript_current_line, TRANSCRIPT_MAX_COLUMNS*2, f_transcript_buffer);
                }
                /* Check if were yelling. */
                else if(is_yelling) {
                        // redrawwin(yell_win);
                        // wrefresh(yell_win);
                        int index = ch - 97;
            

                        /* Write our comment to our transcript window.  */
                        if(index >= 0 && index < 26) {
                                if( yell_messages[index][0] != '\0' ) {
                                        memset(client_buffer, '\0', sizeof(client_buffer));
                                        strcpy(client_buffer, yell_messages[index]);
                                        write_out(client_id);
                                }
                        }
                        
                        is_yelling = 0;
                        redrawwin(client_chat_window);
                        wrefresh(client_chat_window);
                        redrawwin(transcript_window);
                        wrefresh(transcript_window);
                } else {
                    /* Check what keys we pressed. */
                    switch(ch) {
                            /*
                             * Check if we pressed a control key. */
                            if(iscntrl(ch)) {
                                    case 7:  /* CTRL-G */
                                            gaudy_mode_on = (gaudy_mode_on == 1) ? 0 : 1;
                                            if(gaudy_mode_on) {
                                                    client_buffer[ client_cursor_position++ ] = 2;
                                            } else {
                                                    client_buffer[ client_cursor_position++ ] = 3;
                                            }

                                            /* Print out updates to the window. */
                                            print_client_chat_buffer();
                                            break;

                                    case 127:/* Backsapce Key (grok hack) */
                                    case 8:  /* CTRL-H */
                                            client_buffer[ strlen(client_buffer)-1 ] = '\0';
                                            print_client_chat_buffer();
                                            break;
                                            
                                    case 9:  /* CTRL-I   /   TAB */
                                            if(!sending_im) {
                                                    sending_im = 1;
                                                    draw_im_window();
                                            }
                                            break;
                                            
                                    case 10: /* CTRL-J and CTRL-M */
                    /* UNCOMMENT ME FOR USE WITH SERVER */
                                    //	{
    //						char *buf = NULL;


                                                    /* If we had gaudy mode on, we need to disable it. */
                                                    if(gaudy_mode_on) {
                                                            client_buffer[ client_cursor_position++ ] = 3;
                                                            client_buffer[ client_cursor_position++ ] = ' ';
                                                            gaudy_mode_on = 0;
                                                    }


#if 0						/* Get our buffer togther to write to the transcript window. */
                                                    buf = (char*)malloc( (strlen("[Client Says]: ")+strlen(client_buffer)+1) * sizeof(char) );
                                                    sprintf(buf, "[Client Says]: %s", client_buffer);
                                                    write_to_transcript_window(buf);
                                            }
                                            clear_text_from_client_typing_window();
#endif
                                            write_out(client_id);                  /*  enter key is pressed so send a message to the server. */
                                                        break;

                                    case 11: /* CTRL-K */
                                            {
                                                    int i;
                                                    for(i = client_cursor_position+1; i < strlen(client_buffer); i ++) {
                                                            client_buffer[i] = '\0';
                                                    }
                                            }
                                            break;

                                                case 12: /* CTRL-L */
                                                            if(!is_lurking) {
                                                                redrawwin(lurk_win);
                                                                wrefresh(lurk_win);
                                                                is_lurking = 1;
                                                                write_lurk(client_id);
                                                            }
                                                        break;

                                    case 14: /* CTRL-N */
                                            alarm(5);
                                            user_is_scrolling = 1;
                                            transcript_current_line ++;

                                            window_page_down( transcript_window,
                                                              &transcript_current_line,
                                                              TRANSCRIPT_MAX_COLUMNS,
                                                              transcript_buffer );
                                                              
                                        /*    window_page_down( fullscreen_transcript_window,
                                                              &transcript_current_line,
                                                              TRANSCRIPT_MAX_COLUMNS*2,
                                                              f_transcript_buffer ); */
                                            break;
                                    case 16: /* CTRL-P */
                                            alarm(5);
                                            user_is_scrolling = 1;
                                            transcript_current_line --;

                                            window_page_up( transcript_window,
                                                            &transcript_current_line,
                                                            TRANSCRIPT_MAX_COLUMNS,
                                                            transcript_buffer );
                                                            
                                       /*     window_page_up( fullscreen_transcript_window,
                                                              &transcript_current_line,
                                                              TRANSCRIPT_MAX_COLUMNS*2,
                                                              f_transcript_buffer ); */
                                            break;
                    
                                    case 17: /* CTRL-Q */ 
                                            log_writeln(" > ... recived quit signal from client");
                                            is_running = 0;
                                            break;
                    
                                    case 20: /* CTRL-T */
                                            if(transcript_maxed) {
                                                    transcript_maxed = 0;
                                                    window_page_up(transcript_window, &transcript_current_line, TRANSCRIPT_MAX_COLUMNS, transcript_buffer);
                                                    wclear(fullscreen_transcript_window);
                                                    wrefresh(fullscreen_transcript_window);
                                            } else {
                                                    transcript_maxed = 1;
                                                    window_page_up(fullscreen_transcript_window, &transcript_current_line, TRANSCRIPT_MAX_COLUMNS*2, f_transcript_buffer);
                                                    wclear(transcript_window);
                                                    wrefresh(transcript_window);
                                            }
                                            break;
                    
                                    case 21: /* CTRL-U */
                                            client_current_line = 0;
                                            client_cursor_position = 0;
                                            memset(client_buffer, '\0', strlen(client_buffer)+1);	
                                            print_client_chat_buffer();
                                            break;

                                    case 23: /* CTRL-W */
                                            delete_last_word_in_buffer(client_buffer);
                                            print_client_chat_buffer();
                                            break;
                                    case 25: /* CTRL-Y */
                                            {
                                                if(!is_yelling)
                                                {
                                                    redrawwin(yell_win);
                                                    wrefresh(yell_win);
                                                    is_yelling = 1;
                                                }
                                            }
                                            break; 
                                    case 29: /* CTRL-] */
                                            alarm(0);
                                            user_is_scrolling = 0;
                                            print_transcript_chat_buffer();
                                            break;
                                    case 30: /* CTRL-6 */
                                            if(!in_deepsix)
                                            {
                                                draw_deepsix_window();
                                                in_deepsix = 1;
                                            }
                                            break;

                                    /*
                                     * If we encountered an unkown escape charcter, break out of here so we don't
                                     * print it. */
                                    break;              /* TODO: Fix me! */
                            }

#if 0
                            /* Scroll the clients typing window down. */
                            case KEY_DOWN:
                                    client_current_line ++;
                                    if(client_current_line*MAX_COLUMNS > strlen(client_buffer)) client_current_line --;

                                                wclear(client_chat_window);
                                                wprintw(client_chat_window, &client_buffer[client_current_line*MAX_COLUMNS]);
                                                break;
                            /* Scroll the clients typing window up. */
                            case KEY_UP:
                                    client_current_line --;
                                    if(client_current_line < 0) client_current_line = 0;

                                                wclear(client_chat_window);
                                                wprintw(client_chat_window, &client_buffer[client_current_line*MAX_COLUMNS]);
                                                break;
#endif
                            /* Delete the previous chracter. */
                            case KEY_BACKSPACE:
                                    /* Check if were deleting the last character. */
                                    if( client_cursor_position == strlen(client_buffer) ) {
                                            client_buffer[ client_cursor_position-1 ] = '\0';
                                            client_cursor_position --;
                                            print_client_chat_buffer();
                                    } else {
                                            /* If were here, that means were NOT deleting the last character. */
                                            int i;
                                            for(i = client_cursor_position-2; i < strlen(client_buffer); i ++) {
                                                    client_buffer[i] = client_buffer[i+1];
                                            }
                                            client_cursor_position --;
                                            print_client_chat_buffer();
                                    }
                                    break;

                            /* If were here, that means we didn't press any "special" keys so that means were
                             * trying to write some generic characters to our chat window. */
                            default:
                                    /* Make sure we don't print a control character. */
                                    if(!iscntrl(ch) ) {
                                            /* Check if were inserting a character before the end of our client
                                             * typing buffer. */
                                            if( client_cursor_position != strlen(client_buffer) ) {
            
                                                    /* Move all characters in front of the cursor up one. */
                                                    int i;
                                                    for(i = strlen(client_buffer)+1; i > client_cursor_position; i --) {
                                                            client_buffer[i] = client_buffer[i-1];
                                                    }
                                            }

                                            /* Add the character to our buffer. */
                                            client_buffer[ client_cursor_position++ ] = ch;

                                            /* Print our new/updated buffer. */
                                            print_client_chat_buffer();
                                            break;

                                    }
                            }


                            /* Resend server the client buffer ever keypress! */
                            write_status(client_id);

                    }

                    /* Read from the server. */
                    /* josh-note
                                    Uncomment this! */

                }   
                /* Check if were getting server info! */
                if( FD_ISSET(client_id, &testfds) )
                {
                    read_from_server(client_id);
                }


                /* Update our status window. */
            	wclear(status_win);
        	wprintw(status_win, status_win_text);

                /* Set our info window text. */
                wclear(info_win);
                wprintw(info_win, "            Black Chat  v1.0\n");
                wprintw(info_win, "UI: Henry Stratmann|Client: Josh Hartman\n");
                wprintw(info_win, "Server: Tyler Reid |Protocol: Tim Rhodes\n");


                /* resfresh */
                refresh_all_windows(is_lurking);
            }
        }

	log_writeln(" > ... [ending transcript]");
	log_writeln(" > ... freeing resources");
	free_other_windows();
	free(transcript_buffer);

	delwin(transcript_window);
	delwin(fullscreen_transcript_window);
	delwin(client_chat_window);
	delwin(status_win);
	delwin(info_win);
	delwin(im_win);
	delwin(deepsix_win);
	delwin(yell_win);
	endwin();
        
    /* josh-note
    		This should be uncommented to close down the client socket. */
        close_client(client_id);

	log_writeln(" > ... closing client log");
	log_writeln(" > ... bye bye for now!");
	log_close();
	return 0;
}
