//////////////////////////////////////////////////////////////////////////
//https://www.cnblogs.com/cxq0017/p/6497352.html
#include "TcpSocket.h"
#include <code/PC/DBG_func.h>
#include <vector>

TcpSocket::TcpSocket()
{
	m_socketListen = INVALID_SOCKET;
}

TcpSocket::~TcpSocket()
{
}

bool TcpSocket::start(int nPort, const ONDATAFUNC &onData, const ONLOGFUNC &onLog)
{
	m_onData = onData;
	m_onLog = onLog;

	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		return false;
	}
	m_socketListen = socket(AF_INET, SOCK_STREAM, 0);
	if(m_socketListen == INVALID_SOCKET)
	{
		goto flagFailed;
	}

	sockaddr_in local;
	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_port = htons(nPort);
	if(bind(m_socketListen, (sockaddr*)&local, sizeof(local)) != 0)
	{
		goto flagFailed;
	}
	if(m_onLog != nullptr)
	{
		char msg[1024];
		sprintf(msg, "listening on port %d", nPort);
		m_onLog(msg);
	}
	if(listen(m_socketListen, SOMAXCONN) != 0)
	{
		goto flagFailed;
	}
	DWORD dwThread;
	HANDLE hThread;
	hThread = CreateThread(NULL, 0, Thread_Accept, this, 0, &dwThread);
	CloseHandle(hThread);
	return true;
flagFailed:
	if(m_socketListen != INVALID_SOCKET)
	{
		closesocket(m_socketListen);
		m_socketListen = INVALID_SOCKET;
	}
	WSACleanup();
	return false;
}

void TcpSocket::stop()
{
	if(m_socketListen != INVALID_SOCKET)
	{
		closesocket(m_socketListen);
		m_socketListen = INVALID_SOCKET;
	}
}

bool TcpSocket::isStarted()
{
	return m_socketListen != INVALID_SOCKET;
}

DWORD TcpSocket::Thread_Accept(void* param)
{
	TcpSocket *inst = (TcpSocket*)param;
	inst->onAccept();
	return 0;
}

typedef struct
{
	TcpSocket *m_pThis;
	SOCKET m_socket;
} THREAD_RECV_CONTEXT;

DWORD TcpSocket::Thread_Recv(void *param)
{
	THREAD_RECV_CONTEXT *ctx = (THREAD_RECV_CONTEXT*)param;
	ctx->m_pThis->onRecv(ctx->m_socket);
	return 0;
}

void TcpSocket::onAccept()
{
	int i;
	std::vector<SOCKET> allSockConn;
	SOCKET sockConn;
	SOCKADDR_IN addrClt;
	int len = sizeof(SOCKADDR);
	THREAD_RECV_CONTEXT ctx;
	memset(&addrClt, 0, len);
	if(m_onLog != nullptr)
		m_onLog("server waiting for connection...");
	while(1)
	{
		sockConn = accept(m_socketListen, (struct sockaddr*)&addrClt, &len);
		if(sockConn == INVALID_SOCKET)
			break;
		dbg_TRACEA("someone connect:%d.%d.%d.%d:%d(%d)\n",
			addrClt.sin_addr.S_un.S_un_b.s_b1, addrClt.sin_addr.S_un.S_un_b.s_b2, addrClt.sin_addr.S_un.S_un_b.s_b3, addrClt.sin_addr.S_un.S_un_b.s_b4,
			ntohs(addrClt.sin_port), addrClt.sin_port);
		allSockConn.push_back(sockConn);
		ctx.m_pThis = this;
		ctx.m_socket = sockConn;
		DWORD dwThread;
		HANDLE hThread;
		hThread = CreateThread(NULL, 0, Thread_Recv, &ctx, 0, &dwThread);
		CloseHandle(hThread);
	}
	int n = allSockConn.size();
	for(i = 0; i < n; ++i)
	{
		closesocket(allSockConn[i]);
	}
	if(m_onLog != nullptr)
		m_onLog("thread accept terminated");
}

void TcpSocket::onRecv(SOCKET sock)
{
	dbg_TRACEA("recv thread started\n");
	int maxBuff = 4 * 1024 * 1024;
	char *buff = new char[maxBuff];
	int ret;
	while(1)
	{
		ret = recv(sock, buff, sizeof(buff), 0);
		if(ret == SOCKET_ERROR)
			break;
		if(ret == 0)
			break;
		if(ret > 0 && m_onData != nullptr)
		{
			m_onData(sock, buff, ret);
		}
	}
	closesocket(sock);
	delete[] buff;
	dbg_TRACEA("recv thread terminated\n");
}
