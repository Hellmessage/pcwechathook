// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "HWeChat.h"
#include <iostream>

using namespace std;
HMODULE ntdll = GetModuleHandleW(L"ntdll.DLL");
HMODULE GModule = NULL;
HUtil* Util = new HUtil();
void* PvCookie = NULL;

DWORD WINAPI UnloadThreadProc(PVOID context) {
    FreeLibraryAndExitThread(GModule, 0);
    return 0;
}

void UnLoad() {
    HANDLE thread = CreateThread(NULL, 0, UnloadThreadProc, NULL, NULL, NULL);
    if (thread) {
        CloseHandle(thread);
    }
}

void PrintMessage(DWORD esi);
DWORD HookMessageCallAddress = Util->Offset(0x55582880 - 0x55200000);
DWORD HookMessageAddress = Util->Offset(0x554E8C47 - 0x55200000);
DWORD HookMessageReturnAddress = HookMessageAddress + 5;
CHAR HookMessageBackUp[5] = { 0 };

__declspec(naked) void HookMessageCall() {
    __asm {
        pushad;
        pushfd;

        push esi;
        call PrintMessage;
        add esp, 0x4;

        popfd;
        popad;
        call HookMessageCallAddress;
        jmp HookMessageReturnAddress;
    }
}

void PrintMessage(DWORD esi) {
    wstring from = HTools::ReadUnicodeString(esi + 0x40);
    if (from.find(L"@") != wstring::npos) {
        wstring data = HTools::ReadUnicodeString(esi + 0x68);
        cout << HTools::w2s(from).c_str() << "    " << HTools::w2s(data).c_str() << endl;
    }
}


void HookMessage() {
    BYTE JMPCODE[5] = { 0 };
    JMPCODE[0] = 0xE9;//JMP 
    *(DWORD*)&JMPCODE[1] = (DWORD)HookMessageCall - HookMessageAddress - 5;
    ReadProcessMemory(GetCurrentProcess(), (LPVOID)HookMessageAddress, HookMessageBackUp, 5, 0);
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)HookMessageAddress, JMPCODE, 5, 0);
}

void UnHookMessage() {
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)HookMessageAddress, HookMessageBackUp, 5, 0);
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
    PfnLdrRegisterDllNotification PLdrRegisterDllNotification;
    PfnLdrUnregisterDllNotification PLdrUnregisterDllNotification;
    switch (reason) {
        case DLL_PROCESS_ATTACH:{
            GModule = module;
            HookMessage();
            break;
        }
        case DLL_PROCESS_DETACH:{
            UnHookMessage();
            break;
        }
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }
    return TRUE;
}

