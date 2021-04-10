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

bool GetUserInfoByWxid(string wxid, WxUserInfo* buffer) {
    wstring temp = HTools::s2w(wxid);
    WxString queryid = { 0 };
    queryid.pstr = (wchar_t*)temp.c_str();
    queryid.len = temp.size();
    queryid.maxLen = temp.size() * 2;

    DWORD call_init = Util->Offset(0x787641C0 - 0x781F0000);
    DWORD call_sql = Util->Offset(0x78251900 - 0x781F0000);
    DWORD call_query = Util->Offset(0x78509E70 - 0x781F0000);

    int ret = 0;
    __asm {
        pushad;
        pushfd;
        mov edi, buffer;
        push edi;
        sub esp, 0x14;
        lea eax, queryid;
        mov ecx, esp;
        push eax;
        call call_init;
        call call_sql;
        call call_query;
        mov ret, eax;
        popfd;
        popad;
    }
    return ret == 1;
}

bool GetGroupInfoByWxid(wstring wxid, WxGroupInfo* info) {
    WxString groupid = { 0 };
    groupid.pstr = (wchar_t*)wxid.c_str();
    groupid.len = wxid.size();
    groupid.maxLen = wxid.size() * 2;

    DWORD call_init = Util->Offset(0x78F2A8C0 - 0x78A30000);
    DWORD call_query = Util->Offset(0x78D2D6A0 - 0x78A30000);
    int ret = 0;
    __asm {
        pushad;
        pushfd;
        mov ecx, info;
        call call_init;
        mov eax, info;
        push eax;
        lea ebx, groupid;
        push ebx;
        call call_query;
        mov ret, eax;
        popfd;
        popad;
    }
    return ret == 1;
}





void SendImageMessage(wstring wxid, wstring picPath) {
    WxString to = { 0 };
    to.pstr = (wchar_t*)wxid.c_str();
    to.len = wxid.size();
    to.maxLen = wxid.size() * 2;
    WxString pic = { 0 };
    pic.pstr = (wchar_t*)picPath.c_str();
    pic.len = picPath.size();
    pic.maxLen = picPath.size() * 2;
    char* asmTo = (char*)&to.pstr;

    char buff[0x20] = { 0 };
    char buff2[0x1024] = { 0 };
    

    /*
    78B3A2CF    83EC 14         sub esp,0x14
    78B3A2D2    8D47 E0         lea eax,dword ptr ds:[edi-0x20]
    78B3A2D5    8BCC            mov ecx,esp                              ; 图片地址
    78B3A2D7    8965 BC         mov dword ptr ss:[ebp-0x44],esp
    78B3A2DA    50              push eax                                 ; WeChatWi.7A2D3AC8
    78B3A2DB    E8 E09E4600     call WeChatWi.78FA41C0
    78B3A2E0    53              push ebx                                 ; 图片地址
    78B3A2E1    8D85 78FFFFFF   lea eax,dword ptr ss:[ebp-0x88]          ; 接收人
    78B3A2E7    C645 FC 09      mov byte ptr ss:[ebp-0x4],0x9
    78B3A2EB    50              push eax                                 ; 接收人
    78B3A2EC    8D85 B0FCFFFF   lea eax,dword ptr ss:[ebp-0x350]         ; 缓存区
    78B3A2F2    50              push eax                                 ; WeChatWi.7A2D3AC8
    78B3A2F3    E8 C876F5FF     call WeChatWi.78A919C0
    78B3A2F8    8BC8            mov ecx,eax                              ; WeChatWi.7A2D3AC8
    78B3A2FA    C645 FC 01      mov byte ptr ss:[ebp-0x4],0x1
    78B3A2FE    E8 9D622900     call WeChatWi.78DD05A0
    */

    DWORD call_init = Util->Offset(0x78FA41C0 - 0x78A30000);
    DWORD call_push = Util->Offset(0x78A919C0 - 0x78A30000);
    DWORD call_send = Util->Offset(0x78DD05A0 - 0x78A30000);

    __asm {
        pushad;
        pushfd;

        sub esp, 0x14;
        lea eax, buff;
        mov ecx, esp;
        push eax;
        call call_init;
        lea ebx, pic;
        push ebx;
        lea eax, to;
        push eax;
        lea eax, buff2;
        push eax;
        call call_push;
        mov ecx, eax;
        call call_send;

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
        } else if (msg.pSender->GetName() == TEXT("GetUserinfoByWxid")) {
            string wxid = HTools::w2s(GWin->GetControlValue(L"WxidSet"));
            WxUserInfo info = { 0 };
            if (GetUserInfoByWxid(wxid, &info)) {
                wstring nickname = HTools::ReadUnicodeString((DWORD)&info.name);
                GWin->GetPM()->FindControl(TEXT("NickNameSet"))->SetText(info.name.pstr);
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
            //HANDLE thread = CreateThread(NULL, 0, HWindowThreadProc, NULL, NULL, NULL);
            //if (thread) {
            //    CloseHandle(thread);
            //}

            //Util->OpenConsole();
            //WxGroupInfo info = { 0 };
            //if (GetGroupInfoByWxid(L"22880602991@chatroom", &info)) {
            //    string member = HTools::ReadAsciiString((DWORD)info.member);
            //    vector<string> ids = HTools::Split(member, "^G");
            //    vector<string>::iterator it;
            //    for (it = ids.begin(); it != ids.end(); it++) {
            //        cout << it->c_str() << endl;
            //    }
            //}

            SendImageMessage(L"filehelper", L"C:\\Users\\Hell\\Desktop\\a.jpg");


            if (ntdll != NULL) {
                PLdrRegisterDllNotification = (PfnLdrRegisterDllNotification)GetProcAddress(ntdll, "LdrRegisterDllNotification");
                PLdrRegisterDllNotification(0, LdrDllNotification, NULL, &PvCookie);
            }

            //UnLoad();
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

