#pragma once

#include "eCEntity.h"

class Entity {
public:
	eCEntity *entity;

	Entity(eCEntity &entity);
	~Entity();
};