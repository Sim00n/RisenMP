#pragma once

#include "Entity.h"

const unsigned SERVER_PLAYER_LIMIT = 500;
typedef unsigned PLAYERID;

class EntityManager
{
public:
	Entity *entities[SERVER_PLAYER_LIMIT];
private:
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