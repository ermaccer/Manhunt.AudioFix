// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "MemoryMgr.h"

using namespace Memory::VP;


BOOL __stdcall ReadFile_hook(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {

	BOOL result = ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);

	if (lpOverlapped->Offset >= GetFileSize(hFile,NULL))
		SetLastError(ERROR_HANDLE_EOF);
	else
     	SetLastError(ERROR_IO_PENDING);

	return result;
}



void Init()
{

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

