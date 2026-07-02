#include <ctype.h>
#include <signal.h>
#include <stdlib.h>

#include <curses.h>
#include <enet/enet.h>

WINDOW *log_win;
WINDOW *log_border_win;
WINDOW *input_win;

bool running = true;
void handle_sigint(int _) { running = false; }

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

void log_message(const char *message) {
	wmove(log_win, getmaxy(log_win)-1, 0);
	waddstr(log_win, message);
	wrefresh(log_win);
	wscrl(log_win, 1);
}

void render_input(char *buffer, size_t length) {
	werase(input_win);
	mvwprintw(input_win, 1, 1, "> %s", buffer);
	wmove(input_win, 1, length+3);
	box(input_win, 0, 0);
	wrefresh(input_win);
}

bool poll_input(char *buffer, size_t *length, size_t max_length) {
	if (*length >= max_length)
		return false;

	int ch = wgetch(input_win);
	if (ch == ERR) return false;

	if (ch == KEY_BACKSPACE) {
		*length && (buffer[--(*length)] = (char)0);
	} else if (isprint(ch)) {
		buffer[(*length)++] = (char)ch;
		buffer[(*length)]   = (char)0;
	}

	return (ch == KEY_ENTER) || (ch == '\n') || (ch == '\r');
}

int main() {
	signal(SIGINT, handle_sigint);

	int error;
	if ((error = enet_initialize()) != 0) {
		fprintf(stderr, "Failed to initialize enet: %d\n", error);
		return EXIT_FAILURE;
	}

	ENetHost *host = enet_host_create(NULL, 1, 2, 0, 0);
	if (host == NULL) {
		fprintf(stderr, "Failed to create client host\n");
		return EXIT_FAILURE;
	}

	ENetAddress address;
	enet_address_set_host(&address, "localhost");
	address.port = 42069;

	ENetPeer *peer = enet_host_connect(host, &address, 1, 0);
	if (peer == NULL) {
		fprintf(stderr, "Failed to connect to server\n");
		return EXIT_FAILURE;
	}

	ENetEvent event;
	if (enet_host_service(host, &event, 1000) <= 0) {
		fprintf(stderr, "Failed to connect to server (timeout)\n");
		return EXIT_FAILURE;
	}

	initscr();
	noecho();
	cbreak();
	curs_set(1);

	init_windows();
	keypad(input_win, TRUE);
	wtimeout(input_win, 50);

	char input[256] = { 0 };
	size_t length = 0;

	while (running) {
		enet_host_service(host, &event, 0);
		render_input(input, length);
		if (poll_input(input, &length, sizeof(input))) {
			log_message(input);
			input[(length = 0)] = '\0';
		}
	}

	enet_peer_disconnect(peer, 0);
	enet_host_service(host, &event, 50);
	endwin();
	return 0;
}
