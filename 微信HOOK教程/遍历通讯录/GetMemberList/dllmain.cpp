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

VOID NTAPI LdrDllNotification(ULONG NotificationReason, PCLDR_DLL_NOTIFICATION_DATA NotificationData, PVOID Context) {
    if (NotificationReason == LDR_DLL_NOTIFICATION_REASON_LOADED) {
        if (wcscmp(NotificationData->Loaded.BaseDllName->Buffer, L"WeChatWin.dll") == 0) {
            //注入代码

        }
    }
}

int mNum = 0;

void Ergodic(const char* str, DWORD start, DWORD end) {
    mNum++;
    cout << str << "    " << mNum << hex << "    0x" << start;
    wstring wxid = HTools::ReadUnicodeString(start + 0x30);
    cout << "    " << HTools::w2s(wxid).c_str();
    wstring name = HTools::ReadUnicodeString(start + 0x8c);
    cout << "    " << HTools::w2s(name).c_str() << endl;

    DWORD left = HTools::ReadInt(start);
    if (left != end) {
        Ergodic("Left", left, end);
    }

    DWORD right = HTools::ReadInt(start + 0x8);
    if (right != end) {
        Ergodic("Right", right, end);
    }
}

void GetMemberList() {
    DWORD listAddress = HTools::ReadInt(Util->Offset(0x1886B38)) + 0x28 + 0x8c;
    cout << "通讯录链表指针: 0x" << hex << listAddress << endl;
    DWORD list = HTools::ReadInt(listAddress);
    cout << "通讯录链表: 0x" << hex << list << endl;
    mNum = 0;
    Ergodic("Base", HTools::ReadInt(list + 0x4), list);
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
    switch (reason) {
        case DLL_PROCESS_ATTACH:{
            GModule = module;
            Util->OpenConsole();
            GetMemberList();
            break;
        }
        case DLL_PROCESS_DETACH:{
            Util->CloseConsole();
            break;
        }
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }
    return TRUE;
}

