#pragma once

#include "tools/tools.h"
#include "eCEntity.h"

class gCSession
{
public:
	unsigned char pad[40];
	eCEntityProxy player;
	eCEntityProxy originalPlayer;

	eCEntity * SpawnEntity(const bCString &model, const bCMatrix &spawnLocation, const bool unkBool);

	eCDynamicEntity *GetPlayer(void) const
	{
		eCEntity *const entity = player.GetEntity();
		if (!entity) {
			return NULL;
		}
		return static_cast<eCDynamicEntity *>(entity);
	}
};