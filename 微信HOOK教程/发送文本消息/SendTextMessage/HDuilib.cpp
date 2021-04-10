#include "pch.h"
#include "HDuilib.h"
#include "HTools.h"

#pragma comment(lib, "DuiLib_d.lib")

using namespace std;


bool InitDuilib(HMODULE module) {
	HRESULT Hr = CoInitialize(NULL);
	if (FAILED(Hr)) return FALSE;
	CPaintManagerUI::SetInstance((HINSTANCE)module);
	CPaintManagerUI::SetResourceDll((HINSTANCE)module);

	REGIST_DUICONTROL(CVBoxUI);
	REGIST_DUICONTROL(CHBoxUI);

	return TRUE;
}



CPaintManagerUI CBaseWnd::GetPM() {
	return m_pm;
}

CControlUI* CBaseWnd::CreateControl(LPCTSTR name) {
	CControlUI* ui = nullptr;
	CDialogBuilder builder;
	if (wcscmp(name, TEXT("HEHeader")) == 0) {
		ui = builder.Create(IDX_HEADER, TEXT("xml"));
	} else if (wcscmp(name, TEXT("HEFooter")) == 0) {
		ui = builder.Create(IDX_FOOTER, TEXT("xml"));
	}
	return ui;
}

void CBaseWnd::InitResource() {

}

LPVOID CVBoxUI::GetInterface(LPCTSTR name) {
	if (wcscmp(name, TEXT("VBox")) == 0) {
		return static_cast<CVerticalLayoutUI*>(this);
	}
	return CVerticalLayoutUI::GetInterface(name);
}

LPVOID CHBoxUI::GetInterface(LPCTSTR name) {
	if (wcscmp(name, TEXT("HBox")) == 0) {
		return static_cast<CHorizontalLayoutUI*>(this);
	}
	return CHorizontalLayoutUI::GetInterface(name);
}






DUI_BEGIN_MESSAGE_MAP(HDuilib, WindowImplBase)

DUI_END_MESSAGE_MAP()

HDuilib::HDuilib(HDuilibNotify notify) {
	mNotifyProc = notify;
}

wstring HDuilib::GetControlValue(wstring name) {
	wstring str;
	CControlUI* view = GetPM()->FindControl(name.c_str());
	if (view != NULL) {
		CDuiString value = view->GetText();
		wstring temp = value.GetData();
		if (HTools::EndWith(temp.c_str(), L"\n")) {
			temp.pop_back();
		}
		str = temp;
	}
	return str;
}

CPaintManagerUI* HDuilib::GetPM() {
	return m_PainManager;
}

LPCTSTR HDuilib::GetWindowClassName() const {
	return TEXT("HWeChatMainWin");
}

CDuiString HDuilib::GetSkinFile() {
	return HTools::itow(IDX_MAIN).c_str();
}

CDuiString HDuilib::GetSkinFolder() {
	return TEXT("");
}

CDuiString HDuilib::GetSkinType() {
	return TEXT("xml");
}

void HDuilib::Notify(TNotifyUI& msg) {
	if (mNotifyProc != NULL) {
		mNotifyProc(msg);
	}
}

LRESULT HDuilib::HandleMessage(UINT msg, WPARAM wp, LPARAM lp) {
	return __super::HandleMessage(msg, wp, lp);
}

