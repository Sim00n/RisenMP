#include "Entity.h"
#include "tools/address.h"

Entity::Entity(eCEntity &entity)
{
	this->entity = &entity;
}

Entity::~Entity()
{

}