#include <string.h>

#include "network.h"

void network_broadcast(ENetHost *host, PacketType type, const char *message, uint8_t length) {
	if (!length) return;
	uint8_t data[PACKET_MESSAGE_LENGTH+2];
	data[0] = type;
	data[1] = length;
	memcpy(&data[2], message, length);
	enet_host_broadcast(host, 0, enet_packet_create(data, length+2, ENET_PACKET_FLAG_RELIABLE));
}

void network_send(ENetPeer *peer, PacketType type, const char *message, uint8_t length) {
	if (!length) return;
	uint8_t data[PACKET_MESSAGE_LENGTH+2];
	data[0] = type;
	data[1] = length;
	memcpy(&data[2], message, length);
	enet_peer_send(peer, 0, enet_packet_create(data, length+2, ENET_PACKET_FLAG_RELIABLE));
}

void network_receive(uint8_t *data, PacketType *type, char *message) {
	*type = data[0];
	uint8_t length = data[1];
	if (length) memcpy(message, &data[2], length);
	message[length] = '\0';
}
