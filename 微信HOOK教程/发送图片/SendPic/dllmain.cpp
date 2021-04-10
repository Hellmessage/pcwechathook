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




void SendPic(wstring to, wstring pic) {
    WxString wxid = { 0 };
    WxString img = { 0 };
    WxString temp = { 0 };
    char buff[0x500] = { 0 };

    wxid.pstr = (wchar_t*)to.c_str();
    wxid.len = to.size();
    wxid.maxLen = to.size() * 2;

    img.pstr = (wchar_t*)pic.c_str();
    img.len = pic.size();
    img.maxLen = pic.size() * 2;


    /*
    78A30000
    78B3A2CF    83EC 14         sub esp,0x14
    78B3A2D2    8D47 E0         lea eax,dword ptr ds:[edi-0x20]           ; WxString
    78B3A2D5    8BCC            mov ecx,esp                               ; 图片地址
    78B3A2DA    50              push eax
    78B3A2DB    E8 E09E4600     call WeChatWi.78FA41C0                    ; 发送图片_初始化句柄
    78B3A2E0    53              push ebx                                  ; 图片地址
    78B3A2E1    8D85 78FFFFFF   lea eax,dword ptr ss:[ebp-0x88]           ; 接收人
    78B3A2EB    50              push eax                                  ; 接收人
    78B3A2EC    8D85 B0FCFFFF   lea eax,dword ptr ss:[ebp-0x350]          ; 缓存区
    78B3A2F2    50              push eax
    78B3A2F3    E8 C876F5FF     call WeChatWi.78A919C0
    78B3A2F8    8BC8            mov ecx,eax
    78B3A2FE    E8 9D622900     call WeChatWi.78DD05A0                    ; 图片发送关键call
    */

    DWORD call_init = Util->Offset(0x78FA41C0 - 0x78A30000);
    DWORD call_query = Util->Offset(0x78A919C0 - 0x78A30000);
    DWORD call_send = Util->Offset(0x78DD05A0 - 0x78A30000);

    __asm {
        pushad;
        pushfd;

        sub esp, 0x14;
        lea eax, temp;
        mov ecx, esp;
        push eax;
        call call_init;
        lea ebx, img;
        push ebx;
        lea eax, wxid;
        push eax;
        lea eax, buff;
        push eax;
        call call_query;
        mov ecx, eax;
        call call_send;


        popfd;
        popad;
    }
}


BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
    switch (reason) {
        case DLL_PROCESS_ATTACH:{
            GModule = module;

            SendPic(L"filehelper", L"C:\\Users\\Hell\\Desktop\\a.jpg");


            UnLoad();
            break;
        }
        case DLL_PROCESS_DETACH:{

            break;
        }
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }
    return TRUE;
}

