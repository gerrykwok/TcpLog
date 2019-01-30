//////////////////////////////////////////////////////////////////////////
#include "TcpLog.h"
#include <code/PC/DBG_func.h>

static SOCKET g_socket = INVALID_SOCKET;

void TcpLog_init(const char *serverIp, int serverPort)
{
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		return;
	}
	g_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(g_socket == INVALID_SOCKET)
	{
		goto flagFailed;
	}
	SOCKADDR_IN addrSrv;
	memset(&addrSrv, 0, sizeof(addrSrv));
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_addr.s_addr = inet_addr(serverIp);
	addrSrv.sin_port = htons(serverPort);
	
	if(connect(g_socket, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)) != 0)
	{
		dbg_TRACEA("connect to %s:%d failed\n", serverIp, serverPort);
		goto flagFailed;
	}
	return;

flagFailed:
	if(g_socket != INVALID_SOCKET)
	{
		closesocket(g_socket);
		g_socket = INVALID_SOCKET;
	}
	WSACleanup();
}

void TcpLog_send(const char *appid, const char *msg)
{
	if(g_socket == INVALID_SOCKET)
		return;
	send(g_socket, msg, strlen(msg), 0);
}
