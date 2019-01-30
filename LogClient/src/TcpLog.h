//////////////////////////////////////////////////////////////////////////
#pragma once

void TcpLog_init(const char *serverIp, int serverPort);
void TcpLog_send(const char *appid, const char *msg);
