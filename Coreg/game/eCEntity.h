#pragma once

#include "../Shared/tools.h"
#include "gCSkills_PS.h"

class eCEntity
{
public:
	unsigned vtab;
	unsigned char pad[48];
	bCMatrix matrix;
	gCSkills_PS *skills = nullptr;

	eCEntityPropertySet *GetPropertySet(bCString const &str);
	
	void Initialize();
	void Kill();
};

class eCGeometryEntity : public eCEntity {
public:
	void SetWorldPosition(const Vec3 &newPos);
	void GetWorldPosition(Vec3 &position);
	void SetLocalRotation(const bCQuaternion &newRotation);
	void GetLocalRotation(bCQuaternion &rotation);
};
class eCDynamicEntity : public eCGeometryEntity {};

class eCEntityProxy
{
private:
	struct envlope_level_1
	{
		eCEntity * entity;
	};

	unsigned vtable;
	envlope_level_1 *en;

public:
	eCEntity *GetEntity() const
	{
		if (!en) {
			return NULL;
		}
		return en->entity;
	}
};