#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include <curses.h>
#include <enet/enet.h>

#include "chat_interface.h"
#include "network.h"

bool running = true;
void handle_sigint(int _) { running = false; }

bool connect_server(
	const char *server_address,
	enet_uint16 port,
	const char *username,
	ENetHost **host,
	ENetPeer **peer
) {
	if (enet_initialize() != 0) {
		fprintf(stderr, "Failed to initialize enet\n");
		return false;
	}

	*host = enet_host_create(NULL, 1, 2, 0, 0);
	if (*host == NULL) {
		fprintf(stderr, "Failed to create client host\n");
		return false;
	}

	ENetAddress address;
	enet_address_set_host(&address, server_address);
	address.port = port;

	*peer = enet_host_connect(*host, &address, 1, 0);
	if (*peer == NULL) {
		fprintf(stderr, "Failed to connect to server\n");
		enet_host_destroy(*host);
		return false;
	}

	ENetEvent event;
	if (enet_host_service(*host, &event, 1000) <= 0) {
		fprintf(stderr, "Failed to connect to server (timeout)\n");
		enet_host_destroy(*host);
		return false;
	}

	if (username != NULL)
		network_send(*peer, PACKET_TYPE_CLIENT_NAME, username, strlen(username));

	return true;
}

int main(int argc, const char **argv) {
	signal(SIGINT, handle_sigint);

	ENetHost *host;
	ENetPeer *peer;
	const char *address = argc > 1 ? argv[1] : "localhost";
	const char *username = argc > 2 ? argv[2] : NULL;
	if (!connect_server(address, 42069, username, &host, &peer))
		return EXIT_FAILURE;
	chat_interface_init();

	ENetEvent event;
	char input[PACKET_MESSAGE_LENGTH/2] = { 0 };
	size_t input_length = 0;
	while (running) {
		enet_host_service(host, &event, 0);

		if (event.type == ENET_EVENT_TYPE_RECEIVE) {
			PacketType type;
			char message[PACKET_MESSAGE_LENGTH];
			network_receive(event.packet->data, &type, message);
			chat_interface_add_message(message);
		}

		chat_interface_render_input(input, input_length);
		if (chat_interface_poll_input(input, &input_length, sizeof(input))) {
			network_send(peer, PACKET_TYPE_CLIENT_MESSAGE, input, input_length);
			input[(input_length = 0)] = '\0';
		}
	}

	enet_peer_disconnect(peer, 0);
	enet_host_service(host, &event, 50);
	endwin();
	return 0;
}
