#include "Client.h"
#include "hooking/Hooking.h"
#include "tools/address.h"

#include <assert.h>

extern unsigned EngineBase = 0;
extern unsigned GameBase = 0;
extern unsigned ScriptBase = 0;

extern unsigned MainLoopReturn = 0;

extern unsigned SetWorldPositionAddr = 0;
extern unsigned GetWorldPositionAddr = 0;
extern unsigned GetLocalPositionAddr = 0;
extern unsigned SpawnEntityAddr = 0;

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

Client::Client(void)
{
	Client::Instance = this;
	Hooking::Initialize();
	EngineBase = (unsigned)GetModuleHandle("Engine.dll");
	GameBase = (unsigned)GetModuleHandle("Game.dll");
	//ScriptBase = (unsigned)GetModuleHandle("Script.dll");

	SetWorldPositionAddr = EngineBase + 0x002E8700;
	GetWorldPositionAddr = EngineBase + 0x002E77A0;
	GetLocalPositionAddr = EngineBase + 0x002E77D0;
	SpawnEntityAddr = GameBase + 0x002C8AC0;
	
	MainLoopReturn = EngineBase+ 0x00061D86;
	Hooking::JmpHook(EngineBase + 0x00061D80, (DWORD)MainLoopHook);
}

struct Vec3 {
	float x, y, z;
};

struct bCMatrix
{
	float matrix[4][4];

	void Identity(void)
	{
		for (int x = 0; x < 4; ++x) {
			for (int y = 0; y < 4; ++y) {
				matrix[x][y] = 0.0f;
			}
		}

		matrix[0][0] = matrix[1][1] = matrix[2][2] = matrix[3][3] = 1.0f;
	}

	void Transform(const Vec3 &pos)
	{
		matrix[3][0] = pos.x;
		matrix[3][1] = pos.y;
		matrix[3][2] = pos.z;
	}
};

class eCEntity
{
public:
	unsigned vtab;
	unsigned char pad[48];
	bCMatrix matrix;
};

class eCGeometryEntity : public eCEntity {
public:
	void SetWorldPosition(const Vec3 &newPos);
	void GetWorldPosition(Vec3 &position);
	void GetLocalPosition(Vec3 &position);
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

class bCString
{
private:
	struct bCStringData
	{
		unsigned int m_size;
		unsigned int m_unknown;

		bCStringData()
			: m_size(0)
			, m_unknown(0)
		{

		}
	};

	char *m_buffer;
public:
	explicit bCString(const char *buffer)
	{
		const size_t bufferSize = strlen(buffer);

		char *dataBuffer = new char[8 + bufferSize + 1];

		bCStringData *const data = new (dataBuffer) bCStringData;
		data->m_size = static_cast<unsigned>(bufferSize);

		m_buffer = static_cast<char *>(dataBuffer + 8);

		memcpy(m_buffer, buffer, bufferSize);
		m_buffer[bufferSize] = 0;
	}

	bCString(void) = delete;
	bCString(const bCString&) = delete;

	~bCString()
	{
		char *const data = (m_buffer - 8);
		assert(data);
		if (data) {
			delete data;
		}
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

void _declspec(naked) eCGeometryEntity::GetLocalPosition(Vec3 &position)
{
	_asm {
		mov eax, GetLocalPositionAddr
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

void Client::Pulse()
{
	ScriptBase = (unsigned)GetModuleHandle("Script.dll");
	if (!ScriptBase) {
		return;
	}

	static bool postInit = false;
	if (postInit) {
		Hooking::PostInit();
		postInit = true;
	}

	gCSession *const gcsession = *(gCSession **)(ScriptBase + 0x03109A0);

	if (gcsession)
	{
		eCGeometryEntity *const entity = gcsession->GetPlayer();
		
		if (GetAsyncKeyState(VK_F5))
		{
			Vec3 pos;

			entity->GetWorldPosition(pos);
			printf("WorldPos: %f, %f, %f\n", pos.x, pos.y, pos.z);

			entity->GetLocalPosition(pos);
			printf("LocalPos: %f, %f, %f\n", pos.x, pos.y, pos.z);

			printf("----\n");
		}

		if (GetAsyncKeyState(VK_F6))
		{
			Vec3 pos;
			pos.x = entity->matrix.matrix[3][0] + 500;
			pos.y = entity->matrix.matrix[3][1] + 500;
			pos.z = entity->matrix.matrix[3][2] + 500;

			entity->SetWorldPosition(pos);
		}

		/*if (GetAsyncKeyState(VK_F7))
		{
			bCString str("PC_HERO");
			bCMatrix mtx;

			Vec3 playerPos;
			entity->GetLocalPosition(playerPos);
			
			playerPos.x += 50;

			mtx.Identity();
			mtx.Transform(playerPos);
			eCGeometryEntity *bot = static_cast<eCGeometryEntity *>(gcsession->SpawnEntity(str, mtx, true));
			assert(bot);

		}*/
	}
}

Client::~Client(void)
{
	Client::Instance = nullptr;
}