/*
 * BlackChat
 *
 * Nice curses tutorials:
 * http://snap.nlc.dcccd.edu/learn/fuller3/chap6/chap6.html
 */


#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <ncurses.h>
#include "clientsocket.h"
#include "client.h"


#define MAX_ROWS					3
#define MAX_COLUMNS 					40
#define MAX_CHARS_IN_CLIENT_TYPING_WINDOW 	MAX_ROWS * MAX_COLUMNS

#define TRANSCRIPT_MAX_ROWS				23
#define TRANSCRIPT_MAX_COLUMNS				40
#define MAX_CHARS_IN_TRANSCRIPT_WINDOW		TRANSCRIPT_MAX_ROWS * TRANSCRIPT_MAX_COLUMNS



char client_buffer[1024];
int client_current_line = 0;

char transcript_buffer[1024 * 4];
int transcript_current_line = 0;

WINDOW *transcript_window;
WINDOW *client_chat_window;
WINDOW *other_chat_windows[9];


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


/* Get number of lines in buffer. */
static int str_get_num_lines(char *str)
{
        int i;
        int num_lines = 0;

        for(i = 0; i < strlen(str); i++) {
                if(str[i] == '\n') num_lines ++;
        }

        return num_lines;
}


/* Page the specified window down by "line" number of lines. */
void window_page_down(WINDOW *win, int *line, int max_columns, char *buffer)
{
        int i = 0;
        int found_num_lines = 0;
        int actual_number_of_lines = str_get_num_lines(buffer);


	if((*line) >= actual_number_of_lines) {
                (*line) = actual_number_of_lines - 1;
                return;
        }


        for(i = 0; i < strlen(buffer); i ++) {
                if(buffer[i] == '\n') found_num_lines ++;
                if(found_num_lines >= (*line)) {
                        break;
                }
        }


        wclear(win);
        wprintw(win, &buffer[i]);
}

/* Page the specified window up by "line" number of lines. */
void window_page_up(WINDOW *win, int *line, int max_columns, char *buffer)
{
	int i = 0;
        int found_num_lines = 0;


	if((*line) < 0) {
            (*line) = 0;
            return;
        }


        for(i = 0; i < strlen(buffer); i ++) {
                if(buffer[i] == '\n') found_num_lines ++;
                if(found_num_lines >= (*line)) {
                        break;
                }
        }


        wclear(win);
        wprintw(win, &buffer[i]);
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
	/* Clear the chat window for writing. */
	wclear(win);

	/* Here, we need to figure out how much text is already in our chat window.
	 * The reason is because every time we want to add a new character to our 
	 * window, we don't want it to scroll to the top.  We want our window to stay
	 * were it's at. */
	if( (strlen(buffer) - max_chars) > 0 ) {
		int num_chars_off_screen =  strlen(buffer) - max_chars;
		(*curr_line) = 0;

		while(num_chars_off_screen > 0) {
			(*curr_line) ++;
			num_chars_off_screen -= max_columns;
		}

		wprintw(win, &buffer[(*curr_line)*max_columns]);
	} else {
		wprintw(win, buffer);
	}
}

/* Print out the current client chat-typing window. */
static void print_client_chat_buffer()
{
	print_buffer_to_window( client_chat_window, 
				MAX_CHARS_IN_CLIENT_TYPING_WINDOW,
				MAX_COLUMNS,
				client_buffer,
				&client_current_line );
}

/* Print out the current client chat-typing window. */
static void print_transcript_chat_buffer()
{
	print_buffer_to_window( transcript_window, 
				MAX_CHARS_IN_TRANSCRIPT_WINDOW,
				TRANSCRIPT_MAX_COLUMNS,
				transcript_buffer,
				&transcript_current_line );
}


/*
 =================================================================
 =================================================================
 =================================================================
 */


/* Get the text inside our chat window. */
char *grab_text_from_client_typing_window(void)
{
	return client_buffer;
}

/* Clear the text from our chat window. */
void clear_text_from_client_typing_window(void)
{
	client_current_line = 0;
	wclear(client_chat_window);
	memset(client_buffer, '\0', sizeof(client_buffer));
}

/* Add some text to our transcript window. */
void write_to_transcript_window(char *text)
{
	if( (strlen(transcript_buffer) + strlen(text)) >= sizeof(transcript_buffer) ) {
		/* TODO: handle buffer overrun */
	}

	strcat(transcript_buffer, text);
	print_transcript_chat_buffer();
}



int main(int argc, char* argv[])
{
	int is_running = 1;
	int x_terminal_size, y_terminal_size;
        int client_id = init_client();                 /*  create a client. */
	
        memset(client_buffer, '\0', sizeof(client_buffer));
	memset(transcript_buffer, '\0', sizeof(transcript_buffer));

	get_terminal_size(&x_terminal_size, &y_terminal_size);
	
	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();

	transcript_window  = newwin(23,40,0,0);
	client_chat_window = newwin(MAX_ROWS,MAX_COLUMNS,20,40);

	write_to_transcript_window("**************************************\n");
	write_to_transcript_window("******** Wecome to BlackChat! ********\n");
	write_to_transcript_window("**************************************\n");

	while(is_running) {
		int ch = getch();
/*		char buf[512];

		sprintf(buf, "key pressed: '%c'  int value: %d\n", ch, ch);
		write_to_transcript_window(buf);
*/

                /* Read from the server. */
                read_from_server(client_id);


                /* Check what keys we pressed. */
		switch(ch) {
			/*
			 * Check if we pressed a control key. */
			if(iscntrl(ch)) {
                                case 127:/* Backsapce Key (grok hack) */
				case 8:  /* CTRL-H */
					client_buffer[ strlen(client_buffer)-1 ] = '\0';
					print_client_chat_buffer();
					break;
				case 10: /* CTRL-J and CTRL-M */
					write_out(client_id);                    /* enter key is pressed so send a message to the server. */
                                        break;
				case 14: /* CTRL-N */
					transcript_current_line ++;
					window_page_down( transcript_window,
							  &transcript_current_line,
							  TRANSCRIPT_MAX_COLUMNS,
							  transcript_buffer );
					break;
				case 16: /* CTRL-P */
					transcript_current_line --;
					window_page_up( transcript_window,
						        &transcript_current_line,
							TRANSCRIPT_MAX_COLUMNS,
							transcript_buffer );
					break;
				case 17: /* CTRL-Q */ 
					is_running = 0;
					break;
			}

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
		
			/* Delete the previous chracter. */
			case KEY_BACKSPACE:
				client_buffer[ strlen(client_buffer)-1 ] = '\0';
				print_client_chat_buffer();
				break;

			/* If were here, that means we didn't press any "special" keys so that means were
			 * trying to write some generic characters to our chat window. */
			default:
				/* Store the new char in our buffer. */
				client_buffer[ strlen(client_buffer) ] = ch;
				
				/* Print our new/updated buffer. */
				print_client_chat_buffer();
				break;
		}

		wrefresh(transcript_window);
		wrefresh(client_chat_window);
	}

	delwin(transcript_window);
	delwin(client_chat_window);
	endwin();
        
        close_client(client_id);

	return 0;
}
