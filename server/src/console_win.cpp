
// System Includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
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
HWND logWindow = 0;			// Log Window
HWND inputWindow = 0;		// Input Textfield
HWND mainWindow = 0;		// Main Window
HINSTANCE appInstance = 0;	// Application Instance
HFONT font = 0;				// The font we'll use
unsigned int inputHeight = 0; // For measuring the height of the input field

/*
	Directly taken from MSDN
 */
static QString getErrorString()
{
	LPVOID lpMsgBuf;

	if( !FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, 0, GetLastError(), MAKELANGID( LANG_NEUTRAL , SUBLANG_DEFAULT ), (LPTSTR)&lpMsgBuf, 0, 0 ) )
	   return QString( "Unknown Error" );

	QString result( (char*)lpMsgBuf );

	// Free the buffer.
	LocalFree( lpMsgBuf );

	return result;
}

LRESULT CALLBACK wpWindowProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	CHARFORMAT cf;
	LOGFONT lfont;

	switch( msg )
	{
	case WM_CREATE:
		// Create Richedit Box
		logWindow = CreateWindow( RICHEDIT_CLASS, 0, ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY | WS_CHILD | WS_VISIBLE | WS_VSCROLL, 0, 0, 10, 10, hwnd, 0, appInstance, 0 );

		if( logWindow == 0 )
		{
			MessageBox( 0, QString( "Couldn't create the logwindow: " + getErrorString() ).latin1(), "Wolfpack", MB_OK|MB_ICONERROR );
			DestroyWindow( hwnd );
			return TRUE;
		}

		// Set up the fonts we need        
		ZeroMemory( &lfont, sizeof( LOGFONT ) );
		qstrcpy( lfont.lfFaceName, "Courier" );
		font = CreateFontIndirect( &lfont );
           
		// Set the font of our logwindow
		SendMessage( logWindow, WM_SETFONT, (WPARAM)font, 0 );
		SendMessage( logWindow, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(0,0,0) );

		// Default Charformat
        ZeroMemory( &cf, sizeof( CHARFORMAT ) );
        cf.cbSize = sizeof( CHARFORMAT );
        cf.dwMask = CFM_COLOR;
        cf.crTextColor = RGB( 0xAF,0xAF,0xAF ); 

		SendMessage( logWindow, EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM)&cf );
		SendMessage( logWindow, EM_AUTOURLDETECT, 1, 0 );
		SendMessage( logWindow, EM_SETEVENTMASK, 0, ENM_LINK|ENM_MOUSEEVENTS|ENM_KEYEVENTS );

		// Create InputWindow
		inputWindow = CreateWindow( "EDIT", 0, ES_LEFT|ES_AUTOHSCROLL|WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP, 0, 0, 10, 10, hwnd, 0, appInstance, 0 );

		return 0;
	
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

			MoveWindow( logWindow, 0, 0, width, height - inputHeight, TRUE );
			MoveWindow( inputWindow, 0, height - inputHeight, width, inputHeight, TRUE );
		}

		return 0;
		
	case WM_DESTROY:
		keeprun = 0;
		return 0;

	default:
		return DefWindowProc( hwnd, msg, wparam, lparam ); 
	}
}

class cGuiThread : public QThread
{
protected:
	virtual void run()
	{
		char **argv = (char**)malloc( 1 * sizeof( char* ) );
		argv[0] = "wolfpack.exe";
		int argc = 1;

		main( argc, argv );	

		free( argv[0] );
		free( argv );

		PostQuitMessage( 0 );
	}
};

cGuiThread *guiThread = 0;

int WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
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
	wpClass.hIcon = LoadIcon( hInstance, MAKEINTRESOURCE( IDI_ICON2 ) );
	wpClass.hbrBackground = GetSysColorBrush( COLOR_BTNFACE );
	wpClass.lpszClassName = WOLFPACK_CLASS;
	wpClass.style = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;
	
	if( !RegisterClass( &wpClass ) )
	{
		MessageBox( 0, "Couldn't register Window Class.", "Window Class", MB_OK|MB_ICONERROR );
		return 1;
	}

	// Create the Window itself
	mainWindow = CreateWindow( WOLFPACK_CLASS, "Wolfpack", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 400, NULL, 0, hInstance, NULL );

	if( mainWindow == 0 )
	{
		MessageBox( 0, QString( "Couldn't create the window: " + getErrorString() ).latin1(), "Wolfpack", MB_OK|MB_ICONERROR );
		return 1;
	}

	ShowWindow( mainWindow, SW_NORMAL );

	guiThread = new cGuiThread;
	guiThread->start();

	MSG msg;

	while( GetMessage( &msg, mainWindow, 0, 0 ) > 0 )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	keeprun = 0; // We quit, so let's quit the server too

	guiThread->wait();
	delete guiThread;

	return 0;
}

void cConsole::start()
{
}

void cConsole::poll()
{
}

void cConsole::stop()
{
}

void cConsole::send(const QString &sMessage)
{
	// process \b properly

	SendMessage( logWindow, EM_REPLACESEL, FALSE, (LPARAM)sMessage.latin1() );
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
