#include "Server.h"

/** Constructor
 *
 * Constructor which initializes the local variables including a Raknet
 * Peer Interface and our Entity Manager.
 */
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

/** Starts the server
 *
 * Starts the Raknet networking threads, opens up a server, sets the client limit
 * and bandwidth limits. It sets an appropriate flags and prints errors if needed.
 *
 * @note Even though the server has started, it's actually an attempt to start the
 *		 server, not the final state. That will be called through a packet sent to
 *		 itself.
 *
 * @return bool True if the server has 
 */
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

/** Server main loop
 *
 * Server main loop
 */
void Server::Pulse()
{
	if (!hasStarted)
	{
		INFORM(SYSTEM, "Can not Pulse because server hasn't started.");
		return;
	}

	// Go through each packet that has been received since last Pulse
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
			// The server has started, opened its sockets and is ready to operate
			case ID_CONNECTION_REQUEST_ACCEPTED: {
				INFORM(RAKNET, "Connection request accepted. Begin receiving data ...");
			} break;

			// New connection coming from a client - initial contact
			case ID_NEW_INCOMING_CONNECTION: {
				INFORM(RAKNET, "New incoming connection.");
				INFORMS("     '- Client IP: %s\n", packet->systemAddress.ToString());
			} break;

			// Client notyfing the server that they're disconnecting
			case ID_DISCONNECTION_NOTIFICATION: {
				INFORM(RAKNET, "A client has disconnected.");
				INFORMS("     '- Client IP: %s\n", packet->systemAddress.ToString());
			} break;

			// Client timed out
			case ID_CONNECTION_LOST: {
				INFORM(RAKNET, "A client lost the connection.");
				INFORMS("     '- Client IP: %s\n", packet->systemAddress.ToString());
			} break;

			// Client sending initial information about itself
			case ENTITY_INIT: {
				INFORM(RAKNET, "Getting init from clinet.");
				INFORMS("     '- Client IP: %s\n", packet->systemAddress.ToString());
				
				Entity *newEntity = entity_manager->CreateEntity();
				data.Read((char *)&newEntity->nickname, sizeof(MAX_NICKNAME_LENGTH));
				data.Read((char *)&newEntity->pos, sizeof(Vec3));
				data.Read((char *)&newEntity->rot, sizeof(bCQuaternion));
				
				newEntity->guid = packet->guid;
				newEntity->ipaddress = packet->systemAddress;
				newEntity->initialized = true;

				SendInitSuccess(newEntity->systemID, packet->systemAddress);
			} break;

			// Client sending update with its position and rotation
			case ENTITY_POSROT: {
				PLAYERID playerID = entity_manager->getEntityID(packet->guid);
				if (playerID != INVALID_PLAYERID)
				{
					Entity *player = entity_manager->getEntity(playerID);
					if (player->initialized)
					{
						data.Read((char *)&player->pos, sizeof(Vec3));
						data.Read((char *)&player->rot, sizeof(bCQuaternion));
						//player->printDebug();
						RelayClientInformation(playerID, *player);
					} else {
						SendNotInitialized(packet->systemAddress);
					}
				}
			} break;

			// Catch-all in case some other callback is called
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

/** Send INIT_SUCCESS flag to client
 *
 * Sends a message to the client that ackgnowledges
 * the server receiving its initial contact.
 *
 * @param RakNet::SystemAddress the address (client) to which the flag should be sent
 */
void Server::SendInitSuccess(PLAYERID playerID, RakNet::SystemAddress addr)
{
	RakNet::BitStream bsOut;
	unsigned char raknet_identifier = ENTITY_INIT_SUCCESS;

	bsOut.Write(raknet_identifier);
	bsOut.Write(playerID);
	peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addr, false);
}

/** Send ENTITY_NOT_INITIALIZED flag to client
 *
 * Sends a message to the client that indicates that the client haven't sent
 * initial information but tried to send other packets. This won't be allowed.
 *
 * @param RakNet::SystemAddress the address (client) to which the flag should be sent
 */
void Server::SendNotInitialized(RakNet::SystemAddress addr)
{
	RakNet::BitStream bsOut;
	unsigned char raknet_identifier = ENTITY_NOT_INITIALIZED;

	bsOut.Write(raknet_identifier);
	peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addr, false);
}

void Server::SendNewClientNotification(PLAYERID connectingID, Entity &connectingPlayer)
{
	RakNet::BitStream bsOut;
	unsigned char raknet_identifier = ENTITY_NEW_CLIENT_NOTE;

	bsOut.Write(raknet_identifier);
	bsOut.Write(connectingID);
	bsOut.Write((const char *)&connectingPlayer.nickname, MAX_NICKNAME_LENGTH);
	bsOut.Write((const char *)&connectingPlayer.pos, sizeof(Vec3));
	bsOut.Write((const char *)&connectingPlayer.rot, sizeof(bCQuaternion));

	for (PLAYERID i = 0; i < SERVER_PLAYER_LIMIT; i++)
	{
		if(entity_manager->entities[i] && i != connectingID)
		{
			peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, entity_manager->entities[i]->ipaddress, false);
		}
	}
}

void Server::SendCLientLeavingNotification(PLAYERID disconnectingID)
{
	RakNet::BitStream bsOut;
	unsigned char raknet_identifier = ENTITY_CLIENT_LEAVING_NOTE;

	bsOut.Write(raknet_identifier);
	bsOut.Write(disconnectingID);

	for (PLAYERID i = 0; i < SERVER_PLAYER_LIMIT; i++)
	{
		if (entity_manager->entities[i] && i != disconnectingID)
		{
			peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, entity_manager->entities[i]->ipaddress, false);
		}
	}
}

void Server::RelayClientInformation(PLAYERID playerID, Entity &player)
{
	RakNet::BitStream bsOut;
	unsigned char raknet_identifier = ENTITY_POSROT;

	bsOut.Write(raknet_identifier);
	bsOut.Write(playerID);
	bsOut.Write((const char *)&player.pos, sizeof(Vec3));
	bsOut.Write((const char *)&player.rot, sizeof(bCQuaternion));

	for (PLAYERID i = 0; i < SERVER_PLAYER_LIMIT; i++)
	{
		if (entity_manager->entities[i] && i != playerID)
		{
			peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, entity_manager->entities[i]->ipaddress, false);
		}
	}
}





/** Tells whether the server is shutting down.
 *
 * Tells whether the server is shutting down.
 *
 * @return bool True if the server is shutting down; False otherwise
 */
bool Server::isShuttingDown()
{
	return shuttingDown;
}

/** Destructor
 *
 * Destructor which cleans up the server variables.
 */
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