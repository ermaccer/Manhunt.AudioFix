// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "MemoryMgr.h"

using namespace Memory::VP;

DWORD GetFilePointer(HANDLE hFile)
{
	return SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
}




BOOL __stdcall ReadFile_hook(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {

	BOOL result = ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);

	if (lpOverlapped)
	{
		if (lpOverlapped->Offset >= GetFileSize(hFile, NULL))
			SetLastError(ERROR_HANDLE_EOF);
		else
			SetLastError(ERROR_IO_PENDING);
	}
	else
	{
		if (GetFilePointer(hFile) >= GetFileSize(hFile, NULL))
			SetLastError(ERROR_HANDLE_EOF);
		else
			SetLastError(ERROR_IO_PENDING); // not needed?
	}

	return result;
}




void Init()
{
	Patch<int>(0x81B374, (int)ReadFile_hook);
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

