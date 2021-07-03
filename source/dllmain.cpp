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

void __declspec(naked) Movies_hook()
{
	__asm
	{		
		mov     eax, dword ptr ds: [7C9970h] // 7C9970h
		cmp     eax, 0
		jnz     SECONDPART
		
		mov     eax, dword ptr ds: [736DB8h] // Skel
		mov     edx, eax
		mov     eax, edx
		mov     ecx, [eax+0Ch]
		mov     ebx, [ecx]
		call    dword ptr [ebx+2Ch]
		
		mov     eax, dword ptr ds: [736DB8h] // Skel
		mov     edx, eax
		mov     eax, edx
		mov     ecx, [eax+0Ch]
		mov     ebx, [ecx]
		call    dword ptr [ebx+28h]
		
		mov     ecx, 725A4Ch // g_TextLevel
		mov     eax, 4932F0h
		call    eax          // CText::LoadLevel
		
		mov     dword ptr ds: [7C9970h], 1
		
		mov     eax, 4C3650h
		call    eax          // CApp::PlayBinkMovie
		
		mov     eax, 4D7E30h
		jmp     eax
		
SECONDPART:
		mov     ecx, 7A9EBCh // MusicManager
		mov     eax, 5A5A50h
		call    eax          // cMusicManager::PlayLevelCompletedMusic
		
		
		push    2
		mov     eax, 5DBDC0h
		call    eax          // CGameInfo::Show
		pop     ecx
		
		mov     dword ptr ds: [7C86E0h], 0 // CFrontendMenu::m_bOkToLoadNextLevel
	
		mov     dword ptr ds: [755E4Ch], 6 // CApp::ms_mode
		
		mov     eax, 4D7E26h
		jmp     eax
	}
}



void Init()
{
	Patch<int>(0x81B374, (int)ReadFile_hook);
	Patch<int>(0x673F30, (int)Movies_hook);
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

