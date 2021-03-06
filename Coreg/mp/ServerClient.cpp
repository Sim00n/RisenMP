#include "ServerClient.h"


ServerClient::ServerClient(NPCManager &npc_manager,  eCGeometryEntity &entity)
{
	this->npc_manager = &npc_manager;
	player = &entity;

	peer = RakNet::RakPeerInterface::GetInstance();
	assert(peer);

	connectionGUID.rakNetGuid = peer->GetMyGUID();
	connectionGUID.systemAddress = peer->GetMyBoundAddress();
	//connectionGUID.rakNetGuid = RakNet::UNASSIGNED_RAKNET_GUID;
	//connectionGUID.systemAddress = RakNet::UNASSIGNED_SYSTEM_ADDRESS;
	
	LOG("GUID: %s, Addr: %s\n", connectionGUID.rakNetGuid.ToString(), connectionGUID.systemAddress.ToString());

	myID = INVALID_PLAYERID;
	packet = nullptr;
	hasConnection = false;
}

bool ServerClient::Connect()
{
	return Connect(SERVER_HOST, SERVER_PORT);
}

bool ServerClient::Connect(const char *host, const unsigned port)
{
	INFORM(NETWORK, "Starting connection process ... ");

	// Get current connection state
	RakNet::ConnectionState connection_state = peer->GetConnectionState(connectionGUID);
	
	// Show connection errors
	switch (connection_state)
	{
		case RakNet::IS_PENDING: INFORM(NETWORK, "The connection is pending."); break;
		case RakNet::IS_CONNECTING: INFORM(NETWORK, "Processing the connection attempt."); break;
		case RakNet::IS_DISCONNECTED: INFORM(NETWORK, "The connectiong will be terminated once all the messages are delivered."); break;
		case RakNet::IS_SILENTLY_DISCONNECTING: INFORM(NETWORK, "Connection failed and will be aborted."); break;
	}

	if (connection_state != RakNet::IS_DISCONNECTED && connection_state != RakNet::IS_NOT_CONNECTED)
	{
		INFORM(NETWORK, "A connection has been established and we can not open another one now.");
		return FALSE;
	}

	// Attempt to create a network thread and init RakNet
	RakNet::StartupResult startup_result = peer->Startup(1, &RakNet::SocketDescriptor(), 1);

	// Show network errors
	switch (startup_result)
	{
		case RakNet::RAKNET_ALREADY_STARTED: INFORM(NETWORK, "Attempted to restart RakNet."); break;
		case RakNet::SOCKET_PORT_ALREADY_IN_USE: INFORM(NETWORK, "Socket port already in use when starting RakNet."); break;
		case RakNet::SOCKET_FAILED_TO_BIND: INFORM(NETWORK, "Socket failed to bind when starting RakNet."); break;
		case RakNet::PORT_CANNOT_BE_ZERO: INFORM(NETWORK, "Tried to bind a port of 0 when starting RakNet."); break;
		case RakNet::FAILED_TO_CREATE_NETWORK_THREAD: INFORM(NETWORK, "Failed to create network thread when starting RakNet."); break;
		case RakNet::STARTUP_OTHER_FAILURE: INFORM(NETWORK, "Unknown error when starting RakNet."); break;
	}

	if (startup_result == RakNet::RAKNET_STARTED)
	{
		// Attempt to connect to the server
		RakNet::ConnectionAttemptResult connection_attemp = peer->Connect(host, port, 0, 0);

		// Show connection errors
		switch (connection_attemp)
		{
			case RakNet::INVALID_PARAMETER: INFORM(NETWORK, "Invalid parameter when attempting to connect."); break;
			case RakNet::CANNOT_RESOLVE_DOMAIN_NAME: INFORM(NETWORK, "Cannot resolve domain name when attempting to connect."); break;
			case RakNet::ALREADY_CONNECTED_TO_ENDPOINT: INFORM(NETWORK, "Attempted to reconnect to the server."); break;
			case RakNet::CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS: INFORM(NETWORK, "Already attempting to connect."); break;
			case RakNet::SECURITY_INITIALIZATION_FAILED: INFORM(NETWORK, "Security initialization failed"); break;
		}

		if (connection_attemp == RakNet::CONNECTION_ATTEMPT_STARTED)
		{
			return TRUE;
		}
	}

	return FALSE;
}

