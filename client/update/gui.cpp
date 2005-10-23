
#include "gui.h"
#include "resource.h"
#include <commctrl.h>

#define CLASS_NAME "UoClientUpdaterClass"

cGui::cGui(HINSTANCE instance) {
	window_ = 0;
	label_ = 0;
	instance_ = instance;
	progressBar_ = 0;
	running_ = true;
}

cGui::~cGui() {
}

static LRESULT CALLBACK windowMessages(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
	// Set the class-instance window pointer
	if (message == WM_CREATE) {
		LPCREATESTRUCT info = (LPCREATESTRUCT)lparam;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)(info->lpCreateParams));
	}

	// Forward the message to the GUI
    cGui *gui = reinterpret_cast<cGui*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	return gui->processMessage(hwnd, message, wparam, lparam);
}

bool cGui::registerClass() {
	WNDCLASSEX wndclass;
	ZeroMemory(&wndclass, sizeof(WNDCLASSEX));
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	wndclass.hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
	wndclass.hIcon = (HICON)LoadImage(instance_, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 32, 32, LR_SHARED);
	wndclass.hIconSm = (HICON)LoadImage(instance_, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 16, 16, LR_SHARED);
	wndclass.hInstance = instance_;
	wndclass.lpfnWndProc = windowMessages;
	wndclass.lpszClassName = CLASS_NAME;
	wndclass.lpszMenuName = 0;
	wndclass.style = CS_HREDRAW|CS_VREDRAW|CS_NOCLOSE;
	
	// Try registering the class
	if (RegisterClassEx(&wndclass) == 0) {
		return false;
	}

	return true;
}

bool cGui::initialize() {
	InitCommonControls();

	if (!registerClass()) {
		return false;
	}

	// Create the Window
	const DWORD styles = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~ WS_THICKFRAME;

	clientWidth_ = 400;
	clientHeight_ = 110;

	height_ = clientHeight_ + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME) * 2;
	width_ = clientWidth_ + GetSystemMetrics(SM_CXFRAME) * 2;

	// Measure X,Y for window.
	RECT rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
	int x = (rect.right - width_) / 2;
	int y = (rect.bottom - height_) / 2;

	window_ = CreateWindow(CLASS_NAME, "Installing Client Updates", styles, x, y, width_, height_, 0, 0, instance_, this);

	return window_ != 0;
}

void cGui::show() {
	ShowWindow(window_, SW_NORMAL);
	UpdateWindow(window_);
}

void cGui::hide() {
	ShowWindow(window_, SW_HIDE);
}

void cGui::processMessages() {
    MSG msg;

	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		
		// The window system wants us to quit
		if (msg.message == WM_QUIT) {
			running_ = false;
			break;
		}
	}
}

LRESULT cGui::processMessage(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
	switch (message) {
		case WM_CREATE:
			return onCreate(hwnd, wparam, lparam);

		case WM_DESTROY:
			return onDestroy(hwnd, wparam, lparam);

		case WM_PAINT:
			return onPaint(hwnd, wparam, lparam);

		case WM_COMMAND:
			return onCommand(hwnd, wparam, lparam);

		case WM_CTLCOLORSTATIC:
			return onCtlColorStatic(hwnd, wparam, lparam);
	}

	return DefWindowProc(hwnd, message, wparam, lparam);
}

LRESULT cGui::onCreate(HWND hwnd, WPARAM wparam, LPARAM lparam) {
	grayBrush = GetSysColorBrush(COLOR_BTNSHADOW);
	whiteBrush = GetSysColorBrush(COLOR_3DHILIGHT);
	dialogFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

	// Create font for top label
	topFont = CreateFont(-MulDiv(14, GetDeviceCaps(GetDC(hwnd), LOGPIXELSY), 72),
		0, 0, 0, 700, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, 6, DEFAULT_PITCH, "Arial");

	// Create an icon on the upper left corner
	HBITMAP image = (HBITMAP)LoadImage(instance_, MAKEINTRESOURCE(IDB_LOGO), IMAGE_BITMAP, 0, 0, LR_SHARED);
	HWND icon = CreateWindow("STATIC", 0, SS_BITMAP|WS_CHILD|WS_VISIBLE, 0, 0, clientWidth_ + 1, 50, hwnd, 0, instance_, 0);
	
	if (icon) {
		SendMessage(icon, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)(UINT)image);
	}

	// Top label
	topLabel = CreateWindow("STATIC", "Updating...", WS_CHILD|WS_VISIBLE, 49, 14, clientWidth_, 30, hwnd, 0, instance_, 0);
    SendMessage(topLabel, WM_SETFONT, (WPARAM)topFont, TRUE);

	label_ = CreateWindow("STATIC", "Copying ...", WS_CHILD|WS_VISIBLE, 10, 59, clientWidth_, 20, hwnd, 0, instance_, 0);
	SendMessage(label_, WM_SETFONT, (WPARAM)dialogFont, TRUE);

	// Progress bar
	progressBar_ = CreateWindowEx(0, PROGRESS_CLASS, 0, PBS_SMOOTH|WS_CHILD|WS_VISIBLE, 10, 78, clientWidth_ - 20, 22, hwnd, 0, instance_, 0);

	return TRUE;
}

LRESULT cGui::onDestroy(HWND hwnd, WPARAM wparam, LPARAM lparam) {
	DeleteObject(topFont);
	PostQuitMessage(0);
	return FALSE;
}

LRESULT cGui::onPaint(HWND hwnd, WPARAM wparam, LPARAM lparam) {
	PAINTSTRUCT paint;
	HDC dc = BeginPaint(hwnd, &paint);
	
	RECT rect;
	// Paint the upper white box
	SetRect(&rect, 0, 0, clientWidth_ + 1, 50);
	FillRect(dc, &rect, whiteBrush);

	// Paint a bevel
	SetRect(&rect, 0, 50, clientWidth_ + 1, 51);
	FillRect(dc, &rect, grayBrush);
	SetRect(&rect, 0, 51, clientWidth_ + 1, 52);
	FillRect(dc, &rect, whiteBrush);

	EndPaint(hwnd, &paint);
	return FALSE;
}

LRESULT cGui::onCommand(HWND hwnd, WPARAM wparam, LPARAM lparam) {
	return DefWindowProc(hwnd, WM_COMMAND, wparam, lparam);
}

LRESULT cGui::onCtlColorStatic(HWND hwnd, WPARAM wparam, LPARAM lparam) {
	if ((HWND)lparam == topLabel || (HWND)lparam == label_) {		
		HBRUSH brush = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
		SetBkMode((HDC)wparam, TRANSPARENT);
		return (LRESULT)brush;
	} else {
		return DefWindowProc(hwnd, WM_CTLCOLORSTATIC, wparam, lparam);
	}
}

void cGui::setProgressRange(unsigned int min, unsigned int max) {
	SendMessage(progressBar_, PBM_SETRANGE32, min, max);
}

void cGui::setProgressValue(unsigned int value) {
	SendMessage(progressBar_, PBM_SETPOS, value, 0);
}

void cGui::setStatusText(const char *text) {
	SetWindowText(label_, text);
	InvalidateRect(window_, 0, TRUE);
}

cGui *Gui = 0;
