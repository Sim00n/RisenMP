#include "RakPeerInterface.h"
#include "BitStream.h"
#include "MessageIdentifiers.h"
#include "RakNetTypes.h"

#include "tools/address.h"
#include "../Shared/tools.h"
#include "game/GUI2.h"
#include <assert.h>
#include <ctime>

#include "mp/ServerClient.h"
#include "mp/NPCManager.h"
#include "hooking/Hooking.h"

#include "game/gCSession.h"
#include "game/eCEntity.h"

#include "Client.h"

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

extern unsigned ContinueAddr = 0;
extern unsigned OpenAlchemyAddr = 0;
extern unsigned OpenAlterAddr = 0;
extern unsigned OpenAnvilAddr = 0;
extern unsigned OpenCookingAddr = 0;
extern unsigned OpenCoolWeaponAddr = 0;
extern unsigned OpenForgeAddr = 0;
extern unsigned OpenFryingAddr = 0;
extern unsigned OpenGoldsmithAddr = 0;
extern unsigned OpenGrindStoneAddr = 0;
extern unsigned OpenLootAddr = 0;
extern unsigned OpenPickpocketAddr = 0;
extern unsigned OpenWriteScrollAddr = 0;


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
	this->initialized = false;
	this->npc_manager = nullptr;
	this->gcsession = nullptr;
	this->player = nullptr;

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

	ContinueAddr = ScriptBase + 0x00038520;
	OpenAlchemyAddr = ScriptBase + 0x00037A20;
	OpenAlterAddr = ScriptBase + 0x00038080;
	OpenAnvilAddr = ScriptBase + 0x00037E40;
	OpenCookingAddr = ScriptBase + 0x00037AD0;
	OpenCoolWeaponAddr = ScriptBase + 0x00037EF0;
	OpenForgeAddr = ScriptBase + 0x00037D90;
	OpenFryingAddr = ScriptBase + 0x00037B80;
	OpenGoldsmithAddr = ScriptBase + 0x00037C30;
	OpenGrindStoneAddr = ScriptBase + 0x00037FA0;
	OpenLootAddr = ScriptBase + 0x00038210;
	OpenPickpocketAddr = ScriptBase + 0x00038160;
	OpenWriteScrollAddr = ScriptBase + 0x00037CE0;

	// Get the game session (important!)
	this->gcsession = *(gCSession **)(ScriptBase + 0x03109A0);
	if (!this->gcsession) {
		return;
	}

	this->npc_manager = new NPCManager(*gcsession);
	this->player = gcsession->GetPlayer();
	this->server = new ServerClient(*npc_manager, *player);
	assert(this->server);

	this->initialized = true;
}

bool test = false;
bool test2 = false;
PLAYERID npcID;
long long elapsedTime = 0;
long long lastTime = clock();
long long sysTime = clock();

void Client::Pulse()
{
	// Initialize  the rest of the DLLs and get gCSession
	// GUI2::Continue will force the user GUI click on "Continue" button
	if (!this->initialized) {
		this->Initialize();
		GUI2::Continue();
	}

	if (gcsession)
	{
		if (!test2) {
			this->server->Connect();
			test2 = true;
		}

		sysTime = clock();
		elapsedTime += (sysTime - lastTime);
		//printf("Elapsed: %lld, system: %lld, last: %lld\n", elapsedTime, sysTime, lastTime);
		if (elapsedTime/CLOCKS_PER_SEC > .016)
		{
			server->Pulse();
			elapsedTime = 0;
		}
		lastTime = sysTime;
		
		/*if (GetAsyncKeyState(VK_F4))
		{
			static eCEntityPropertySet* propertySet = player->GetPropertySet(bCString("gCSkills_PS"));
			assert(propertySet);
			static gCSkills_PS* skills = static_cast<gCSkills_PS*>(propertySet);
			assert(skills);
			if (skills)
				printf("Skill %d Value %d\n", 0, skills->GetSkillValue(0));
		}*/

		/*if (GetAsyncKeyState(VK_F3))
		{
			static eCEntityPropertySet* propertySet = player->GetPropertySet(bCString("gCSkills_PS"));
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

	if (this->server) {
		delete this->server;
		this->server = nullptr;
	}

	if (this->player) {
		delete this->player;
		this->player = nullptr;
	}

	Client::Instance = nullptr;
}