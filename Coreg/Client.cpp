#include "Client.h"
#include "hooking/Hooking.h"
#include "tools/address.h"
#include "tools/tools.h"
#include <assert.h>

extern unsigned EngineBase = 0;
extern unsigned GameBase = 0;
extern unsigned ScriptBase = 0;

extern unsigned MainLoopReturn = 0;
extern unsigned GetSkillValueReturn = 0;

extern unsigned SetWorldPositionAddr = 0;
extern unsigned GetWorldPositionAddr = 0;
extern unsigned SetLocalRotationAddr = 0;
extern unsigned GetLocalRotationAddr = 0;
extern unsigned SpawnEntityAddr = 0;
extern unsigned KillAddr = 0;
extern unsigned GetSkillValueAddr = 0;
extern unsigned SetSkillValueAddr = 0;
extern unsigned GetPropertySetAddr = 0;
extern unsigned GetSkillMaxValueAddr = 0;
extern unsigned GetSkillBaseValueAddr = 0;

Client *Client::Instance = nullptr;

static void MainLoopListener()
{
	if (Client::Instance) {
		Client::Instance->Pulse();
	}
}

void _declspec(naked) MainLoopHook()
{
	_asm
	{
		pushad
			call MainLoopListener
		popad

		xor		eax, eax
		cmp		[esp+0x10], al
		jmp		MainLoopReturn
	}
}

static void GetSkillValueListener(int test)
{
	if(GetAsyncKeyState(VK_F2))
		printf("GetSkillValue: d: %d, h: %x\n", test, test);
}

void _declspec(naked) GetSkillValueHook()
{
	_asm {
		pushad
			push [esp + 0x20 + 8]
			call GetSkillValueListener
			add esp, 0x04
		popad

		push edi
		mov edi, [esp + 0x0C]
		push edi
		jmp GetSkillValueReturn;

		//.text:202D8D60                 push    esi
		//.text : 202D8D61                 push    edi
		//.text : 202D8D62                 mov     edi, [esp + 0Ch]
		//.text : 202D8D66                 push    edi
	}
}

Client::Client(void)
{
	Client::Instance = this;
	Hooking::Initialize();
	EngineBase = (unsigned)GetModuleHandle("Engine.dll");
	GameBase = (unsigned)GetModuleHandle("Game.dll");

	// Functions
	SetWorldPositionAddr = EngineBase + 0x002E8700;
	GetWorldPositionAddr = EngineBase + 0x002E77A0;
	SetLocalRotationAddr = EngineBase + 0x002E78C0;
	GetLocalRotationAddr = EngineBase + 0x002E7860;
	KillAddr = EngineBase + 0x002DF210;
	GetSkillValueAddr = GameBase + 0x002D8D60;
	SetSkillValueAddr = GameBase + 0x002D8A70;
	GetPropertySetAddr = EngineBase + 0x002E1160;
	GetSkillMaxValueAddr = GameBase + 0x002D8FF0;
	GetSkillBaseValueAddr = GameBase + 0x002D8A50;
	
	// Class instances
	SpawnEntityAddr = GameBase + 0x002C8AC0;
	
	// Return Addresses
	MainLoopReturn = EngineBase+ 0x00061D86;
	GetSkillValueReturn = GameBase + 0x002D8D67;

	// Hooks
	Hooking::JmpHook(EngineBase + 0x00061D80, (DWORD)MainLoopHook);
	Hooking::JmpHook(GameBase + 0x002D8D61, (DWORD)GetSkillValueHook);
}

class eCEntityPropertySet 
{
public:
	virtual void IsCopyable(void) = 0;
};

class eCEntity
{
public:
	unsigned vtab;
	unsigned char pad[48];
	bCMatrix matrix;

	void Kill();
	eCEntityPropertySet *GetPropertySet(bCString const &str);

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

class gCSession
{
public:
	unsigned char pad[40];
	eCEntityProxy player;
	eCEntityProxy originalPlayer;

