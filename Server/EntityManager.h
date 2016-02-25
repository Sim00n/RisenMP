#pragma once

#include "Entity.h"

const unsigned SERVER_PLAYER_LIMIT = 500;
typedef unsigned PLAYERID;
const PLAYERID INVALID_PLAYERID(~0);

class EntityManager
{
private:
	Entity *entities[SERVER_PLAYER_LIMIT];
	PLAYERID findEmptyID();

public:
	EntityManager();
	~EntityManager();

public:
	Entity *CreateEntity();
	void DestroyEntity(PLAYERID id);

	bool hasEntity(RakNet::RakNetGUID);
	Entity *getEntity(PLAYERID);
	PLAYERID getEntityID(RakNet::RakNetGUID);

};