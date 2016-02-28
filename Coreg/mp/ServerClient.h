#pragma once

#include "RakPeerInterface.h"
#include "BitStream.h"
#include "MessageIdentifiers.h"
#include "RakNetTypes.h"

#include "../../Shared/tools.h"
#include "NPCManager.h"
#include "game/eCEntity.h"

static char *SERVER_HOST = "127.0.0.1";
static unsigned SERVER_PORT = 6666;
const char nickname[MAX_NICKNAME_LENGTH] = "Sim00n";

class ServerClient
{
private:
	RakNet::AddressOrGUID connectionGUID;
	RakNet::Packet *packet;
	RakNet::RakPeerInterface *peer;

	PLAYERID myID;

	NPCManager *npc_manager;
	eCGeometryEntity *player;

	bool hasConnection;
	bool initialized;
public:
	ServerClient(NPCManager &npc_manager, eCGeometryEntity &entity);
	~ServerClient();

	bool Connect(const char *host, const unsigned port);
	bool Connect();
	void Pulse();
	bool isConnected();

	void SendInit();
	void SendPosRot();
};