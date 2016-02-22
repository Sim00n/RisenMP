#include "Client.h"
#include "hooking/Hooking.h"
#include "tools/address.h"
#include <cmath>

#include <assert.h>

extern unsigned EngineBase = 0;
extern unsigned GameBase = 0;
extern unsigned ScriptBase = 0;

extern unsigned MainLoopReturn = 0;

extern unsigned SetWorldPositionAddr = 0;
extern unsigned GetWorldPositionAddr = 0;
extern unsigned SetLocalRotationAddr = 0;
extern unsigned GetLocalRotationAddr = 0;
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

	// Functions
	SetWorldPositionAddr = EngineBase + 0x002E8700;
	GetWorldPositionAddr = EngineBase + 0x002E77A0;
	SetLocalRotationAddr = EngineBase + 0x002E78C0;
	GetLocalRotationAddr = EngineBase + 0x002E7860;
	
	// Class instances
	SpawnEntityAddr = GameBase + 0x002C8AC0;
	
	// Return Addresses
	MainLoopReturn = EngineBase+ 0x00061D86;

	// Hooks
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

struct bCQuaternion {
	float x, y, z, theta;
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

/**
 * @brief Implementation of bCString class based on Genome engine implementation.
 *
 * This class is located in SharedBase.dll
 */
class bCString
{
private:
	/// Structure holding string data.
	struct bCStringData
	{
		unsigned m_size; // Amount of the stored characters (without null terminator)
		unsigned m_refs; // Reference count of this data. (Probably: bCAtomic<unsigned> m_refs)

		/// Constructor.
		bCStringData() : m_size(0), m_refs(0) {}
	};

	/// The string buffer.
	char *m_buffer;
public:
	/// Explict constructor creating string from standard char array.
	explicit bCString(const char *const buffer)
	{
	    assert(buffer);

		const size_t bufferSize = strlen(buffer);

		char *const dataBuffer = new char[8 + bufferSize + 1];

		bCStringData *const data = new (dataBuffer) bCStringData;
		
		// As we are running only in one thread for now we can simply set
		// reference count here to 1 without need of doing atomic operation.
		data->m_refs = 1;

		data->m_size = static_cast<unsigned>(bufferSize);

		m_buffer = static_cast<char *>(dataBuffer + 8);

		memcpy(m_buffer, buffer, bufferSize);
		m_buffer[bufferSize] = 0;
	}

	/**
	 * @name Set of the methods that are removed for now as we do not handle them correctly.
	 *
	 * @todo: Implement this set of methods.
	 */
	//@{
	bCString(void) = delete;
	bCString(const bCString&) = delete;
	bCString(bCString&&) = delete;

	void operator=(const char *) = delete;
	void operator=(const bCString&) = delete;
	void operator=(bCString&&) = delete;
	//@}

	/// Destructor.
	~bCString()
	{
		if (!m_buffer) {
			return;
		}

		char *const data = (m_buffer - 8);
		if (data) {
			delete []data;
		}
		m_buffer = nullptr;
	}
};

class gCSession
{
public:
	unsigned char pad[40];
	eCEntityProxy player;
	eCEntityProxy originalPlayer;

	eCEntity * SpawnEntity(const bCString &model, const bCMatrix &spawnLocation, const bool unkBool, bCMatrix &secondMatrix, char unkChar, float unkFloat);

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

eCEntity _declspec(naked) * gCSession::SpawnEntity(const bCString &model, const bCMatrix &spawnLocation, const bool unkBool, bCMatrix &secondMatrix, char unkChar, float unkFloat)
{
	_asm {
		mov eax, SpawnEntityAddr
		jmp eax
	}
}

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
		
		if (GetAsyncKeyState(VK_F5))
		{
			Vec3 pos;
			entity->GetWorldPosition(pos);
			printf("WorldPos: %f, %f, %f\n", pos.x, pos.y, pos.z);
		}

		if (GetAsyncKeyState(VK_F6))
		{
			Vec3 pos;
			pos.x = entity->matrix.matrix[3][0] + 500;
			pos.y = entity->matrix.matrix[3][1] + 500;
			pos.z = entity->matrix.matrix[3][2] + 500;

			entity->SetWorldPosition(pos);
		}

		if (GetAsyncKeyState(VK_F7))
		{
			bCQuaternion rotation;
			entity->GetLocalRotation(rotation);
			printf("Rotation: %f, %f, %f, %f\n", rotation.x, rotation.y, rotation.z, rotation.theta);
			printf("-----\n");

		}

		if (GetAsyncKeyState(VK_F4))
		{
			bCQuaternion rotation;
			entity->GetLocalRotation(rotation);
			rotation.x = 0.0;
			rotation.y = 0.0;
			rotation.z = 0.0;
			rotation.theta = 1.0;
			entity->SetLocalRotation(rotation);
			printf("Rotation set.\n");
		}

		/*if (GetAsyncKeyState(VK_F7))
		{
			bCString str("PC_HERO");
			bCMatrix mtx, mtx2;

			char testChar = '0';
			float testFloat = 0.0;

			Vec3 playerPos;
			entity->GetWorldPosition(playerPos);
			
			playerPos.x += 50;

			mtx.Identity();
			mtx2.Identity();
			mtx.Transform(playerPos);
			mtx2.Transform(playerPos);
			eCGeometryEntity *bot = static_cast<eCGeometryEntity *>(gcsession->SpawnEntity(str, mtx, false, mtx2, testChar, testFloat));
			//assert(bot);

		}*/
	}
}

Client::~Client(void)
{
	Client::Instance = nullptr;
}