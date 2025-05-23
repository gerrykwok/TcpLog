//////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <map>
#include <string>
#include <code/PC/DBG_func.h>
#include <code/PC/MyListViewW.h>
#include "TcpSocket.h"
#include "resource.h"

#define SVR_COUNTOF(arr)		sizeof(arr)/sizeof(arr[0])

#define SVR_LOG_DEFAULT			0
#define SVR_LOG_VERBOSE			1
#define SVR_LOG_DEBUG			2
#define SVR_LOG_INFO			3
#define SVR_LOG_WARN			4
#define SVR_LOG_ERROR			5

const wchar_t *SVR_levelDesc[] =
{
	L"",
	L"V",
	L"D",
	L"I",
	L"W",
	L"E",
};

const COLORREF SVR_levelColor[] =
{
	RGB(255, 255, 255),
	RGB(0, 0, 0),
	RGB(0, 0, 127),
	RGB(0, 127, 0),
	RGB(255, 127, 0),
	RGB(255, 0, 0),
};

typedef struct
{
	char level;
	char *app;
	char *tag;
	char *text;
} SVR_LOG;

typedef struct
{
	void *addr;
	int size;
	int sizeValid;
} SVR_BUFFER;

TcpSocket g_tcp;
HWND g_hDlg;
IMyListViewW *g_listview;
typedef std::map<SOCKET, SVR_BUFFER> CONN_DATA;
CONN_DATA g_allData;
static BOOL SVR_DlgProc(HWND hDlg, UINT uMsg, WPARAM wparam, LPARAM lparam);
void SVR_OnInitDialog();
void SVR_OnStartStop();
void SVR_OnSize();
void SVR_OnNotify(NMHDR *pHDR);
void SVR_OnClearLog();
void SVR_addLog(int level, const char *app, const char *tag, const char *text);
double SVR_getSystemScale();

int WINAPI WinMain( __in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in LPSTR lpCmdLine, __in int nShowCmd )
{
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, (DLGPROC)SVR_DlgProc);
	return 0;
}

BOOL SVR_DlgProc(HWND hDlg, UINT uMsg, WPARAM wparam, LPARAM lparam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		g_hDlg = hDlg;
		SVR_OnInitDialog();
		SVR_OnSize();
		break;
	case WM_CLOSE:
		::EndDialog(hDlg, 0);
		return TRUE;
	case WM_COMMAND:
		if(HIWORD(wparam) == BN_CLICKED) switch(LOWORD(wparam))
		{
		case IDOK:
//			::EndDialog(hDlg, 0);
			return TRUE;
		case IDCANCEL:
//			::EndDialog(hDlg, 0);
			return TRUE;
		case IDC_STARTSTOP:
			SVR_OnStartStop();
			break;
		case IDC_CLEAR:
			SVR_OnClearLog();
			break;
		case IDC_TOPMOST:
			if(IsDlgButtonChecked(hDlg, IDC_TOPMOST) == BST_CHECKED)
			{
				SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
			}
			else
			{
				SetWindowPos(hDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
			}
			break;
		default:break;
		}
		break;
	case WM_SIZE:
		SVR_OnSize();
		break;
	case WM_NOTIFY:
		SVR_OnNotify((NMHDR*)lparam);
		break;
	default: break;
	}
	return FALSE;
}

void SVR_OnInitDialog()
{
	::SendMessage(g_hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON_MAIN)));
	SetDlgItemTextA(g_hDlg, IDC_PORT, "1104");
	gwjListViewCreate(IID_IMyListViewW, (void**)&g_listview, NULL);
	g_listview->Attach01(g_hDlg, IDC_LIST_LOG, LVS_EX_FULLROWSELECT);
	g_listview->SetSelectedBkColor(RGB(154, 182, 205));
	g_listview->AppendColumn2W(L"level", 25, LVCFMT_LEFT);
	g_listview->AppendColumn2W(L"time", 150, LVCFMT_LEFT);
	g_listview->AppendColumn2W(L"app", 100, LVCFMT_LEFT);
	g_listview->AppendColumn2W(L"tag", 50, LVCFMT_LEFT);
	g_listview->AppendColumn2W(L"text", 500, LVCFMT_LEFT);
	CheckDlgButton(g_hDlg, IDC_SCROLL_BOTTOM, BST_CHECKED);
}

