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
#include "logger.h"


#define MAX_USER_NAME_LENGTH				21
#define OTHER_WINDOW_BUFFER_SIZE			256


#define MAX_ROWS					1
#define MAX_COLUMNS 					80
#define MAX_CHARS_IN_CLIENT_TYPING_WINDOW 	MAX_ROWS * MAX_COLUMNS

#define TRANSCRIPT_MAX_ROWS				23
#define TRANSCRIPT_MAX_COLUMNS				40
#define MAX_CHARS_IN_TRANSCRIPT_WINDOW		TRANSCRIPT_MAX_ROWS * TRANSCRIPT_MAX_COLUMNS



char client_buffer[1024];
int client_current_line = 0;
int client_cursor_position = 0;

char *transcript_buffer;
int transcript_buffer_size = 256;
int transcript_current_line = 0;

WINDOW *transcript_window;
WINDOW *client_chat_window;
WINDOW *lurk_win;

typedef struct other_window_t
{
	WINDOW *window;
	char userName[MAX_USER_NAME_LENGTH];
	char buffer[OTHER_WINDOW_BUFFER_SIZE];
} other_window;

other_window *other_chat_windows;
int user_is_scrolling = 0;
int gaudy_mode_on     = 0;


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

        refresh_other_windows();
	wrefresh(transcript_window);

        if(!is_lurking)
	    wrefresh(client_chat_window);

}

/* Get number of lines in buffer. */
static int str_get_num_lines(char *str)
{
        int i;
        int num_lines = 0;

        for(i = 0; i < strlen(str); i++) {
                if( str[i] == '\n' || 
		   (i % TRANSCRIPT_MAX_COLUMNS) == 0) {
			num_lines ++;
		}
        }

        return num_lines;
}


