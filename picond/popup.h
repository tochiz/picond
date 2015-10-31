#pragma once

#include <Windows.h>

// Popup Window Util Functions
VOID ReorderWnd();

// Popup Window Class
ATOM				MyRegisterPopupClass(HINSTANCE hInstance);
HWND				InitPopupInstance(int type, LPTSTR message);
