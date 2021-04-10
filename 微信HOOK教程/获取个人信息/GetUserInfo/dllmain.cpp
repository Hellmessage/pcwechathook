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

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
    PfnLdrRegisterDllNotification PLdrRegisterDllNotification;
    PfnLdrUnregisterDllNotification PLdrUnregisterDllNotification;
    switch (reason) {
        case DLL_PROCESS_ATTACH:{
            GModule = module;
            if (ntdll != NULL) {
                PLdrRegisterDllNotification = (PfnLdrRegisterDllNotification)GetProcAddress(ntdll, "LdrRegisterDllNotification");
                PLdrRegisterDllNotification(0, LdrDllNotification, NULL, &PvCookie);
            }
            UnLoad();
            break;
        }
        case DLL_PROCESS_DETACH:{
            if (ntdll != NULL) {
                PLdrUnregisterDllNotification = (PfnLdrUnregisterDllNotification)GetProcAddress(ntdll, "LdrUnregisterDllNotification");
                PLdrUnregisterDllNotification(PvCookie);
            }
            break;
        }
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }
    return TRUE;
}

