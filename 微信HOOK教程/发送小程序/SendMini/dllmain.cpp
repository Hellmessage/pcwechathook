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




void SendMini(wstring wxid, wstring xml, wstring pic) {
    WxString to = { 0 };
    WxString img = { 0 };
    WxString data = { 0 };
    WxString from = { 0 };

    DWORD tempPathAddress = Util->Offset(0x18A3550);
    string fidtemp = HTools::ReadAsciiString(HTools::ReadInt(tempPathAddress + 0x34));
    wstring fid = HTools::s2w(fidtemp);
    from.pstr = (wchar_t*)fid.c_str();
    from.len = fid.size();
    from.maxLen = fid.size() * 2;

    to.pstr = (wchar_t*)wxid.c_str();
    to.len = wxid.size();
    to.maxLen = wxid.size() * 2;

    img.pstr = (wchar_t*)pic.c_str();
    img.len = pic.size();
    img.maxLen = pic.size() * 2;

    data.pstr = (wchar_t*)xml.c_str();
    data.len = xml.size();
    data.maxLen = xml.size() * 2;


    /*
    78A30000
    78CE946B    FFB5 DCF2FFFF   push dword ptr ss:[ebp-0xD24]             ; 0x21
    78CE9471    8D85 78FFFFFF   lea eax,dword ptr ss:[ebp-0x88]           ; 缓存区
    78CE9477    50              push eax
    78CE9478    8D85 64FFFFFF   lea eax,dword ptr ss:[ebp-0x9C]           ; 图片
    78CE947E    50              push eax
    78CE947F    8D85 20FFFFFF   lea eax,dword ptr ss:[ebp-0xE0]           ; xml数据
    78CE9485    50              push eax
    78CE9486    57              push edi                                  ; 接收人
    78CE9487    8D95 B8F3FFFF   lea edx,dword ptr ss:[ebp-0xC48]          ; 发送人
    78CE948D    8D8D B0FCFFFF   lea ecx,dword ptr ss:[ebp-0x350]          ; ecx
    78CE9493    E8 68DEFFFF     call WeChatWi.78CE7300                    ; 发送小程序关键call
    78CE9498    83C4 14         add esp,0x14

    78A30000
    78CE9241    8D8D B0FCFFFF   lea ecx,dword ptr ss:[ebp-0x350]
    78CE9247    C645 FC 0B      mov byte ptr ss:[ebp-0x4],0xB
    78CE924B    E8 4034DAFF     call WeChatWi.78A8C690                    ; 发送小程序ecx 初始化call

    78CE951B    68 00512C7A     push WeChatWi.7A2C5100
    78CE9520    68 00512C7A     push WeChatWi.7A2C5100
    78CE9527    8D8D B0FCFFFF   lea ecx,dword ptr ss:[ebp-0x350]
    78CE952D    E8 DEDFFFFF     call WeChatWi.78CE7510



    */

    char buff[0x100] = { 0 };
    WxMessage message = { 0 };

    DWORD call_init = Util->Offset(0x78A8C690 - 0x78A30000);
    DWORD call_ui = Util->Offset(0x78CE7300 - 0x78A30000);
    DWORD call_send = Util->Offset(0x78CE7510 - 0x78A30000);

    DWORD params = Util->Offset(0x7A2C5100 - 0x78A30000);

    __asm {
        pushad;
        pushfd;

        lea ecx, message;
        call call_init;

        push 0x21;
        lea eax, buff;
        push eax;
        lea eax, img;
        push eax;
        lea eax, data;
        push eax;
        lea edi, to;
        push edi;
        lea edx, from;
        lea ecx, message;
        call call_ui;
        add esp, 0x14;

        push params;
        push params;
        lea ecx, message;
        call call_send;
        add esp, 0x8;

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
        } else if (msg.pSender->GetName() == TEXT("SendMini")) {
            wstring wxid = GWin->GetControlValue(TEXT("Wxid"));
            wstring mini = GWin->GetControlValue(TEXT("MiniData"));
            wstring pic = GWin->GetControlValue(TEXT("PicData"));

            wstring xml = L"<msg>    <fromusername>wxid_c2klxikonw522</fromusername>    <scene>0</scene>    <commenturl></commenturl>    <appmsg appid=\"wx77af438b3505c00e\" sdkver=\"\">        <title>美团优选</title>        <des></des>        <action>view</action>        <type>33</type>        <showtype>0</showtype>        <content></content>        <url>https://mp.weixin.qq.com/mp/waerrpage?appid=wx77af438b3505c00e&amp;amp;type=upgrade&amp;amp;upgradetype=3#wechat_redirect</url>        <dataurl></dataurl>        <lowurl></lowurl>        <lowdataurl></lowdataurl>        <recorditem>            <![CDATA[]]>        </recorditem>        <thumburl></thumburl>        <messageaction></messageaction>        <extinfo></extinfo>        <sourceusername></sourceusername>        <sourcedisplayname>美团优选 果蔬肉禽蛋日用百货</sourcedisplayname>        <commenturl></commenturl>        <appattach>            <totallen>0</totallen>            <attachid></attachid>            <emoticonmd5></emoticonmd5>            <fileext>jpg</fileext>            <cdnthumburl>304e02010004473045020100020420d051d802032f50e502048e31227502046060c6f5042033616630663130393438303361643533386639396434316635643336373764320204010808030201000400</cdnthumburl>            <cdnthumblength>2119</cdnthumblength>            <cdnthumbheight>100</cdnthumbheight>            <cdnthumbwidth>100</cdnthumbwidth>            <aeskey>f8699453796b5d90d034bb15c3286409</aeskey>            <cdnthumbaeskey>f8699453796b5d90d034bb15c3286409</cdnthumbaeskey>            <encryver>1</encryver>            <cdnthumblength>2119</cdnthumblength>            <cdnthumbheight>100</cdnthumbheight>            <cdnthumbwidth>100</cdnthumbwidth>        </appattach>        <weappinfo>            <pagepath>pages/index/index.html?from=from_share_pages/index/index&amp;reqid=1616955119942-210&amp;sappnm=meituanyouxuan_wxapp&amp;scid=c_youxuan_shouye&amp;shareid=0-1616955119942-91939989&amp;spoi=0&amp;shareid=0-1616955119941-17155207&amp;share_type=url</pagepath>            <username>gh_84b9766b95bc@app</username>            <appid>wx77af438b3505c00e</appid>            <type>2</type>            <weappiconurl>http://mmbiz.qpic.cn/mmbiz_png/Vno5Z1HYSKMz6QPx1F0uRU20S0rN3K3G7ezVibkJOFm4WeeqhiaR1MVa3rQ4AuxiaoYzpxQWQMne6AZI3nCCqsVaQ/640?wx_fmt=png&amp;wxfrom=200</weappiconurl>            <appservicetype>0</appservicetype>            <shareId>2_wx77af438b3505c00e_822743083_1616955122_1</shareId>        </weappinfo>        <websearch />        <finderFeed>            <objectId>0</objectId>            <objectNonceId>0</objectNonceId>            <feedType>-1</feedType>            <nickname></nickname>            <username></username>            <avatar></avatar>            <desc></desc>            <mediaCount>0</mediaCount>            <localId>0</localId>            <mediaList />        </finderFeed>        <finderLive>            <finderLiveID>0</finderLiveID>            <finderUsername></finderUsername>            <finderObjectID>0</finderObjectID>            <nickname></nickname>            <desc></desc>            <finderNonceID>0</finderNonceID>            <headUrl></headUrl>            <liveStatus>-1</liveStatus>            <media>                <thumbUrl></thumbUrl>                <videoPlayDuration>0</videoPlayDuration>                <url></url>                <coverUrl></coverUrl>                <height>0</height>                <width>0</width>                <mediaType>-1</mediaType>            </media>        </finderLive>    </appmsg>    <appinfo>        <version>1</version>        <appname>Window wechat</appname>    </appinfo></msg>";

            SendMini(wxid, xml, pic);
            
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
            //
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

