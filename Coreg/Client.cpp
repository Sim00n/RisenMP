#include "RakPeerInterface.h"

#include "tools\address.h"
#include "tools\tools.h"

#include "mp/NPCManager.h"

#include "Client.h"
#include "hooking\Hooking.h"

#include "game\gCSession.h"
#include "game\eCEntity.h"

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

static RakNet::RakPeerInterface *g_rakPeer = nullptr;

Client::Client(void)
{
	g_rakPeer = RakNet::RakPeerInterface::GetInstance();

	g_rakPeer->SetIncomingPassword("QWERTY", 7);


	this->initialized = false;
	this->npc_manager = nullptr;
	this->gcsession = nullptr;

	Client::Instance = this;
	Hooking::Initialize();
	EngineBase = (unsigned)GetModuleHandle("Engine.dll");
	GameBase = (unsigned)GetModuleHandle("Game.dll");

	// Main Loop
	MainLoopReturn = EngineBase+ 0x00061D86;
	
	// eCEntity
	SetWorldPositionAddr = EngineBase + 0x002E8700;
	GetWorldPositionAddr = EngineBase + 0x002E77A0;
	SetLocalRotationAddr = EngineBase + 0x002E78C0;
	GetLocalRotationAddr = EngineBase + 0x002E7860;
	SpawnEntityAddr = GameBase + 0x002C8AC0;
	KillAddr = EngineBase + 0x002DF210;
	
	// gCSkills_PS
	GetSkillValueAddr = GameBase + 0x002D8D60;
	SetSkillValueAddr = GameBase + 0x002D8A70;
	GetPropertySetAddr = EngineBase + 0x002E1160;
	GetSkillMaxValueAddr = GameBase + 0x002D8FF0;
	GetSkillBaseValueAddr = GameBase + 0x002D8A50;	

	// Hooks
	Hooking::JmpHook(EngineBase + 0x00061D80, (DWORD)MainLoopHook);
}

void Client::Initialize()
{
	// Loading Script.dll here instead of the Client's constructor because Pirania
	const bool hadScript = ScriptBase != 0;
	if (!hadScript) {
		ScriptBase = (unsigned)GetModuleHandle("Script.dll");
		if (!ScriptBase) {
			return;
		}
		Hooking::PostInit();
	}

	// Get the game session (important!)
	this->gcsession = *(gCSession **)(ScriptBase + 0x03109A0);
	if (!this->gcsession) {
		return;
	}

	this->npc_manager = new NPCManager(*gcsession);
	this->initialized = true;
}

bool test = false;
NPCID npcID;

void Client::Pulse()
{
	// Initialize  the rest of the DLLs and get gCSession
	if(!this->initialized)
		this->Initialize();

	if (gcsession)
	{
		eCGeometryEntity *const entity = gcsession->GetPlayer();

		if (GetAsyncKeyState(VK_F3) && !test)
		{
			Vec3 playerPos;
			entity->GetWorldPosition(playerPos);
			npcID = npc_manager->SpawnEntity("Titan", playerPos);
			DEBUG_M("Created NPC with ID: %d\n", npcID);
			test = true;
		}

		if (GetAsyncKeyState(VK_F4) && test)
		{
			npc_manager->KillEntity(npcID);
			DEBUG_M("Deleted NPC with ID: %d\n", npcID);
			npcID = INVALID_NPC_ID;
			test = false;
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

		/*if (GetAsyncKeyState(VK_F3))
		{
			static eCEntityPropertySet* propertySet = entity->GetPropertySet(bCString("gCSkills_PS"));
			static gCSkills_PS* skills = static_cast<gCSkills_PS*>(propertySet);

			if (skills) {
				printf("HP1: %d, HP2: %d\n", skills->GetSkillValue(gCSkills_PS::ATRIB_HP), skills->GetSkillValue(gCSkills_PS::STAT_HP));
				
				if (GetAsyncKeyState(VK_F4)) {
					skills->SetSkillValue(gCSkills_PS::ATRIB_HP, 50);
					skills->SetSkillValue(gCSkills_PS::STAT_HP, 100);
				}

				printf("HP1: %d, HP2: %d\n", skills->GetSkillValue(gCSkills_PS::ATRIB_HP), skills->GetSkillValue(gCSkills_PS::STAT_HP));
				printf("----\n");
			}
		}*/
	}
}

Client::~Client(void)
{
	if (this->npc_manager) {
		delete this->npc_manager;
		this->npc_manager = nullptr;
	}

	if (g_rakPeer) {
		RakNet::RakPeerInterface::DestroyInstance(g_rakPeer);
		g_rakPeer = nullptr;
	}

	Client::Instance = nullptr;
}