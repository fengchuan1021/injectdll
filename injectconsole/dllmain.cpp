// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <iostream>
#include<tchar.h>
#include <Python.h>
#include <string>
using namespace std;
struct WxString
{
    // 存字符串
    wchar_t* buffer;

    // 存字符串长度
    int size;

    // 存字符串空间
    int capacity;

    // 8个空余的0
    char fill[8];
};

DWORD sendaddr = 0;
 void  sendmsg(const wchar_t* pwxid, const wchar_t* pmsg) {
    if (sendaddr == 0){
        sendaddr= (DWORD)GetModuleHandle(L"WeChatWin.dll")+0x34CA50;
  
    }

    wstring wsWxId = (wchar_t*)pwxid;
    wstring wsTextMsg = (wchar_t*)pmsg;
    WxString wxWxid = { 0 };
    wxWxid.buffer = (wchar_t*)wsWxId.c_str();
    wxWxid.size = wsWxId.size();
    wxWxid.capacity = wsWxId.capacity();

    // 构造消息结构
    WxString wxTextMsg = { 0 };
    wxTextMsg.buffer = (wchar_t*)wsTextMsg.c_str();
    wxTextMsg.size = wsTextMsg.size();
    wxTextMsg.capacity = wsTextMsg.capacity();

    // 构造空结构
    WxString wxNULL = { 0 };

    // 构造空buffer
    char buffer[0x738] = { 0 };
    __asm {
        push 0x1;
        lea eax, wxNULL;
        push eax;

        lea ebx, wxTextMsg;
        push ebx;

        lea edx, wxWxid;
        lea ecx, buffer;

        call sendaddr;
        add esp, 0xc;

    }


}
 //6e72a744
 DWORD tmpaddress = 0;
 DWORD addressoffset = 0;
 DWORD backaddressoffset = 0;
 DWORD debugaddr = 0;
 __declspec(naked) void calloutdebug() {
     __asm {
         pushad;
         pushfd;
         push[ebp - 0x40e0];
         call debugaddr;
         popfd;
         popad;
         mov ecx,dword ptr [tmpaddress];

         jmp backaddressoffset;

     }
 
 }
 void opendebug() {
     DWORD base = (DWORD)GetModuleHandle(L"WeChatWin.dll");

     DWORD kbase = (DWORD)GetModuleHandle(L"kernel32.dll");
     debugaddr = (DWORD)GetProcAddress((HMODULE)kbase, "OutputDebugStringA");
     int *p=(int *)(base + 0x6c00ab61-1788215296);
     *p = 1;
     int* pl = (int*)(base + 0x51E88D6C - 0x50830000);
     *pl = 0;
     addressoffset = base + 0x6B386F7D - 0x6a620000;
     backaddressoffset = addressoffset + 6;
     tmpaddress = base + 0x6e72a744 - 0x6d080000;
     BYTE jmpcode[5] = { 0 };
     *(DWORD*)&jmpcode[1] = (DWORD)calloutdebug - addressoffset;
   //  WriteProcessMemory(GetCurrentProcess(), (LPVOID)addressoffset, jmpcode, 5, 0);

 }
void CreateConsole()
{
    if (!AllocConsole()) {
        // Add some error handling here.
        // You can call GetLastError() to get more info about the error.
        return;
    }

    // std::cout, std::clog, std::cerr, std::cin
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONIN$", "r", stdin);
    std::cout.clear();
    std::clog.clear();
    std::cerr.clear();
    std::cin.clear();

    // std::wcout, std::wclog, std::wcerr, std::wcin
    HANDLE hConOut = CreateFile(_T("CONOUT$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE hConIn = CreateFile(_T("CONIN$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    SetStdHandle(STD_OUTPUT_HANDLE, hConOut);
    SetStdHandle(STD_ERROR_HANDLE, hConOut);
    SetStdHandle(STD_INPUT_HANDLE, hConIn);
    std::wcout.clear();
    std::wclog.clear();
    std::wcerr.clear();
    std::wcin.clear();
}

DWORD WINAPI uithread(LPVOID p) {
    CreateConsole();
   // opendebug();
    wchar_t pyhome[] = _T("C:\\Python36-32");
    Py_SetPythonHome(pyhome);
    Py_Initialize();
    PyRun_InteractiveLoop(stdin, "<stdin>");
    return 0;

}
void  __stdcall mythread(wchar_t*) {
    //AfxBeginThread(myproc,0,0,0,0);

    CreateThread(0, 0, uithread, 0, 0, 0);


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
        sendmsg(L"filehelper",L"hell");
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

