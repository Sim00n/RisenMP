#include "Server.h"

Server::Server(ServerConfig &sc)
{
	server_config = sc;
	
	peer = RakNet::RakPeerInterface::GetInstance();
	assert(peer);

	entity_manager = new EntityManager();
	assert(entity_manager);

	packet = nullptr;
	hasStarted = false;
	shuttingDown = false;
}

bool Server::Start()
{
	INFORM(SYSTEM, "Starting Raknet server ... ");
	
	RakNet::SocketDescriptor sd(server_config.port, 0);

	// Attempt to create a network thread and init RakNet
	RakNet::StartupResult startup_result = peer->Startup(server_config.max_clients, &sd, 1);

	// Show network errors
	switch (startup_result)
	{
		case RakNet::RAKNET_ALREADY_STARTED: INFORM(RAKNET, "Attempted to restart RakNet."); break;
		case RakNet::SOCKET_PORT_ALREADY_IN_USE: INFORM(RAKNET, "Socket port already in use when starting RakNet."); break;
		case RakNet::SOCKET_FAILED_TO_BIND: INFORM(RAKNET, "Socket failed to bind when starting RakNet."); break;
		case RakNet::PORT_CANNOT_BE_ZERO: INFORM(RAKNET, "Tried to bind a port of 0 when starting RakNet."); break;
		case RakNet::FAILED_TO_CREATE_NETWORK_THREAD: INFORM(RAKNET, "Failed to create network thread when starting RakNet."); break;
		case RakNet::STARTUP_OTHER_FAILURE: INFORM(RAKNET, "Unknown error when starting RakNet."); break;
	}

	if (startup_result == RakNet::RAKNET_STARTED)
	{
		// Attempt to connect to the server
		RakNet::ConnectionAttemptResult connection_attemp = peer->Connect(server_config.host, server_config.port, 0, 0);

		// Show connection errors
		switch (connection_attemp)
		{
			case RakNet::INVALID_PARAMETER: INFORM(RAKNET, "Invalid parameter when attempting to connect."); break;
			case RakNet::CANNOT_RESOLVE_DOMAIN_NAME: INFORM(RAKNET, "Cannot resolve domain name when attempting to connect."); break;
			case RakNet::ALREADY_CONNECTED_TO_ENDPOINT: INFORM(RAKNET, "Attempted to reconnect to the server."); break;
			case RakNet::CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS: INFORM(RAKNET, "Already attempting to connect."); break;
			case RakNet::SECURITY_INITIALIZATION_FAILED: INFORM(RAKNET, "Security initialization failed"); break;
		}

		// If we're connecting, set other settings
		if (connection_attemp == RakNet::CONNECTION_ATTEMPT_STARTED)
		{
			// Set max players
			peer->SetMaximumIncomingConnections(server_config.max_clients);
			// Set max bandwitch per player
			peer->SetPerConnectionOutgoingBandwidthLimit(server_config.outgoing_limit);

			hasStarted = true;
			INFORM(SYSTEM, "Server started correctly.");
			return TRUE;
		}
	}

	shuttingDown = true;
	INFORM(SYSTEM, "Server failed to start.");
	return FALSE;
}

void Server::Pulse()
{
	if (!hasStarted)
	{
		INFORM(SYSTEM, "Can not Pulse because server hasn't started.");
		return;
	}

	while (packet = peer->Receive())
	{
		// Get the packet data into a BitStream
		RakNet::BitStream data(packet->data, packet->length, false);

		// Extract the first byte which tells us what kind of packet this is
		const unsigned char raknet_identifier = 0;
		data.Read(raknet_identifier);

		//switch ((unsigned) raknet_identifier)
		switch (packet->data[0])
		{
			case ID_CONNECTION_REQUEST_ACCEPTED: {
				INFORM(RAKNET, "Connection request accepted. Begin receiving data ...");
			} break;
			case ID_NEW_INCOMING_CONNECTION: {
				INFORM(RAKNET, "New incoming connection.");
				INFORMS(" '- Client IP: %s\n", packet->systemAddress.ToString());
			} break;
			case ID_DISCONNECTION_NOTIFICATION: {
				INFORM(RAKNET, "A client has disconnected.");
				INFORMS(" '- Client IP: %s\n", packet->systemAddress.ToString());
			} break;
			case ID_CONNECTION_LOST: {
				INFORM(RAKNET, "A client lost the connection.");
				INFORMS(" '- Client IP: %s\n", packet->systemAddress.ToString());
			} break;
			case ENTITY_INIT: {
				INFORM(RAKNET, "Getting init from clinet.");
				Entity *newEntity = entity_manager->CreateEntity();
				data.Read((char *)&newEntity->nickname, sizeof(MAX_NICKNAME_LENGTH));
				data.Read((char *)&newEntity->pos, sizeof(Vec3));
				data.Read((char *)&newEntity->rot, sizeof(bCQuaternion));
				
				newEntity->guid = packet->guid;
				newEntity->ipaddress = packet->systemAddress.ToString();
				newEntity->initialized = true;

				RakNet::BitStream bsOut;
				unsigned char raknet_identifier = ENTITY_INIT_SUCCESS;

				bsOut.Write(raknet_identifier);
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			} break;
			case ENTITY_POSROT: {
				INFORM(RAKNET, "Getting posrot from client.");
				PLAYERID playerID = entity_manager->getEntityID(packet->guid);
				if (playerID != INVALID_PLAYERID)
				{
					Entity *player = entity_manager->getEntity(playerID);
					if (player->initialized)
					{
						INFORM(RAKNET, "Posrot, processing data");
						data.Read((char *)&player->pos, sizeof(Vec3));
						data.Read((char *)&player->rot, sizeof(bCQuaternion));
						player->printDebug();
					}
					else {
						INFORM(RAKNET, "Posrot, not initialized else");
						RakNet::BitStream bsOut;
						unsigned char raknet_identifier = ENTITY_NOT_INITIALIZED;

						bsOut.Write(raknet_identifier);
						peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
					}
				}
				else {
					INFORM(ERR, "Can not find player ID");
				}
			} break;
			default: {
				INFORM(RAKNET, "Received some other data. Here it is:");
				unsigned char *otherData = data.GetData();
				for (int i = 0; i < sizeof(otherData); i++) {
					printf("%d (%c) | ", *(otherData + i), *(otherData + i));
				}
			} break;
		}

		peer->DeallocatePacket(packet);
	}
}

bool Server::isShuttingDown()
{
	return shuttingDown;
}

Server::~Server()
{
	if (peer) {
		RakNet::RakPeerInterface::DestroyInstance(peer);
		peer = nullptr;
	}

	if (entity_manager)
	{
		delete entity_manager;
		entity_manager = nullptr;
	}
}