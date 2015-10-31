#include "stdafx.h"
#include "picond.h"
#include "popup.h"
#include "task.h"

#include <list>
#include <string>
#include <locale>
#include <codecvt>
#include <memory>

#define WIN_PADDING 10
#define WIN_MARGIN  5


extern HINSTANCE hInst;
extern HWND      hMainWnd;
extern TCHAR szPopupWindowClass[MAX_LOADSTRING];
extern HFONT hFontBold;
extern HFONT hFontNormal;
extern std::list<HWND> hWndList;
extern CRITICAL_SECTION critWndSection;


LRESULT CALLBACK PopupWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


VOID ReorderWnd()
{
	RECT rectwork;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rectwork, 0);

	LONG cur_right = rectwork.right - WIN_MARGIN;
	LONG cur_bottom = rectwork.bottom - WIN_MARGIN;
	LONG next_right = rectwork.right - WIN_MARGIN;
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


ATOM MyRegisterPopupClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = PopupWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PICOND));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szPopupWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}


HWND InitPopupInstance(int iType, LPTSTR lpText)
{
	HWND hWnd;
	DWORD dwStyle = WS_POPUP;


	hWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_COMPOSITED, szPopupWindowClass, lpText, dwStyle,
		CW_USEDEFAULT, 0, 20, 20, hMainWnd, NULL, hInst, NULL);

	if (!hWnd)
	{
		return NULL;
	}

	EnterCriticalSection(&critWndSection);
	hWndList.push_back(hWnd);
	LeaveCriticalSection(&critWndSection);

	return hWnd;
}


LRESULT CALLBACK PopupWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case WM_CREATE:
		return DefWindowProc(hWnd, message, wParam, lParam);
	case WM_DESTROY:
		return DefWindowProc(hWnd, message, wParam, lParam);
	case WM_PAINT:
	{
		//
		// main draw
		//

		PAINTSTRUCT ps;
		HDC hdc;
		int len;
		TCHAR * str;
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

			AddTask(PICOND_TASK_REORDER, NULL, 0, NULL);
		}

		// set and delete
		delete[] str;
	}
	break;
	case WM_LBUTTONUP:
		AddTask(PICOND_TASK_DELETE, hWnd, 0, NULL);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
