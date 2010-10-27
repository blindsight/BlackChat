/*
 * BlackChat
 *
 * Nice curses tutorials:
 * http://snap.nlc.dcccd.edu/learn/fuller3/chap6/chap6.html
 */


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <ncurses.h>


#define MAX_ROWS					3
#define MAX_COLUMNS 					40
#define MAX_CHARS_IN_CLIENT_TYPING_WINDOW 	MAX_ROWS * MAX_COLUMNS




char client_buffer[1024 * 4];
int client_current_line = 0;

WINDOW *transcript_window;
WINDOW *client_chat_window;



void get_terminal_size(int *x, int *y)
{
	struct winsize ws;
	if(ioctl(0, TIOCGWINSZ, &ws) != 0) {
		fprintf(stderr, "ERROR TIOCGWINSZ:%s\n", strerror(errno));
		exit(1);
	}

	*x = ws.ws_col;
	*y = ws.ws_row;
}


/* Print our the current client chat-typing window. */
void print_client_chat_buffer()
{
	/* Clear the chat window for writing. */
	wclear(client_chat_window);

	/* Here, we need to figure out how much text is already in our chat window.
	 * The reason is because every time we want to add a new character to our 
	 * window, we don't want it to scroll to the top.  We want our window to stay
	 * were it's at. */
	if( (strlen(client_buffer) - MAX_CHARS_IN_CLIENT_TYPING_WINDOW) > 0 ) {
		int num_chars_off_screen =  strlen(client_buffer) - MAX_CHARS_IN_CLIENT_TYPING_WINDOW;
		client_current_line = 0;

		while(num_chars_off_screen > 0) {
			client_current_line ++;
			num_chars_off_screen -= MAX_COLUMNS;
		}

		wprintw(client_chat_window, &client_buffer[client_current_line*MAX_COLUMNS]);
	} else {
		wprintw(client_chat_window, client_buffer);
	}
}


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
	wprintw(transcript_window, text);
}



int main(int argc, char* argv[])
{
	int is_running = 1;
	int x_terminal_size, y_terminal_size;

	memset(client_buffer, '\0', sizeof(client_buffer));
	get_terminal_size(&x_terminal_size, &y_terminal_size);
	
	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();

	transcript_window  = newwin(23,40,0,0);
	client_chat_window = newwin(MAX_ROWS,MAX_COLUMNS,20,40);

	write_to_transcript_window("Hello!\n");
	write_to_transcript_window("Hey all!\nI love BlackChat, it's so awesome!\n");
	write_to_transcript_window("Press the \'q\' key to quit!");

	while(is_running) {
		int ch = getch();
		switch(ch) {
			case 'q': 
				is_running = 0;
				break;

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
				
				/* Print our new chat buffer. */
				print_client_chat_buffer();
				break;
		}

		wrefresh(transcript_window);
		wrefresh(client_chat_window);
	}

	delwin(transcript_window);
	delwin(client_chat_window);
	endwin();

	return 0;
}
