#include <Windows.h>
#include "resource.h"
#include <string>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <sstream>


wchar_t BUFFER[MAX_PATH + 1];

std::wstringstream EUWPing[5];
DWORD dwRetVal = 0;
DWORD ReplySize;
LPVOID ReplyBuffer;
HANDLE hIcmpFile;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

class LimitSingleInstance
{
protected:
	HANDLE Mutex;

public:
	explicit LimitSingleInstance(std::wstring const& strMutexName)
	{
		Mutex = CreateMutex(nullptr, 0, strMutexName.c_str());
	}

	~LimitSingleInstance()
	{
		if (Mutex)
		{
			CloseHandle(Mutex);
			Mutex = nullptr;
		}
	}

	BOOL AnotherInstanceRunning()
	{
		return ERROR_ALREADY_EXISTS == GetLastError();
	}
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	HDC hDC;
	PAINTSTRUCT ps;
	RECT localLabel = { 0, 0, 270, 20 };
	RECT localLabel1 = { 0, 0, 270, 30 };
	RECT localLabel2 = { 0, 0, 270, 40 };
	switch (msg)
	{
	case WM_PAINT:
		hDC = BeginPaint(hwnd, &ps);
		SetTextColor(hDC, RGB(0, 0, 0));
		SetBkMode(hDC, TRANSPARENT);
		DrawText(hDC, std::wstring(L"EUW: " + EUWPing[0].str()).c_str(), -1, &localLabel, DT_CENTER);
		EndPaint(hwnd, &ps);
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		exit(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wp, lp);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	LimitSingleInstance appGUID(L"Global\\{L0LP1NGCH3CK3R-BYL0GG4N08@G17HUB-V3RYR4ND0M4NDR4R3MUCHWOW}");
	if (appGUID.AnotherInstanceRunning())
		return 0;

	MSG Msg = {};

	WNDCLASSEX wc{ sizeof(WNDCLASSEX), CS_DROPSHADOW | CS_PARENTDC, WndProc, 0, 0, hInstance, LoadIcon(hInstance, MAKEINTRESOURCE(MAINICON)), nullptr, static_cast<HBRUSH>(GetSysColorBrush(COLOR_3DFACE)), nullptr, L"mainwindow", LoadIcon(hInstance, MAKEINTRESOURCE(MAINICON)) };

	RegisterClassEx(&wc);

	auto hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, L"mainwindow", L"LoLPingChecker", WS_TILEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 290, 120, nullptr, nullptr, hInstance, nullptr);

	hIcmpFile = IcmpCreateFile();
	ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(BUFFER);
	ReplyBuffer = static_cast<VOID*>(malloc(ReplySize));
	auto EUW = inet_addr("185.40.64.69");
	auto EUNE = inet_addr("chat.eune1.lol.riotgames.com");
	auto NA = inet_addr("chat.na1.lol.riotgames.com");
	dwRetVal = IcmpSendEcho(hIcmpFile, EUW, BUFFER, sizeof(BUFFER), nullptr, ReplyBuffer, ReplySize, 1000);
	auto pEchoReply = static_cast<PICMP_ECHO_REPLY>(ReplyBuffer);
	EUWPing[0] << pEchoReply->RoundTripTime;



	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	while (GetMessage(&Msg, nullptr, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return static_cast<int>(Msg.wParam);
}