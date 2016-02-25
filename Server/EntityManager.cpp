#include "EntityManager.h"

EntityManager::EntityManager()
{

	for (PLAYERID i = 0; i < SERVER_PLAYER_LIMIT; i++)
		entities[i] = nullptr;
}

bool EntityManager::hasEntity(RakNet::RakNetGUID guid)
{
	for (PLAYERID i = 0; i < SERVER_PLAYER_LIMIT; i++)
	{
		if (entities[i])
		{
			if (entities[i]->guid == guid)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

Entity * EntityManager::CreateEntity()
{
	PLAYERID newID = findEmptyID();
	if (newID == INVALID_PLAYERID)
	{
		INFORM(ERR, "Can not create a player id");
		return nullptr;
	}

	entities[newID] = new Entity();

	return entities[newID];
}

void EntityManager::DestroyEntity(PLAYERID id)
{
	if (entities[id])
	{
		delete entities[id];
		entities[id] = nullptr;
	}
}


Entity * EntityManager::getEntity(PLAYERID id)
{
	if (entities[id])
	{
		return entities[id];
	}

	return nullptr;
}

PLAYERID EntityManager::getEntityID(RakNet::RakNetGUID guid)
{
	for (PLAYERID i = 0; i < SERVER_PLAYER_LIMIT; i++)
	{
		if (entities[i])
		{
			if (entities[i]->guid == guid)
			{
				return i;
			}
		}
	}

	return INVALID_PLAYERID;
}

PLAYERID EntityManager::findEmptyID()
{
	for (PLAYERID i = 0; i < SERVER_PLAYER_LIMIT; i++)
	{
		if (!entities[i])
			return i;
	}

	return INVALID_PLAYERID;
}

EntityManager::~EntityManager()
{
	if (entities)
	{
		for (PLAYERID i = 0; i < SERVER_PLAYER_LIMIT; i++)
		{
			if (entities[i])
			{
				delete entities[i];
				entities[i] = nullptr;
			}
		}

		delete *entities;
		*entities = nullptr;
	}
}