void SVR_addLog(int level, const char *app, const char *tag, const char *text)
{
	wchar_t wsz[2048];
	SYSTEMTIME t;
	GetLocalTime(&t);
	swprintf_s(wsz, SVR_COUNTOF(wsz), L"%d-%02d-%02d,%02d:%02d:%02d.%03d", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);

	int iItem;
	iItem = g_listview->AppendItemW(SVR_levelDesc[level]);

	g_listview->SetLineColor(iItem, SVR_levelColor[level], SVR_levelColor[level]);
	g_listview->SetLineTxFormat(iItem, DT_LEFT);

	g_listview->SetItemTextW(iItem, 1, wsz);
	MultiByteToWideChar(CP_UTF8, 0, app, -1, wsz, SVR_COUNTOF(wsz));
	g_listview->SetItemTextExW(iItem, 2, L"%s", wsz);
	MultiByteToWideChar(CP_UTF8, 0, tag, -1, wsz, SVR_COUNTOF(wsz));
	g_listview->SetItemTextExW(iItem, 3, L"%s", wsz);
	MultiByteToWideChar(CP_UTF8, 0, text, -1, wsz, SVR_COUNTOF(wsz));
	g_listview->SetItemTextExW(iItem, 4, L"%s", wsz);

	RECT rt;
	if(IsDlgButtonChecked(g_hDlg, IDC_SCROLL_BOTTOM) == BST_CHECKED && g_listview->GetItemRect(0, &rt, LVIR_BOUNDS)) {
		ListView_Scroll(g_listview->GetHWND(), 0, (g_listview->GetItemCount() + 1) * (rt.bottom - rt.top));
	}
}

void SVR_OnClearLog()
{
	g_listview->DeleteAllItems();
}

void SVR_OnStartStop()
{
	bool isStarted = g_tcp.isStarted();
	if(!isStarted)
	{
		int port = GetDlgItemInt(g_hDlg, IDC_PORT, NULL, FALSE);
		if(port != 0)
		{
			ONDATAFUNC funcOnData = [=](SOCKET sock, const char *data, int datasize) {
//				dbg_TRACEA("datasize=%d\n", datasize);
				CONN_DATA::iterator it = g_allData.find(sock);
				if(it == g_allData.end())
				{
					SVR_BUFFER buf;
					buf.size = 32;
					buf.sizeValid = 0;
					buf.addr = malloc(buf.size);
					g_allData[sock] = buf;
					it = g_allData.find(sock);
				}
				SVR_BUFFER &buff = it->second;
				if(buff.sizeValid + datasize > buff.size)
				{
					buff.addr = realloc(buff.addr, buff.sizeValid + datasize);
					buff.size = buff.sizeValid + datasize;
				}
				memcpy((char*)buff.addr + buff.sizeValid, data, datasize);
				buff.sizeValid += datasize;
				if(buff.sizeValid >= 4)
				{
					int pkgSize = *((int*)buff.addr);
					if(buff.sizeValid >= pkgSize+4)//够一个包了
					{
						SVR_LOG oneLog;
						oneLog.level = *((char*)buff.addr + 4);
						oneLog.app = (char*)buff.addr + 4 + 1;
						oneLog.tag = oneLog.app + strlen(oneLog.app) + 1;
						oneLog.text = oneLog.tag + strlen(oneLog.tag) + 1;
						SVR_addLog(oneLog.level, oneLog.app, oneLog.tag, oneLog.text);

						memmove(buff.addr, (char*)buff.addr+(pkgSize+4), buff.sizeValid-(pkgSize+4));
						buff.sizeValid -= pkgSize+4;
					}
				}
			};
			ONLOGFUNC funcOnLog = [=](const char *msg) {
				SVR_addLog(SVR_LOG_INFO, "server", "", msg);
			};
			g_tcp.start(port, funcOnData, funcOnLog);
			::SetDlgItemTextA(g_hDlg, IDC_STARTSTOP, "stop");
		}
	}
	else
	{
		g_tcp.stop();
		::SetDlgItemTextA(g_hDlg, IDC_STARTSTOP, "start");
	}
}