/* Page the specified window down by "line" number of lines. */
void window_page_down(WINDOW *win, int *line, int max_columns, char *buffer)
{
        int i = 0;
        int found_num_lines = 0;
        int actual_number_of_lines = str_get_num_lines(buffer);


	/* Make sure the user didn't pass to large of a value. */
	if((*line) >= actual_number_of_lines) {
                (*line) = actual_number_of_lines - 1;
                return;
        }


	/* Figure out where to start printing from. */
        for(i = 0; i < strlen(buffer); i ++) {
                if( (i % TRANSCRIPT_MAX_COLUMNS) == 0 ) found_num_lines ++;
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


	/* Make sure we didn't get passed to small of a value. */
	if((*line) < 0) {
            (*line) = 0;
            return;
        }


	/* Start counting the number of lines and figure out where the (*line) number
	 * we want to print from starts. */
        for(i = 0; i < strlen(buffer); i ++) {
                if( (i % TRANSCRIPT_MAX_COLUMNS) == 0 ) found_num_lines ++;
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
	int buf_len    = strlen(buffer);
	int using_bold = 0;
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


	/* Now write our buffer to the window. */
	for(i = (*curr_line)*max_columns; i <= buf_len; i ++) {
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
				MAX_CHARS_IN_TRANSCRIPT_WINDOW,
				TRANSCRIPT_MAX_COLUMNS,
				transcript_buffer,
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
		memset(other_chat_windows[i].userName, '\0', sizeof(other_chat_windows[i].userName));
		memset(other_chat_windows[i].buffer, '\0', sizeof(other_chat_windows[i].buffer));

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
        char nameToPrint[MAX_USER_NAME_LENGTH];
        char textToPrint[OTHER_WINDOW_BUFFER_SIZE];
	int i, j, index;



        /* Print the text. */
	for(i = 0; i < 10; i ++) {
                
                /* Make sure we only print the end of the buffer AND/OR print white space so the
                 * color continues to the end of the window. */
                memset(textToPrint, '\0', sizeof(other_chat_windows[i].buffer));
                strcpy(textToPrint, other_chat_windows[i].buffer);
                if( strlen(textToPrint) > 20 ) {
                        index = 20;
                        for(j = strlen(textToPrint); index >= 0; j--) {
                                textToPrint[ index ] = other_chat_windows[i].buffer[j];
                                index --;
                        }
                } else {
                        for(j = strlen(textToPrint); j < 20; j ++) {
                            strcat(textToPrint, " ");
                        }
                }

                
                /* Do the same thing but for the user name.*/
                memset(nameToPrint, '\0', sizeof(other_chat_windows[i].userName));
                strcpy(nameToPrint, other_chat_windows[i].userName);
                for(j = strlen(nameToPrint); j < 20; j ++) {
                        strcat(nameToPrint, " ");
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

/* Set the window user name. */
void set_window_user_name(int num, char *name)
{
        strcpy(other_chat_windows[num].userName, name);
}

/* Append text to the specified user window. */
void append_text_to_window(int num, char *text)
{
        strcat(other_chat_windows[num].buffer, text);
}

/* Clear the text from the specified user window. */
void clear_user_window_text(int num)
{
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
static void write_to_ts_win(char *str)
{
	int   size = sizeof(char) * (strlen(str)+(TRANSCRIPT_MAX_ROWS*2));
	char *text = (char*)malloc(size);
	int   i;


	/* Copy over string. */
	memset(text, '\0', size);
	strcpy(text,str);

	/* Pad out the string so it takes up the whole line. */
	while( strlen(text) % TRANSCRIPT_MAX_COLUMNS != 0 ) {
		strcat(text, " ");
	}
	/* ---- */
	for(i = 0; i < s_strlen(text); i ++) {
		strcat(text, " ");
	}


        /* Deal with buffer over run for the transcript window. */
	while( (strlen(transcript_buffer) + strlen(text)) > transcript_buffer_size ) {
		int i = 0;
		char *temp_buffer = (char*)malloc( sizeof(char) * (transcript_buffer_size*2) );

		/* Copy over old values. */
		for(i = 0; i < strlen(transcript_buffer); i++) {
			temp_buffer[i] = transcript_buffer[i];
		}

		free(transcript_buffer);		/* Free old buffer. */
		transcript_buffer_size *= 2;
		transcript_buffer = temp_buffer;
	}       


        /* Append text to transcript window. */
	strcat(transcript_buffer, text);
	if(user_is_scrolling == 0) {
		print_transcript_chat_buffer();
	} else {
		window_page_up( transcript_window,
			        &transcript_current_line,
				TRANSCRIPT_MAX_COLUMNS,
				transcript_buffer );

	}

	log_writeln(text);
	free(text);
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

			write_to_ts_win(s);     /* Append the text to our transcript window. */
			found_nl_index = i+1;   /* Increment our new line index. */

			free(s);
		}
	}
}



int main(int argc, char* argv[])
{
	int is_running = 1;
	int x_terminal_size, y_terminal_size;
        int is_lurking = 0;

/*	int client_id = init_client();                   create a client. */

	log_init();
	log_writeln(" --------------------------- ");
	log_writeln(" > Starting BlackChat");

	transcript_buffer = (char*)malloc(sizeof(char)*transcript_buffer_size);
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
	transcript_window  = newwin(23,40,0,0);
	client_chat_window = newwin(MAX_ROWS,MAX_COLUMNS,24,0);
        lurk_win = newwin(MAX_ROWS, MAX_COLUMNS, 24, 0);
        wcolor_set(lurk_win,           4, NULL);
        wcolor_set(transcript_window,  3, NULL);
        wcolor_set(client_chat_window, 4, NULL);

        wprintw(lurk_win, "Lurking... Use CTRL-L to unLurk or CTRL-Q to quit.");

	log_writeln(" > ... creating other 9 windows");
	init_other_windows();

	log_writeln(" > ... [beginning transcript]");
	write_to_transcript_window("**************************************");
	write_to_transcript_window("******** Wecome to BlackChat! ********");
	write_to_transcript_window("**************************************");
        
	set_window_user_name(0, "bob");
	set_window_user_name(1, "sue");
	set_window_user_name(2, "dan");
	set_window_user_name(3, "joe");
	append_text_to_window(0, "Hello World, my name is bob!");
	append_text_to_window(1, "yo everyone, I'm in love with blackchat!");
	append_text_to_window(2, "hey, my name is Dan!");
	append_text_to_window(3, "Whats up!?");


	while(is_running) {
		int ch = getch();
/*		char buf[512];

		sprintf(buf, "key pressed: '%c'  int value: %d\n", ch, ch);
		write_to_transcript_window(buf);
*/

	/* Check if were in "Lurk" mode. */
        if(is_lurking) {
        	switch(ch) {
                case 12: /* lurk-off */
                    is_lurking = 0;
                    redrawwin(client_chat_window);
                    wrefresh(client_chat_window);
                    break;
                case 17: /* lurk-quit */
                    is_running = 0;
                    break;
                default:
                    redrawwin(lurk_win);
                    wrefresh(lurk_win);
            }
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
				case 10: /* CTRL-J and CTRL-M */
		/* UNCOMMENT ME FOR USE WITH SERVER */
					{
						char *buf = NULL;


						/* If we had gaudy mode on, we need to disable it. */
						if(gaudy_mode_on) {
							client_buffer[ client_cursor_position++ ] = 3;
							gaudy_mode_on = 0;
						}


						/* Get our buffer togther to write to the transcript window. */
						buf = (char*)malloc( (strlen("[Client Says]: ")+strlen(client_buffer)+1) * sizeof(char) );
						sprintf(buf, "[Client Says]: %s", client_buffer);
						write_to_transcript_window(buf);
					}
					clear_text_from_client_typing_window();
				/*	write_out(client_id);                    enter key is pressed so send a message to the server. */
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
						    {
							if(!is_lurking){

							    redrawwin(lurk_win);
							    wrefresh(lurk_win);
							    is_lurking = 1;
							}
						    }
						    break;

				case 14: /* CTRL-N */
					user_is_scrolling = 1;
					transcript_current_line ++;
					window_page_down( transcript_window,
							  &transcript_current_line,
							  TRANSCRIPT_MAX_COLUMNS,
							  transcript_buffer );
					break;
				case 16: /* CTRL-P */
					user_is_scrolling = 1;
					transcript_current_line --;
					window_page_up( transcript_window,
								&transcript_current_line,
							TRANSCRIPT_MAX_COLUMNS,
							transcript_buffer );
					break;
		
				case 17: /* CTRL-Q */ 
					log_writeln(" > ... recived quit signal from client");
					is_running = 0;
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

				case 29: /* CTRL-] */
					user_is_scrolling = 0;
					print_transcript_chat_buffer();
					break;

		    /*
		     * If we encountered an unkown escape charcter, break out of here so we don't
		     * print it. */
		    break;              /* TODO: Fix me! */
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
				/* Check if were deleting the last character. */
				if( client_cursor_position == strlen(client_buffer) ) {
					client_buffer[ client_cursor_position-1 ] = '\0';
					client_cursor_position --;
					print_client_chat_buffer();
				} else {
					/* If were here, that means were NOT deleting the last character. */
					int i;
					for(i = client_cursor_position-1; i < strlen(client_buffer); i ++) {
						client_buffer[i] = client_buffer[i+1];
					}
					client_cursor_position --;
					print_client_chat_buffer();
				}
				break;

			/* If were here, that means we didn't press any "special" keys so that means were
			 * trying to write some generic characters to our chat window. */
			default:
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

        	/* Read from the server. */
/*      	read_from_server(client_id); */

        	refresh_all_windows(is_lurking);
	}

	log_writeln(" > ... [ending transcript]");
	log_writeln(" > ... freeing resources");
	free_other_windows();
	free(transcript_buffer);

	delwin(transcript_window);
	delwin(client_chat_window);
	endwin();
        
/*  close_client(client_id); */

	log_writeln(" > ... closing client log");
	log_writeln(" > ... bye bye for now!");
	log_close();
	return 0;
}
