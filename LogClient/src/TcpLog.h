//////////////////////////////////////////////////////////////////////////
#pragma once

#define TcpLog_LEVEL_DEFAULT			0
#define TcpLog_LEVEL_VERBOSE			1
#define TcpLog_LEVEL_DEBUG				2
#define TcpLog_LEVEL_INFO				3
#define TcpLog_LEVEL_WARN				4
#define TcpLog_LEVEL_ERROR				5

void TcpLog_init(const char *serverIp, int serverPort);
void TcpLog_send(int level, const char *app, const char *tag, const char *text);
