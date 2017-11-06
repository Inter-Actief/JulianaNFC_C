#pragma once

#include <Windows.h>
#include <CommCtrl.h>
#include <shellapi.h>
#include <Shlwapi.h>
#include <strsafe.h>

#include "resource.h"
#include "version.h"

#define APPLICATION_NAME L"JulianaNFC"
#define CLASSNAME L"JulianaNFCClass"
#define TLS_CERT_FILENAME "juliana.crt"
#define TLS_KEY_FILENAME "juliana.key"

#define EVER ;;

#define WM_NOTIFYICON (WM_USER + 0x00)

#pragma comment(lib, "ComCtl32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Winscard.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

typedef struct {
	BYTE uidLen;
	BYTE uid[8];
	BYTE atqa[2];
	BYTE sak;
} RFIDTAG_A;

typedef struct {
	BYTE uidLen;
	BYTE uid[8];
	BYTE rfu[3];
} RFIDTAG_B;

typedef struct {
	BYTE type;
	union {
		RFIDTAG_A a;
		RFIDTAG_B b;
	};
} RFIDTAG, *LPRFIDTAG;

typedef const RFIDTAG *LPCRFIDTAG;

BOOL notify_icon_toast(LPCWSTR lpText, LPCWSTR lpCaption);
VOID console_append(LPCWSTR lpMsg, ...);

LONG nfc_init();
DWORD WINAPI nfc_run(LPVOID lpParam);

VOID websocket_broadcast_tag(LPCRFIDTAG tag);
VOID websocket_init();
DWORD WINAPI websocket_run(LPVOID lpParam);

#define RFIDTAG_TYPE_A 0x00
#define RFIDTAG_TYPE_B 0x01
