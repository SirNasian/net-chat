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

int main(int argc, char **argv) {
	signal(SIGINT, handle_sigint);

	int error;
	if ((error = enet_initialize()) != 0) {
		fprintf(stderr, "Failed to initialize enet: %d\n", error);
		return EXIT_FAILURE;
	}

	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = 42069;

	ENetHost *host = enet_host_create(&address, 32, 1, 0, 0);
	if (host == NULL) {
		fprintf(stderr, "Failed to create server host\n");
		return EXIT_FAILURE;
	}

	char ip[16] = { 0 };
	char message[PACKET_MESSAGE_LENGTH];
	PacketType type;
	ENetEvent event;
	while (running) {
		if (enet_host_service(host, &event, 50) < 0) break;
		switch (event.type) {
			case ENET_EVENT_TYPE_NONE:
				break;
			case ENET_EVENT_TYPE_CONNECT:
				readable_ip(event.peer->address.host, ip);
				printf("Peer connected (%s:%u)\n", ip, event.peer->address.port);
				event.peer->data = strdup(ip);
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				free(event.peer->data);
				readable_ip(event.peer->address.host, ip);
				printf("Peer disconnect (%s:%u)\n", ip, event.peer->address.port);
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				network_receive(event.packet->data, &type, message);
				switch (type) {
					case PACKET_TYPE_CLIENT_NAME:
						free(event.peer->data);
						event.peer->data = strdup(message);
						break;
					case PACKET_TYPE_CLIENT_MESSAGE: {
						char enriched_message[PACKET_MESSAGE_LENGTH];
						sprintf(enriched_message, "[%s] %s\n", (char*)event.peer->data, message);
						printf("%s\n", enriched_message);
						network_broadcast(host, PACKET_TYPE_SERVER_MESSAGE, enriched_message, strlen(enriched_message));
						break;
					}
					default:
						break;
				}
				enet_packet_destroy(event.packet);
				break;
		}
	}

	enet_host_destroy(host);
	return 0;
}
