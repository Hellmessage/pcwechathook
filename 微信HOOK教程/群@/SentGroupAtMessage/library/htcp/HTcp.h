#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <string>
#include <WinSock2.h>
#include "../cjson/CJsonObject.h"
#include "../hlock/HLock.h"


#define HTCP_HEARTBEAT			0
#define HTCP_REGISTRY			90000

typedef void (*HTcpNotifyProc)(neb::CJsonObject);

class HTcp {
public:
	HTcp();
	virtual ~HTcp();

	static HTcp* GetInstance();

	void StartWartung(const char* ip, unsigned short port, HTcpNotifyProc proc);
	void StopWartung();

	BOOL Connect(const char* ip, unsigned short port);

	BOOL Registry(std::string wxid);
	BOOL HeartBeat();

	BOOL Send(int code, neb::CJsonObject content);
	BOOL Send(std::string json);

	std::string Read();

	void Close();
	void Exit();
private:
	BOOL SetBlock(bool block);
	
	BOOL Read(char* buffer, int len);
	

private:
	SOCKET mSocket = NULL;
	HMutex mMutex;
public:
	const char* mIP = "127.0.0.1";
	int mPort = 6174;
	BOOL mConnect = FALSE;
	int mLastHeartBeat = 0;
	int mTimeout = 15;
	BOOL mWartung = false;
	BOOL mExit = false;
	HTcpNotifyProc mProc = NULL;
};