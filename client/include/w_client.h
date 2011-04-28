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


#ifndef __BCCLIENT__
#define __BCCLIENT__


/* Get the text inside our chat window. */
wchar_t *grab_text_from_client_typing_window(void);


/* Clear the text from our chat window. */
void clear_text_from_client_typing_window(void);


/* Add some text to our transcript window. */
void write_to_transcript_window(wchar_t *str);






/* This sets the name of one of the client windows.
 * @param num  - The number of the window to set the name on.
 * @param name - The name of the person in the client window.
 */
void set_window_user_name(int num, wchar_t *name);

/* Adds the specified string to the specified chat window.
 * @param num  - The number of the window to append the text to.
 * @param text - The text to append to the chat window.
 */
void append_text_to_window(int num, wchar_t *text);

/* Clears the text from the specified user window. 
 * @param num  - The number of the window to clear the text from.
 */
void clear_user_window_text(int num);

/* Enables/disables deep-six vote on the given user.
 * @param num       - The number of the window to clear the text from.
 * @param can_vote  - Boolean (true/false).  If true, than we can deep-six this user. If false, then we cannot.
 */
void can_deepsix_user(int num, int can_vote);

/* Sets the status of this user (which appears as a single wchar_tacter after there username).
 * @param num     - The number of the window to clear the text from.
 * @param status  - The user's current status expressed as a wchar_tacter.
 */
void set_user_status(int num, wchar_t status);

/* Removes the current user from the chat window.  You should call this if the 
 * user disconnects from the server.
 * @param num  - The number/index of the window/user you want to remove.
 */
void remove_user_from_window(int num); 





/* Sets the text that will appear in the status window.
 * @text  - The text to appear in the status window.
 */
void set_text_in_status_window(wchar_t *text);





/* This sets the yell message at the given index.
 * @param index   - Should be a number 0-25, where 0=A, 1=B, 2=C, ... 25=Z
 * @param message - The message to be associted with this index/letter.
 * */
void set_yell_message(int index, wchar_t *message);


/* Returns clients name. */
wchar_t *get_client_name();


#endif

