#ifndef CHAT_INTERFACE_H
#define CHAT_INTERFACE_H

#include <stdbool.h>

#include <curses.h>

void chat_interface_init(void);
void chat_interface_add_message(const char *message);
void chat_interface_render_input(const char *buffer, size_t length);
bool chat_interface_poll_input(char *buffer, size_t *length, size_t max_length);

#endif
