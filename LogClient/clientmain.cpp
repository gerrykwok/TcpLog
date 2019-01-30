//////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <stdio.h>
#pragma comment(lib, "ws2_32")
#include "src/TcpLog.h"

//int WINAPI WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in LPSTR lpCmdLine, __in int nShowCmd)
int main(int argc, char **argv)
{
	TcpLog_init("127.0.0.1", 1104);
	int nTick = 0;
	char msg[1024];
	while(1)
	{
		sprintf(msg, "tick:%d", ++nTick);
		printf("do log\n");
		TcpLog_send(TcpLog_LEVEL_WARN, "logclient", "client", msg);
		Sleep(1000);
	}
	return 0;
}