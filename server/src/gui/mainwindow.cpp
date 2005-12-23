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
	connect( Server::instance(), SIGNAL(finished()), this, SLOT(onServerStoped()) );
}

bool MainWindow::event( QEvent* e )
{
	if ( e->type() > QEvent::User )
	{
		switch ( e->type() )
		{
		case QEvent::User + 1:
			handleConsoleMessage( static_cast<QWPConsoleSendEvent*>(e)->message() );
			break;
		case QEvent::User + 2:
			setWindowTitle( static_cast<QWPConsoleTitleEvent*>(e)->title() );
			break;
		case QEvent::User + 3:
			ui.logWindow->setTextColor( static_cast<QWPConsoleChangeColorEvent*>(e)->color() );
			break;
		case QEvent::User + 4:
			ui.logWindow->setCurrentFont( static_cast<QWPConsoleChangeFontEvent*>(e)->format() );
		default:
			qDebug("MainWindow: Unknown custom event!");
			return QMainWindow::event( e );
		}
		e->accept();
		return true;
	}
	else
		return QMainWindow::event( e );
	return false; // Keep some compilers happy
}

void MainWindow::closeEvent ( QCloseEvent * e ) 
{
	Server::instance()->cancel();
	if ( Server::instance()->isFinished() )
	{
		e->accept();
	}
	else
		e->ignore();
}

void MainWindow::handleConsoleMessage( const QString& msg )
{
	QString message ( msg );
	if ( message.endsWith('\n') )
		message.chop(1);
	ui.logWindow->append( message );
}

void MainWindow::onServerStoped()
{
	handleConsoleMessage( tr( "\nThe server has been shut down. You can close this window now.\n" ) );
}
/*

	Wolfpack Console class Interface
*/

extern MainWindow* mainWindow; // Ugly, but well... easier.

void cConsole::send( const QString& sMessage )
{
	// If a progress message is waiting, remove it.
	if ( !progress.isEmpty() )
	{
		QString temp = progress;
		progress = QString::null;
		rollbackChars( temp.length() );
		progress = temp;
	}

	QApplication::postEvent( mainWindow, new QWPConsoleSendEvent( sMessage ) );
	// Append to the linebuffer
	linebuffer_.append( sMessage );

	// Resend the Progress message if neccesary.
	if ( !progress.isEmpty() )
	{
		QString temp = progress;
		progress = QString::null;
		send( temp );
		progress = temp;
	}

}

void cConsole::rollbackChars( unsigned int count )
{

}

void cConsole::changeColor( enConsoleColors color )
{
	QColor qcolor;
	switch ( color )
	{
	case WPC_GREEN:
		qcolor.setRgb( 0x00, 0xFF, 0x00 );
		break;

	case WPC_RED:
		qcolor.setRgb( 0xFF, 0x00, 0x00 );
		break;

	case WPC_YELLOW:
		qcolor.setRgb( 0x00, 0xFF, 0xFF );
		break;

	case WPC_BROWN:
		qcolor.setRgb( 204, 204, 153 );
		break;

	case WPC_NORMAL:
		qcolor.setRgb( 0xAF, 0xAF, 0xAF );
		break;

	case WPC_WHITE:
		qcolor.setRgb( 0xFF, 0xFF, 0xFF );
		break;
	}
	QApplication::postEvent( mainWindow, new QWPConsoleChangeColorEvent( qcolor ) );
}

void cConsole::setConsoleTitle( const QString& title )
{
	QApplication::postEvent( mainWindow, new QWPConsoleTitleEvent( title ) );
}

void cConsole::setAttributes( bool bold, bool italic, bool underlined, unsigned char r, unsigned char g, unsigned char b, unsigned char size, enFontType font )
{
	QTextCharFormat format;
	QFont f;
	switch ( font )
	{
	case FONT_SERIF:
		f.setFamily( "Courier" );
		break;

	case FONT_NOSERIF:
		f.setFamily( "Arial" );
		break;

	case FONT_FIXEDWIDTH:
		f.setFamily( "Fixedsys" );
		break;
	}
	f.setBold( bold );
	f.setUnderline( underlined );
	if ( size > 0 )
		f.setPointSize( size );
	QApplication::postEvent( mainWindow, new QWPConsoleChangeFontEvent( f ) );
	QApplication::postEvent( mainWindow, new QWPConsoleChangeColorEvent( QColor(r, g, b) ) );
}

void cConsole::notifyServerState( enServerState newstate )
{
}

void cConsole::start()
{
}

// Yeah, it's exactly the same in all 3 files, we could consolidade in console.cpp
void cConsole::poll()
{
	// Poll for new Commands
	commandMutex.lock();
	QStringList commands = commandQueue;
	commandQueue.clear();
	commandMutex.unlock();

	while ( commands.count() > 0 )
	{
		handleCommand( commands.front() );
		commands.pop_front();
	}
}

void cConsole::stop()
{
}
