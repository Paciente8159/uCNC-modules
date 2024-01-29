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

#if (UCNC_MODULE_VERSION < 10807 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

DECL_BUFFER(uint8_t, wsocket_rx, 128);
DECL_BUFFER(uint8_t, wsocket_tx, 128);
// #ifdef DECL_SERIAL_STREAM
// DECL_BUFFER(uint8_t, web_pendant_stream_buffer, RX_BUFFER_SIZE);
// static uint8_t webui_getc(void)
// {
// 	uint8_t c = 0;
// 	BUFFER_DEQUEUE(web_pendant_stream_buffer, &c);
// 	return c;
// }

// uint8_t web_pendant_available(void)
// {
// 	return BUFFER_READ_AVAILABLE(web_pendant_stream_buffer);
// }

// void web_pendant_clear(void)
// {
// 	BUFFER_CLEAR(web_pendant_stream_buffer);
// }

// DECL_SERIAL_STREAM(web_pendant_stream, web_pendant_getc, web_pendant_available, web_pendant_clear, NULL, NULL);

// uint8_t web_pendant_send_cmd(const char *__s)
// {
// 	// if machine is running (not jogging) rejects the command
// 	if (cnc_get_exec_state(EXEC_RUN | EXEC_JOG) == EXEC_RUN)
// 	{
// 		return STATUS_SYSTEM_GC_LOCK;
// 	}

// 	uint8_t len = strlen(__s);
// 	uint8_t w;

// 	if (BUFFER_WRITE_AVAILABLE(web_pendant_stream_buffer) < len)
// 	{
// 		return STATUS_STREAM_FAILED;
// 	}

// 	BUFFER_WRITE(web_pendant_stream_buffer, __s, len, w);

// 	return STATUS_OK;
// }

// #endif

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
				endpoint_send(200, "application/json", "FW version:0.9.200 # FW target:grbl # FW HW:Direct SD # primary sd:/ # secondary sd: # authentication: no # webcommunication: Sync: 23");
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
}
CREATE_EVENT_LISTENER(websocket_client_connected, ws_connected);

bool ws_disconnected(void *args)
{
	websocket_event_t *e = args;
}
CREATE_EVENT_LISTENER(websocket_client_disconnected, ws_disconnected);

bool ws_data(void *args)
{
	websocket_event_t *e = args;
	if(e->event == WS_EVENT_TEXT){
		uint8_t w;
		BUFFER_WRITE(wsocket_rx, e->data, e->length, w);
	}
}
CREATE_EVENT_LISTENER(websocket_client_client_data, ws_data);

DECL_MODULE(webui)
{
	// serial_stream_register(&web_pendant_stream);
	endpoint_add("/", 0, &webui_request, NULL);
	endpoint_add("/command", 0, &webui_request, NULL);

	ADD_EVENT_LISTENER(websocket_client_connected, ws_connected);
	ADD_EVENT_LISTENER(websocket_client_disconnected, ws_disconnected);
	ADD_EVENT_LISTENER(websocket_client_client_data, ws_data);
}

#endif
