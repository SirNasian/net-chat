#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <enet/enet.h>

#include "network.h"

bool running = true;
void handle_sigint(int _) { running = false; }

void readable_ip(enet_uint32 address, char *buffer) {
	sprintf(buffer, "%u.%u.%u.%u",
		(address >>  0) & 0xFF,
		(address >>  8) & 0xFF,
		(address >> 16) & 0xFF,
		(address >> 24) & 0xFF
	);
}

ENetHost* start_server(const char *listen_address, int port) {
	if (enet_initialize() != 0) {
		fprintf(stderr, "Failed to initialize enet\n");
		return NULL;
	}

	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = port;

	if (listen_address != NULL)
		enet_address_set_host(&address, listen_address);

	ENetHost *host = enet_host_create(&address, 32, 1, 0, 0);
	if (host == NULL)
		fprintf(stderr, "Failed to create server host\n");

	return host;
}

void handle_connect(ENetEvent *event) {
	char ip[16] = { 0 };
	readable_ip(event->peer->address.host, ip);
	event->peer->data = strdup(ip);
}

void handle_disconnect(ENetHost *host, ENetEvent *event) {
	char message[PACKET_MESSAGE_LENGTH];
	sprintf(message, "%s has disconnected", (char*)event->peer->data);
	network_broadcast(host, PACKET_TYPE_SERVER_MESSAGE, message, strlen(message));
	free(event->peer->data);
}

void handle_data_receive(ENetHost *host, ENetEvent *event) {
	PacketType type;
	char message[PACKET_MESSAGE_LENGTH];
	network_receive(event->packet->data, &type, message);
	switch (type) {
		case PACKET_TYPE_CLIENT_NAME:
			free(event->peer->data);
			event->peer->data = strdup(message);
			sprintf(message, "%s has connected", (char*)event->peer->data);
			network_broadcast(host, PACKET_TYPE_SERVER_MESSAGE, message, strlen(message));
			break;
		case PACKET_TYPE_CLIENT_MESSAGE: {
			char enriched_message[PACKET_MESSAGE_LENGTH];
			sprintf(enriched_message, "[%s] %s", (char*)event->peer->data, message);
			network_broadcast(host, PACKET_TYPE_SERVER_MESSAGE, enriched_message, strlen(enriched_message));
			break;
		}
		default:
			break;
	}
}

int main(int argc, char **argv) {
	signal(SIGINT, handle_sigint);

	const char *address = argc > 1 ? argv[1] : NULL;
	ENetHost *host = start_server(address, 42069);
	if (host == NULL)
		return EXIT_FAILURE;

	ENetEvent event;
	while (running) {
		if (enet_host_service(host, &event, 50) < 0) break;
		switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
				handle_connect(&event);
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				handle_disconnect(host, &event);
				break;
			case ENET_EVENT_TYPE_RECEIVE: {
				handle_data_receive(host, &event);
				enet_packet_destroy(event.packet);
				break;
			}
			default:
				break;
		}
	}

	network_broadcast(host, PACKET_TYPE_SERVER_MESSAGE, "server is shutting down!", 24);
	enet_host_service(host, &event, 50);
	enet_host_destroy(host);
	return 0;
}
