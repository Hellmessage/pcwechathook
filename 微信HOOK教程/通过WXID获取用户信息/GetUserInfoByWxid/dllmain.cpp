// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "HWeChat.h"
#include "HDuilib.h"
#include <iostream>

using namespace std;
HMODULE ntdll = GetModuleHandleW(L"ntdll.DLL");
HMODULE GModule = NULL;
HUtil* Util = new HUtil();
HDuilib* GWin = NULL;
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


/*
7ACB0000
7AF1FF65    8DBE 200F0000   lea edi,dword ptr ds:[esi+0xF20]         ; 信息结构体
7AF1FF6B    57              push edi
7AF1FF6C    83EC 14         sub esp,0x14
7AF1FF6F    8D45 08         lea eax,dword ptr ss:[ebp+0x8]
7AF1FF72    8BCC            mov ecx,esp
7AF1FF77    50              push eax                                 ; wxid
7AF1FF78    E8 43423000     call WeChatWi.7B2241C0                   ; wxid查询用户信息call1
7AF1FF81    E8 7A19DFFF     call WeChatWi.7AD11900                   ; wxid查询用户信息call2
7AF1FF8A    E8 E19E0A00     call WeChatWi.7AFC9E70                   ; wxid查询用户信息call3
*/
bool GetUserInfoByWxid(wstring wxid, WxUserInfo* info) {
    WxString temp = { 0 };
    temp.pstr = (wchar_t*)wxid.c_str();
    temp.len = wxid.size();
    temp.maxLen = wxid.size() * 2;

    DWORD call_1 = Util->Offset(0x7B2241C0 - 0x7ACB0000);
    DWORD call_2 = Util->Offset(0x7AD11900 - 0x7ACB0000);
    DWORD call_3 = Util->Offset(0x7AFC9E70 - 0x7ACB0000);

    int ret = 0;
    __asm {
        pushad;
        pushfd;

        mov edi, info;
        push edi;
        sub esp, 0x14;
        lea eax, temp;
        mov ecx, esp;
        push eax;

        call call_1;
        call call_2;
        call call_3;

        mov ret, eax;

        popfd;
        popad;
    }
    return ret == 1;
}



void HWindowNotifyProc(TNotifyUI& msg) {
    if (msg.sType == TEXT("click")) {
        if (msg.pSender->GetName() == TEXT("CloseButton")) {
            GWin->ShowWindow(false);
        } else if (msg.pSender->GetName() == TEXT("MinButton")) {
            SendMessage(GWin->GetHWND(), WM_SYSCOMMAND, SC_MINIMIZE, 0);
        } else if (msg.pSender->GetName() == TEXT("GetUserinfoByWxid")) {
            wstring wxid = GWin->GetControlValue(TEXT("Wxid"));
            WxUserInfo info = { 0 };
            if (GetUserInfoByWxid(wxid, &info)) {
                GWin->GetPM()->FindControl(TEXT("NickName"))->SetText(info.name.pstr);
                GWin->GetPM()->FindControl(TEXT("VData"))->SetText(info.vData.pstr);
                GWin->GetPM()->FindControl(TEXT("LogoUrl"))->SetText(info.pic.pstr);
            }
        }
    }
}

DWORD WINAPI HWindowThreadProc(PVOID context) {
    GWin = new HDuilib(HWindowNotifyProc);
    GWin->Create(NULL, TEXT("地狱微信工具"), UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
    HICON icon = LoadIcon((HINSTANCE)GModule, MAKEINTRESOURCE(IDI_LOGO));
    SendMessage(GWin->GetHWND(), STM_SETICON, IMAGE_ICON, (LPARAM)(UINT)icon);
    GWin->SetIcon(IDI_LOGO);
    GWin->CenterWindow();
    GWin->ShowModal();
    delete GWin;
    CoUninitialize();
    return 0;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
    PfnLdrRegisterDllNotification PLdrRegisterDllNotification;
    PfnLdrUnregisterDllNotification PLdrUnregisterDllNotification;
    switch (reason) {
        case DLL_PROCESS_ATTACH:{
            GModule = module;

            if (!InitDuilib(module)) {
                return 0;
            }

            HANDLE thread = CreateThread(NULL, 0, HWindowThreadProc, NULL, NULL, NULL);
            if (thread) {
                CloseHandle(thread);
            }

            if (ntdll != NULL) {
                PLdrRegisterDllNotification = (PfnLdrRegisterDllNotification)GetProcAddress(ntdll, "LdrRegisterDllNotification");
                PLdrRegisterDllNotification(0, LdrDllNotification, NULL, &PvCookie);
            }

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

