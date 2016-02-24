#pragma once

#include "game\gCSession.h"

class NPCManager;

class Client
{
private:
	bool initialized;
	NPCManager *npc_manager;

public:
	Client(void);
	void Initialize();
	void Pulse();
	~Client(void);

	static Client *Instance;
	gCSession *gcsession;
};