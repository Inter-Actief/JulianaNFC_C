#include "common.h"

HINSTANCE g_hInstance;
HWND hwndMain;
HWND hwndConsole;
NOTIFYICONDATA nid;

BOOL notify_icon_create()
{
	memset(&nid, 0, sizeof(nid));
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hwndMain;
	nid.uCallbackMessage = WM_NOTIFYICON;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_INFO;
	//lstrcpy(nid.szInfo, L"Gebruik het icoon in het systeemvak om het statusscherm te openen.");
	//lstrcpy(nid.szInfoTitle, L"JulianaNFC draait op de achtergrond");
	lstrcpy(nid.szTip, APPLICATION_NAME);
	LoadIconMetric(g_hInstance, MAKEINTRESOURCE(IDI_MAIN), LIM_SMALL, &nid.hIcon);

	return Shell_NotifyIcon(NIM_ADD, &nid);
}

VOID notify_icon_toast(LPCWSTR lpText, LPCWSTR lpCaption)
{
	lstrcpy(nid.szInfo, lpText);
	lstrcpy(nid.szInfoTitle, lpCaption);
	Shell_NotifyIcon(NIM_MODIFY, &nid);
}

VOID notify_icon_destroy()
{
	Shell_NotifyIcon(NIM_DELETE, &nid);
}

INT_PTR CALLBACK SplashProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam) {
	switch (Msg) {
	case WM_INITDIALOG:
		SetTimer(hDlg, 0, 5000, NULL);
		return TRUE;
	case WM_TIMER:
		DestroyWindow(hDlg);
		return TRUE;
	default:
		return FALSE;
	}

	return TRUE;
}

LRESULT WINAPI WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CLOSE:
		ShowWindow(hWnd, SW_HIDE);
		notify_icon_toast(L"Gebruik de rechtermuisknop op het icoon in het systeemvak om af te sluiten.", L"Juliana draait op de achtergrond");
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == ID_QUIT) {
			PostQuitMessage(0);
		}
		break;
	case WM_NOTIFYICON:
		if (lParam == WM_LBUTTONDOWN) {
			if (!IsWindowVisible(hwndMain)) {
				ShowWindow(hwndMain, SW_SHOW);
			}
			else {
				SetForegroundWindow(hwndMain);
			}
		}
		else if (lParam == WM_RBUTTONDOWN) {
			HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_TRAYMENU));
			hMenu = GetSubMenu(hMenu, 0);

			POINT cur;
			GetCursorPos(&cur);

			SetForegroundWindow(hwndMain);
			TrackPopupMenuEx(hMenu, TPM_RIGHTALIGN, cur.x, cur.y, hwndMain, NULL);
		}
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

VOID console_append_notime(LPCWSTR lpMsg, ...)
{
	WCHAR buf[1024];

	va_list argptr;
	va_start(argptr, lpMsg);
	StringCbVPrintf(buf, sizeof(buf) - 2, lpMsg, argptr);
	va_end(argptr);

	StringCbCat(buf, sizeof(buf), L"\r\n");

	DWORD length = GetWindowTextLength(hwndConsole);
	SendMessage(hwndConsole, EM_SETSEL, length, length);
	SendMessage(hwndConsole, EM_REPLACESEL, FALSE, (LPARAM)buf);
}

VOID console_append(LPCWSTR lpMsg, ...)
{
	WCHAR time[16];
	GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL, NULL, time, sizeof(time));

	WCHAR msg[1024];
	va_list argptr;
	va_start(argptr, lpMsg);
	StringCbVPrintf(msg, sizeof(msg), lpMsg, argptr);
	va_end(argptr);

	WCHAR buf[1024];
	StringCbPrintf(buf, sizeof(buf), L"[%s] %s", time, msg);

	console_append_notime(buf);
}

VOID juliana_init(HINSTANCE hInstance)
{
	g_hInstance = hInstance;

	CreateDialog(hInstance, MAKEINTRESOURCE(IDD_SPLASH), NULL, SplashProc);

	WNDCLASSEX wcex;

	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MAIN));
	wcex.hInstance = g_hInstance;
	wcex.lpfnWndProc = WndProc;
	wcex.lpszClassName = CLASSNAME;

	RegisterClassEx(&wcex);

	hwndMain = CreateWindowEx(0, CLASSNAME, APPLICATION_NAME, WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX, 0, 0, 640, 480, NULL, NULL, hInstance, NULL);

	RECT rect;
	GetClientRect(hwndMain, &rect);
	hwndConsole = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 0, 0, rect.right, rect.bottom, hwndMain, NULL, NULL, NULL);
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
	SendMessage(hwndConsole, WM_SETFONT, (WPARAM)CreateFontIndirect(&ncm.lfMessageFont), FALSE);

	ShowWindow(hwndMain, SW_HIDE);

	console_append_notime(APPLICATION_NAME L" " JULIANANFC_VERSION_STR_FULL L" (by Cas Ebbers)");
	console_append_notime(L"Support: www@inter-actief.net");
	console_append_notime(L"");

	notify_icon_create();
}

UINT juliana_run()
{
	BOOL bRet;
	MSG msg;

	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if (bRet == -1) {
			return 0;
		}
		else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	notify_icon_destroy();

	return (UINT)msg.wParam;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
	juliana_init(hInstance);

	nfc_init();
	CreateThread(NULL, 0, nfc_run, NULL, 0, NULL);

	websocket_init();
	CreateThread(NULL, 0, websocket_run, NULL, 0, NULL);

	return juliana_run();
}
