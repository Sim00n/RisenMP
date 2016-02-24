#pragma once

#include "tools\tools.h"
#include "game\gCSession.h"
#include "game\eCEntity.h"
#include <vector>

typedef unsigned NPCID;

static const NPCID INVALID_NPC_ID(~0);
//static const NPCID MAX_NPC_COUNT(~0 - 1);
static const NPCID MAX_NPC_COUNT = 100;

class NPCManager
{
private:
	gCSession *gcsession;
	eCGeometryEntity *entities[MAX_NPC_COUNT];
	NPCID findEmptyIndex();
public:
	NPCManager(gCSession &gcsesson);
	~NPCManager();

	NPCID SpawnEntity(char *name, Vec3 pos);
	NPCID SpawnEntity(char *name);
	NPCID SpawnEntity();
	void KillEntity(NPCID id);
	eCGeometryEntity * GetNPC(NPCID id);
};