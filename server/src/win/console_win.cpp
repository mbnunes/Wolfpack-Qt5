/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2004 by holders identified in AUTHORS.txt
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In addition to that license, if you are running this program or modified
 * versions of it on a public system you HAVE TO make the complete source of
 * the version used by you available or provide people with a location to
 * download it.
 *
 * Wolfpack Homepage: http://wpdev.sf.net/
 */

// System Includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

// Wolfpack Includes
#include "../accounts.h"
#include "../console.h"
#include "../resource.h"
#include "../python/engine.h"
#include "../network/network.h"
#include "../player.h"
#include "../server.h"

#define _WIN32_IE 0x0500
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <richedit.h>
#include <commctrl.h>
#include <qthread.h>
#include "../verinfo.h"

#if !defined(CFM_WEIGHT)
# define CFM_WEIGHT 0x00400000
#endif 

/*
	This file includes the Windows GUI implementation of our Console.
 */

#define WOLFPACK_CLASS "wolfpack_class"

extern int main( int argc, char **argv );

// Variables important for this GUI implementation
#define CONTROL_LOGWINDOW 0x10
#define CONTROL_INPUT 0x11
#define WM_TRAY_NOTIFY WM_USER + 1

CHARFORMAT cf;
struct {
	DWORD cbSize;
	HWND hWnd;
	UINT uID;
	UINT uFlags;
	UINT uCallbackMessage;
	HICON hIcon;
#if (_WIN32_IE < 0x0500)
	CHAR szTip[64];
#else
	CHAR szTip[128];
#endif
#if (_WIN32_IE >= 0x0500)
	DWORD dwState;
	DWORD dwStateMask;
	CHAR   szInfo[256];
	union {
		UINT  uTimeout;
		UINT  uVersion;
	} DUMMYUNIONNAME;
	CHAR   szInfoTitle[64];
	DWORD dwInfoFlags;
#endif
#if (_WIN32_IE >= 0x600)
	GUID guidItem;
#endif
} icondata;
HMENU hmMainMenu;
HICON iconRed = 0, iconGreen = 0;
HBITMAP hLogo = 0;
HWND lblUptime = 0, bmpLogo;
HWND tooltip = 0;
HBRUSH hbSeparator = 0, hbBackground = 0;
DWORD guiThread;
HWND statusIcon = 0;
HWND logWindow = 0;			// Log Window
HWND inputWindow = 0;		// Input Textfield
HWND mainWindow = 0;		// Main Window
HINSTANCE appInstance = 0;	// Application Instance
HFONT font = 0;				// The font we'll use
HFONT arialFont = 0;		// ARIAL
unsigned int inputHeight = 0; // For measuring the height of the input field
unsigned int logLimit = 0;	// How many characters fit into the log window
unsigned int uptimeTimer = 0;
bool canClose = false;

/*
	Directly taken from MSDN
 */
static QString getErrorString()
{
	LPVOID lpMsgBuf;

	if( !FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, 0, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPTSTR)&lpMsgBuf, 0, 0 ) )
	   return QString( "Unknown Error" );

	QString result( (char*)lpMsgBuf );

	// Free the buffer.
	LocalFree( lpMsgBuf );

	return result;
}

/*!
	\internal
	Fill a rectangular on a specific context
*/
void paintRect( HDC dc, INT32 x, INT32 y, INT32 width, INT32 height, HBRUSH brush )
{
	RECT rect;
	rect.bottom = y + height;
	rect.right = x + width;
	rect.top = y;
	rect.left = x;

	FillRect( dc, &rect, brush );
}

void drawWindow( HWND window )
{
	PAINTSTRUCT paintInfo;
	HDC dc = BeginPaint( window, &paintInfo );

	RECT rect;
	GetClientRect( window, &rect );

	paintRect( dc, 0, 87, rect.right, 1, hbSeparator );

	EndPaint( window, &paintInfo );
}

