//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2003 by holders identified in authors.txt
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================

// System Includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <richedit.h>
#include <qthread.h>

// Wolfpack Includes
#include "console.h"
#include "resource.h"
#include "globals.h"

/*
	This file includes the Windows GUI implementation of our Console.
 */

#define WOLFPACK_CLASS "wolfpack_class"

extern int main( int argc, char **argv );

// Variables important for this GUI implementation
#define CONTROL_LOGWINDOW 0x10
#define CONTROL_INPUT 0x11

HMENU hmMainMenu;
HBITMAP hLogo = 0;
HBRUSH hbSeparator = 0, hbBackground = 0;
HWND logWindow = 0;			// Log Window
HWND inputWindow = 0;		// Input Textfield
HWND mainWindow = 0;		// Main Window
HINSTANCE appInstance = 0;	// Application Instance
HFONT font = 0;				// The font we'll use
unsigned int inputHeight = 0; // For measuring the height of the input field
unsigned int logLimit = 0;	// How many characters fit into the log window
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

// Fill a rectangular on a specific context
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
	GetClientRect( window,  &rect );

	paintRect( dc, 0, 0, rect.right, 87, hbBackground );
	paintRect( dc, 0, 87, rect.right, 1, hbSeparator );

	// Draw our Logo
	HDC tempdc = CreateCompatibleDC( dc );
	HGDIOBJ oldobj = SelectObject( tempdc, hLogo );

	BITMAP bm;
	GetObject( hLogo, sizeof(bm), &bm );

	BitBlt( dc, 0, 0, bm.bmWidth, bm.bmHeight, tempdc, 0, 0, SRCCOPY );

	SelectObject( tempdc, oldobj );
	DeleteDC( tempdc );

	EndPaint( window, &paintInfo );
}

bool handleMenuSelect( unsigned int id )
{
	bool result = true;

	switch( id )
	{
	case IDC_EXIT:
		keeprun = 0;

		if( canClose )
			DestroyWindow( mainWindow );

		break;

	case ID_RELOAD_ACCOUNTS:
		Console::instance()->send( "RELOADING ACCOUNTS\n" );
		break;

	case ID_RELOAD_PYTHON:
		Console::instance()->send( "RELOADING PYTHON\n" );
		break;

	case ID_RELOAD_SCRIPTS:
		Console::instance()->send( "RELOADING SCRIPTS\n" );
		break;

	case ID_RELOAD_CONFIGURATION:
		Console::instance()->send( "RELOADING CONFIGURATION\n" );
		break;

	default:
		result = false;
	}

	return result;
}

LRESULT CALLBACK wpWindowProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	if( mainWindow && hwnd != mainWindow )
		return DefWindowProc( hwnd, msg, wparam, lparam ); 

	CHARFORMAT cf;	
	LOGFONT lfont;
	NMHDR *notify = (NMHDR*)lparam;

	switch( msg )
	{
	case WM_COMMAND:
		if( handleMenuSelect( wparam ) )
			return 0;
		break;

	case WM_CREATE:
		hLogo = LoadBitmap( appInstance, MAKEINTRESOURCE( IDB_LOGO ) );
		hbSeparator = CreateSolidBrush( RGB( 0xAF, 0xAF, 0xAF ) );
		hbBackground = CreateSolidBrush( RGB( 0, 64, 38 ) );

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

	case WM_ERASEBKGND:
		return 1;

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
		keeprun = 0;
		
		if( canClose )
			DestroyWindow( mainWindow );

		return 1;

	case WM_DESTROY:
		DestroyMenu( hmMainMenu );
		DeleteObject( hLogo );
		DeleteObject( hbSeparator );
		DeleteObject( hbBackground );
		keeprun = 0;
		PostQuitMessage( 0 );
		return 0;
	}

	return DefWindowProc( hwnd, msg, wparam, lparam ); 
}

class cGuiThread : public QThread
{
	LPSTR cmdLine;
	int returnValue_;

public:

	cGuiThread( LPSTR lpCmdLine ) : cmdLine( lpCmdLine ) {}

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

		returnValue_ = main( argc, argv.data() );

		if( returnValue_ != 0 )
		{
			Console::instance()->send( "\nThe server has been shut down. You can close this window now.\n" );
			canClose = true;
		}
		else
		{
			PostMessage( mainWindow, WM_QUIT, 0, 0 );
		}
	}
};