	eCEntity * SpawnEntity(const bCString &model, const bCMatrix &spawnLocation, const bool unkBool);

	eCDynamicEntity *GetPlayer(void) const
	{
		eCEntity *const entity = player.GetEntity();
		if (!entity) {
			return NULL;
		}
		return static_cast<eCDynamicEntity *>(entity);
	}
};

class gCSkills_PS : public eCEntityPropertySet
{
public:
	unsigned char pad[72];
	long GetSkillValue(const int &unkInt);
	void SetSkillValue(int unkInt, long value);
	long GetSkillMaxValue(const int &test);
	int GetSkillBaseValue(const int &test);
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

long _declspec(naked) gCSkills_PS::GetSkillValue(const int &unkInt) {
	_asm {
		mov eax, GetSkillValueAddr
		jmp eax
	}
}

void _declspec(naked) gCSkills_PS::SetSkillValue(int unkInt, long value)
{
	_asm {
		mov eax, SetSkillValueAddr
		jmp eax
	}
}

long _declspec(naked) gCSkills_PS::GetSkillMaxValue(const int &unkInt)
{
	_asm {
		mov eax, GetSkillMaxValueAddr
		jmp eax
	}
}

int _declspec(naked) gCSkills_PS::GetSkillBaseValue(const int &unkInt)
{
	_asm {
		mov eax, GetSkillBaseValueAddr
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

bool test = false;

void Client::Pulse()
{
	// Loading Script.dll here instead of the Client's constructor because Pirania
	ScriptBase = (unsigned)GetModuleHandle("Script.dll");
	if (!ScriptBase) {
		return;
	}

	// Init hooking for Script.dll
	static bool postInit = false;
	if (postInit) {
		Hooking::PostInit();
		postInit = true;
	}

	gCSession *const gcsession = *(gCSession **)(ScriptBase + 0x03109A0);

	if (gcsession)
	{
		eCGeometryEntity *const entity = gcsession->GetPlayer();
		static eCGeometryEntity *bot = nullptr;

		

		if (!test) {
			bCString str("PC_Hero");
			bCMatrix mtx;
			Vec3 playerPos;

			entity->GetWorldPosition(playerPos);
			playerPos.x += 50;

			mtx.Identity();
			mtx.Transform(playerPos);

			bot = static_cast<eCGeometryEntity *>(gcsession->SpawnEntity(str, mtx, false));
			test = true;
		}

		/*if (GetAsyncKeyState(VK_F4))
		{
			static eCEntityPropertySet* propertySet = entity->GetPropertySet(bCString("gCSkills_PS"));
			assert(propertySet);
			static gCSkills_PS* skills = static_cast<gCSkills_PS*>(propertySet);
			assert(skills);
			if (skills)
				printf("Skill %d Value %d\n", 0, skills->GetSkillValue(0));
		}*/

		if (GetAsyncKeyState(VK_F3))
		{
			static eCEntityPropertySet* propertySet = entity->GetPropertySet(bCString("gCSkills_PS"));
			assert(propertySet);
			static gCSkills_PS* skills = static_cast<gCSkills_PS*>(propertySet);
			assert(skills);
			
			if (skills) {
				printf("Skill %d Value %d\n", 0, skills->GetSkillValue(0));
				printf("Skill %d Value %d\n", 1, skills->GetSkillValue(1));
				printf("Skill %d Value %d\n", 2, skills->GetSkillValue(2));
				printf("Skill %d Value %d\n", 3, skills->GetSkillValue(3));
				printf("Skill %d Value %d\n", 4, skills->GetSkillValue(4));
				printf("Skill %d Value %d\n", 5, skills->GetSkillValue(5));
				printf("Skill %d Value %d\n", 6, skills->GetSkillValue(6));
				printf("----\n");
			}
		}
	}
}

Client::~Client(void)
{
	Client::Instance = nullptr;
}