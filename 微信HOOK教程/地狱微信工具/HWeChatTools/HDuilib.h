#pragma once
#include <string>
using namespace DuiLib;

bool InitDuilib(HMODULE module);

class CBaseWnd : public WindowImplBase {
public:
	CPaintManagerUI GetPM();

	virtual CControlUI* CreateControl(LPCTSTR name);
	virtual void InitResource();
};

class CVBoxUI : public CVerticalLayoutUI {
public:
	virtual LPVOID GetInterface(LPCTSTR name);
};

class CHBoxUI : public CHorizontalLayoutUI {
public:
	virtual LPVOID GetInterface(LPCTSTR name);
};


typedef void(*HDuilibNotify)(TNotifyUI& msg);

class HDuilib : public CBaseWnd {
public:
	HDuilib(HDuilibNotify notify);

	std::wstring GetControlValue(std::wstring name);


	CPaintManagerUI* GetPM();
protected:
	LPCTSTR GetWindowClassName() const;
	CDuiString GetSkinFile();
	CDuiString GetSkinFolder();
	CDuiString GetSkinType();
	void Notify(TNotifyUI& msg);
	LRESULT HandleMessage(UINT msg, WPARAM wp, LPARAM lp);
private:
	HDuilibNotify mNotifyProc = NULL;
public:
	DUI_DECLARE_MESSAGE_MAP()
};
