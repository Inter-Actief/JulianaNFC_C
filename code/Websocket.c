#include <libwebsockets.h>
#include "common.h"

static CHAR broadcastMsg[128];

static INT callback_nfc(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	static char name[64];

	switch (reason) {
	case LWS_CALLBACK_ESTABLISHED:
		lws_get_peer_simple(wsi, name, 64);
		console_append(L"Inkomende verbinding (%S)", name);
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
		lws_write(wsi, broadcastMsg, strlen(broadcastMsg), LWS_WRITE_TEXT);
		break;
	case LWS_CALLBACK_CLOSED:
		lws_get_peer_simple(wsi, name, 64);
		console_append(L"%S heeft de verbinding verbroken", name);
	default:
		break;
	}

	return 0;
}

static INT callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	switch (reason)
	{
	case LWS_CALLBACK_HTTP:
		lws_return_http_status(wsi, HTTP_STATUS_OK, NULL);
		break;
	default:
		break;
	}

	return 0;
}

static struct lws_context* context;
static struct lws_protocols protocols[] = {
	{ "http-only", callback_http, 0, 0 },
	{ "nfc",       callback_nfc,  0, 0 },
	{ NULL,        NULL,          0, 0 }
};

void b2hex(const BYTE* in, size_t len, char* out) {
	char* pos = out;

	for (unsigned int i = 0; i < len; i++) {
		if (i != 0) {
			pos += sprintf_s(pos, 2, ":");
		}
		pos += sprintf_s(pos, 3, "%02x", in[i]);
	}

	pos += sprintf_s(pos, 2, "\0");
}

VOID websocket_broadcast_tag(LPCRFIDTAG tag) {
	if (tag->type == RFIDTAG_TYPE_A) {
		char atqa[32];
		char uid[32];

		b2hex(tag->a.atqa, 2, atqa);
		b2hex(tag->a.uid, tag->a.uidLen, uid);
		sprintf_s(broadcastMsg, 128, "{\"type\":\"iso-a\",\"atqa\":\"%s\",\"uid\":\"%s\",\"sak\":\"%02x\"}", atqa, uid, tag->a.sak);

		console_append(L"ISO Type A -- UID<%S>  ATQA<%S>  SAK<%02x>", uid, atqa, tag->a.sak);
	}
	else if (tag->type == RFIDTAG_TYPE_4) {
		char atqa[32];
		char uid[32];

		b2hex(tag->a.atqa, 2, atqa);
		b2hex(tag->a.uid, tag->a.uidLen, uid);
		sprintf_s(broadcastMsg, 128, "{\"type\":\"iso-4\",\"atqa\":\"%s\",\"uid\":\"%s\",\"sak\":\"%02x\"}", atqa, uid, tag->a.sak);

		console_append(L"ISO DESFire -- UID<%S>  ATQA<%S>  SAK<%02x>", uid, atqa, tag->a.sak);
	}
	else if(tag->type == RFIDTAG_TYPE_B) {
		char uid[32];

		b2hex(tag->b.uid, tag->b.uidLen, uid);
		sprintf_s(broadcastMsg, 128, "{\"type\":\"iso-b\",\"uid\":\"%s\"}", uid);

		console_append(L"ISO Type B -- UID<%S>", uid);
	}

	lws_callback_on_writable_all_protocol(context, &protocols[1]);
}

VOID websocket_init()
{
	struct lws_context_creation_info info;

	memset(&info, 0, sizeof(info));
	info.vhost_name = "localhost";
	info.port = 3000;
	info.protocols = protocols;

	if (PathFileExists(TEXT(TLS_CERT_FILENAME)) && PathFileExists(TEXT(TLS_KEY_FILENAME))) {
		info.ssl_cert_filepath = TLS_CERT_FILENAME;
		info.ssl_private_key_filepath = TLS_KEY_FILENAME;
		info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
		console_append(L"SSL geactiveerd");
	}
	else {
		console_append(L"Geen certificaat gevonden, SSL uitgeschakeld");
	}

	context = lws_create_context(&info);

	console_append(L"Websocket geïnitialiseerd");
}

DWORD WINAPI websocket_run(LPVOID lpParam)
{
	int n = 0;

	while (n >= 0) {
		n = lws_service(context, 50);
	}
	
	lws_context_destroy(context);

	return n;
}
