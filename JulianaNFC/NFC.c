#include "common.h"

#define SCARD_PNP_NOTIFICATION L"\\\\?PnP?\\Notification"

static SCARDCONTEXT hContext;
static SCARD_READERSTATE rgReaderStates[MAXIMUM_SMARTCARD_READERS];
static DWORD dwReaderCount;
static BYTE listCmd[] = { 0xff, 0x00, 0x00, 0x00, 0x04, 0xd4, 0x4a, 0x01, 0x00 };


VOID nfc_get_iso_a(SCARDHANDLE hCard, LPRFIDTAG tag)
{
	BYTE recv[32];
	DWORD cchRecv = 32;

	// Send APDU
	SCardTransmit(hCard, SCARD_PCI_T1, listCmd, sizeof(listCmd), NULL, recv, &cchRecv);

	tag->type = RFIDTAG_TYPE_A;
	tag->a.sak = recv[6];
	tag->a.uidLen = recv[7];
	memcpy(tag->a.atqa, &recv[4], 2);
	memcpy(tag->a.uid, &recv[8], tag->a.uidLen);
}

VOID nfc_get_iso_b(LPBYTE bAtr, LPRFIDTAG tag)
{
	tag->type = RFIDTAG_TYPE_B;
	tag->b.uidLen = 4;
	memcpy(tag->b.uid, &bAtr[5], 4);
}

VOID nfc_get_tags(DWORD dwReader)
{
	SCARDHANDLE hCard;
	BYTE bATR[32];
	DWORD cchATR = 32;
	RFIDTAG tag;

	// Select card
	SCardConnect(hContext, rgReaderStates[dwReader].szReader, SCARD_SHARE_EXCLUSIVE, SCARD_PROTOCOL_T1, &hCard, NULL);

	// Get ATR to detect type A/B
	SCardStatus(hCard, NULL, 0, NULL, NULL, (LPBYTE)&bATR, &cchATR);

	BOOL bValid = FALSE;
	// Valid ATR?
	if (bATR[0] == 0x3b) {
		switch (bATR[4]) {
		case 0x80: // type A
			bValid = TRUE;
			nfc_get_iso_a(hCard, &tag);
			break;
		case 0x50: // type B
			bValid = TRUE;
			nfc_get_iso_b(bATR, &tag);
			break;
		default:
			notify_icon_toast(L"Er is een RFID-kaart gedetecteerd, maar het type is niet bekend.", L"Onbekende kaart gescand");
		}
	}

	if (bValid) {
		websocket_broadcast_tag(&tag);
	}

	SCardDisconnect(hCard, SCARD_LEAVE_CARD);
}

VOID nfc_refresh_readers()
{
	LPWSTR szReaders, szReader;
	DWORD cchReaders = SCARD_AUTOALLOCATE;

	memset(&rgReaderStates[1], 0, sizeof(SCARD_READERSTATE) * (MAXIMUM_SMARTCARD_READERS - 1));
	dwReaderCount = 1;

	int err = SCardListReaders(hContext, NULL, (LPWSTR)&szReaders, &cchReaders);

	if (SCARD_S_SUCCESS == err) {
		int i;
		szReader = szReaders;
		for (i = 1; i < MAXIMUM_SMARTCARD_READERS; i++) {
			if (*szReader == '\0') {
				break;
			}

			rgReaderStates[i].szReader = szReader;
			rgReaderStates[i].dwCurrentState = SCARD_STATE_UNAWARE;

			console_append(L"Kaartlezer gevonden: %s", szReader);

			szReader += lstrlen(szReader) + 1;
		}
		dwReaderCount = i;
	}
	else if (SCARD_E_SERVICE_STOPPED == err) {
		console_append(L"Geen kaartlezers aanwezig");
		// Windows kills the Smart Card service when the last reader disconnects,
		// so acquire a new context to force the service to run.
		SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hContext);
	}

	if (dwReaderCount > 1) {
		notify_icon_toast(L"De kaartlezer is nu klaar voor gebruik.", L"Kaartlezer aangesloten");
	}
}

LONG nfc_init()
{
	LONG lReturn;

	lReturn = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hContext);
	if (SCARD_S_SUCCESS != lReturn) {
		return lReturn;
	}

	rgReaderStates[0].szReader = SCARD_PNP_NOTIFICATION;
	dwReaderCount = 1;

	return SCARD_S_SUCCESS;
}

DWORD WINAPI nfc_run(LPVOID lpParam)
{
	DWORD dwI;

	for (EVER)
	{
		LONG lReturn = SCardGetStatusChange(hContext, INFINITE, rgReaderStates, dwReaderCount);
		for (dwI = 0; dwI < dwReaderCount; dwI++)
		{
			if (rgReaderStates[dwI].dwEventState & SCARD_STATE_CHANGED) {
				if (lstrcmp(rgReaderStates[dwI].szReader, SCARD_PNP_NOTIFICATION) == 0) {
					nfc_refresh_readers();
				}

				if (rgReaderStates[dwI].dwEventState & SCARD_STATE_PRESENT) {
					nfc_get_tags(dwI);
				}
			}
			
			rgReaderStates[dwI].dwCurrentState = rgReaderStates[dwI].dwEventState;
		}
	}

	return 0;
}
