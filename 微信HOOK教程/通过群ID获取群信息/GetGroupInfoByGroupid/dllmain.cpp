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

/*
78A30000
78AB7AA5    8D8D 6CFEFFFF   lea ecx,dword ptr ss:[ebp-0x194]
78AB7AAB    E8 102E4700     call WeChatWi.78F2A8C0                   ; 取群信息缓存区初始化call
78AB7AFA    8D85 6CFEFFFF   lea eax,dword ptr ss:[ebp-0x194]
78AB7B00    50              push eax                                 ; 缓存区
78AB7B01    53              push ebx                                 ; 群id
78AB7B02    E8 995B2700     call WeChatWi.78D2D6A0                   ; 通过群ID查询群信息
*/

bool GetGroupInfoByGroupID(wstring groupid, WxGroupInfo *info) {
    WxString id = { 0 };
    id.pstr = (wchar_t*)groupid.c_str();
    id.len = groupid.size();
    id.maxLen = groupid.size() * 2;

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
        lea ebx, id;
        push ebx;
        call call_query;
        mov ret, eax;

        popfd;
        popad;
    }
    return ret == 1;
}




BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
    switch (reason) {
        case DLL_PROCESS_ATTACH:{
            GModule = module;

            Util->OpenConsole();
            //0113C43C  14FD3B30  UNICODE "25065881010@chatroom"


            WxGroupInfo info = { 0 };
            if (GetGroupInfoByGroupID(L"25065881010@chatroom", &info)) {
                string member = HTools::ReadAsciiString((DWORD)info.member);
                cout << member.c_str() << endl;
            }
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

