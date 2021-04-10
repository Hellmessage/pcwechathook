#include <time.h>
#include "HTcp.h"
#pragma comment(lib,"ws2_32.lib")

using namespace std;
using namespace neb;

static HTcp* GInstance = NULL;

HTcp::HTcp() :mSocket(NULL) {
	WORD version = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(version, &data) != 0) {
		MessageBox(NULL, TEXT("无法创建WSA 2.2"), TEXT("温馨提示"), MB_OK);
	}
}

HTcp::~HTcp() {

}

DWORD WINAPI TcpLogicProc(PVOID context) {
	HTcp* Client = HTcp::GetInstance();
	while (!Client->mExit && Client->mWartung) {
		if (!Client->mConnect) {
			if (!Client->Connect(Client->mIP, Client->mPort)) {
				OutputDebugString(L"HTCP 无法连接服务器,将在5秒后重试...");
				Sleep(5000);
				continue;
			}
		}
		OutputDebugString(L"HTCP 客户端已建立连接");
		while (Client->mConnect) {
			string read = Client->Read();
			if (read.empty()) {
				continue;
			}
			CJsonObject json;
			//json.Parse(HTools::u2g(read));
			json.Parse(read);
			int code;
			if (json.Get("code", code)) {
				if (code == HTCP_HEARTBEAT) {
					Client->HeartBeat();
				} else {
					if (Client->mProc != NULL) {
						Client->mProc(json);
					}
				}
			}
		}
	}
	return 0;
}

void HTcp::StartWartung(const char* ip, unsigned short port, HTcpNotifyProc proc) {
	if (!mWartung) {
		mWartung = true;
		mProc = proc;
		mIP = ip;
		mPort = port;
		HANDLE thread = CreateThread(NULL, 0, TcpLogicProc, NULL, 0, NULL);
		if (thread != NULL) {
			CloseHandle(thread);
		}
	}
}

void HTcp::StopWartung() {
	mWartung = false;
}

BOOL HTcp::Connect(const char* ip, unsigned short port) {
	mConnect = FALSE;
	if (mSocket == NULL) {
		mSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (mSocket == INVALID_SOCKET) {
			return FALSE;
		}
	}
	sockaddr_in cin;
	cin.sin_family = AF_INET;
	cin.sin_port = htons(port);
	cin.sin_addr.s_addr = inet_addr(ip);
	SetBlock(false);
	fd_set set;
	if (connect(mSocket, (sockaddr*)&cin, sizeof(cin)) != 0) {
		FD_ZERO(&set);
		FD_SET(mSocket, &set);
		timeval to;
		to.tv_sec = 0;
		to.tv_usec = mTimeout * 1000;
		if (select(mSocket + 1, 0, &set, 0, &to) <= 0) {
			return FALSE;
		}
	}
	SetBlock(true);
	time_t t;
	time(&t);
	mLastHeartBeat = (int)t;
	mConnect = TRUE;
	return TRUE;
}

BOOL HTcp::SetBlock(bool block) {
	if (mSocket <= 0) {
		return false;
	}
	u_long ul = 0L;
	if (!block) {
		ul = 1;
	}
	ioctlsocket(mSocket, FIONBIO, &ul);
	return true;
}

void HTcp::Close() {
	mConnect = FALSE;
	if (mSocket > 0) {
		closesocket(mSocket);
		mSocket = NULL;
	}
}

BOOL HTcp::Registry(string wxid) {
	CJsonObject reg;
	reg.Add("wxid", wxid);
	reg.Add("pid", (int)GetCurrentProcessId());
	return Send(HTCP_REGISTRY, reg);
}

BOOL HTcp::HeartBeat() {
	time_t t;
	time(&t);
	mLastHeartBeat = (int)t;
	CJsonObject data;
	data.Add("code", HTCP_HEARTBEAT);
	return Send(data.ToString());
}

BOOL HTcp::Send(int code, CJsonObject content) {
	CJsonObject data;
	data.Add("code", code);
	data.Add("data", content);
	return Send(data.ToString());
}

BOOL HTcp::Send(string json) {
	hlock(mMutex) {
		int len = json.size();
		if (len > 0) {
			int res = send(mSocket, (char *)json.size(), 4, 0);
			if (res != SOCKET_ERROR) {
				res = send(mSocket, json.c_str(), len, 0);
				return res != SOCKET_ERROR;
			}
		}
	}
	return FALSE;
}

string HTcp::Read() {
	string str;
	int len = 0;
	if (Read((CHAR*)&len, 4)) {
		if (len > 0) {
			char* buffer = new char[len + 1];
			if (Read(buffer, len)) {
				buffer[len] = '\0';
				str.append(buffer);
			}
			delete[] buffer;
		}
	}
	return str;
}

BOOL HTcp::Read(char* buffer, int len) {
	while (len > 0) {
		int ret = recv(mSocket, buffer, len, 0);
		if (ret == SOCKET_ERROR) {
			Close();
			return false;
		}
		len = len - ret;
		buffer += ret;
	}
	return true;
}

DWORD WINAPI TcpTimeoutProc(PVOID context) {
	while (!GInstance->mExit) {
		if (GInstance->mConnect) {
			if (GInstance->mLastHeartBeat > 0) {
				time_t t;
				time(&t);
				int timeout = (int)t - GInstance->mLastHeartBeat;
				if (timeout > GInstance->mTimeout) {
					GInstance->Close();
					OutputDebugString(TEXT("HTCP 15秒未响应[判断掉线]..."));
				}
			}
		}
		Sleep(100);
	}
	return 0;
}

HTcp* HTcp::GetInstance() {
	if (GInstance == NULL) {
		GInstance = new HTcp();
		HANDLE thread = CreateThread(NULL, 0, TcpTimeoutProc, NULL, 0, NULL);
		if (thread != NULL) {
			CloseHandle(thread);
		}
	}
	return GInstance;
}

void HTcp::Exit() {
	mExit = true;
	StopWartung();
	Close();
}
