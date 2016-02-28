#pragma once

#include "../Shared/tools.h"
#include "game/gCSession.h"
#include "game/eCEntity.h"
#include <vector>

//static const PLAYERID MAX_NPC_COUNT(~0 - 1);
static const PLAYERID MAX_NPC_COUNT = 100;

class NPCManager
{
private:
	gCSession *gcsession;
	eCGeometryEntity *entities[MAX_NPC_COUNT];
public:
	NPCManager(gCSession &gcsesson);
	~NPCManager();

	eCGeometryEntity * SpawnEntity(PLAYERID playerID, char *name, Vec3 pos);
	eCGeometryEntity * SpawnEntity(PLAYERID playerID, char *name);
	eCGeometryEntity * SpawnEntity(PLAYERID playerID);
	void KillEntity(PLAYERID id);
	eCGeometryEntity * GetNPC(PLAYERID id);

	bool isSpawned(PLAYERID playerID);
};