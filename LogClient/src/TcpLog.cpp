//////////////////////////////////////////////////////////////////////////
#include "TcpLog.h"
#include <code/PC/DBG_func.h>

static SOCKET g_socket = INVALID_SOCKET;
static char *g_buffer = NULL;
static int g_bufSize = 0;

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

static int TcpLog_fillWithByte(char *buffer, int offset, int value)
{
	*((unsigned char*)(buffer + offset)) = value;
	return offset + 1;
}

static int TcpLog_fillWithInt(char *buffer, int offset, int value)
{
	*((int*)(buffer + offset)) = value;
	return offset + 4;
}

static int TcpLog_fillWithString(char *buffer, int offset, const char *value)
{
	int len = strlen(value);
	memcpy(buffer + offset, value, len);
	buffer[offset+len] = 0;
	return offset + len + 1;
}

void TcpLog_send(int level, const char *app, const char *tag, const char *text)
{
	if(g_socket == INVALID_SOCKET)
		return;
	int sizeApp = strlen(app);
	int sizeTag = strlen(tag);
	int sizeText = strlen(text);

	int bufSize = sizeof(int) + sizeApp+1 + sizeTag+1 + sizeText+1;
	if(bufSize > g_bufSize)
	{
		if(g_buffer) free(g_buffer);
		g_buffer = (char*)malloc(bufSize);
		g_bufSize = bufSize;
	}
	int offset = 0;
	offset = TcpLog_fillWithByte(g_buffer, offset, level);
	offset = TcpLog_fillWithString(g_buffer, offset, app);
	offset = TcpLog_fillWithString(g_buffer, offset, tag);
	offset = TcpLog_fillWithString(g_buffer, offset, text);

	send(g_socket, (char*)&bufSize, 4, 0);
	send(g_socket, g_buffer, bufSize, 0);
}
