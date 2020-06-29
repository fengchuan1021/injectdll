// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <thread>
#include <iostream>
#include "resource.h"
#ifdef _WIN64
#include <polyhook2/Detour/x64Detour.hpp>
#else
#include <polyhook2/Detour/x86Detour.hpp>
#endif
#include <polyhook2/CapstoneDisassembler.hpp>

using namespace std;
HMODULE hmodule = 0;
HWND edit;
HWND hmaindlg;

#ifdef _WIN64

PLH::x64Detour* unptr=0;
#else
PLH::x86Detour* unptr=0;
#endif

uint64_t hooktextoutTramp = NULL;
void mylog(char* str) {
	SendMessageW(edit, WM_SETTEXT, 0, (LPARAM)str);
}
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		hmaindlg = hDlg;
		edit=GetDlgItem(hDlg, IDC_EDIT1);
		return TRUE;
	case WM_COMMAND:
	{
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
	}
	break;
	case WM_CLOSE:
	{	if(unptr!=0)
		unptr->unHook();
		PostQuitMessage(1);
		FreeLibraryAndExitThread(GetModuleHandleA("injectdll.dll"), 0);
		return TRUE;
	}
	}
	return FALSE;
}
BOOL  WINAPI myExtTextOutW(HDC hdc, int x, int y, UINT options, CONST RECT* lprect, LPCWSTR lpString, UINT c, CONST INT* lpDx) {
	HWND tmp=GetActiveWindow();
	if (tmp == hmaindlg) {
	
	}
	else {
		if (options & ETO_GLYPH_INDEX) {}else{
			//if (x == 0x18a) {
				int iSize=WideCharToMultiByte(CP_ACP, 0, lpString, c, NULL, 0, NULL, NULL);
				char* pszMultiByte = (char*)malloc(iSize * sizeof(char)+1);
				WideCharToMultiByte(CP_ACP, 0, lpString, c, pszMultiByte, iSize+1, NULL, NULL);
				pszMultiByte[iSize] = '\0';
				mylog(pszMultiByte);
				free(pszMultiByte);

			//}
		}
	}

	//wstring tmp(lpString,c);

	return PLH::FnCast(hooktextoutTramp, &ExtTextOutW)(hdc,x, y,options,  lprect, lpString,c, lpDx);
}
DWORD WINAPI uithread(LPVOID p){
	HWND hAbout=CreateDialogW(hmodule, MAKEINTRESOURCE(IDD_DIALOG1),NULL, (DLGPROC)About);
	ShowWindow(hAbout, SW_SHOW);
	UpdateWindow(hAbout);
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

 void  __stdcall mythread(wchar_t*) {
    //AfxBeginThread(myproc,0,0,0,0);

    CreateThread(0,0,uithread,0,0,0);


}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:{
        hmodule = hModule;
#ifdef _WIN64

		PLH::CapstoneDisassembler dis(PLH::Mode::x64);
		unptr = new PLH::x64Detour((char*)&ExtTextOutW, (char*)&myExtTextOutW, &hooktextoutTramp, dis);
#else
		PLH::CapstoneDisassembler dis(PLH::Mode::x86);
		unptr = new PLH::x86Detour((char*)&ExtTextOutW, (char*)&myExtTextOutW, &hooktextoutTramp, dis);
#endif


		
		unptr->hook();
	}
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

