#include "GUI2.h"
#include "tools/address.h"

void _declspec(naked) GUI2::Continue()
{
	_asm {
		mov eax, ContinueAddr
		jmp eax
	}
}

void _declspec(naked) GUI2::OpenAlchemy()
{
	_asm {
		mov eax, OpenAlchemyAddr
		jmp eax
	}
}

void _declspec(naked) GUI2::OpenAlter()
{
	_asm {
		mov eax, OpenAlterAddr
		jmp eax
	}
}

void _declspec(naked) GUI2::OpenAnvil()
{
	_asm {
		mov eax, OpenAnvilAddr
		jmp eax
	}
}

void _declspec(naked) GUI2::OpenCooking()
{
	_asm {
		mov eax, OpenCookingAddr
		jmp eax
	}
}

void _declspec(naked) GUI2::OpenCoolWeapon()
{
	_asm {
		mov eax, OpenCoolWeaponAddr
		jmp eax
	}
}

void _declspec(naked) GUI2::OpenForge()
{
	_asm {
		mov eax, OpenForgeAddr
		jmp eax
	}
}

void _declspec(naked) GUI2::OpenFrying()
{
	_asm {
		mov eax, OpenFryingAddr
		jmp eax
	}
}

void _declspec(naked) GUI2::OpenGoldsmith()
{
	_asm {
		mov eax, OpenGoldsmithAddr
		jmp eax
	}
}

void _declspec(naked) GUI2::OpenGrindStone()
{
	_asm {
		mov eax, OpenGrindStoneAddr
		jmp eax
	}
}

void _declspec(naked) GUI2::OpenLoot()
{
	_asm {
		mov eax, OpenLootAddr
		jmp eax
	}
}

void _declspec(naked) GUI2::OpenPickpocket()
{
	_asm {
		mov eax, OpenPickpocketAddr
		jmp eax
	}
}

void _declspec(naked) GUI2::OpenWriteScroll()
{
	_asm {
		mov eax, OpenWriteScrollAddr
		jmp eax
	}
}