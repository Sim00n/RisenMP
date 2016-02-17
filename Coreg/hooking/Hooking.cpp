/////////////////////////////////////////////
//
//	    Mafia 2 Online @ Hooking.cpp
//        Project: core (Client)
//
//////////////////////////////////////////////

#include <Windows.h>
#include <stdio.h>

#include "Hooking.h"

namespace Hooking
{
void Hook(DWORD dwAddress, DWORD dwHookAddress, unsigned char ucOpcode)
{
	// OP {AA DD RR SS} XX
	// 0x44AA6699
	// 99 66 AA 44
	*(BYTE *)dwAddress = ucOpcode;
	*(DWORD*)(dwAddress + 1) = dwHookAddress - (dwAddress + 5);
}

void JmpHook(DWORD dwAddress, DWORD dwHookAddress)
{
	Hook(dwAddress,dwHookAddress,0xE9);
}

void CallHook(DWORD dwAddress, DWORD dwHookAddress)
{
	Hook(dwAddress,dwHookAddress,0xE8);
}

void MethodHook(DWORD dwAddress, DWORD dwHookAddress)
{
	*(DWORD *)dwAddress = dwHookAddress;
}

static void UnprotectSegmentsFor(const char *moduleName)
{
	// Prepare headers
	PBYTE pbImageBase = (PBYTE)GetModuleHandle(moduleName);
	if (!pbImageBase) {
		printf("UPS (%s)\n", moduleName);
		return;
	}
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pbImageBase;
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)(pbImageBase + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNtHeaders);

	// Loop thought all sections
	for (int iSection = 0; iSection < pNtHeaders->FileHeader.NumberOfSections; iSection++, pSection++)
	{
		char * szSectionName = (char*)pSection->Name;
		if (!strcmp(szSectionName, ".text") || !strcmp(szSectionName, ".rdata"))
		{
			// Unprotect segment
			DWORD dwOld = 0;//Temp variable
			VirtualProtect((void *)(pbImageBase + pSection->VirtualAddress), ((pSection->Misc.VirtualSize + 4095)&~4095), PAGE_EXECUTE_READWRITE, &dwOld);
		}
	}
}

void Initialize(void)
{
	UnprotectSegmentsFor(NULL);
	UnprotectSegmentsFor("Engine.dll");
	UnprotectSegmentsFor("Game.dll");
}

void PostInit(void)
{
	UnprotectSegmentsFor("Script.dll");
}
};
