
#include "windows/gmtoolwnd.h"
#include "network/uosocket.h"
#include "network/outgoingpackets.h"
#include "utilities.h"
#include "log.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static HINSTANCE instance = 0;
static HWND messageSinkWnd = 0;
static const char *className = "Ultima Online";

// The message sink for our command catcher
static LRESULT CALLBACK messageSink(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static QString command;

	switch (uMsg) {
		// Dummy function
		case WM_CREATE:
			return TRUE;

		// Simply send it to the log for now
		case WM_CHAR:
			if (wParam == 13) {
				if (!command.isEmpty()) {
					Log->print(LOG_NOTICE, QString("Received command from gm tool: %1\n").arg(command));
					UoSocket->send(cSendUnicodeSpeechPacket(SPEECH_REGULAR, command, 0x3b2, 3));
				}
				command.clear();
			} else {
				command.append(QChar(wParam));
			}
            return TRUE;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Register the window class for our command catcher
static bool registerWindowClass() {
	WNDCLASS wndclass;
	memset(&wndclass, 0, sizeof(WNDCLASS));
	wndclass.style = CS_NOCLOSE;
	wndclass.hInstance = instance;
	wndclass.lpfnWndProc = messageSink;
	wndclass.lpszClassName = className;
	
	return RegisterClass(&wndclass) != FALSE;
}

// Enable the GM Window message sink
void enableGmToolWnd() {
	instance = (HINSTANCE)GetModuleHandle(0); // Get our instance

	if (!registerWindowClass()) {
		Log->print(LOG_ERROR, tr("Unable to register class for the gm message sink window.\n"));
		return;
	}

	uint style = WS_POPUP;
	messageSinkWnd = CreateWindow(className, "Ultima Online", style, -1, -1, 1, 1, 0, 0, instance, 0);

	if (messageSinkWnd == 0) {
		Log->print(LOG_ERROR, tr("Unable to create the gm message sink window.\n"));
		return;
	}
}

void disableGmToolWnd() {
	DestroyWindow(messageSinkWnd);
	UnregisterClass(className, instance);
}
