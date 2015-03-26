#pragma once

#include <Windows.h>

// Popup Window Util Functions
VOID ClearhWndList();
VOID AddhWndList(HWND hWnd);
VOID DeletehWndList(HWND hWnd);
VOID ReorderWnd();

// Popup Window Class
ATOM				MyRegisterPopupClass(HINSTANCE hInstance);
BOOL				InitPopupInstance(int type, LPTSTR message);
