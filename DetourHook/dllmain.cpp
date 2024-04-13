// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "pch.h"
#include "debugapi.h"
#include "shlwapi.h"
#pragma comment(lib, "shlwapi.lib")
#include "winbase.h"
#include <tchar.h>
#include <stdlib.h>
#include "detours.h"

INT(WINAPI* oMessageBoxW)(HWND, LPCWSTR, LPCWSTR, UINT) = MessageBoxW;

// Hooker for MessageBox
int WINAPI hookedMessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
	lpText = L"Hooked!";
	return oMessageBoxW(hWnd, lpText, lpCaption, uType);
}

// Redirect file location to my document of current user
void GetRedirectPath(LPCWSTR path, LPWSTR redirect)
{
	const DWORD buff_size = 1000;
	LPWSTR buff = new TCHAR[buff_size];
	GetEnvironmentVariableW(L"USERPROFILE", buff, buff_size);

	wsprintfW(redirect, L"%s\\Documents\\%s", buff, PathFindFileNameW(path));
	OutputDebugString(redirect);
}

HANDLE(WINAPI* oCreateFileW)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE) = CreateFileW;

// Hooker for CreatFile
HANDLE WINAPI hookedCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	LPWSTR redirect = new TCHAR[1000];
	if (wcsstr(lpFileName, L"D:\\Wang"))
		GetRedirectPath(lpFileName, redirect);
	else
		wsprintfW(redirect, L"%s", lpFileName);
	return oCreateFileW(
			redirect, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
	{
		OutputDebugString(L"Add detour hooker...");
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)oMessageBoxW, hookedMessageBoxW);
		DetourAttach(&(PVOID&)oCreateFileW, hookedCreateFileW);
		LONG lError = DetourTransactionCommit();
		if (lError != NO_ERROR) {
			MessageBox(HWND_DESKTOP, L"Unable to add detour hooker", L"Detour Error", MB_OK);
			return FALSE;
		}
		OutputDebugString(L"Completed to add detour hooker...");
	}
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


