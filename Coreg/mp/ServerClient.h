#pragma once

#include "RakPeerInterface.h"
#include "BitStream.h"
#include "MessageIdentifiers.h"
#include "RakNetTypes.h"

#include "game/eCEntity.h"

static char *SERVER_HOST = "127.0.0.1";
static unsigned SERVER_PORT = 6666;

enum NETWORK_MESSAGE {
	ENTITY_POSITION = ID_USER_PACKET_ENUM + 1,
	ENTITY_ROTATION = ID_USER_PACKET_ENUM + 2
};

class ServerClient
{
private:
	RakNet::AddressOrGUID connectionGUID;
	RakNet::Packet *packet;
	RakNet::RakPeerInterface *peer;

	eCGeometryEntity *player;

	bool hasConnection;
public:
	ServerClient(eCGeometryEntity &entity);
	~ServerClient();

	bool Connect(const char *host, const unsigned port);
	bool Connect();
	void Pulse();
	bool isConnected();

	void SendPosition();
	void SendRotation();
};