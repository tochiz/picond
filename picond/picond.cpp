#include "stdafx.h"
#include "picond.h"
#include "popup.h"
#include "tray.h"
#include "listen.h"
#include "task.h"


HINSTANCE hInst;
HWND      hMainWnd;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szTrayWindowClass[MAX_LOADSTRING];
TCHAR szPopupWindowClass[MAX_LOADSTRING];
HMENU hMenu;
HFONT hFontBold;
HFONT hFontNormal;
CRITICAL_SECTION critSection;
CRITICAL_SECTION critWndSection;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HANDLE hThr;
	HANDLE hTaskThr;

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_PICONDTRAY, szTrayWindowClass, MAX_LOADSTRING);
	LoadString(hInstance, IDC_PICONDPOPUP, szPopupWindowClass, MAX_LOADSTRING);
	
	MyRegisterTrayClass(hInstance);
	MyRegisterPopupClass(hInstance);

	// create critical seciton
	InitializeCriticalSectionAndSpinCount(&critSection, 0x00000400);
	InitializeCriticalSectionAndSpinCount(&critWndSection, 0x00000400);

	// Initialze Task
	InitializeTask();

	// create some resources
	hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_PICOND));

	// bold font
	hFontBold = CreateFont(12, 0, 0, 0, FW_BOLD, false, false, false,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("MS Shell Dlg 2"));

	// normal font
	hFontNormal = CreateFont(12, 0, 0, 0, FW_NORMAL, false, false, false,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("MS Shell Dlg 2"));


	// make main window
	if (!InitTrayInstance(hInstance, nCmdShow))
	{
		DeleteCriticalSection(&critSection);
		return FALSE;
	}


	// make "picond started." window
	AddTask(PICOND_TASK_ADD, NULL, 0, _T("picond(system)\npicond started."));


	// network listen thread begin...
	hThr = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, 0, 0, NULL);

	// task thread begin...
	hTaskThr = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TaskThreadProc, 0, 0, NULL);

	if (hThr && hTaskThr)
	{
		DWORD dwRes = STILL_ACTIVE;
		DWORD dwTaskRes = STILL_ACTIVE;

		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		
		while (dwRes == STILL_ACTIVE && dwTaskRes == STILL_ACTIVE)
		{
			GetExitCodeThread(hThr, &dwRes);
			GetExitCodeThread(hTaskThr, &dwTaskRes);
			Sleep(1);
		}
	}

	DestroyMenu(hMenu);
	DeleteObject(hFontBold);
	DeleteObject(hFontNormal);

	DeleteCriticalSection(&critSection);
	DeleteCriticalSection(&critWndSection);

	return (int)msg.wParam;
}
