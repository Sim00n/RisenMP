#include <Windows.h>
#include <stdio.h>

#include "Client.h"

Client *g_client = nullptr;

DWORD WINAPI CoregThread(LPVOID lpParam);

BOOL WINAPI DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH: {
			
			DisableThreadLibraryCalls(hinstDLL);
			//MessageBox(NULL, "God damn it", "Dziala DLL", MB_ICONHAND);

			DWORD dwThreadId;
			HANDLE hThread = CreateThread(NULL, 0, CoregThread, NULL, 0, &dwThreadId);
			
		} break;
		case DLL_PROCESS_DETACH: {

			FreeConsole();

			delete g_client;
		} break;
	}

	return TRUE;
}

DWORD WINAPI CoregThread(LPVOID lpParam)
{
	//MessageBox(NULL, "God damn it", "CoregThread dziala", MB_ICONHAND);
	
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);

	printf("Starting client in the new process ...\n\n");
	g_client = new Client();

	return TRUE;
}