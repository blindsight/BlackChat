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
#include <wchar.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <ncurses.h>
#include <curses.h>
#include "clientsocket.h"
#include "client.h"
#include "logger.h"


#define MAX_USER_NAME_LENGTH				21
#define OTHER_WINDOW_BUFFER_SIZE			256
#define MAX_MESSAGE_LENGTH                              128


#define MAX_ROWS					1
#define MAX_COLUMNS 					80
#define MAX_wchar_tS_IN_CLIENT_TYPING_WINDOW 	MAX_ROWS * MAX_COLUMNS

#define TRANSCRIPT_MAX_ROWS				23
#define TRANSCRIPT_MAX_COLUMNS				40
#define MAX_wchar_tS_IN_TRANSCRIPT_WINDOW		TRANSCRIPT_MAX_ROWS * TRANSCRIPT_MAX_COLUMNS



wchar_t client_buffer[1024];
int  client_current_line = 0;
int  client_cursor_position = 0;

wchar_t *transcript_buffer;
int   transcript_buffer_size = 256;
int   transcript_current_line = 0;
wchar_t *f_transcript_buffer;
int   f_transcript_buffer_size = 256;


wchar_t yell_messages[26][MAX_MESSAGE_LENGTH];

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
	wchar_t userName[MAX_USER_NAME_LENGTH];
	wchar_t buffer[OTHER_WINDOW_BUFFER_SIZE];
	wchar_t status;
	int  canDeepSix;
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
		if(!transcript_maxed) {
        	refresh_other_windows();
        	wrefresh(transcript_window);
        } else {
        	wrefresh(fullscreen_transcript_window);
        }
        
        wrefresh(status_win);
        wrefresh(info_win);

        if(!is_lurking) {
	    	wrefresh(client_chat_window);
		}
}

/* Get number of lines in buffer. */
static int str_get_num_lines(wchar_t *str, int max_columns)
{
        int i;
        int num_lines = 0;

        for(i = 0; i < wcslen(str); i++) {
                if( str[i] == '\n' || 
		   (i % max_columns) == 0) {
			num_lines ++;
		}
        }

        return num_lines;
}


