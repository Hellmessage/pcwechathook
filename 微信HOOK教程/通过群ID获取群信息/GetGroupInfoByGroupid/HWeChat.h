#pragma once
#include "pch.h"
#include "HTools.h"
#include "HUtil.h"



typedef const UNICODE_STRING* PCUNICODE_STRING;

typedef struct _LDR_DLL_LOADED_NOTIFICATION_DATA {
	ULONG Flags;
	PCUNICODE_STRING FullDllName;
	PCUNICODE_STRING BaseDllName;
	PVOID DllBase;
	ULONG SizeOfImage;
} LDR_DLL_LOADED_NOTIFICATION_DATA, * PLDR_DLL_LOADED_NOTIFICATION_DATA;

typedef struct _LDR_DLL_UNLOADED_NOTIFICATION_DATA {
	ULONG Flags;
	PCUNICODE_STRING FullDllName;
	PCUNICODE_STRING BaseDllName;
	PVOID DllBase;
	ULONG SizeOfImage;
} LDR_DLL_UNLOADED_NOTIFICATION_DATA, * PLDR_DLL_UNLOADED_NOTIFICATION_DATA;

typedef union _LDR_DLL_NOTIFICATION_DATA {
	LDR_DLL_LOADED_NOTIFICATION_DATA Loaded;
	LDR_DLL_UNLOADED_NOTIFICATION_DATA Unloaded;
} LDR_DLL_NOTIFICATION_DATA, * PLDR_DLL_NOTIFICATION_DATA;

typedef const PLDR_DLL_NOTIFICATION_DATA PCLDR_DLL_NOTIFICATION_DATA;

typedef VOID(NTAPI* PLDR_DLL_NOTIFICATION_FUNCTION)(ULONG NotificationReason, PCLDR_DLL_NOTIFICATION_DATA NotificationData, PVOID Context);
typedef NTSTATUS(NTAPI* PfnLdrRegisterDllNotification)(ULONG Flags, PLDR_DLL_NOTIFICATION_FUNCTION NotificationFunction, void* Context, void** Cookie);
typedef NTSTATUS(NTAPI* PfnLdrUnregisterDllNotification)(void* Cookie);


#define LDR_DLL_NOTIFICATION_REASON_LOADED			1 
#define LDR_DLL_NOTIFICATION_REASON_UNLOADED		2

typedef struct _WXLINK {
	struct _WXLINK* p;
	struct _WXLINK* n;
	struct _WXLINK* d;
} WxLink;

typedef struct _WXSTRING {
	wchar_t* pstr;
	int len;
	int maxLen;
	int fill1 = 0;
	int fill2 = 0;
} WxString, * PWxString;

typedef struct _WXUSERINFO {
	char top[0x8];
	WxString wxid;
	WxString account;
	WxString vData;
	int un_0;
	int un_1;
	char un_2[0x18];
	WxString name;
	char un_3[0x28];
	int un_4;
	WxString nameUpper;
	WxString nameUn;
	char un_5[0x28];
	WxString pic;
	WxString img;
	char* md5;
	char un_6[0xC];
	int un_7;
	int addSource;
	char un_8[0x2AC];
} WxUserInfo, * PWxUserInfo;


typedef struct _WXGROUPINFO {
	char top[0x4];
	WxString groupid;
	char* member;
	char un_0[0xc];
	int un_1;
	int un_2;
	WxString un_3;
	int un_4;
	WxString admin;
	char buff[0x118];
} WxGroupInfo, * PWxGroupInfo;


typedef struct _WXMESSAGE {
	char fill[0x24C];
	WxLink* link;
	char fill2[0x20];
	char fill3[0x68];
} WxMessage;

typedef struct _WXDBPASS {
	char* pass;
	int len;
} WxDbPass;

typedef struct _WXATWXID {
	WxString* wxidArr;
	DWORD end;
	DWORD end2;
	int fill = 0;
} WxAtWxid;

VOID NTAPI LdrDllNotification(ULONG NotificationReason, PCLDR_DLL_NOTIFICATION_DATA NotificationData, PVOID Context);