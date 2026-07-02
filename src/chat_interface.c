#include "chat_interface.h"

#include <ctype.h>

WINDOW *log_win;
WINDOW *log_border_win;
WINDOW *input_win;

void chat_interface_init(void) {
	initscr();
	noecho();
	cbreak();
	curs_set(1);

	int rows, cols;
	getmaxyx(stdscr, rows, cols);

	log_border_win = newwin(rows-3, cols, 0, 0);
	box(log_border_win, 0, 0);
	wrefresh(log_border_win);

	log_win = derwin(log_border_win, getmaxy(log_border_win)-2, getmaxx(log_border_win)-2, 1, 1);
	scrollok(log_win, TRUE);
	wrefresh(log_win);

	input_win = newwin(3, cols, rows-3, 0);
	box(input_win, 0, 0);
	wrefresh(input_win);

	keypad(input_win, TRUE);
	wtimeout(input_win, 50);
}

void chat_interface_add_message(const char *message) {
	wmove(log_win, getmaxy(log_win)-1, 0);
	waddstr(log_win, message);
	wrefresh(log_win);
	wscrl(log_win, 1);
}

void chat_interface_render_input(const char *buffer, size_t length) {
	werase(input_win);
	mvwprintw(input_win, 1, 1, "> %s", buffer);
	wmove(input_win, 1, length+3);
	box(input_win, 0, 0);
	wrefresh(input_win);
}

bool chat_interface_poll_input(char *buffer, size_t *length, size_t max_length) {
	int ch = wgetch(input_win);
	if (ch == ERR) return false;

	if (ch == KEY_BACKSPACE) {
		*length && (buffer[--(*length)] = (char)0);
	} else if (isprint(ch) && *length < max_length) {
		buffer[(*length)++] = (char)ch;
		buffer[(*length)]   = (char)0;
	}

	return (ch == KEY_ENTER) || (ch == '\n') || (ch == '\r');
}
