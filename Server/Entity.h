#pragma once

#include "RakNetTypes.h"
#include "../Shared/tools.h"

class Entity
{
public:
	RakNet::RakNetGUID guid;
	PLAYERID systemID;

	char nickname[MAX_NICKNAME_LENGTH];
	RakNet::SystemAddress ipaddress;
	
	Vec3 pos;
	bCQuaternion rot;
	
	bool initialized;

public:
	Entity();
	~Entity();

	bool isComplete();
	void printDebug();
	
};