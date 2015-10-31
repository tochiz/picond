#pragma once

#include <windows.h>

#define PICOND_TASK_ADD     0x01
#define PICOND_TASK_DELETE  0x02
#define PICOND_TASK_REORDER 0x03
#define PICOND_TASK_CLEAR   0x04

VOID InitializeTask();
VOID AddTask(UINT type, HWND hWnd, UINT winType, LPTSTR text);
VOID CALLBACK TaskThreadProc(LPVOID);
