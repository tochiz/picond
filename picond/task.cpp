#include "stdafx.h"
#include "task.h"
#include "popup.h"

#include <list>

extern HWND hMainWnd;
extern CRITICAL_SECTION critSection;
extern CRITICAL_SECTION critWndSection;

typedef struct picond_task {
	UINT type;
	HWND hWnd;
	UINT winType;
	LPTSTR text;
} PICOND_TASK, *LPPICOND_TASK;

std::list<HWND> hWndList;
std::list<PICOND_TASK> tasklist;

VOID InitializeTask() {
	EnterCriticalSection(&critSection);
	tasklist.clear();
	LeaveCriticalSection(&critSection);
}

VOID AddTask(UINT type, HWND hWnd, UINT winType, LPTSTR text) {
	
	PICOND_TASK task;
	task.type = type;
	task.hWnd = hWnd;
	task.winType = winType;
	task.text = NULL;

	if (task.type == PICOND_TASK_ADD) {
		int iSize = sizeof(TCHAR) * (lstrlen(text) + 1);
		task.text = (LPTSTR)malloc(iSize);
		if (task.text != NULL) {
			memcpy_s(task.text, iSize, text, iSize);
		}
	}

	// append task to tasklist
	EnterCriticalSection(&critSection);
	tasklist.push_back(task);
	LeaveCriticalSection(&critSection);
}

VOID ArrangeTask(std::list<PICOND_TASK> *tmp_tasklist) {

	EnterCriticalSection(&critSection);

	// copy add task
	for (auto itr = tasklist.begin(); itr != tasklist.end(); ++itr) {
		if (itr->type == PICOND_TASK_ADD) {
			tmp_tasklist->push_back(*itr);
		}
	}

	// copy delete task
	for (auto itr = tasklist.begin(); itr != tasklist.end(); ++itr) {
		if (itr->type == PICOND_TASK_DELETE) {
			tmp_tasklist->push_back(*itr);
		}
	}

	// clear task
	for (auto itr = tasklist.begin(); itr != tasklist.end(); ++itr) {
		if (itr->type == PICOND_TASK_CLEAR) {
			tmp_tasklist->clear();
			tmp_tasklist->push_back(*itr);
		}
	}

	// append reorder task
	if (!tasklist.empty()) {
		PICOND_TASK task;
		task.type = PICOND_TASK_REORDER;
		task.winType = 0;
		task.hWnd = NULL;
		task.text = NULL;
		tmp_tasklist->push_back(task);
	}

	// clear current task
	tasklist.clear();
	LeaveCriticalSection(&critSection);
}

VOID ExecTask(std::list<PICOND_TASK> *tmp_tasklist) {
	for (auto itr = tmp_tasklist->begin(); itr != tmp_tasklist->end(); ++itr) {
		switch (itr->type) {
		case PICOND_TASK_ADD:
			if (itr->text != NULL) {

				COPYDATASTRUCT cd;
				cd.dwData = itr->winType;
				cd.cbData = (DWORD)((lstrlen(itr->text) + 1) * sizeof(TCHAR));
				cd.lpData = (PVOID)itr->text;

				SendMessage(hMainWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cd);

				free(itr->text);
			}
			break;
		case PICOND_TASK_DELETE:
			if (itr->hWnd != NULL) {
				HWND hWnd = itr->hWnd;
				SendMessage(hWnd, WM_CLOSE, 0, 0);
				EnterCriticalSection(&critWndSection);
				hWndList.remove_if([hWnd](const HWND& hWndFor) {return (hWndFor == hWnd); });
				LeaveCriticalSection(&critWndSection);
			}
			break;
		case PICOND_TASK_CLEAR:

			EnterCriticalSection(&critWndSection);
			for (auto wnditr = hWndList.rbegin(); wnditr != hWndList.rend(); ++wnditr) {
				SendMessage(*wnditr, WM_CLOSE, 0, 0);
			}
			hWndList.clear();
			LeaveCriticalSection(&critWndSection);
			break;
		case PICOND_TASK_REORDER:

			EnterCriticalSection(&critWndSection);
			ReorderWnd();
			LeaveCriticalSection(&critWndSection);
			break;
		}
	}
}

VOID CALLBACK TaskThreadProc(LPVOID)
{
	// clear task...
	hWndList.clear();

	while (IsWindow(hMainWnd))
	{
		std::list<PICOND_TASK> tmp_tasklist;
		ArrangeTask(&tmp_tasklist);
		ExecTask(&tmp_tasklist);
		Sleep(100);
	}

	return;
}
