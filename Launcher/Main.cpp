#include <Windows.h>
#include <stdio.h>
#include "sdk/SteamAPI/steam_api.h"
#include "Shared/Strings.h"

bool InjectDLL(const HANDLE process, const char *const dllPath)
{
	const size_t libPathLen = strlen(dllPath) + 1;
	SIZE_T bytesWritten = 0;

	void *const remoteLibPath = VirtualAllocEx(process, NULL, libPathLen, MEM_COMMIT, PAGE_READWRITE);
	if (!remoteLibPath)
	{
		return false;
	}

	if (!WriteProcessMemory(process, remoteLibPath, dllPath, libPathLen, &bytesWritten))
	{
		VirtualFreeEx(process, remoteLibPath, sizeof(remoteLibPath), MEM_RELEASE);
		return false;
	}

	const HMODULE kernel32dll = GetModuleHandle("Kernel32");
	if (!kernel32dll)
	{
		VirtualFreeEx(process, remoteLibPath, sizeof(remoteLibPath), MEM_RELEASE);
		return false;
	}

	const FARPROC pfnLoadLibraryA = GetProcAddress(kernel32dll, "LoadLibraryA");
	if (!pfnLoadLibraryA)
	{
		FreeModule(kernel32dll);
		VirtualFreeEx(process, remoteLibPath, sizeof(remoteLibPath), MEM_RELEASE);
		return false;
	}

	const HANDLE hThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)pfnLoadLibraryA, remoteLibPath, 0, NULL);
	if (!hThread)
	{
		FreeModule(kernel32dll);
		VirtualFreeEx(process, remoteLibPath, sizeof(remoteLibPath), MEM_RELEASE);
		return false;
	}

	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	FreeModule(kernel32dll);
	VirtualFreeEx(process, remoteLibPath, sizeof(remoteLibPath), MEM_RELEASE);
	
	return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	char installFolder[MAX_PATH] = "D:\\Programs\\Steam\\steamapps\\common\\Risen\\bin";
	
	AFixedString<MAX_PATH> gamePath(installFolder);
	AFixedString<MAX_PATH> gameExePath(gamePath);
	gameExePath += "\\Risen.exe";

	STARTUPINFO startupInfo = { 0 };
	PROCESS_INFORMATION processInformation = { 0 };
	startupInfo.cb = sizeof(startupInfo);

	if (GetFileAttributes(gameExePath) == INVALID_FILE_ATTRIBUTES)
	{
		MessageBox(NULL, "Unable to find game .exe file.\n\nGame file:" + gameExePath, "Fatal Error", MB_ICONERROR);
		return 0;
	}

	SetEnvironmentVariable("SteamGameId", "40300");
	SetEnvironmentVariable("SteamAppID", "40300");

	if (!CreateProcess(gameExePath, NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, gamePath, &startupInfo, &processInformation))
	{
		const unsigned lastError = GetLastError();
		MessageBox(NULL, "Cannot create game process.\n\nGame file: " + gameExePath + "\nDirectory: " + gamePath + "\n\n(Error code: " + lastError + ")", "Fatal Error", MB_ICONERROR);
		return 0;
	}

	char cPath[MAX_PATH] = "C:\\Users\\Sim00n\\Documents\\Visual Studio 2015\\Projects\\RisenMP\\Debug\\";
	//GetModuleFileName(NULL, cPath, MAX_PATH);
	AFixedString<MAX_PATH> path(cPath);

	const unsigned lastPathSeparator = path.FindLast('\\');
	path.CutAt(lastPathSeparator);
	path += "\\Coreg.dll";

	if (GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES)
	{
		MessageBox(NULL, "Cannot find Coreg.dll file.", "Error", MB_ICONERROR);
		return 0;
	}

	if (!InjectDLL(processInformation.hProcess, path))
	{
		MessageBox(NULL, "Could not inject dll into the game process. Please try launching the game again.", "Fatal Error", MB_ICONERROR);
		TerminateProcess(processInformation.hProcess, 0);
		return 0;
	}
	
	ResumeThread(processInformation.hThread);
	return 1;
}