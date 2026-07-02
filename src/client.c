#include <curses.h>

WINDOW* log_win;
WINDOW* log_border_win;
WINDOW* input_win;

void init_windows() {
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
}

void poll_input(char *buffer, size_t length) {
	werase(input_win);
	box(input_win, 0, 0);
	mvwprintw(input_win, 1, 1, "> ");
	wrefresh(input_win);

	echo();
	curs_set(1);
	wmove(input_win, 1, 3);
	wgetnstr(input_win, buffer, (int)length-1);
	noecho();
	curs_set(0);
}

void log_message(const char *message) {
	wmove(log_win, getmaxy(log_win)-1, 0);
	waddstr(log_win, message);
	wrefresh(log_win);
	wscrl(log_win, 1);
}

int main() {
	initscr();
	noecho();
	cbreak();
	curs_set(0);

	init_windows();

	char input[256];
	while (true) {
		poll_input(input, sizeof(input));
		log_message(input);
	}

	endwin();
	return 0;
}