LRESULT CALLBACK AboutDialog(HWND hwnd, unsigned int msg, WPARAM wparam, LPARAM lparam) {
	HWND richtext;

	switch (msg) {
		case WM_COMMAND:
			if (HIWORD(wparam) == BN_CLICKED && (HWND) lparam == GetDlgItem(hwnd, IDOK)) {
				EndDialog(hwnd, 0);
			}
			break;

		case WM_CLOSE:
			EndDialog(hwnd, 0);
			break;

		case WM_INITDIALOG:
			richtext = GetDlgItem(hwnd, IDC_RICHEDIT);

			if (richtext) {
				HRSRC resource = FindResource(appInstance, MAKEINTRESOURCE(IDD_CREDITS), RT_RCDATA);
				HGLOBAL rData = LoadResource(appInstance, resource);

				const char *data = (const char*)LockResource(rData);
				QStringList creditList = QStringList::split(",", data);
				UnlockResource(rData);
				FreeResource(rData);

				CHARRANGE cr;
				CHARFORMAT2 cf;
				ZeroMemory(&cf, sizeof(cf));
				cf.cbSize = sizeof(cf);

				// Add a version information header (just like the console)
				QString version = QString("%1 %2 %3\n").arg(productString(), productBeta(), productVersion());

				cf.dwMask = CFM_COLOR|CFM_WEIGHT|CFM_SIZE;
				cf.yHeight = 20 * 14;
				cf.wWeight = FW_BOLD;
				cf.crTextColor = RGB(60, 140, 70);
				SendMessage(richtext, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				SendMessage(richtext, EM_REPLACESEL, FALSE, (LPARAM)version.latin1());
				cf.dwMask = CFM_COLOR|CFM_WEIGHT|CFM_SIZE;
				cf.yHeight = 20 * 8;
				cf.wWeight = FW_NORMAL;
				cf.crTextColor = RGB(0, 0, 0);

				QString credits;
				credits += "Compiled: " __DATE__ " " __TIME__ "\n";
				credits += QString("QT: %1 %2 (Compiled: %3)\n").arg(qVersion()).arg(qSharedBuild() ? "Shared" : "Static").arg(QT_VERSION_STR);

				QString pythonBuild = Py_GetVersion();
				pythonBuild = pythonBuild.left(pythonBuild.find(' '));

				#if defined(Py_ENABLE_SHARED)
				credits += QString("Python: %1 Shared (Compiled: %2)\n").arg(pythonBuild).arg(PY_VERSION);
				#else
				credits += QString("Python: %1 Static (Compiled: %2)\n").arg(pythonBuild).arg(PY_VERSION);
				#endif

				cr.cpMin = GetWindowTextLength(richtext);
				cr.cpMax = cr.cpMin;
				SendMessage(richtext, EM_EXSETSEL, 0, (LPARAM)&cr);
				SendMessage(richtext, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				SendMessage(richtext, EM_REPLACESEL, FALSE, (LPARAM)credits.latin1());

				credits = "\nThis is an unsorted and not neccesarily complete list of people who contributed to Wolfpack:\n\n";

				cr.cpMin = GetWindowTextLength(richtext);
				cr.cpMax = cr.cpMin;
				cf.wWeight = FW_BOLD;
				SendMessage(richtext, EM_EXSETSEL, 0, (LPARAM)&cr);
				SendMessage(richtext, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				SendMessage(richtext, EM_REPLACESEL, FALSE, (LPARAM)credits.latin1());
				cf.wWeight = FW_NORMAL;

				credits = "";
				for (unsigned int i = 0; i < creditList.size(); ++i) {
					credits.append(creditList[i]);
				}

				cr.cpMin = GetWindowTextLength(richtext);
				cr.cpMax = cr.cpMin;
				SendMessage(richtext, EM_EXSETSEL, 0, (LPARAM)&cr);
				SendMessage(richtext, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				SendMessage(richtext, EM_REPLACESEL, FALSE, (LPARAM)credits.latin1());
			}
	}

	return FALSE;
}

bool handleMenuSelect( unsigned int id )
{
	bool result = true;

	cUOSocket *mSock;
	unsigned int i;

	switch( id )
	{
	case IDC_EXIT:
		Server::instance()->cancel();

		if( canClose )
			DestroyWindow( mainWindow );

		break;

	case ID_HELP_ABOUT:
		DialogBox(appInstance, MAKEINTRESOURCE(IDD_DIALOGABOUT), mainWindow, (DLGPROC)AboutDialog);
		break;

	case ID_HELP_WOLFPACKHOMEPAGE:
		ShellExecute(mainWindow, "open", "http://www.wpdev.org", 0, 0, SW_NORMAL);
		break;

	case ID_RELOAD_ACCOUNTS:
		Server::instance()->queueAction( RELOAD_ACCOUNTS );
		break;

	case ID_RELOAD_PYTHON:
		Server::instance()->queueAction( RELOAD_PYTHON );
		break;

	case ID_RELOAD_SCRIPTS:
		Server::instance()->queueAction( RELOAD_SCRIPTS );
		break;

	case ID_RELOAD_CONFIGURATION:
		Server::instance()->queueAction( RELOAD_CONFIGURATION );
		break;

	case ID_SERVER_SAVEWORLD:
		Server::instance()->queueAction( SAVE_WORLD );
		break;

	case ID_SERVER_LISTUSERS:
		// We simply do our thread safety manually here
		Network::instance()->lock();

		// Generate a list of Users
		mSock = Network::instance()->first();
		i = 0;

		for( mSock = Network::instance()->first(); mSock; mSock = Network::instance()->next() )
		{
			if( mSock->player() )
				Console::instance()->send( QString("%1) %2 [%3]\n").arg(++i).arg(mSock->player()->name()).arg(QString::number( mSock->player()->serial(), 16) ) );
		}

		Network::instance()->unlock();

		Console::instance()->send( QString( "Total Users Online: %1\n" ).arg( i ) );

		break;

	default:
		result = false;
	}

	return result;
}

LRESULT CALLBACK wpWindowProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	LOGFONT lfont;
	NMHDR *notify = (NMHDR*)lparam;
	HDC dc;

	switch( msg )
	{
	case WM_TRAY_NOTIFY:
		switch (lparam) {
			// Show a context menu (?)
			case WM_RBUTTONDOWN:
				break;

			// Show/Hide the main window
			case WM_LBUTTONUP:
				if (IsWindowVisible(mainWindow)) {
					ShowWindow(mainWindow, SW_HIDE);
					UpdateWindow(mainWindow);
				} else {
					ShowWindow(mainWindow, SW_NORMAL);
					UpdateWindow(mainWindow);
				}
				break;
		}

		return TRUE;

	case WM_CTLCOLORSTATIC:
		if ((HWND)lparam == lblUptime) {
			dc = (HDC)wparam;

			//SelectObject( dc, GetStockObject(ANSI_VAR_FONT) );
			SetTextColor(dc, RGB(0xAF, 0xAF, 0xAF));

			SetBkMode( dc, TRANSPARENT );
			//SelectObject( dc, GetStockObject( SYSTEM_FONT ) );
			return (LRESULT)hbBackground;
		} else if ((HWND)lparam == statusIcon) {
			dc = (HDC)wparam;
			SetBkMode( dc, TRANSPARENT );
			return (LRESULT)hbBackground;
		}
		return DefWindowProc( hwnd, msg, wparam, lparam );

	case WM_COMMAND:
		if( handleMenuSelect( wparam ) )
			return 0;
		break;

	case WM_CREATE:
		hLogo = LoadBitmap( appInstance, MAKEINTRESOURCE( IDB_LOGO ) );

		// Create Richedit Box
		logWindow = CreateWindow( RICHEDIT_CLASS, 0, ES_LEFT|ES_MULTILINE|ES_AUTOVSCROLL|ES_READONLY|WS_CHILD|WS_VISIBLE|WS_VSCROLL, 0, 0, 10, 10, hwnd, (HMENU)CONTROL_LOGWINDOW, appInstance, 0 );

		if( logWindow == 0 )
		{
			MessageBox( 0, QString( "Couldn't create the logwindow: " + getErrorString() ).latin1(), "Wolfpack", MB_OK|MB_ICONERROR );
			DestroyWindow( hwnd );
			return TRUE;
		}

		logLimit = SendMessage( logWindow, EM_GETLIMITTEXT, 0, 0 );

		// Set up the fonts we need
		ZeroMemory(&lfont, sizeof(LOGFONT));
		qstrcpy(lfont.lfFaceName, "Arial");
		lfont.lfQuality = ANTIALIASED_QUALITY;
		lfont.lfHeight = -MulDiv(10, GetDeviceCaps(GetWindowDC(hwnd), LOGPIXELSY), 72);
		arialFont = CreateFontIndirect( &lfont );

		ZeroMemory( &lfont, sizeof( LOGFONT ) );
		qstrcpy( lfont.lfFaceName, "Fixedsys" );
		lfont.lfQuality = ANTIALIASED_QUALITY;
		font = CreateFontIndirect( &lfont );
		if ( !font )
		{
			ZeroMemory( &lfont, sizeof( LOGFONT ) );
			qstrcpy( lfont.lfFaceName, "Courier" );
			font = CreateFontIndirect( &lfont );
		}

		// Set the font of our logwindow
		SendMessage( logWindow, WM_SETFONT, (WPARAM)font, 0 );
		SendMessage( logWindow, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(0,0,0) );

		// Default Charformat
        ZeroMemory( &cf, sizeof( CHARFORMAT ) );
        cf.cbSize = sizeof( CHARFORMAT );
        cf.dwMask = CFM_COLOR;
        cf.crTextColor = RGB( 0xAF,0xAF,0xAF );

		SendMessage( logWindow, EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM)&cf );
		SendMessage( logWindow, EM_AUTOURLDETECT, TRUE, 0 );
		SendMessage( logWindow, EM_SETEVENTMASK, 0, ENM_LINK|ENM_MOUSEEVENTS|ENM_KEYEVENTS );

		// Create InputWindow
		inputWindow = CreateWindow( "EDIT", 0, ES_LEFT|ES_AUTOHSCROLL|WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP, 0, 0, 10, 10, hwnd, (HMENU)CONTROL_INPUT, appInstance, 0 );

		bmpLogo = CreateWindow( "STATIC", 0, SS_BITMAP|WS_CHILD|WS_VISIBLE, 0, 0, 586, 87, hwnd, 0, appInstance, 0 );
		SendMessage( bmpLogo, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hLogo );

		statusIcon = CreateWindow("STATIC", 0, SS_ICON|WS_CHILD|WS_VISIBLE, 380, 15, 0, 0, hwnd, 0, appInstance, 0);
		SendMessage(statusIcon, STM_SETIMAGE, IMAGE_ICON, (LPARAM)iconRed);

		lblUptime = CreateWindow( "STATIC", 0, WS_CHILD|WS_VISIBLE, 400, 15, 250, 25, hwnd, 0, appInstance, 0 );
		SendMessage( lblUptime, WM_SETFONT, (WPARAM)arialFont, 0 );

		// Set up our timer to refresh the nice Uptime Counter
		uptimeTimer = SetTimer( NULL, 0, 500, 0 );

		return 0;

	// Autosize our Window Elements
	case WM_SIZE:
		if( logWindow && inputWindow && wparam != SIZE_MINIMIZED && wparam != SIZE_MAXHIDE )
		{
			unsigned int width = LOWORD( lparam );
			unsigned int height = HIWORD( lparam );

			// Measure the Height of our Input Field
			if( !inputHeight )
			{
				HFONT font = (HFONT)SendMessage( mainWindow, WM_GETFONT, 0, 0 );

				if( !font )
					font = (HFONT)GetStockObject( SYSTEM_FONT );

				LOGFONT logfont;
				if( GetObject( font, sizeof( LOGFONT ), &logfont ) == sizeof( LOGFONT ) )
					inputHeight = logfont.lfHeight + 4;
			}

			// Note: 88 pixel spacer are always on top
			MoveWindow( logWindow, 0, 88, width, height - inputHeight - 88, TRUE );
			MoveWindow( inputWindow, 0, height - inputHeight, width, inputHeight, TRUE );
		}

		return 0;

	case WM_PAINT:
		drawWindow( hwnd );
		return DefWindowProc( hwnd, msg, wparam, lparam );

	case WM_NOTIFY:
		if( wparam == CONTROL_LOGWINDOW )
		{
			if( notify->code == EN_LINK )
			{
				ENLINK *link = (ENLINK*)notify;

				if( link->msg == WM_LBUTTONDOWN )
				{
					char *string = new char[ ( link->chrg.cpMax - link->chrg.cpMin ) + 1 ];

					TEXTRANGE tr;
					tr.chrg = link->chrg;
					tr.lpstrText = string;

					SendMessage( logWindow, EM_GETTEXTRANGE, 0, (LPARAM)&tr );

					// String contains the link
					ShellExecute( mainWindow, "open", string, 0, 0, SW_NORMAL );

					delete [] string;
				}
			}
			else if( notify->code == EN_MSGFILTER )
			{
				/*MSGFILTER *msg = (MSGFILTER*)notify;

				// Append to the Input Control
				if( msg->msg == WM_CHAR )
				{
					SendMessage( inputWindow, WM_SETFOCUS, 0, 0 );
					SendMessage( inputWindow, WM_CHAR, msg->wParam, msg->lParam );
				}*/
			}
		}
		return 0;

	case WM_CLOSE:
		Server::instance()->cancel();

		if (canClose) {
			DestroyWindow( mainWindow );
		}

		return 1;

	case WM_DESTROY:
		KillTimer(NULL, uptimeTimer);
		DestroyMenu(hmMainMenu);
		DeleteObject(hLogo);
		DeleteObject(hbSeparator);
		DeleteObject(hbBackground);
		DeleteObject(iconRed);
		DeleteObject(iconGreen);
		Server::instance()->cancel();
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc( hwnd, msg, wparam, lparam );
}

class cServerThread : public QThread
{
	LPSTR cmdLine;
	int returnValue_;

public:

	cServerThread( LPSTR lpCmdLine ) : cmdLine( lpCmdLine ) {}

	int returnValue() { return returnValue_; }

protected:

	void run()
	{
		QMemArray<pchar> argv( 8 );
		/*
			Since Windows programs don't get passed the command name as the
			first argument, we need to fetch it explicitly.
		*/
		static char appFileName[256];
		GetModuleFileNameA( 0, appFileName, sizeof(appFileName) );
		int argc = 1;
		argv[0] = appFileName;

		/*
			Parse the Windows command line string.  If an argument begins with a
			double quote, then spaces are considered part of the argument until the
			next double quote.  The argument terminates at the second quote. Note
			that this is different from the usual Unix semantics.
		*/

		char *p = cmdLine;
		char *p_end = p + strlen(p);

		while ( *p && p < p_end )
		{
			while ( isspace( (uchar)*p ) )			// skip whitespace
				p++;

		    if (*p == '\0')
				break;

			if (*p == '"')
			{
				p++;
				if ( argc >= (int)argv.size()-1 )
					argv.resize( argv.size()*2 );
				argv[argc++] = p;
				while ( (*p != '\0') && (*p != '"') )
					p++;
			} else {
				if ( argc >= (int)argv.size()-1 )
					argv.resize( argv.size()*2 );
				argv[argc++] = p;
				while (*p != '\0' && !isspace( (uchar)*p ) )
					p++;
			}
			if (*p != '\0') {
				*p = '\0';
				p++;
			}
		}
		argv[argc] = 0;

		if (Server::instance()->run(argc, argv.data())) {
			returnValue_ = 0;
        } else {
			returnValue_ = 1;
		}

		if (returnValue_ != 0) {
			Console::instance()->send( "\nThe server has been shut down. You can close this window now.\n" );
			canClose = true;
		} else {
			PostMessage(mainWindow, WM_QUIT, 0, 0);
		}
	}
};

int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{
#if defined(_DEBUG)
/*	AllocConsole();
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = 500;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

	int hConHandle;
	long lStdHandle;
	FILE *fp;

	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );

	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stderr = *fp;
	setvbuf( stderr, NULL, _IONBF, 0 );

	QString consoleTitle = QString("%1 %2 %3 - Debug Console").arg(productString()).arg(productBeta()).arg(productVersion());
	SetConsoleTitle(consoleTitle.latin1());*/
#endif

	INITCOMMONCONTROLSEX initex;
	initex.dwICC = ICC_WIN95_CLASSES;
	initex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	InitCommonControlsEx(&initex);
#pragma comment(lib, "comctl32.lib") // needed for InitCommonControlsEx call
	appInstance = hInstance;
	guiThread = GetCurrentThreadId();

	// Try to load riched20.dll
	HMODULE hRiched = LoadLibrary("riched20.dll");

	if (!hRiched)
	{
		MessageBox( 0, "The riched20.dll library could not be found on your system.\nPlease install Microsoft Internet Explorer 4.0 or later.", "Missing DLL", MB_OK|MB_ICONERROR );
		return 1;
	}

	hbSeparator = CreateSolidBrush( RGB( 0xAF, 0xAF, 0xAF ) );
	hbBackground = CreateSolidBrush( RGB( 0, 64, 38 ) );
	iconGreen = (HICON)LoadImage(appInstance, MAKEINTRESOURCE(IDI_ICONGREEN), IMAGE_ICON, 16, 16, 0);
	iconRed = (HICON)LoadImage(appInstance, MAKEINTRESOURCE(IDI_ICONRED), IMAGE_ICON, 16, 16, 0);

	// Create the WindowClass
	WNDCLASSEX wpClass;
	ZeroMemory( &wpClass, sizeof( WNDCLASSEX ) );
	wpClass.cbSize = sizeof(WNDCLASSEX);
	wpClass.hInstance = hInstance;
	wpClass.lpfnWndProc = wpWindowProc;
	wpClass.hCursor = LoadCursor( NULL, MAKEINTRESOURCE( IDC_ARROW ) );
	wpClass.hIcon = LoadIcon( hInstance, MAKEINTRESOURCE( IDI_ICON1 ) );
	wpClass.hbrBackground = hbBackground;
	wpClass.lpszClassName = WOLFPACK_CLASS;
	wpClass.hIconSm = iconRed;
	wpClass.style = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;

	if (!RegisterClassEx(&wpClass))
	{
		MessageBox(0, "Couldn't register Window Class.", "Window Class", MB_OK|MB_ICONERROR);
		return 1;
	}

	// Create the Window itself
	hmMainMenu = LoadMenu( appInstance, MAKEINTRESOURCE( IDR_MAINMENU ) );
	mainWindow = CreateWindow( WOLFPACK_CLASS, "Wolfpack", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, hmMainMenu, hInstance, NULL );

	if( mainWindow == 0 )
	{
		MessageBox( 0, QString( "Couldn't create the window: " + getErrorString() ).latin1(), "Wolfpack", MB_OK|MB_ICONERROR );
		return 1;
	}

	ShowWindow( mainWindow, SW_NORMAL );

	// Create the System Tray Icon
	ZeroMemory(&icondata, sizeof(icondata));
	icondata.cbSize = sizeof(icondata);
	icondata.hWnd = mainWindow;
	icondata.uID = 0;
	icondata.uFlags = NIF_MESSAGE|NIF_ICON;
	icondata.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	icondata.uCallbackMessage = WM_TRAY_NOTIFY;

#if !defined(TTS_BALLOON)
# define TTS_BALLOON             0x40
#endif

	// This is "ported" from MFC
	tooltip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, WS_POPUP|TTS_NOPREFIX|TTS_ALWAYSTIP|TTS_BALLOON,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, mainWindow, NULL, hInstance, NULL);

	if (tooltip) {
		TOOLINFO info;
		info.cbSize = sizeof(info);
		info.uFlags = TTF_TRANSPARENT|TTF_CENTERTIP;
		info.hwnd = mainWindow;
		info.uId = 0;
		info.hinst = 0;
		info.lpszText = LPSTR_TEXTCALLBACK;
		GetClientRect(mainWindow, &info.rect);
		SendMessage(tooltip, TTM_ADDTOOL, 0, (LPARAM)&info);
	}
	Shell_NotifyIconA(NIM_ADD, (PNOTIFYICONDATAA)&icondata);

	cServerThread serverThread(lpCmdLine);
	serverThread.start();

	MSG msg;

	while( GetMessage( &msg, 0, 0, 0 ) > 0 )
	{
		if( msg.message == WM_CHAR && msg.hwnd == inputWindow && msg.wParam == '\r' )
		{
			if( Server::instance()->getState() == RUNNING )
			{
				char command[512] = { 0, };
				GetWindowText( inputWindow, command, 512 );
				SetWindowText( inputWindow, "" );

				// We are in a different Thread. Remember that.
				Console::instance()->queueCommand( command );
			}

			continue;
		}
		else if( msg.message == WM_TIMER )
		{
			char message[512];

			unsigned int msecs, seconds, minutes, hours, days;
			days = Server::instance()->time() / 86400000;
			hours = (Server::instance()->time() % 86400000) / 3600000;
			minutes = (( Server::instance()->time() % 86400000 ) % 3600000 ) / 60000;
			seconds = ((( Server::instance()->time() % 86400000 ) % 3600000 ) % 60000 ) / 1000;

			sprintf( message, "Uptime: %u:%02u:%02u:%02u", days, hours, minutes, seconds );
			SetWindowText( lblUptime, message );

			// Update the icon
			static unsigned int lastState = 0xFFFFFFFF;

			if (lastState != Server::instance()->getState())
			{
				if (Server::instance()->getState() == RUNNING)
				{
					SendMessage(mainWindow, WM_SETICON, ICON_SMALL, (WPARAM)iconGreen);
					SendMessage(statusIcon, STM_SETIMAGE, IMAGE_ICON, (LPARAM)iconGreen);
				}
				else
				{
					SendMessage(mainWindow, WM_SETICON, ICON_SMALL, (WPARAM)iconRed);
					SendMessage(statusIcon, STM_SETIMAGE, IMAGE_ICON, (LPARAM)iconRed);
				}
			}
			lastState = Server::instance()->getState();
		}

		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	Shell_NotifyIconA(NIM_DELETE, (PNOTIFYICONDATAA)&icondata);

	Server::instance()->cancel();

	serverThread.wait();

	return serverThread.returnValue();
}

void cConsole::start()
{
}

void cConsole::poll()
{
	// Poll for new Commands
	commandMutex.lock();
	QStringList commands = commandQueue;
	commandQueue.clear();
	commandMutex.unlock();

	while( commands.count() > 0 )
	{
		handleCommand( commands.front() );
		commands.pop_front();
	}
}

void cConsole::stop()
{
}

void cConsole::send(const QString &sMessage)
{
	// If a progress message is waiting, remove it.
	if (!progress.isEmpty()) {
		QString temp = progress;
		progress = QString::null;
		for (uint i = 0; i < temp.length() + 4; ++i) {
			send("\b");
		}
		progress = temp;
	}

	unsigned int ctrlLength = GetWindowTextLength( logWindow );
	unsigned int textLength = sMessage.length();

	// Check for the caret
	SendMessage( logWindow, EM_SETSEL, ctrlLength, ctrlLength );

	// Delete lines from the beginning if we exceed the maximum limit.
	if( ctrlLength + textLength > logLimit )
	{
		unsigned int linecount = 0;
		unsigned int textcount = 0;

		do
		{
			char buffer[1024] = { 0, };
			((short*)buffer)[0] = 1024;
			textcount += SendMessage( logWindow, EM_GETLINE, linecount++, (WPARAM)buffer );	// We have to wait here.
		}
		while( textcount < ( ctrlLength + textLength ) - logLimit );

		SendMessage( logWindow, EM_SETSEL, 0, textcount );
		SendMessage( logWindow, EM_REPLACESEL, FALSE, (LPARAM)"" );
	}

	// process \b properly
	if ( sMessage.contains("\b") )
	{
		// Split the message
		uint pos = sMessage.find("\b");
		if ( pos > 0 )
			send( sMessage.right(pos));
		else
		{
			CHARRANGE range;
			SendMessage( logWindow, EM_EXGETSEL, 0, (LPARAM)&range );
			range.cpMin -= 1;
			SendMessage( logWindow, EM_EXSETSEL, 0, (LPARAM)&range );
			SendMessage( logWindow, EM_REPLACESEL, FALSE, 0 );

			if (sMessage.length() > 1) {
				send( sMessage.left( sMessage.length() - 1 ) );
			}
			return;
		}
	}

	unsigned int tLength = GetWindowTextLength(logWindow);
	SendMessage(logWindow, EM_SETSEL, tLength, tLength);

	// Set it to the current charformat
	SendMessage(logWindow, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

	// Now it will get right, even if the user had selected sth.
	SendMessage(logWindow, EM_REPLACESEL, FALSE, (LPARAM)sMessage.latin1());

	// And ofcourse if not some control is currently capturing the input
	if( !GetCapture() )
		SendMessage( logWindow, WM_VSCROLL, SB_BOTTOM, 0 );

	// Update linebuffer_, so that web console works as well.
	if (sMessage.contains("\n")) {
		incompleteLine_.append( sMessage ); // Split by \n
		QStringList lines = QStringList::split( "\n", incompleteLine_, true );

		// Insert all except the last element
		for( uint i = 0; i < lines.count() - 1; ++i )
			linebuffer_.push_back( lines[i] );

		incompleteLine_ = lines[ lines.count() - 1 ];
	} else {
		incompleteLine_.append(sMessage);
	}

	// Resend the Progress message if neccesary.
	if (!progress.isEmpty()) {
		QString temp = progress;
		progress = QString::null;
		sendProgress(temp);
	}
}

void cConsole::changeColor(enConsoleColors color) {
	unsigned int tLength = GetWindowTextLength(logWindow);
	SendMessage(logWindow, EM_SETSEL, tLength, tLength);

	ZeroMemory(&cf, sizeof(CHARFORMAT));
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR;

	switch (color) {
	case WPC_GREEN:
		cf.crTextColor = RGB(0x00,0xFF,0x00);
		break;

	case WPC_RED:
		cf.crTextColor = RGB(0xFF,0x00,0x00);
		break;

	case WPC_YELLOW:
		cf.crTextColor = RGB(0x00,0xFF,0xFF);
		break;

	case WPC_BROWN:
		cf.crTextColor = RGB(204, 204, 153);
		break;

	case WPC_NORMAL:
        cf.crTextColor = RGB(0xAF,0xAF,0xAF);
		break;

	case WPC_WHITE:
		cf.crTextColor = RGB(0xFF,0xFF,0xFF);
		break;

	};
}

void cConsole::setConsoleTitle(const QString& data) {
	SetWindowText(mainWindow, data.latin1());
}

void cConsole::setAttributes( bool bold, bool italic, bool underlined, unsigned char r, unsigned char g, unsigned char b, unsigned char size, enFontType font )
{
	CHARFORMAT cf;
	ZeroMemory( &cf, sizeof( CHARFORMAT ) );
	cf.cbSize = sizeof( CHARFORMAT );

	SendMessage(logWindow, EM_GETCHARFORMAT, SCF_SELECTION, (WPARAM)&cf);

	if( bold )
	{
		cf.dwMask |= CFM_BOLD;
		cf.dwEffects |= CFE_BOLD;
	}

	if( italic )
	{
		cf.dwMask |= CFM_ITALIC;
		cf.dwEffects |= CFE_ITALIC;
	}

	if( underlined )
	{
		cf.dwMask |= CFM_UNDERLINE;
		cf.dwEffects |= CFE_UNDERLINE;
	}

	cf.dwMask |= CFM_COLOR;
	cf.crTextColor = RGB( r, g, b );

	if( size )
	{
		cf.dwMask |= CFM_SIZE;
		cf.yHeight = size * 20;
	}

	cf.dwMask |= CFM_FACE;

	switch( font )
	{
	case FONT_SERIF:
		strcpy( cf.szFaceName, "Courier" );
		break;

	case FONT_NOSERIF:
		strcpy( cf.szFaceName, "Arial" );
		break;

	case FONT_FIXEDWIDTH:
		strcpy( cf.szFaceName, "Fixedsys" );
		break;
	}

	SendMessage( logWindow, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf );
}

void cConsole::notifyServerState(enServerState newstate) {

	// Required ugly stuff
#if !defined (NIF_INFO)
# define NIF_INFO        0x00000010
#endif
#if !defined(NIIF_INFO)
# define NIIF_INFO       0x00000001
#endif

	icondata.uFlags = NIF_ICON;

	if (newstate == RUNNING) {
		icondata.hIcon = iconGreen;
	} else {
		icondata.hIcon = iconRed;
	}

#if (_WIN32_IE >= 0x0500)
	qstrcpy(icondata.szInfoTitle, "Wolfpack Server Status");
	// Startup has finished
	if (Server::instance()->getState() == STARTUP && newstate == RUNNING)
	{
		icondata.uFlags |= NIF_INFO;
		icondata.uTimeout = 2500;
		icondata.dwInfoFlags = NIIF_INFO;
		qstrcpy(icondata.szInfo, "Wolfpack has started up and is now ready to use.");
	}
	else if (Server::instance()->getState() == SCRIPTRELOAD && newstate == RUNNING)
	{
		icondata.uFlags |= NIF_INFO;
		icondata.uTimeout = 2500;
		icondata.dwInfoFlags = NIIF_INFO;
		qstrcpy(icondata.szInfo, "Wolfpack has finished reloading the scripts.");
	}
	else if (newstate == SHUTDOWN)
	{
		icondata.uFlags |= NIF_INFO;
		icondata.uTimeout = 2500;
		icondata.dwInfoFlags = NIIF_INFO;
		qstrcpy(icondata.szInfo, "Wolfpack is now shutting down.");
	}
#endif

	Shell_NotifyIconA(NIM_MODIFY, (PNOTIFYICONDATAA)&icondata);
}
