#include "Entity.h"

Entity::Entity()
{
	guid = RakNet::UNASSIGNED_RAKNET_GUID;
	
	for (int i = 0; i < MAX_NICKNAME_LENGTH; i++)
		nickname[i] = '\0';
	
	ipaddress = nullptr;
	
	pos = Vec3(0.0f, 0.0f, 0.0f);
	rot = bCQuaternion(0.0f, 0.0f, 0.0f, 0.0f);

	initialized = false;
}

bool Entity::isComplete()
{
	// This will need more code as we require the client to send more information.

	if (!initialized)
		return FALSE;

	if (sizeof(nickname) <= 0)
		return FALSE;

	if (guid.size <= 0)
		return FALSE;
	
	return TRUE;
}

void Entity::printDebug()
{
	INFORMS("[DEBUG] %s raportuje pozycje: %f, %f, %f", ipaddress, pos.x, pos.y, pos.z);
	INFORMS("[DEBUG] %s raportuje rotacje: %f, %f, %f, %f", ipaddress, rot.x, rot.y, rot.z, rot.theta);
}

Entity::~Entity()
{
	if (ipaddress)
	{
		delete ipaddress;
		ipaddress = nullptr;
	}
}