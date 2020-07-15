// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "MemoryMgr.h"

using namespace Memory::VP;

DWORD GetFilePointer(HANDLE hFile)
{
	return SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
}


HANDLE __stdcall CreateFile_hook(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	dwFlagsAndAttributes ^= FILE_FLAG_OVERLAPPED;
	return  CreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);;
}


BOOL __stdcall ReadFile_hook(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {

	BOOL result = ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);

	if (GetFilePointer(hFile) >= GetFileSize(hFile,NULL))
		SetLastError(ERROR_HANDLE_EOF);
	else
		SetLastError(ERROR_IO_PENDING);

	return result;
}



void Init()
{
	Nop(0x45A0B3, 6);
	InjectHook(0x45A0B3, CreateFile_hook, PATCH_CALL);
	Nop(0x45A24B, 6);
	InjectHook(0x45A24B, ReadFile_hook, PATCH_CALL);
	Nop(0x45A2FE, 6);
	InjectHook(0x45A2FE, ReadFile_hook, PATCH_CALL);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		Init();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

