#ifndef NETWORK_H
#define NETWORK_H

#include <stddef.h>

#include <enet/enet.h>

#define PACKET_MESSAGE_LENGTH 256

typedef enum PacketType {
	PACKET_TYPE_SERVER_MESSAGE,
	PACKET_TYPE_CLIENT_MESSAGE,
	PACKET_TYPE_CLIENT_NAME
} PacketType;

void network_broadcast(ENetHost *host, PacketType type, const char *message, uint8_t length);
void network_send(ENetPeer *peer, PacketType type, const char *message, uint8_t length);
void network_receive(uint8_t *data, PacketType *type, char *message);

#endif
