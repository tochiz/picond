#pragma once

#include <Windows.h>

// Tray Window
ATOM				MyRegisterTrayClass(HINSTANCE hInstance);
BOOL				InitTrayInstance(HINSTANCE, int);
