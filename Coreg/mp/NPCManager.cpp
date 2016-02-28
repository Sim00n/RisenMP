#include <assert.h>
#include "mp/NPCManager.h"

NPCManager::NPCManager(gCSession &gcsession)
{
	this->gcsession = &gcsession;

	// Initialize entities array
	for (PLAYERID i = 0; i < MAX_NPC_COUNT; i++) {
		entities[i] = nullptr;
	}
}

NPCManager::~NPCManager() {}

eCGeometryEntity * NPCManager::SpawnEntity(PLAYERID playerID, char *name, Vec3 pos)
{
	if (playerID == INVALID_PLAYERID)
	{
		throw "Reached the limit for NPCs. This must be a memory leak. Pls fix.";
		return nullptr;
	}

	bCString npcName(name);
	bCMatrix posMtx;

	posMtx.Identity();
	posMtx.Transform(pos);

	entities[playerID] = static_cast<eCGeometryEntity *>(this->gcsession->SpawnEntity(npcName, posMtx, false));
	return entities[playerID];
}

eCGeometryEntity * NPCManager::SpawnEntity(PLAYERID playerID, char *name)
{
	Vec3 pos(0.0f, 0.0f, 0.0f);
	return this->SpawnEntity(playerID, name, pos);
}

eCGeometryEntity * NPCManager::SpawnEntity(PLAYERID playerID)
{
	char *name = "UnknownNPC";
	Vec3 pos(0.0f, 0.0f, 0.0f);
	return this->SpawnEntity(playerID, name, pos);
}

void NPCManager::KillEntity(PLAYERID id)
{
	if (entities[id]) {
		entities[id]->Kill();
		entities[id] = nullptr;
	}
}

bool NPCManager::isSpawned(PLAYERID playerID)
{
	return entities[playerID] != nullptr;
}

eCGeometryEntity * NPCManager::GetNPC(PLAYERID id)
{
	return entities[id];
}