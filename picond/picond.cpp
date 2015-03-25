#include "stdafx.h"
#include "picond.h"
#include <WinSock2.h>
#include <shellapi.h>
#include <list>
#include <string>
#include <locale>
#include <codecvt>
#include <memory>

#define MAX_LOADSTRING 128
#define WIN_PADDING 10
#define WIN_MARGIN  5

HINSTANCE hInst;
HWND      hMainWnd;
std::list<HWND> hWndList;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szMainWindowClass[MAX_LOADSTRING];
TCHAR szChildWindowClass[MAX_LOADSTRING];
NOTIFYICONDATA nid = { 0 };
UINT iWMTASKCREATE;
HFONT hFontBold;
HFONT hFontNormal;


// Util Functions
VOID				AddhWndList(HWND hWnd);
VOID				DeletehWndList(HWND hWnd);
VOID				ReorderWnd();

// Main Window
ATOM				MyRegisterMainClass(HINSTANCE hInstance);
BOOL				InitMainInstance(HINSTANCE, int);
LRESULT CALLBACK	MainWndProc(HWND, UINT, WPARAM, LPARAM);

// Child Window
ATOM				MyRegisterChildClass(HINSTANCE hInstance);
BOOL				InitChildInstance(int, LPTSTR);
LRESULT CALLBACK	ChildWndProc(HWND, UINT, WPARAM, LPARAM);

// Network Thread
VOID	CALLBACK	ThreadProc(LPVOID);


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HANDLE hThr;

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_PICONDMAIN, szMainWindowClass, MAX_LOADSTRING);
	LoadString(hInstance, IDC_PICONDCHILD, szChildWindowClass, MAX_LOADSTRING);
	
	MyRegisterMainClass(hInstance);
	MyRegisterChildClass(hInstance);

	// make list clean.
	hWndList.clear();


	// make main window
	if (!InitMainInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	// create some resources

	hFontBold = CreateFont(12, 0, 0, 0, FW_BOLD, false, false, false,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("MS Shell Dlg 2"));

	hFontNormal = CreateFont(12, 0, 0, 0, FW_NORMAL, false, false, false,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("MS Shell Dlg 2"));

	// make "picond started." window
	// todo: write some code...

	// network listen thread begin...
	hThr = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, 0, 0, NULL);

	if (hThr)
	{
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		DWORD dwRes = STILL_ACTIVE;
		while (dwRes == STILL_ACTIVE)
		{
			GetExitCodeThread(hThr, &dwRes);
			Sleep(1);
		}
	}

	DeleteObject(hFontBold);
	DeleteObject(hFontNormal);

	return (int)msg.wParam;
}

VOID AddhWndList(HWND hWnd)
{
	hWndList.push_back(hWnd);
}

VOID DeletehWndList(HWND hWnd)
{
	hWndList.remove_if([hWnd](const HWND& hWndFor){return (hWndFor == hWnd); });
}

VOID ReorderWnd()
{
	RECT rectwork;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rectwork, 0);

	LONG cur_right  = rectwork.right   - WIN_MARGIN;
	LONG cur_bottom = rectwork.bottom  - WIN_MARGIN;
	LONG next_right = rectwork.right   - WIN_MARGIN;
	LONG next_bottom = rectwork.bottom - WIN_MARGIN;


	for (auto itr = hWndList.rbegin(); itr != hWndList.rend(); ++itr) {
		
		RECT rect;
		GetWindowRect(*itr, &rect);

		LONG width = rect.right - rect.left;
		LONG height = rect.bottom - rect.top;

		SetWindowPos(*itr, HWND_TOPMOST, cur_right - width, cur_bottom - height, 0, 0, 
			SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
		InvalidateRect(*itr, NULL, false);

		if (next_right > cur_right - width - WIN_MARGIN)
		{
			next_right = cur_right - width - WIN_MARGIN;
		}
		cur_bottom = cur_bottom - height - WIN_MARGIN;

		if (cur_bottom < rectwork.top)
		{
			cur_right = next_right;
			cur_bottom = next_bottom;
		}
	}
}

ATOM MyRegisterMainClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = MainWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PICOND));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szMainWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}




// 
BOOL InitMainInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	hMainWnd = CreateWindow(szMainWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hMainWnd)
	{
		return FALSE;
	}


	//
	// Create task tray icon
	//

	iWMTASKCREATE = RegisterWindowMessage(_T("TaskbarCreated"));

	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.uFlags = (NIF_ICON | NIF_MESSAGE | NIF_TIP);
	nid.hWnd = hMainWnd;
	nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SMALL));
	nid.uID = 1;
	nid.uCallbackMessage = WM_TASKTRAY;
	lstrcpy(nid.szTip, szTitle);
	nid.uTimeout = 10000;
	nid.dwState = NIS_HIDDEN;

	Shell_NotifyIcon(NIM_ADD, &nid);

	return TRUE;
}


LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message)
	{
	case WM_COPYDATA:
		{
			COPYDATASTRUCT* cd;
			cd = (COPYDATASTRUCT *)lParam;
			InitChildInstance(cd->dwData, (wchar_t *)cd->lpData);
		}
		break;
	case WM_TASKTRAY:
		switch (lParam)
		{
		case WM_RBUTTONDOWN:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		}
		break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		if (message == iWMTASKCREATE)
		{
			Shell_NotifyIcon(NIM_DELETE, &nid);
			Shell_NotifyIcon(NIM_ADD, &nid);
			break;
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}



/*
 *
 * Child Window
 *
 */

ATOM MyRegisterChildClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = ChildWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PICOND));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szChildWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}


