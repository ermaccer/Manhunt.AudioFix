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



struct ADPCMDATA;

typedef void (*fnDecodeADPCM_MONO_t)(void *out, void const *in, int out_len, int in_len, ADPCMDATA *adpcm_data);

fnDecodeADPCM_MONO_t fnDecodeADPCM_MONO = NULL;

void DecodeADPCM_MONO(void *out, void const *in, int out_len, int in_len, ADPCMDATA *adpcm_data)
{
    __try
    {
        fnDecodeADPCM_MONO(out, in, out_len, in_len, adpcm_data);
    }
    __except ((GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    { }
}

void Init()
{
	Patch<int>(0x81B374, (int)ReadFile_hook);
	Patch<int>(0x673F30, (int)Movies_hook);
	
    // Unhandled exception at 0x2114036A (mss32.dll) in manhunt.exe.20191006175924.dmp: 0xC0000005: Access violation reading location 0x211C1379.
	// https://github.com/ThirteenAG
    
	HMODULE mss32 = GetModuleHandle("mss32");
	if ( mss32 )
	{		
		fnDecodeADPCM_MONO = (fnDecodeADPCM_MONO_t)((DWORD)mss32 + 0x402EB);
		
		InjectHook((DWORD)mss32 + 0x091D6, DecodeADPCM_MONO);
		InjectHook((DWORD)mss32 + 0x0AF56, DecodeADPCM_MONO);
		InjectHook((DWORD)mss32 + 0x2A3C2, DecodeADPCM_MONO);
		InjectHook((DWORD)mss32 + 0x2BFFE, DecodeADPCM_MONO);
	}
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

