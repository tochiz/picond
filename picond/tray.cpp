#include "stdafx.h"
#include "picond.h"
#include "popup.h"

#include <shellapi.h>


NOTIFYICONDATA nid = { 0 };
UINT iWMTASKCREATE;

extern HINSTANCE hInst;
extern HWND      hMainWnd;
extern TCHAR szTitle[MAX_LOADSTRING];
extern TCHAR szTrayWindowClass[MAX_LOADSTRING];


LRESULT CALLBACK	TrayWndProc(HWND, UINT, WPARAM, LPARAM);


ATOM MyRegisterTrayClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = TrayWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PICOND));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szTrayWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}




// 
BOOL InitTrayInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	hMainWnd = CreateWindow(szTrayWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
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


LRESULT CALLBACK TrayWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message)
	{
	case WM_COPYDATA:
	{
		COPYDATASTRUCT* cd;
		cd = (COPYDATASTRUCT *)lParam;
		InitPopupInstance(cd->dwData, (wchar_t *)cd->lpData);
	}
	break;
	case WM_TASKTRAY:
		switch (lParam)
		{
		case WM_RBUTTONDOWN:
			// TODO: create popup menu...

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
	case WM_CLOSE:
		Shell_NotifyIcon(NIM_DELETE, &nid);
		return DefWindowProc(hWnd, message, wParam, lParam);
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