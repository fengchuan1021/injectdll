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
FILETIME fCreateTime, fAccessTime, fWriteTime = {0,0};
PyObject* pModule = NULL;
PyObject* pFunc = NULL;
void inipythonscript() {

    PyRun_SimpleString("import sys");
    PyRun_SimpleString("a=1 if 'd:\\wechatscript' in sys.path else sys.path.append('d:\\wechatscript')");
    if (pModule == NULL)
        pModule = PyImport_ImportModule("msg");
    else
        PyImport_ReloadModule(pModule);
    if (!pModule) {
        cout << "get module failed" << endl;
        return;
    }
    pFunc = PyObject_GetAttrString(pModule, "callbackfun");
    if (!pFunc || !PyCallable_Check(pFunc))
    {
        cout << "Can't find funftion " << endl;
        return ;
    }
    HANDLE hfile = CreateFileW(L"D:\\wechatscript\\msg.py",  GENERIC_READ,
        FILE_SHARE_READ,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hfile != INVALID_HANDLE_VALUE) {
        GetFileTime(hfile, &fCreateTime, &fAccessTime, &fWriteTime);
    }
    CloseHandle(hfile);
}
DWORD coveraddress = 0;
DWORD retaddress = 0;
void hookmsg(DWORD ebp) {
   HANDLE hfile= CreateFileW(L"D:\\wechatscript\\msg.py", GENERIC_READ,
        FILE_SHARE_READ,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
   if (hfile != INVALID_HANDLE_VALUE) {
       if (fWriteTime.dwHighDateTime==0 && fWriteTime.dwLowDateTime==0) {
           inipythonscript();
       }
       else {
           FILETIME tmptime;
           GetFileTime(hfile, &fCreateTime, &fAccessTime, &tmptime);
           if (fWriteTime.dwHighDateTime == tmptime.dwHighDateTime && fWriteTime.dwLowDateTime == tmptime.dwLowDateTime) {
           }
           else {
               inipythonscript();
           }
       }
      

   }
   CloseHandle(hfile);
   if (pFunc != NULL) {
      // PyObject* arg1 = Py_BuildValue("i", ebp);
       cout << "beforecall" << endl;
       //PyObject* result= PyObject_CallObject(pFunc, NULL);
       PyObject* result = PyObject_CallFunction(pFunc, "i",ebp);
       
    //  if (arg1)
     //      Py_DECREF(arg1);
       if (result)
           Py_DECREF(result);
   }
       
}
_declspec(naked) void listenrecvmsg()
{
    
    __asm {
        pushad;
        pushfd;
        push ebp;
        call hookmsg;
        add esp, 0x4;
        popfd;
        popad;
        push coveraddress;
        jmp retaddress;

    
    }
}

void installhook() {
    DWORD dllbase = (DWORD)GetModuleHandle(L"WeChatWin.dll");
    DWORD hookoffset = 0x329743;
    DWORD hookaddress = dllbase + hookoffset;
     retaddress = hookaddress + 5;
    BYTE jmpcode[5] = { 0 };
    coveraddress = dllbase + 0x14003C0;
    jmpcode[0] = 0xe9;
    *(DWORD*)(&jmpcode[1]) = (DWORD)listenrecvmsg - hookaddress - 5;
    cout << "before write memory" << endl;
    cout << dllbase << endl << hookaddress << endl << *(DWORD*)(&jmpcode[1]) << endl;
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)hookaddress, jmpcode, 5, 0);


}
void getmoney(wchar_t* transferid, wchar_t* wxid)
{
    wstring tmpid = transferid;
    wstring twxid = wxid;
    std::wcout << tmpid << endl;
    std::wcout << twxid << endl;
    struct CllectMoneyStruct
    {
        wchar_t* ptransferid;
        int transferidLen;
        int transferidMaxLen;
        char full[0x8] = { 0 };
        wchar_t* pwxid;
        int wxidLen;
        int wxidMaxLen;
        char full2[0x8] = { 0 };
    };

    CllectMoneyStruct cllect;
    cllect.ptransferid = transferid;
    cllect.transferidLen = wcslen(transferid) + 1;
    cllect.transferidMaxLen = (wcslen(transferid) + 1) * 2;
    cllect.pwxid = wxid;
    cllect.wxidLen = wcslen(wxid) + 1;
    cllect.wxidMaxLen = (wcslen(wxid) + 1) * 2;

    char* asmBuff = (char*)&cllect.ptransferid;
    DWORD dwWeChatWinAddr = (DWORD)GetModuleHandle(L"WeChatWin.dll");
	DWORD dwCall1 = dwWeChatWinAddr +0x5c28dc90-0x5ba70000;
	DWORD dwCall2 = dwWeChatWinAddr + 0x5c28dd10 - 0x5ba70000;
	__asm
	{
		sub esp, 0x30;
		mov ecx, esp;
		mov eax, asmBuff;
		push eax;
		call dwCall1;
		call dwCall2;
		add esp, 0x30;
	}
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
    
    wchar_t pyhome[] = _T("C:\\Python36-32");
    Py_SetPythonHome(pyhome);
    Py_Initialize();
    inipythonscript();
    installhook();
   // PyRun_InteractiveLoop(stdin, "<stdin>");
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

