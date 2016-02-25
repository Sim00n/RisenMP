#pragma once

#include "RakNetTypes.h"
#include "../Shared/tools.h"

class Entity
{
public:
	RakNet::RakNetGUID guid;

	char nickname[MAX_NICKNAME_LENGTH];
	const char *ipaddress;
	
	Vec3 pos;
	bCQuaternion rot;
	
	bool initialized;

public:
	Entity();
	~Entity();

	bool isComplete();
	void printDebug();
	
};