#include <assert.h>
#include "mp\NPCManager.h"

NPCManager::NPCManager(gCSession &gcsession)
{
	this->gcsession = &gcsession;

	// Initialize entities array
	for (NPCID i = 0; i < MAX_NPC_COUNT; i++) {
		entities[i] = nullptr;
	}
}

NPCManager::~NPCManager() {}

NPCID NPCManager::SpawnEntity(char *name, Vec3 pos)
{
	NPCID newID = findEmptyIndex();
	if (newID == INVALID_NPC_ID)
	{
		throw "Reached the limit for NPCs. This must be a memory leak. Pls fix.";
		return newID;
	}

	bCString npcName(name);
	bCMatrix posMtx;

	posMtx.Identity();
	posMtx.Transform(pos);

	entities[newID] = static_cast<eCGeometryEntity *>(this->gcsession->SpawnEntity(npcName, posMtx, false));
	return newID;
}

NPCID NPCManager::SpawnEntity(char *name)
{
	Vec3 pos(0.0f, 0.0f, 0.0f);
	return this->SpawnEntity(name, pos);
}

NPCID NPCManager::SpawnEntity()
{
	char *name = "UnknownNPC";
	Vec3 pos(0.0f, 0.0f, 0.0f);
	return this->SpawnEntity(name, pos);
}

void NPCManager::KillEntity(NPCID id)
{
	if (entities[id]) {
		entities[id]->Kill();
		entities[id] = nullptr;
	}
}

eCGeometryEntity * NPCManager::GetNPC(NPCID id)
{
	return entities[id];
}

NPCID NPCManager::findEmptyIndex()
{
	for (NPCID i = 0; i < MAX_NPC_COUNT; i++) {
		if (entities[i] == nullptr)
			return i;
	}
	
	return INVALID_NPC_ID;
}