/*
*     Wolfpack Emu (WP)
* UO Server Emulation Program
*
* Copyright 2001-2005 by holders identified in AUTHORS.txt
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
* Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
*/

#include "mainwindow.h"
#include "qwpevents.h"

#include "../console.h"
#include "../server.h"

MainWindow::MainWindow() : QMainWindow( 0, 0 )
{
	ui.setupUi( this );

	// Fire up the server thread
	Server::instance()->start();
}

bool MainWindow::event( QEvent* e )
{
	switch ( e->type() )
	{
	case QEvent::User + 1:
		ui.logWindow->append( static_cast<QWPConsoleSendEvent*>(e)->message() );
	default:
		break;
	}
	return QMainWindow::event( e );
}


/*

	Wolfpack Console class Interface
*/

extern MainWindow* mainWindow; // Ugly, but well... easier.

void cConsole::send( const QString& sMessage )
{
	QApplication::postEvent( mainWindow, new QWPConsoleSendEvent( sMessage ) );
}

void cConsole::rollbackChars( unsigned int count )
{

}

void cConsole::changeColor( enConsoleColors color )
{

}

void cConsole::setConsoleTitle( const QString& title )
{

}

void cConsole::setAttributes( bool bold, bool italic, bool underlined, unsigned char r, unsigned char g, unsigned char b, unsigned char size, enFontType font )
{
}

void cConsole::notifyServerState( enServerState newstate )
{
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