int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{
	appInstance = hInstance;

	// Try to load riched20.dll
	HMODULE hRiched = LoadLibrary( "riched20.dll" );

	if( hRiched == 0 )
	{
		MessageBox( 0, "The riched20.dll library could not be found on your system.\nPlease install Microsoft Internet Explorer 4.0 or later.", "Riched missing", MB_OK|MB_ICONERROR );
		return 1;
	}

	// Create the WindowClass
	WNDCLASS wpClass;
	ZeroMemory( &wpClass, sizeof( WNDCLASS ) );
	wpClass.hInstance = hInstance;
	wpClass.lpfnWndProc = wpWindowProc;
	wpClass.hIcon = LoadIcon( hInstance, MAKEINTRESOURCE( IDI_ICON1 ) );
	wpClass.hbrBackground = GetSysColorBrush( COLOR_BTNFACE );
	wpClass.lpszClassName = WOLFPACK_CLASS;
	wpClass.style = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;
	
	if( !RegisterClass( &wpClass ) )
	{
		MessageBox( 0, "Couldn't register Window Class.", "Window Class", MB_OK|MB_ICONERROR );
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

	cGuiThread guiThread( lpCmdLine );
	guiThread.start();

	MSG msg;

	while( GetMessage( &msg, 0, 0, 0 ) > 0 )
	{
		if( msg.message == WM_CHAR && msg.hwnd == inputWindow && msg.wParam == '\r' )
		{
			if( serverState == RUNNING )
			{
				char command[512] = { 0, };
				GetWindowText( inputWindow, command, 512 );
				SetWindowText( inputWindow, "" );
      
				// We are in a different Thread. Remember that.
				Console::instance()->queueCommand( command );
			}

			continue;
		}

		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	keeprun = 0; // We quit, so let's quit the server too

	guiThread.wait();
	
	return guiThread.returnValue();
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

	// Place the Caret at the End of the Text
	

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
			send( sMessage.left( sMessage.length() - 1 ) );
			return;
		}
	}

	unsigned int tLength = GetWindowTextLength( logWindow );
	SendMessage( logWindow, EM_SETSEL, tLength, tLength );

	// Now it will get right, even if the user had selected sth.
	SendMessage( logWindow, EM_REPLACESEL, FALSE, (LPARAM)sMessage.latin1() );

	// And ofcourse if not some control is currently capturing the input
	if( !GetCapture() )
		SendMessage( logWindow, WM_VSCROLL, SB_BOTTOM, 0 );
}

void cConsole::ChangeColor( WPC_ColorKeys color )
{
	CHARFORMAT cf;
	ZeroMemory( &cf, sizeof( CHARFORMAT ) );
	cf.cbSize = sizeof( CHARFORMAT );
	cf.dwMask = CFM_COLOR;
	
	switch( color )
	{
	case WPC_GREEN:
		cf.crTextColor = RGB( 0x00,0xFF,0x00 );
		break;

	case WPC_RED:
		cf.crTextColor = RGB( 0xFF,0x00,0x00 );
		break;

	case WPC_YELLOW:
		cf.crTextColor = RGB( 0x00,0xFF,0xFF );
		break;

	case WPC_BROWN:
		cf.crTextColor = RGB( 204, 204, 153 );
		break;

	case WPC_NORMAL:
        cf.crTextColor = RGB( 0xAF,0xAF,0xAF );
		break;

	case WPC_WHITE:
		cf.crTextColor = RGB( 0xFF,0xFF,0xFF );
		break;

	};

	SendMessage( logWindow, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf );
}

void cConsole::setConsoleTitle( const QString& data )
{
	SetWindowText( mainWindow, data.latin1() );
}

// Extended Attributes
void cConsole::setAttributes( bool bold, bool italic, bool underlined, unsigned char r, unsigned char g, unsigned char b, unsigned char size, eFontType font )
{
	CHARFORMAT cf;
	ZeroMemory( &cf, sizeof( CHARFORMAT ) );
	cf.cbSize = sizeof( CHARFORMAT );

	SendMessage( logWindow, EM_GETCHARFORMAT, SCF_SELECTION, (WPARAM)&cf );

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

