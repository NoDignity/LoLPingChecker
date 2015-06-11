#include <Windows.h>
#include "resource.h"
#include <string>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <sstream>

wchar_t BUFFER[MAX_PATH + 1];

auto EUW = inet_addr("185.40.64.69");

std::wstringstream Ping[1];
DWORD dwRetVal = 0;
DWORD ReplySize;
LPVOID ReplyBuffer;
HANDLE hIcmpFile;
HWND hwndButton;
HWND hwnd;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ButtonProc(HWND, UINT, WPARAM, LPARAM);
WNDPROC OldButtonProc;
MSG Msg;
RECT localLabel = { 0, 0, 270, 20 };

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

void Refresh()
{
	*BUFFER = '\0';
	Ping[0].clear();
	hIcmpFile = IcmpCreateFile();
	ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(BUFFER);
	ReplyBuffer = static_cast<VOID*>(malloc(ReplySize));
	dwRetVal = IcmpSendEcho(hIcmpFile, EUW, BUFFER, sizeof(BUFFER), nullptr, ReplyBuffer, ReplySize, 1000);
	auto pEchoReply = static_cast<PICMP_ECHO_REPLY>(ReplyBuffer);
	Ping[0] << pEchoReply->RoundTripTime;
}

LRESULT CALLBACK ButtonProc(HWND, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_LBUTTONDOWN:
		Msg = {};
		Refresh();
		RedrawWindow(
			hwnd,
			&localLabel,
			nullptr,
			RDW_UPDATENOW
			);
	
	}
	return CallWindowProc(OldButtonProc, hwndButton, msg, wp, lp);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	HDC hDC;
	PAINTSTRUCT ps;
	switch (msg)
	{
	case WM_PAINT:
		hDC = BeginPaint(hwnd, &ps);
		SetTextColor(hDC, RGB(0, 0, 0));
		SetBkMode(hDC, TRANSPARENT);
		DrawText(hDC, std::wstring(L"EUW: " + Ping[0].str()).c_str(), -1, &localLabel, DT_CENTER);
		EndPaint(hwnd, &ps);
		break;

	case WM_CREATE:
		hwndButton = CreateWindow(L"BUTTON", L"Refresh", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 20, 20, 100, 50, hwnd, NULL, nullptr, nullptr);

		OldButtonProc = reinterpret_cast<WNDPROC>(SetWindowLong(hwndButton, GWL_WNDPROC, reinterpret_cast<LONG>(ButtonProc)));

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

	Msg = {};

	WNDCLASSEX wc{ sizeof(WNDCLASSEX), CS_DROPSHADOW | CS_PARENTDC, WndProc, 0, 0, hInstance, LoadIcon(hInstance, MAKEINTRESOURCE(MAINICON)), nullptr, static_cast<HBRUSH>(GetSysColorBrush(COLOR_3DFACE)), nullptr, L"mainwindow", LoadIcon(hInstance, MAKEINTRESOURCE(MAINICON)) };

	RegisterClassEx(&wc);

	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, L"mainwindow", L"LoLPingChecker", WS_TILEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 300, 200, nullptr, nullptr, hInstance, nullptr);
	
	Refresh();

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	while (GetMessage(&Msg, nullptr, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return static_cast<int>(Msg.wParam);
}