static void SVR_PlaceCtrlLeftCenter(HWND hCtrl, int x, int y) {
	RECT rt;
	GetWindowRect(hCtrl, &rt);
	SetWindowPos(hCtrl, NULL, x, y-(rt.bottom-rt.top)/2, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
}

void SVR_OnSize()
{
	HWND hListView = g_listview->GetHWND();
	RECT rt;
	POINT pt;
	RECT rtDlg;
	double scale = SVR_getSystemScale();
	GetClientRect(g_hDlg, &rtDlg);
	GetWindowRect(hListView, &rt);
	pt.x = (LONG)(10 * scale);
	pt.y = (LONG)(40 * scale);
	MoveWindow(hListView, pt.x, pt.y, rtDlg.right-rtDlg.left-pt.x-pt.x, rtDlg.bottom-rtDlg.top-pt.y-pt.x, TRUE);

	int y = (int)(20 * scale);
	SVR_PlaceCtrlLeftCenter(GetDlgItem(g_hDlg, IDC_STATIC_PORT), (int)(10 * scale), y);
	SVR_PlaceCtrlLeftCenter(GetDlgItem(g_hDlg, IDC_PORT), (int)(46 * scale), y);
	SVR_PlaceCtrlLeftCenter(GetDlgItem(g_hDlg, IDC_STARTSTOP), (int)(118 * scale), y);
	SVR_PlaceCtrlLeftCenter(GetDlgItem(g_hDlg, IDC_CLEAR), (int)(210 * scale), y);
	SVR_PlaceCtrlLeftCenter(GetDlgItem(g_hDlg, IDC_TOPMOST), (int)(300 * scale), y);
	HWND hWnd = GetDlgItem(g_hDlg, IDC_SCROLL_BOTTOM);
	GetWindowRect(hWnd, &rt);
	SVR_PlaceCtrlLeftCenter(hWnd, rtDlg.right - (rt.right - rt.left), y);
}

void SVR_OnNotify(NMHDR *pHDR) {
	if(pHDR->hwndFrom == g_listview->GetHWND() && pHDR->code == LVN_KEYDOWN) {
		NMLVKEYDOWN *pKeyDown = (NMLVKEYDOWN*)pHDR;
		DWORD pos;
		int i, j, iColStart, iColEnd;
		std::wstring str;
		wchar_t wsz[4096];
		HGLOBAL hf;
		LPWSTR lp;
		auto pList = g_listview;
		if(GetAsyncKeyState(VK_CONTROL)) switch(pKeyDown->wVKey)
		{
		case 'C'://复制选定的行
//			dbg_TRACEA("copy the log\n");
			pos = pList->GetFirstSelectedItemPosition();
			iColStart = 0;
			iColEnd = 5;
			while(pos)
			{
				i = pList->GetNextSelectedItem(pos);
				for(j = iColStart; j < iColEnd; ++j)
				{
					pList->GetItemTextW(i, j, wsz, sizeof(wsz)/sizeof(wsz[0]));
					str += wsz;
					if(j+1 < iColEnd)
						str += L"  ";
				}
				if(pos)
					str += L"\r\n";
			}
//			dbg_TRACEW(L"str=%s\n", str.c_str());
			hf = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, (str.length()+1) * sizeof(wchar_t));
			lp = (LPWSTR)GlobalLock(hf);
			memcpy(lp, str.c_str(), str.length()*sizeof(wchar_t));
			GlobalUnlock(hf);
			OpenClipboard(g_hDlg);
			EmptyClipboard();
			SetClipboardData(CF_UNICODETEXT, hf);
			CloseClipboard();
			break;
		default: break;
		}
	}
}

double SVR_getSystemScale()
{
	//https://blog.csdn.net/wzxiaodu/article/details/122338649?spm=1001.2101.3001.6661.1&utm_medium=distribute.pc_relevant_t0.none-task-blog-2%7Edefault%7ECTRLIST%7Edefault-1-122338649-blog-121206222.pc_relevant_multi_platform_whitelistv1&depth_1-utm_source=distribute.pc_relevant_t0.none-task-blog-2%7Edefault%7ECTRLIST%7Edefault-1-122338649-blog-121206222.pc_relevant_multi_platform_whitelistv1&utm_relevant_index=1
	UINT dpi = GetDpiForSystem();// GetDpiForWindow(GetDesktopWindow());
	UINT baseDpi = 96;
	double scale = (double)dpi / baseDpi;

	return scale;
}