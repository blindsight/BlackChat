/*
 * BlackChat
 *
 * Nice curses tutorials:
 * http://snap.nlc.dcccd.edu/learn/fuller3/chap6/chap6.html
 */


#ifndef __BCCLIENT__
#define __BCCLIENT__


/* Get the text inside our chat window. */
char *grab_text_from_client_typing_window(void);


/* Clear the text from our chat window. */
void clear_text_from_client_typing_window(void);


/* Add some text to our transcript window. */
void write_to_transcript_window(char *str);


/* This sets the name of one of the client windows.
 * @param num  - The number of the window to set the name on.
 * @param name - The name of the person in the client window.
 */
void set_window_user_name(int num, char *name);


/* This adds the specified string to the specified chat window.
 * @param num  - The number of the window to append the text to.
 * @param text - The text to append to the chat window.
 */
void append_text_to_window(int num, char *text);

/* This clears the text from the specified user window. 
 * @param num  - The number of the window to clear the text from.
 */
void clear_user_window_text(int num);


/* This sets the yell message at the given index.
 * @param index   - Should be a number 0-25, where 0=A, 1=B, 2=C, ... 25=Z
 * @param message - The message to be associted with this index/letter.
 * */
void set_yell_message(int index, char *message);


#endif

