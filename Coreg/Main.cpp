#include <Windows.h>
#include <stdio.h>

#include "Client.h"

Client *g_client = nullptr;

BOOL WINAPI DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH: {
			
			DisableThreadLibraryCalls(hinstDLL);
			MessageBox(NULL, "Dziala DLL", "God damn it", MB_ICONHAND);

			AllocConsole();
			freopen("CONOUT$", "w", stdout);
			freopen("CONIN$", "r", stdin);

			g_client = new Client();
		} break;
		case DLL_PROCESS_DETACH: {

			FreeConsole();

			delete g_client;
		} break;
	}

	return TRUE;
}