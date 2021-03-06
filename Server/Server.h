#pragma once

#include "RakPeerInterface.h"
#include "BitStream.h"
#include "MessageIdentifiers.h"
#include <stdio.h>

#include "EntityManager.h"
#include "../Shared/tools.h"

struct ServerConfig {
	char *host;
	unsigned port;
	unsigned max_clients;
	unsigned outgoing_limit;
};

class Server
{
private:
	RakNet::Packet *packet;
	RakNet::RakPeerInterface *peer;
	
	ServerConfig server_config;
	EntityManager *entity_manager;

	bool hasStarted;
	bool shuttingDown;

public:
	Server(ServerConfig &sc);
	~Server();

	bool Start();
	void Pulse();
	bool isShuttingDown();

	void SendInitSuccess(PLAYERID playerID, RakNet::SystemAddress addr);
	void SendNotInitialized(RakNet::SystemAddress addr);
	
	void SendNewClientNotification(PLAYERID connectingID, Entity &connectingPlayer);
	void SendCLientLeavingNotification(PLAYERID disconnectingID);

	void RelayClientInformation(PLAYERID playerID, Entity &player);
};