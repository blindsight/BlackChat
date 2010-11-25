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


#define MAX_USER_NAME_LENGTH				21
#define OTHER_WINDOW_BUFFER_SIZE			256


#define MAX_ROWS					3
#define MAX_COLUMNS 					40
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

typedef struct other_window_t
{
	WINDOW *window;
	char userName[MAX_USER_NAME_LENGTH];
	char buffer[OTHER_WINDOW_BUFFER_SIZE];
} other_window;

other_window *other_chat_windows;



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
static void refresh_all_windows()
{

        refresh_other_windows();
	wrefresh(transcript_window);
	wrefresh(client_chat_window);

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
				MAX_COLUMNS,
				client_buffer,
				&client_current_line );


	/* Position the cursor in the client window. */
	for(i = 0; i < client_cursor_position; i ++) {
		xPos++;
		if(xPos >= MAX_COLUMNS) {
			yPos ++;
			xPos = 0;
		}
	}
	yPos -= client_current_line;


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

/* Add some text to our transcript window. */
void write_to_transcript_window(char *text)
{
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
	print_transcript_chat_buffer();
}



int main(int argc, char* argv[])
{
	int is_running = 1;
	int x_terminal_size, y_terminal_size;
/*	int client_id = init_client();                   create a client. */

	transcript_buffer = (char*)malloc(sizeof(char)*transcript_buffer_size);
        memset(client_buffer, '\0', sizeof(client_buffer));
	memset(transcript_buffer, '\0', sizeof(transcript_buffer));

	get_terminal_size(&x_terminal_size, &y_terminal_size);
	
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
	
	transcript_window  = newwin(23,40,0,0);
	client_chat_window = newwin(MAX_ROWS,MAX_COLUMNS,20,40);
        wcolor_set(transcript_window,  3, NULL);
        wcolor_set(client_chat_window, 4, NULL);

	init_other_windows();

	write_to_transcript_window("**************************************\n");
	write_to_transcript_window("******** Wecome to BlackChat! ********\n");
	write_to_transcript_window("**************************************\n");
        
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


                /* Check what keys we pressed. */
		switch(ch) {
			/*
			 * Check if we pressed a control key. */
			if(iscntrl(ch)) {
#if 0
				case 1:  /* CTRL-A */
					client_current_line = 0;
					client_cursor_position = 0;
					print_client_chat_buffer();
					break;

				case 2:  /* CTRL-B */
					client_cursor_position --;
					print_client_chat_buffer();
					break;

				case 5:  /* CTRL-E */
					client_cursor_position = strlen(client_buffer);
					print_client_chat_buffer();
					break;

				case 6:  /* CTRL-F */
					client_cursor_position ++;
					print_client_chat_buffer();
					break;
#endif
                                case 127:/* Backsapce Key (grok hack) */
				case 8:  /* CTRL-H */
					client_buffer[ strlen(client_buffer)-1 ] = '\0';
					print_client_chat_buffer();
					break;
				case 10: /* CTRL-J and CTRL-M */
		/* UNCOMMENT ME FOR USE WITH SERVER */
                                        write_to_transcript_window("[Client Says]: ");
					write_to_transcript_window(client_buffer);
                                        write_to_transcript_window("\n");
					clear_text_from_client_typing_window();
				/*	write_out(client_id);                   // enter key is pressed so send a message to the server. */ 
                                        break;

				case 11: /* CTRL-K */
					{
						int i;
						for(i = client_cursor_position+1; i < strlen(client_buffer); i ++) {
							client_buffer[i] = '\0';
						}
					}
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

                /* Read from the server. */
/*                read_from_server(client_id); */
	
                refresh_all_windows();
	}

	free_other_windows();
	free(transcript_buffer);

	delwin(transcript_window);
	delwin(client_chat_window);
	endwin();
        
/*        close_client(client_id); */

	return 0;
}
