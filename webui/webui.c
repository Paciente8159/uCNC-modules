/*
	Name: webui.c
	Description: Implements a WebUI for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 29-01-2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"
#include "../../modules/endpoint.h"
#include "../../modules/websocket.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#if defined(MCU_HAS_WIFI) && defined(MCU_HAS_ENDPOINTS) && defined(MCU_HAS_WEBSOCKETS)

#if (UCNC_MODULE_VERSION < 10801 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

DECL_BUFFER(uint8_t, wsocket_rx, 128);
DECL_BUFFER(uint8_t, wsocket_tx, 128);
static websocket_client_t ws_client;

void webui_request(void)
{
	// if does not have args return the page
	if (!endpoint_request_hasargs())
	{
		if (!endpoint_send_file("/index.html.gz", "text/html"))
		{
			protocol_send_string(__romstr__("Server error. File not found"));
			endpoint_send(404, "text/plain", "FileNotFound");
		}
	}
	else
	{
		char cmd[256];
		memset(cmd, 0, 256);
		if (endpoint_request_arg("plain", cmd, 256))
		{
			if (strcmp("[ESP800]", cmd) == 0)
			{
				endpoint_send(200, "application/json", "FW version:0.9.200 # FW target:grbl # FW HW:Direct SD # primary sd:/ # secondary sd: # authentication: no # webcommunication: Sync: 8080");
				return;
			}

			if (strcmp("[ESP400]", cmd) == 0)
			{
				endpoint_send(200, "application/json", "{\"cmd\":\"400\",\"status\":\"ok\",\"data\":[]}");
				return;
			}
		}
	}
}

bool ws_connected(void *args)
{
	websocket_event_t *e = args;
	ws_client.id = e->id;
	ws_client.ip = e->ip;

	return EVENT_CONTINUE;
}
CREATE_EVENT_LISTENER(websocket_client_connected, ws_connected);

bool ws_disconnected(void *args)
{
	websocket_event_t *e = args;
	if (ws_client.id == e->id)
	{
		ws_client.ip = 0;
		ws_client.id = 0;
	}

	return EVENT_CONTINUE;
}
CREATE_EVENT_LISTENER(websocket_client_disconnected, ws_disconnected);

bool ws_receive(void *args)
{
	websocket_event_t *e = args;
	if (ws_client.ip)
	{
		if (e->event == WS_EVENT_TEXT)
		{
			uint8_t w;
			BUFFER_WRITE(wsocket_rx, e->data, e->length, w);
		}
	}
	return EVENT_CONTINUE;
}
CREATE_EVENT_LISTENER(websocket_client_receive, ws_receive);

/**
 * Creates the serial stream handler functions
 * **/
uint8_t webui_getc(void)
{
	uint8_t c = 0;
	BUFFER_DEQUEUE(wsocket_rx, &c);
	return c;
}

uint8_t webui_available(void)
{
	return BUFFER_READ_AVAILABLE(wsocket_rx);
}

void webui_clear(void)
{
	BUFFER_CLEAR(wsocket_rx);
}

void webui_putc(uint8_t c)
{
	while (BUFFER_FULL(wsocket_tx))
	{
		mcu_uart_flush();
	}
	BUFFER_ENQUEUE(wsocket_tx, &c);
}

void webui_flush(void)
{
	while (!BUFFER_EMPTY(wsocket_tx))
	{
		uint8_t tmp[128 + 1];
		memset(tmp, 0, sizeof(tmp));
		uint8_t r;
		uint8_t max = BUFFER_READ_AVAILABLE(wsocket_tx);

		BUFFER_READ(wsocket_tx, tmp, max, r);
		websocket_client_send(ws_client.id, tmp, max, WS_SEND_TXT);
	}
}

DECL_SERIAL_STREAM(webui_stream, webui_getc, webui_available, webui_clear, webui_putc, webui_flush);

DECL_MODULE(webui)
{
	// serial_stream_register(&web_pendant_stream);
	endpoint_add("/", 0, &webui_request, NULL);
	endpoint_add("/command", 0, &webui_request, NULL);

	ADD_EVENT_LISTENER(websocket_client_connected, ws_connected);
	ADD_EVENT_LISTENER(websocket_client_disconnected, ws_disconnected);
	ADD_EVENT_LISTENER(websocket_client_receive, ws_receive);

	serial_stream_register(&webui_stream);
}

#endif
