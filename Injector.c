#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include <Psapi.h>
#include <strsafe.h>
#pragma comment( lib, "Psapi.lib" )
void ErrorExit(LPTSTR lpszFunction);
DWORD PROCESSMANAGE_GetProcessesIDs(DWORD *pwszProcesses, int size);
PWSTR PROCESSMANAGE_GetProcessName(DWORD dwPid);
DWORD PROCESSMANAGE_GetProcesseIDByName(LPWSTR lpName);
DWORD dwDesiredAccess;
int main(void)
{

	dwDesiredAccess = PROCESS_CREATE_THREAD |
		PROCESS_QUERY_INFORMATION |
		PROCESS_VM_OPERATION |
		PROCESS_VM_WRITE |
		PROCESS_VM_READ;
	
	char injectedll[144] = "C:\\cybellum\\AppInitHook.dll";
	PWSTR processInjected = L"Explorer.EXE";
	
	DWORD injectedPid = PROCESSMANAGE_GetProcesseIDByName(processInjected);
	if (injectedPid)
	{
		printf(" found %d process \n", injectedPid);
		MessageBox(0, L"find injexted", L"ALERT", MB_OK);
	}
	MessageBox(0, L"OpenProcess", L"ALERT", MB_OK);
	HANDLE process = OpenProcess(dwDesiredAccess, FALSE, injectedPid);
	if (process == 0)
	{
		printf("Error: not found  notepad process \n");
	}

	LPVOID addr = (LPVOID)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryA");
	if (addr == NULL)
	{
		ErrorExit(TEXT("GetProcAddress"));
		printf("Error: the Add function was not found inside DLLInjected.dll library.\n");
	}
	// Get a handle to the DLL module.
	LPVOID arg = (LPVOID)VirtualAllocEx(process, NULL, strlen(injectedll), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (arg == NULL) {
		ErrorExit(TEXT("VirtualAllocEx"));
		printf("Error: the memory could not be allocated inside the chosen process.\n");
	}

	int n = WriteProcessMemory(process, arg, injectedll, strlen(injectedll), NULL);
	if (n == 0) {
		ErrorExit(TEXT("WriteProcessMemory"));
		printf("Error: there was no bytes written to the process's address space.\n");
	}

	HANDLE threadID = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)addr, arg, NULL, NULL);
	if (threadID == NULL) {
		ErrorExit(TEXT("CreateRemoteThread"));
		printf("Error: the remote thread could not be created.\n");
	}
	else {
		printf("Success: the remote thread was successfully created.\n");
	}
	if (threadID != 0) {
		WaitForSingleObject(threadID, INFINITE);
		//GetExitCodeThread(threadID, (LPDWORD)&process);
		CloseHandle(threadID);
	}
	
	CloseHandle(process);
	MessageBox(0, L"CloseHandle", L"ALERT", MB_OK);
	return 0;

}
DWORD PROCESSMANAGE_GetProcesseIDByName(LPWSTR processName)
{
	
	DWORD arrdwProcess[1024];
	LPWSTR lpName;
	DWORD pid = PROCESSMANAGE_GetProcessesIDs(arrdwProcess, 1024);
	for (size_t i = 0; i < pid; i++)
	{
		lpName = PROCESSMANAGE_GetProcessName(arrdwProcess[i]);
		wprintf(L"name %s\n", lpName);
		int ret = wcscmp(lpName, processName);
		if (!ret)
		{
			return arrdwProcess[i];

		}
		
	}
	return 0;
}
PWSTR PROCESSMANAGE_GetProcessName(DWORD dwPid)
{
	PWSTR szProcName = malloc(MAX_PATH*sizeof(WCHAR));
	WCHAR szProcessName[MAX_PATH] = L"<unknown name>";
	HANDLE hProcess;
	HMODULE hModule;
	DWORD dwlen;
	
	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPid);
	if (hProcess != 0)
	{
		
		EnumProcessModules(hProcess, &hModule, sizeof(hModule), &dwlen);
		
		GetModuleBaseNameW(hProcess, hModule, szProcessName, sizeof(szProcessName));
		
	}
	CloseHandle(hProcess);
	//wcscpy(szProcName, szProcessName);
	StringCchCopyW(szProcName, sizeof(PWSTR)* 4, szProcessName);
	return szProcName;
}
DWORD PROCESSMANAGE_GetProcessesIDs(DWORD *pwszProcesses, int size)
{

	int usize = sizeof(DWORD)* size;
	*pwszProcesses = malloc(usize);
	DWORD dwlen, dwProcesses;
	EnumProcesses(pwszProcesses, usize, &dwlen);

	dwProcesses = dwlen / sizeof(DWORD);

	return dwProcesses;
}
void ErrorExit(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(dw);
}