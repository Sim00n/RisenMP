#include "tools/address.h"
#include "eCEntity.h"
#include "gCSession.h"

void eCEntity::Initialize() {
	this->skills = static_cast<gCSkills_PS*>(this->GetPropertySet(bCString("gCSkills_PS")));
};

void _declspec(naked) eCEntity::Kill()
{
	_asm {
		mov eax, KillAddr
		jmp eax
	}
}

void _declspec(naked) eCGeometryEntity::SetWorldPosition(const Vec3 &newPos)
{
	_asm {
		mov eax, SetWorldPositionAddr
		jmp eax
	}
}

void _declspec(naked) eCGeometryEntity::GetWorldPosition(Vec3 &position)
{
	_asm {
		mov eax, GetWorldPositionAddr
		jmp eax
	}
}

void _declspec(naked) eCGeometryEntity::SetLocalRotation(const bCQuaternion &newRotation)
{
	_asm {
		mov eax, SetLocalRotationAddr
		jmp eax
	}
}

void _declspec(naked) eCGeometryEntity::GetLocalRotation(bCQuaternion &rotation)
{
	_asm {
		mov eax, GetLocalRotationAddr
		jmp eax
	}
}

eCEntity _declspec(naked) * gCSession::SpawnEntity(const bCString &model, const bCMatrix &spawnLocation, const bool unkBool)
{
	_asm {
		mov eax, SpawnEntityAddr
		jmp eax
	}
}

eCEntityPropertySet _declspec(naked) * eCEntity::GetPropertySet(bCString const &str)
{
	_asm {
		mov eax, GetPropertySetAddr
			jmp eax
	}
}