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
78A30000
78B3A1BB    6A 01           push 0x1                                 ; 压入0x1
78B3A1BD    57              push edi                                 ; 压入缓存区
78B3A1BE    53              push ebx                                 ; 消息内容(WxString)
78B3A1BF    8D95 78FFFFFF   lea edx,dword ptr ss:[ebp-0x88]          ; 接收人(wxid)(WxString)
78B3A1C5    8D8D 58FAFFFF   lea ecx,dword ptr ss:[ebp-0x5A8]         ; 缓存区
78B3A1CB    E8 D06A2900     call WeChatWi.78DD0CA0                   ; 消息发送关键call
78B3A1D0    83C4 0C         add esp,0xC
*/

void SendTextMessage(wstring wxid, wstring message) {
    WxString to = { 0 };
    WxString text = { 0 };

    to.pstr = (wchar_t*)wxid.c_str();
    to.len = wxid.size();
    to.maxLen = wxid.size() * 2;

    text.pstr = (wchar_t*)message.c_str();
    text.len = message.size();
    text.maxLen = message.size();

    char buff[0x20] = { 0 };
    char buff2[0x500] = { 0 };

    DWORD sendCall = Util->Offset(0x78DD0CA0 - 0x78A30000);

    __asm {
        pushad;
        pushfd;

        push 0x1;
        lea edi, buff;
        push edi;
        lea ebx, text;
        push ebx;
        lea edx, to;
        lea ecx, buff2;
        call sendCall;
        add esp, 0xc;
        popfd;
        popad;
    }
}


void SendTextMessage(wstring wxid, wstring message, wstring id) {
    WxString to = { 0 };
    WxString text = { 0 };
    WxString tempid = { 0 };
    tempid.pstr = (wchar_t*)id.c_str();
    tempid.len = id.size();
    tempid.maxLen = id.size() * 2;

    WxAtWxid at = { 0 };
    at.wxidArr = (WxString *)&tempid.pstr;
    at.end = (DWORD)at.wxidArr + sizeof(WxString);
    at.end2 = at.end;

    to.pstr = (wchar_t*)wxid.c_str();
    to.len = wxid.size();
    to.maxLen = wxid.size() * 2;

    text.pstr = (wchar_t*)message.c_str();
    text.len = message.size();
    text.maxLen = message.size();

    char buff2[0x500] = { 0 };

    DWORD sendCall = Util->Offset(0x78DD0CA0 - 0x78A30000);

    __asm {
        pushad;
        pushfd;

        push 0x1;
        lea edi, at;
        push edi;
        lea ebx, text;
        push ebx;
        lea edx, to;
        lea ecx, buff2;
        call sendCall;
        add esp, 0xc;
        popfd;
        popad;
    }
}


void HWindowNotifyProc(TNotifyUI& msg) {
    if (msg.sType == TEXT("click")) {
        if (msg.pSender->GetName() == TEXT("CloseButton")) {
            GWin->ShowWindow(false);
        } else if (msg.pSender->GetName() == TEXT("MinButton")) {
            SendMessage(GWin->GetHWND(), WM_SYSCOMMAND, SC_MINIMIZE, 0);
        } else if (msg.pSender->GetName() == TEXT("SendTextMessage")) {
            wstring wxid = GWin->GetControlValue(TEXT("Wxid"));
            wstring message = GWin->GetControlValue(TEXT("Message"));
            SendTextMessage(wxid, message);
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


            //0113E8C0  150353A0  UNICODE "25065881010@chatroom"
            //14CACA58  14BA3638  UNICODE "wxid_ngpxzj8blk5j12"
            SendTextMessage(L"25065881010@chatroom", L"@Hell 123123123", L"wxid_ngpxzj8blk5j12");


         /*   if (!InitDuilib(module)) {
                return 0;
            }

            HANDLE thread = CreateThread(NULL, 0, HWindowThreadProc, NULL, NULL, NULL);
            if (thread) {
                CloseHandle(thread);
            }*/

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

