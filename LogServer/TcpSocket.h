//////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <functional>

typedef std::function<void(SOCKET, const char *, int)> ONDATAFUNC;
typedef std::function<void(const char *)> ONLOGFUNC;

class TcpSocket
{
public:
	TcpSocket();
	~TcpSocket();
	bool start(int nPort, const ONDATAFUNC &onData, const ONLOGFUNC &onLog);
	void stop();
	bool isStarted();
protected:
	static DWORD WINAPI Thread_Accept(void* param);
	static DWORD WINAPI Thread_Recv(void *param);
	void onAccept();
	void onRecv(SOCKET sock);
private:
	SOCKET m_socketListen;
	ONDATAFUNC m_onData;
	ONLOGFUNC m_onLog;
};