/* Page the specified window down by "line" number of lines. */
void window_page_down(WINDOW *win, int *line, int max_columns, wchar_t *buffer)
{
        int i = 0;
        int num_bold_wchar_ts = 0;
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
        for(i = 0; i < wcslen(buffer); i ++) {
                /* --------- */
                if(buffer[i] == 2) {
                    if(!using_bold) {
                        num_bold_wchar_ts ++;
                    }
                    using_bold = 1;
                /* --------- */
                } else if(buffer[i] == 3) {
                    num_bold_wchar_ts ++;
                    using_bold = 0;
                }
 

                if( (i % max_columns) == 0 ) found_num_lines ++;
                if(found_num_lines >= (*line)) {
                        break;
                }
        }


        wclear(win);
        start = i + num_bold_wchar_ts;


	/* Now write our buffer to the window. */
	for(i = start; i <= wcslen(buffer); i ++) {
		/* Check if we found the start of a gaudy wchar_tacter. */
		if( buffer[i] == 2 ) {
			using_bold = 1;
		} else if(buffer[i] == 3) {
			using_bold = 0;
		} else {
			/* Write the wchar_tacter to the window. */
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
void window_page_up(WINDOW *win, int *line, int max_columns, wchar_t *buffer)
{
	int i = 0;
        int num_bold_wchar_ts = 0;
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
        for(i = 0; i < wcslen(buffer); i ++) {
                /* --------- */
                if(buffer[i] == 2) {
                    if(!using_bold) {
                        num_bold_wchar_ts ++;
                    }
                    using_bold = 1;
                /* --------- */
                } else if(buffer[i] == 3) {
                    num_bold_wchar_ts ++;
                    using_bold = 0;
                }
  
                if( (i % max_columns) == 0 ) found_num_lines ++;
                if(found_num_lines >= (*line)) {
                        break;
                }
        }


        wclear(win);
        start = i + num_bold_wchar_ts;


	/* Now write our buffer to the window. */
	for(i = start; i <= wcslen(buffer); i ++) {
		/* Check if we found the start of a gaudy wchar_tacter. */
		if( buffer[i] == 2 ) {
			using_bold = 1;
		} else if(buffer[i] == 3) {
			using_bold = 0;
		} else {
			/* Write the wchar_tacter to the window. */
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
 * max_wchar_ts	 ~ The max number of wchar_tacters that can appear in the windows view at any given time.
 * max_columns   ~ The max number of wchar_tacters that can be in a given column.
 * buffer	 ~ The wchar_tacter buffer to write to the window.
 * curr_line	 ~ The current line the user is at in the window.
 */
static void print_buffer_to_window(WINDOW *win, int max_wchar_ts, int max_columns, wchar_t *buffer, int *curr_line)
{
	int buf_len        = wcslen(buffer);
        int num_bold_wchar_ts = 0;
	int using_bold     = 0;
	int i;


	/* Clear the chat window for writing. */
	wclear(win);

	/* Here, we need to figure out how much text is already in our chat window.
	 * The reason is because every time we want to add a new wchar_tacter to our 
	 * window, we don't want it to scroll to the top.  We want our window to stay
	 * were it's at. */
	int num_wchar_ts_off_screen =  buf_len - max_wchar_ts;
	(*curr_line) = 0;
	while(num_wchar_ts_off_screen > 0) {
		(*curr_line) ++;
		num_wchar_ts_off_screen -= max_columns;
	}


        /* Figure out how many bold wchar_ts are above the current line where printing. */
        for(i = 0; i < (*curr_line)*max_columns; i ++) {
                /* --------- */
                if(buffer[i] == 2) {
                    num_bold_wchar_ts ++;
                    using_bold = 1;
                } else if(buffer[i] == 3) {
                    num_bold_wchar_ts ++;
                    using_bold = 0;
                }
        }



        /* Reset vars. */
        i = i + num_bold_wchar_ts;

	/* Now write our buffer to the window. */
	for(; i <= buf_len; i ++) {
		/* Check if we found the start of a gaudy wchar_tacter. */
		if( buffer[i] == 2 ) {
			using_bold = 1;
		} else if(buffer[i] == 3) {
			using_bold = 0;
		} else {
			/* Write the wchar_tacter to the window. */
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
	if(client_cursor_position > wcslen(client_buffer)) {
		client_cursor_position = wcslen(client_buffer);
	} else if(client_cursor_position < 0) {
		client_cursor_position = 0;
	}


	/* Print the text to the client window. */
	print_buffer_to_window( client_chat_window, 
				MAX_wchar_tS_IN_CLIENT_TYPING_WINDOW,
			/*	MAX_COLUMNS,	*/
				1,
				client_buffer,
				&client_current_line );


	/* Get the position the cursor in the client window. */
	for(i = 0; i < wcslen(client_buffer); i ++) {
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
static void delete_last_word_in_buffer(wchar_t *buffer)
{
	int word_end      = wcslen(buffer);
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
		memset(other_chat_windows[i].userName, '\0', sizeof(other_chat_windows[i].userName));
		memset(other_chat_windows[i].buffer,   '\0', sizeof(other_chat_windows[i].buffer));
		other_chat_windows[i].canDeepSix = 1;
		other_chat_windows[i].status     = ' ';

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
        wchar_t nameToPrint[MAX_USER_NAME_LENGTH];
        wchar_t textToPrint[OTHER_WINDOW_BUFFER_SIZE];
	int i, j, index;



        /* Print the text. */
	for(i = 0; i < 10; i ++) {
                
                /* Make sure we only print the end of the buffer AND/OR print white space so the
                 * color continues to the end of the window. */
                memset(textToPrint, '\0', sizeof(other_chat_windows[i].buffer));
                wcscpy(textToPrint, other_chat_windows[i].buffer);
                if( wcslen(textToPrint) > 20 ) {
                        index = 20;
                        for(j = wcslen(textToPrint); index >= 0; j--) {
                                textToPrint[ index ] = other_chat_windows[i].buffer[j];
                                index --;
                        }
                } else {
                        for(j = wcslen(textToPrint); j < 20; j ++) {
                            wcscat(textToPrint, L" ");
                        }
                }

                
                /* Do the same thing but for the user name.*/
                memset(nameToPrint, '\0', sizeof(other_chat_windows[i].userName));
                wcscpy(nameToPrint, other_chat_windows[i].userName);
                for(j = wcslen(nameToPrint); j < 20; j ++) {
                		if(j < 19) {
                        	wcscat(nameToPrint, L" ");
                        } else {
                        	nameToPrint[j]   = other_chat_windows[i].status;
                        	nameToPrint[j+1] = '\0';
                        }
                }


                /* Print */
		mvwprintw(  other_chat_windows[i].window, 0,0, nameToPrint);
		mvwprintw(  other_chat_windows[i].window,
                            1,
                            0,
                            textToPrint);

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
		if(other_chat_windows[i].userName[0] != '\0') {
			wprintw(im_win, "%d | %s\n", i, other_chat_windows[i].userName);
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
		if(other_chat_windows[i].userName[0] != '\0' && other_chat_windows[i].canDeepSix == 1) {
			wprintw(deepsix_win, "%d | %s\n", i, other_chat_windows[i].userName);
		} else {
			wprintw(deepsix_win, "%d | ------------\n", i);
		}
	}
	wprintw(deepsix_win, "\nPress any other key to exit.");
	

	/* redraw the deepsix window. */
//	redrawwin(deepsix_win);
	wrefresh(deepsix_win);
}


/* Delete the specified number of wchar_tacters behind the current cursor position. */
#if 0
static void delete_num_wchar_ts_behind_cursor(int ch)
{
	
}
#endif



/*
 =================================================================
 =================================================================
 =================================================================
 */

/* Set a yell message. */
void set_yell_message(int index, wchar_t *message)
{
        int i;
        if(index < 0 || index > 25) {
                log_writeln("WARNING: Can't set yell message because index is not between 0 and 25!");
                return;
        }
        if(wcslen(message) >= MAX_MESSAGE_LENGTH) {
                log_writeln("WARNING: Can't set yell message because message length is greater than MAX_MESSAGE_LENGTH!");
                return;
        }


        memset(yell_messages[index], '\0', MAX_MESSAGE_LENGTH * sizeof(wchar_t));
        wcscpy(yell_messages[index], message);
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
wchar_t *get_client_name()
{
    return L"Henry";
}


/* Set the window user name. */
void set_window_user_name(int num, wchar_t *name)
{
        wcscpy(other_chat_windows[num].userName, name);
}

/* Append text to the specified user window. */
void append_text_to_window(int num, wchar_t *text)
{
        wcscat(other_chat_windows[num].buffer, text);
}

/* Clear the text from the specified user window. */
void clear_user_window_text(int num)
{
        memset(other_chat_windows[num].buffer, '\0', sizeof(other_chat_windows[num].buffer));
}

/* Allow/disallow us to deepsix a user. */
void can_deepsix_user(int num, int can_vote)
{
	other_chat_windows[num].canDeepSix = can_vote;
}

/* Visually set user status. */
void set_user_status(int num, wchar_t status)
{
	other_chat_windows[num].status = status;
}

/* Visually removes the user from the chat */
void remove_user_from_window(int num)
{
	if(num < 0 || num > 9) {
		log_writeln("WARNING: Cannot remove user at index[%d] because it is not a valid index.  (valid index are 0-9).", num);
		return;
	}
	
	
	wclear(other_chat_windows[num].window);
	memset(other_chat_windows[num].userName, '\0', sizeof(other_chat_windows[num].userName));
	memset(other_chat_windows[num].buffer, 	 '\0', sizeof(other_chat_windows[num].buffer));
}


/* Get the text inside our chat window. */
wchar_t *grab_text_from_client_typing_window(void)
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
void set_text_in_status_window(wchar_t *text)
{
	wclear(status_win);
	wprintw(status_win, "%s", text);
}


/* This is a special wcslen function for use in the function below it.
 * We need a special wcslen to pick up the escape wchar_tacters ^B and ^C. 
 * NOTE: This function basically returns how many of the "special" wchar_ts
 *       appear in the string. */
static int s_wcslen(wchar_t *str)
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
static wchar_t *write_to_ts_win(wchar_t *str, int max_col, int max_row, WINDOW *win, wchar_t *buffer, int *line, int *buffer_size)
{
	int   size = sizeof(wchar_t) * (wcslen(str)+(max_col*4));
	wchar_t *text = (wchar_t*)malloc(size);
	int   i;


	/* Copy over string. */
	memset(text, '\0', size);
	wcscpy(text, str);

	/* Pad out the string so it takes up the whole line. */
	while( wcslen(text) % max_col != 0 ) {
		wcscat(text, L" ");
	}
	/* ---- */
	for(i = 0; i < s_wcslen(text); i ++) {
		wcscat(text, L" ");
	}


        /* Deal with buffer over run for the transcript window. */
	while( (wcslen(buffer) + wcslen(text)) > *buffer_size ) {
		int i = 0;
		wchar_t *temp_buffer = (wchar_t*)malloc( sizeof(wchar_t) * (*buffer_size*2) );

		/* Copy over old values. */
		for(i = 0; i < wcslen(buffer); i++) {
			temp_buffer[i] = buffer[i];
		}

		free(buffer);		/* Free old buffer. */
		*buffer_size *= 2;
		buffer = temp_buffer;
	}


        /* Append text to transcript window. */
	wcscat(buffer, text);
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
void write_to_transcript_window(wchar_t *str)
{
	int length = wcslen(str);
	int i, j;
	int found_nl_index = 0;


	/*************************************************/
	for(i = 0; i <= length; i ++) {
		/* We found a new line. */
		if(str[i] == '\n' || str[i] =='\0') {
			wchar_t *s = (wchar_t*)malloc( sizeof(wchar_t) * (i+2) );
			int s_index = 0;

			/* Copy over the string we found so far. */
			for(j = found_nl_index; j < i; j ++) {
				s[s_index] = str[j];
				s_index ++;
			}

			s[s_index] = '\0';

			/* Append the text to our transcript window. */
			transcript_buffer   = write_to_ts_win(s, TRANSCRIPT_MAX_COLUMNS,   TRANSCRIPT_MAX_ROWS, transcript_window, 			  transcript_buffer,   &transcript_current_line,   &transcript_buffer_size);
			f_transcript_buffer = write_to_ts_win(s, TRANSCRIPT_MAX_COLUMNS*2, TRANSCRIPT_MAX_ROWS, fullscreen_transcript_window, f_transcript_buffer, &transcript_current_line, &f_transcript_buffer_size);
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

	/* josh-note:
			These need to be uncommented for the client connection to work. */
/*	int client_id = init_client("Henry");                   create a client. */
/*      init_user_list(client_id);                      init the client size user list. */

	log_init();
	log_writeln(" --------------------------- ");
	log_writeln(" > Starting BlackChat");

        signal(SIGALRM, scroll_ended_handler);

        for(i = 0; i < 26; i ++) {   /* set our message to null */
                memset(yell_messages[i], '\0', MAX_MESSAGE_LENGTH * sizeof(wchar_t)); 
        }

	transcript_buffer   = (wchar_t*)malloc(sizeof(wchar_t)*transcript_buffer_size);
	f_transcript_buffer = (wchar_t*)malloc(sizeof(wchar_t)*f_transcript_buffer_size);
    memset(client_buffer, '\0', sizeof(client_buffer));
	memset(transcript_buffer, '\0', sizeof(transcript_buffer));

	get_terminal_size(&x_terminal_size, &y_terminal_size);
	log_writeln(" > ... detecting current terminal size xy:(%d,%d)", x_terminal_size, y_terminal_size);

	log_writeln(" > ... initializing ncurses screen in raw mode");
	initscr();
//	start_color();
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

        set_yell_message(0, L"Hello World");
        set_yell_message(1, L"Yo dog!");
        set_yell_message(2, L"Hey everyone!");
        set_yell_message(3, L"Whats up?");
        set_yell_message(12,L"I agree.");

        wcolor_set(lurk_win,           4, NULL);
        wcolor_set(transcript_window,  3, NULL);
        wcolor_set(client_chat_window, 4, NULL);
        wcolor_set(yell_win,           2, NULL);
        wprintw(lurk_win, "Lurking... Use CTRL-L to unLurk or CTRL-Q to quit.");
	log_writeln(" > ... creating other 9 windows");
	init_other_windows();

	log_writeln(" > ... [beginning transcript]");
	write_to_transcript_window(L"***************************************");
	write_to_transcript_window(L"******** Wecome to BlackChat! *********");
	write_to_transcript_window(L"***************************************");
        
	set_window_user_name(0, L"chris");
	set_window_user_name(1, L"sue");
	set_window_user_name(2, L"dan");
	set_window_user_name(3, L"joe");
	append_text_to_window(0, L"Sup!");
	append_text_to_window(1, L"yo everyone, I'm in love with blackchat!");
	append_text_to_window(2, L"hey, my name is Dan!");
	append_text_to_window(3, L"Hey!?");

	can_deepsix_user(0, 0);
	set_user_status(1, 'L');
	set_user_status(2, 'L');
	
	/* Set our info window text. */
	wprintw(info_win, "       Black Chat  v1.0\n");
	wprintw(info_win, "UI: Henry Stratmann|Client: Josh Hartman\n");
	wprintw(info_win, "Server: Tyler Reid |Protocol: Tim Rhoads\n");
	

	while(is_running) {
		int ch = getch();
/*		wchar_t buf[512];          //get wchar_ts

		sprintf(buf, "key pressed: '%c'  int value: %d\n", ch, ch);
		write_to_transcript_window(buf);
  */                                      //end get wchar_t

            /* Check if were in "Lurk" mode. */
            if(is_lurking) {
                    switch(ch) {
                            case 12: /* lurk-off */
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
                                    write_to_transcript_window( yell_messages[index] );
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
					client_buffer[ wcslen(client_buffer)-1 ] = '\0';
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
					{
						wchar_t *buf = NULL;


						/* If we had gaudy mode on, we need to disable it. */
						if(gaudy_mode_on) {
							client_buffer[ client_cursor_position++ ] = 3;
                                                	client_buffer[ client_cursor_position++ ] = ' ';
							gaudy_mode_on = 0;
						}


						/* Get our buffer togther to write to the transcript window. */
						buf = (wchar_t*)malloc( (wcslen(L"[Client Says]: ")+wcslen(client_buffer)+1) * sizeof(wchar_t) );
						write_to_transcript_window(L"[Client Says]: ");
						write_to_transcript_window(client_buffer);
						
					//	sprintf(buf, "[Client Says]: %ls", client_buffer);
					//	write_to_transcript_window(buf);
					}
					clear_text_from_client_typing_window();
				/*	write_out(client_id);                    enter key is pressed so send a message to the server. */
						    break;

				case 11: /* CTRL-K */
					{
						int i;
						for(i = client_cursor_position+1; i < wcslen(client_buffer); i ++) {
							client_buffer[i] = '\0';
						}
					}
					break;

					    case 12: /* CTRL-L */
							if(!is_lurking) {
							    redrawwin(lurk_win);
							    wrefresh(lurk_win);
							    is_lurking = 1;
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
							  
					window_page_down( fullscreen_transcript_window,
							  &transcript_current_line,
							  TRANSCRIPT_MAX_COLUMNS*2,
							  f_transcript_buffer );
					break;
				case 16: /* CTRL-P */
                                        alarm(5);
					user_is_scrolling = 1;
					transcript_current_line --;

					window_page_up( transcript_window,
						    	&transcript_current_line,
							TRANSCRIPT_MAX_COLUMNS,
							transcript_buffer );
							
					window_page_up( fullscreen_transcript_window,
							  &transcript_current_line,
							  TRANSCRIPT_MAX_COLUMNS*2,
							  f_transcript_buffer );
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
					memset(client_buffer, '\0', wcslen(client_buffer)+1);	
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
                                 * If we encountered an unkown escape wchar_tcter, break out of here so we don't
                                 * print it. */
                                break;              /* TODO: Fix me! */
			}
#if 0
			/* Scroll the clients typing window down. */
			case KEY_DOWN:
				client_current_line ++;
				if(client_current_line*MAX_COLUMNS > wcslen(client_buffer)) client_current_line --;

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
				/* Check if were deleting the last wchar_tacter. */
				if( client_cursor_position == wcslen(client_buffer) ) {
					client_buffer[ client_cursor_position-1 ] = '\0';
					client_cursor_position --;
					print_client_chat_buffer();
				} else {
					/* If were here, that means were NOT deleting the last wchar_tacter. */
					int i;
					for(i = client_cursor_position-1; i < wcslen(client_buffer); i ++) {
						client_buffer[i] = client_buffer[i+1];
					}
					client_cursor_position --;
					print_client_chat_buffer();
				}
				break;

			/* If were here, that means we didn't press any "special" keys so that means were
			 * trying to write some generic wchar_tacters to our chat window. */
			default:
				/* Make sure we don't print a control wchar_tacter. */
				if(!iscntrl(ch)) {
					/* Check if were inserting a wchar_tacter before the end of our client
					 * typing buffer. */
					if( client_cursor_position != wcslen(client_buffer) ) {
	
						/* Move all wchar_tacters in front of the cursor up one. */
						int i;
						for(i = wcslen(client_buffer)+1; i > client_cursor_position; i --) {
							client_buffer[i] = client_buffer[i-1];
						}
					}

					/* Add the wchar_tacter to our buffer. */
					client_buffer[ client_cursor_position++ ] = ch;

					/* Print our new/updated buffer. */
					print_client_chat_buffer();
					break;
				}
			}
		}

        	/* Read from the server. */
        	/* josh-note
        			Uncomment this! */
/*      	read_from_server(client_id); */

        	refresh_all_windows(is_lurking);
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
/*  close_client(client_id); */

	log_writeln(" > ... closing client log");
	log_writeln(" > ... bye bye for now!");
	log_close();
	return 0;
}
