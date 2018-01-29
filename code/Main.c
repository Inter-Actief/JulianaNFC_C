#include "common.h"

HINSTANCE g_hInstance;
HWND hwndMain;
HWND hwndConsole;
NOTIFYICONDATA nid;

BOOL notify_icon_create(HWND hWnd)
{
	ZeroMemory(&nid, sizeof(nid));

	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_INFO | NIF_SHOWTIP;
	nid.uCallbackMessage = WM_NOTIFYICON;
	nid.dwInfoFlags = NIIF_USER;
	LoadIconMetric(g_hInstance, MAKEINTRESOURCE(IDI_MAIN), LIM_SMALL, &nid.hIcon);
	StringCchCopy(nid.szTip, ARRAYSIZE(nid.szTip), APPLICATION_NAME);
	Shell_NotifyIcon(NIM_ADD, &nid);

	nid.uVersion = NOTIFYICON_VERSION_4;
	return Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

BOOL notify_icon_toast(LPCWSTR lpText, LPCWSTR lpCaption)
{
	StringCchCopy(nid.szInfoTitle, ARRAYSIZE(nid.szInfoTitle), lpCaption);
	StringCchCopy(nid.szInfo, ARRAYSIZE(nid.szInfo), lpText);
	return Shell_NotifyIcon(NIM_MODIFY, &nid);
}

BOOL notify_icon_destroy()
{
	return Shell_NotifyIcon(NIM_DELETE, &nid);
}

INT_PTR CALLBACK SplashProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		SetTimer(hwndDlg, 0, 5000, NULL);
		return TRUE;
	case WM_TIMER:
		DestroyWindow(hwndDlg);
		return TRUE;
	}

	return FALSE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CLOSE:
		ShowWindow(hWnd, SW_HIDE);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == ID_QUIT) {
			PostQuitMessage(0);
		}
		break;
	case WM_NOTIFYICON:
		switch (LOWORD(lParam)) {
		case NIN_SELECT:
			ShowWindow(hwndMain, SW_SHOWNORMAL);
			SetForegroundWindow(hwndMain);
			break;
		case WM_CONTEXTMENU:
			{
				HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_TRAYMENU));
				hMenu = GetSubMenu(hMenu, 0);
				SetForegroundWindow(hwndMain);
				TrackPopupMenuEx(hMenu, TPM_RIGHTALIGN, LOWORD(wParam), HIWORD(wParam), hwndMain, NULL);
			}
			break;
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
	StringCchVPrintf(buf, ARRAYSIZE(buf) - (3 * sizeof(WCHAR)), lpMsg, argptr);
	va_end(argptr);

	StringCchCat(buf, ARRAYSIZE(buf), L"\r\n");

	DWORD length = GetWindowTextLength(hwndConsole);
	SendMessage(hwndConsole, EM_SETSEL, length, length);
	SendMessage(hwndConsole, EM_REPLACESEL, FALSE, (LPARAM)buf);
}

VOID console_append(LPCWSTR lpMsg, ...)
{
	WCHAR time[16];
	GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL, NULL, time, sizeof(time));

	WCHAR msg[1000];
	va_list argptr;
	va_start(argptr, lpMsg);
	StringCchVPrintf(msg, ARRAYSIZE(msg), lpMsg, argptr);
	va_end(argptr);

	WCHAR buf[1024];
	StringCchPrintf(buf, ARRAYSIZE(buf), L"[%s] %s", time, msg);

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

	notify_icon_create(hwndMain);
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

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	CreateMutex(NULL, FALSE, APPLICATION_NAME);
	if (ERROR_ALREADY_EXISTS == GetLastError()) {
		MessageBox(
			NULL,
			APPLICATION_NAME L" is al gestart.",
			APPLICATION_NAME,
			MB_OK | MB_ICONERROR
		);
		return 0;
	}

	juliana_init(hInstance);

	nfc_init();
	CreateThread(NULL, 0, nfc_run, NULL, 0, NULL);

	websocket_init();
	CreateThread(NULL, 0, websocket_run, NULL, 0, NULL);

	return juliana_run();
}
