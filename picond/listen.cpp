#include "stdafx.h"
#include "listen.h"
#include "task.h"

#include <WinSock2.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <locale>
#include <codecvt>
#include <memory>

extern HWND hMainWnd;

VOID	CALLBACK	ThreadProc(LPVOID)
{
	WSAData wsaData;

	SOCKET sock;
	struct sockaddr_in addr;
	int iResult;

	char buf[2048];

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
	{
		MessageBox(NULL, _T("Error at WSAStartup()\n"), _T("picond"), MB_OK);
	}


	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET) {
		std::wostringstream ss(L"");
		ss << _T("Error at socket(): ") << WSAGetLastError() << std::endl;
		MessageBox(NULL, ss.str().c_str(), _T("picond"), MB_OK);
		
		WSACleanup();
		SendMessage(hMainWnd, WM_CLOSE, 0, 0);
		return;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(10514);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;

	iResult = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
	if (iResult != 0)
	{
		std::wostringstream ss(L"");
		ss << _T("Error at bind(): ") << WSAGetLastError() << std::endl;
		MessageBox(NULL, ss.str().c_str(), _T("picond"), MB_OK);

		closesocket(sock);
		WSACleanup();
		SendMessage(hMainWnd, WM_CLOSE, 0, 0);
		return;
	}

	// set recv() non blocking mode
	u_long val = 1;
	iResult = ioctlsocket(sock, FIONBIO, &val);
	if (iResult != NO_ERROR)
	{
		std::wostringstream ss(L"");
		ss << _T("ioctlsocket failed with error: ") << iResult << std::endl;
		MessageBox(NULL, ss.str().c_str(), _T("picond"), MB_OK);

		closesocket(sock);
		WSACleanup();
		SendMessage(hMainWnd, WM_CLOSE, 0, 0);
		return;
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

		AddTask(PICOND_TASK_ADD, NULL, msg_type, (LPWSTR)wbuffer.c_str());
	}

	closesocket(sock);
	WSACleanup();

	return;
}
