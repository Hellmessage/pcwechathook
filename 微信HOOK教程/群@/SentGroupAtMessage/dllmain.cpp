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

map<wstring, DWORD> memberList;


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

void Ergodic(DWORD start, DWORD end) {
    wstring wxid = HTools::ReadUnicodeString(start + 0x30);
    if (wxid.find(L"@") != wstring::npos) {
        memberList[wxid] = start;
    }

    DWORD left = HTools::ReadInt(start);
    if (left != end) {
        Ergodic(left, end);
    }
    DWORD right = HTools::ReadInt(start + 0x8);
    if (right != end) {
        Ergodic(right, end);
    }
}

void ErgodicMemberList() {
   
    memberList.clear();
    CComboUI* member = static_cast<CComboUI*>(GWin->GetPM()->FindControl(TEXT("MemberCombo")));

    DWORD listAddress = HTools::ReadInt(Util->Offset(0x1886B38)) + 0x28 + 0x8c;
    DWORD list = HTools::ReadInt(listAddress);
    Ergodic(HTools::ReadInt(list + 0x4), list);

    map<wstring, DWORD>::iterator it;
    for (it = memberList.begin(); it != memberList.end(); it++) {
        CListLabelElementUI* item = new CListLabelElementUI();
        item->SetText(it->first.c_str());
        item->SetFixedHeight(20);
        member->Add(item);
        item->SetVisible(true);
        item->SetInternVisible(true);
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

void HWindowNotifyProc(TNotifyUI& msg) {
    if (msg.sType == TEXT("click")) {
        if (msg.pSender->GetName() == TEXT("CloseButton")) {
            GWin->ShowWindow(false);
        } else if (msg.pSender->GetName() == TEXT("MinButton")) {
            SendMessage(GWin->GetHWND(), WM_SYSCOMMAND, SC_MINIMIZE, 0);
        } else if (msg.pSender->GetName() == TEXT("GetMember")) {
            ErgodicMemberList();
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