BOOL InitChildInstance(int iType, LPTSTR lpText)
{
	HWND hWnd;
	DWORD dwStyle = WS_POPUP;


	hWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_COMPOSITED, szChildWindowClass, lpText, dwStyle,
		CW_USEDEFAULT, 0, 20, 20, hMainWnd, NULL, hInst, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	return TRUE;
}


LRESULT CALLBACK ChildWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case WM_CREATE:
		AddhWndList(hWnd);
		return DefWindowProc(hWnd, message, wParam, lParam);
	case WM_CLOSE:
		DeletehWndList(hWnd);
		ReorderWnd();
		return DefWindowProc(hWnd, message, wParam, lParam);
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			int len;
			TCHAR * str;
			TCHAR * buf;
			int width = 20;
			int height = 20;
			SIZE szTitle;

			RECT rect;
			
			// get window rect
			GetWindowRect(hWnd, &rect);

			LONG cur_width = rect.right - rect.left;
			LONG cur_height = rect.bottom - rect.top;

			// get print text
			len = GetWindowTextLength(hWnd);
			str = new TCHAR[len + 1];
			GetWindowText(hWnd, str, len + 1);
			str[len] = _T('\0');
			
			
			hdc = BeginPaint(hWnd, &ps);
			SetBkMode(hdc, OPAQUE);
			Rectangle(hdc, 0, 0, cur_width, cur_height);
			SetBkMode(hdc, TRANSPARENT);
			
			// write title
			int cur_str = 0;
			int cur_len = 0;
			int cur_top = WIN_PADDING;
			for (int i = cur_str; i < len + 1; ++i)
			{
				if (str[i] == _T('\0') || str[i] == _T('\n'))
				{
					cur_len = i - cur_str;
					break;
				}
			}
			SelectObject(hdc, hFontBold);
			GetTextExtentPoint32(hdc, &str[cur_str], cur_len, &szTitle);
			TextOut(hdc, WIN_PADDING, cur_top, &str[cur_str], cur_len);
			cur_top += szTitle.cy;
			if (width < szTitle.cx + WIN_PADDING * 2)
				width = szTitle.cx + WIN_PADDING * 2;
			cur_str += cur_len;

			// write some text...
			SelectObject(hdc, hFontNormal);
			while (cur_str < len)
			{
				for (int i = cur_str + 1; i < len + 1; ++i)
				{
					if (str[i] == _T('\0') || str[i] == _T('\n'))
					{
						cur_len = i - cur_str;
						break;
					}
				}

				GetTextExtentPoint32(hdc, &str[cur_str], cur_len, &szTitle);
				TextOut(hdc, WIN_PADDING, cur_top, &str[cur_str], cur_len);
				cur_top += szTitle.cy;
				if (width < szTitle.cx + WIN_PADDING * 2)
					width = szTitle.cx + WIN_PADDING * 2;
				cur_str += cur_len;
			}

			height = cur_top + WIN_PADDING;
			
			EndPaint(hWnd, &ps);

			// get resize
			if (width != cur_width || height != cur_height)
			{
				SetWindowPos(hWnd, NULL, 0, 0, width, height, 
					SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

				// todo: reorder window...
				ReorderWnd();
			}

			// set and delete
			delete[] str;
		}
		break;
	case WM_LBUTTONUP:
		SendMessage(hWnd, WM_CLOSE, 0, 0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

VOID	CALLBACK	ThreadProc(LPVOID)
{
	WSAData wsaData;

	SOCKET sock;
	struct sockaddr_in addr;
	int iResult;

	char buf[2048];

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
		printf("Error at WSAStartup()\n");

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(10514);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;

	iResult = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
	if (iResult != 0)
	{
		printf("Error at bind(): %ld\n", WSAGetLastError());
		closesocket(sock);
		WSACleanup();
	}

	// set recv() non blocking mode
	u_long val = 1;
	iResult = ioctlsocket(sock, FIONBIO, &val);
	if (iResult != NO_ERROR)
	{
		printf("ioctlsocket failed with error: %ld\n", iResult);
	}

	int n = 0;

	while (IsWindow(hMainWnd))
	{

		memset(buf, 0, sizeof(buf));
		n = recv(sock, buf, sizeof(buf), 0);


		if (n < 1) {
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				Sleep(500);
				continue;
			}
			break;
		}

		std::string buffer("");

		char msg_ver = buf[0];
		char msg_type = buf[1];

		buffer.append(&buf[2]);

		// convert to unicode from utf-8
		std::wstring wbuffer(L"");
		wbuffer.append(std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(buffer));


		COPYDATASTRUCT cd;
		cd.dwData = msg_type;
		cd.cbData = (DWORD)((wbuffer.length() + 1) * sizeof(wbuffer.c_str()[0]));
		cd.lpData = (PVOID)wbuffer.c_str();

		SendMessage(hMainWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cd);
	}

	closesocket(sock);
	WSACleanup();

	return;
}