void ServerClient::Pulse()
{
	/**
	 * Receive information from the server
	 */
	while (packet = peer->Receive())
	{
		// We'll store our own connection flag because it's faster than ConnectionState
		if (!hasConnection)
			hasConnection = true;

		// Get the packet data into a BitStream
		RakNet::BitStream data(packet->data, packet->length, false);
		
		// Extract the first byte which tells us what kind of packet this is
		const unsigned char raknet_identifier = 0;
		data.Read(raknet_identifier);
		
		switch (packet->data[0])
		{
			case ID_CONNECTION_REQUEST_ACCEPTED: {
				INFORM(NETWORK, "A successful connection to the server has been established.");
			} break;
			case ID_CONNECTION_ATTEMPT_FAILED: {
				INFORM(NETWORK, "A connection to the server could not be established.");
			} break;
			case ID_DISCONNECTION_NOTIFICATION: {
				INFORM(NETWORK, "We have been disconnected from the server.");
			} break;
			case ID_CONNECTION_LOST: {
				INFORM(NETWORK, "We have lost connection to the server.");
			} break;
			case ENTITY_INIT_SUCCESS: {
				INFORM(NETWORK, "Succesfully initiated with server.");
				data.Read(myID);
				initialized = true;
			} break;
			case ENTITY_NOT_INITIALIZED: {
				SendInit();
			} break;
			case ENTITY_NEW_CLIENT_NOTE: {
				PLAYERID playerID;
				char *name;
				Vec3 pos;
				bCQuaternion rot;

				data.Read(playerID);
				data.Read((char *)&name, MAX_NICKNAME_LENGTH);
				data.Read((char *)&pos, sizeof(Vec3));
				data.Read((char *)&rot, sizeof(bCQuaternion));

				if (playerID != myID)
				{
					eCGeometryEntity *newPlayer = npc_manager->SpawnEntity(playerID, "PC_Hero");
					newPlayer->SetWorldPosition(pos);
					newPlayer->SetLocalRotation(rot);
				}
			} break;
			case ENTITY_CLIENT_LEAVING_NOTE: {
				PLAYERID playerID;
				data.Read(playerID);
				npc_manager->KillEntity(playerID);
			} break;
			case ENTITY_POSROT: {
				PLAYERID playerID;
				Vec3 pos;
				bCQuaternion rot;

				data.Read(playerID);
				data.Read((char *)&pos, sizeof(Vec3));
				data.Read((char *)&rot, sizeof(bCQuaternion));

				if (playerID != myID)
				{
					eCGeometryEntity *player = npc_manager->GetNPC(playerID);
					if (player)
					{
						player->SetWorldPosition(pos);
						player->SetLocalRotation(rot);
					}
				}
			} break;
			default: {
				INFORM(NETWORK, "Received some other packet. #fml");
			} break;
		}

		peer->DeallocatePacket(packet);
	}

	/**
	 * Send information to the server
	 */
	if (hasConnection && initialized)
	{
		SendPosRot();
	}
	else {
		SendInit();
	}
}

bool ServerClient::isConnected()
{
	return hasConnection;

	/*
	// Get current connection state
	RakNet::ConnectionState connection_state = peer->GetConnectionState(connectionGUID);
	
	// Return "not connected" only if actually, *completely* disconnected.
	return !(connection_state == RakNet::IS_DISCONNECTED || connection_state == RakNet::IS_NOT_CONNECTED);
	*/
}

ServerClient::~ServerClient()
{
	if (peer) {
		RakNet::RakPeerInterface::DestroyInstance(peer);
		peer = nullptr;
	}
}

void ServerClient::SendInit()
{
	RakNet::BitStream bsOut;
	unsigned char raknet_identifier = ENTITY_INIT;

	Vec3 playerPosition;
	bCQuaternion playerRotation;
	player->GetWorldPosition(playerPosition);
	player->GetLocalRotation(playerRotation);

	bsOut.Write(raknet_identifier);
	bsOut.Write((const char *)&nickname, MAX_NICKNAME_LENGTH);
	bsOut.Write((const char *)&playerPosition, sizeof(Vec3));
	bsOut.Write((const char *)&playerRotation, sizeof(bCQuaternion));

	RakNet::SystemAddress outAddr(SERVER_HOST, SERVER_PORT);
	peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, outAddr, false);
}

void ServerClient::SendPosRot()
{
	RakNet::BitStream bsOut;
	unsigned char raknet_identifier = ENTITY_POSROT;

	Vec3 playerPosition;
	bCQuaternion playerRotation;
	player->GetWorldPosition(playerPosition);
	player->GetLocalRotation(playerRotation);

	bsOut.Write(raknet_identifier);
	bsOut.Write((const char *)&playerPosition, sizeof(Vec3));
	bsOut.Write((const char *)&playerRotation, sizeof(bCQuaternion));

	RakNet::SystemAddress outAddr(SERVER_HOST, SERVER_PORT);
	peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, outAddr, false);
}