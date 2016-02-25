#pragma once

class NPCManager;
class ServerClient;
class gCSession;
class eCGeometryEntity;

class Client
{
private:
	bool initialized;
	NPCManager *npc_manager;
	ServerClient *server;

public:
	Client(void);
	void Initialize();
	void Pulse();
	~Client(void);

	static Client *Instance;
	gCSession *gcsession;
	eCGeometryEntity *player;
};