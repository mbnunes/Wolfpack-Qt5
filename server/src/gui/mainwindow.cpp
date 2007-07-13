/*
*     Wolfpack Emu (WP)
* UO Server Emulation Program
*
* Copyright 2001-2006 by holders identified in AUTHORS.txt
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
#include <QtGui>

#include "mainwindow.h"
#include "qwpevents.h"
#include "profilerwindow.h"
#include "pythoninteractivewindow.h"
#include "trayicon/trayicon.h"

#include "../console.h"
#include "../server.h"
#include "../exportdefinitions.h"
#include "../network/network.h"
#include "../network/uosocket.h"
#include "../player.h"
#include "../python/pyprofiler.h"

MainWindow::MainWindow()
{
	ui.setupUi( this );

	setWindowIcon( QIcon(":/gui/images/icon_red.png") );
	// Start Menus
	createActions();
    createMenus();

	// Setup trayicon
	trayicon = new TrayIcon( this );
	trayicon->show();
	// Fire up the server thread
	Server::instance()->start();
	connect( Server::instance(), SIGNAL(finished()), this, SLOT(onServerStoped()) );

	// Push Python Profiler to Wolfpack's thread
	PyProfiler::instance()->moveToThread( Server::instance() );
	connect( PyProfiler::instance(), SIGNAL(stopped()), this, SLOT(profilerStopped()) );
}

MainWindow::~MainWindow()
{
	trayicon->hide();
}

bool MainWindow::event( QEvent* e )
{
	if ( e->type() >= QEvent::User )
	{
		QWolfpackConsoleEvent* event = static_cast<QWolfpackConsoleEvent*>( e );
		switch ( e->type() )
		{
		case QWolfpackConsoleEvent::SendEvent:
			handleConsoleMessage( event->data().toString() );
			break;
		case QWolfpackConsoleEvent::SetTitleEvent:
			setWindowTitle( event->data().toString() );
			break;
		case QWolfpackConsoleEvent::ChangeColorEvent:
			ui.logWindow->setTextColor( (QRgb)event->data().toInt() );
			break;
		case QWolfpackConsoleEvent::ChangeFontEvent:
			ui.logWindow->setCurrentFont( event->font() );
			break;
		case QWolfpackConsoleEvent::RollbackCharsEvent:
			handleConsoleRollbackChars( event->data().toInt() );
			break;
		case QWolfpackConsoleEvent::NotifyState:
			handleConsoleNotifyState( (enServerState)event->data().toInt() );
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
	// Save and restore current char format
	QTextCharFormat format = ui.logWindow->currentCharFormat();

	QString message ( msg );
	QTextCursor cursor = ui.logWindow->textCursor();
	cursor.movePosition( QTextCursor::End );
	ui.logWindow->setTextCursor( cursor );
	ui.logWindow->setCurrentCharFormat(format);
	ui.logWindow->insertPlainText( message );
	ui.logWindow->ensureCursorVisible();
}

void MainWindow::handleConsoleRollbackChars( unsigned int count )
{
	// Save and restore current char format
	QTextCharFormat format = ui.logWindow->currentCharFormat();

	QTextCursor cursor = ui.logWindow->textCursor();
	cursor.setPosition( cursor.position() - count, QTextCursor::KeepAnchor );
	cursor.removeSelectedText();
	ui.logWindow->setTextCursor( cursor );

	ui.logWindow->setCurrentCharFormat(format);
}

void MainWindow::handleConsoleNotifyState( enServerState s )
{
	if ( s == RUNNING )
	{
		setWindowIcon(QIcon(":/gui/images/icon_green.png"));
		trayicon->setIcon( QIcon(":/gui/images/icon_green.png") );
	}
	else
	{
		setWindowIcon(QIcon(":/gui/images/icon_red.png"));
		trayicon->setIcon( QIcon(":/gui/images/icon_red.png") );
	}
}

void MainWindow::onServerStoped()
{
	handleConsoleMessage( tr( "\nThe server has been shut down. You can close this window now.\n" ) );
}

/*

    Generating all Menus

*/
void MainWindow::createMenus()
{
	// File Menu
    fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(expdefAct);
	fileMenu->addAction(exitAct);

	// Reload Menu
	reloadMenu = menuBar()->addMenu(tr("&Reload"));
	reloadMenu->addAction(reloadAccountsAct);
	reloadMenu->addAction(reloadConfigAct);
	reloadMenu->addAction(reloadPythonAct);
	reloadMenu->addAction(reloadScriptsAct);

	// Server Menu
	serverMenu = menuBar()->addMenu(tr("&Server"));
	serverMenu->addAction(serverSaveAct);
	serverMenu->addAction(serverUsersAct);

	// Scripting Menu
	scriptingMenu = menuBar()->addMenu( tr("&Scripting") );
	scriptingMenu->addAction( scriptingPythonInterpreter );
	QMenu* subMenu = scriptingMenu->addMenu( tr("&Profiler") );
	subMenu->addAction( scriptingProfilerStart );
	subMenu->addAction( scriptingProfilerStop );

	// Help Menu
	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(helpHPAct);
	helpMenu->addAction(helpAboutAct);
}
/*

    Actions Section

*/
void MainWindow::createActions()
{
	// File
    expdefAct = new QAction(tr("Export &Definitions"), this);
    expdefAct->setShortcut(tr("Ctrl+D"));
    expdefAct->setStatusTip(tr("Export Definitions"));
    connect(expdefAct, SIGNAL(triggered()), this, SLOT(exportDefs()));

	exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+X"));
    exitAct->setStatusTip(tr("Exit"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(closeWP()));

	// Reload
	reloadAccountsAct = new QAction(tr("&Accounts"), this);
    reloadAccountsAct->setShortcut(tr("Ctrl+A"));
    reloadAccountsAct->setStatusTip(tr("Accounts"));
    connect(reloadAccountsAct, SIGNAL(triggered()), this, SLOT(reloadAcc()));

	reloadConfigAct = new QAction(tr("&Configuration"), this);
    reloadConfigAct->setShortcut(tr("Ctrl+C"));
    reloadConfigAct->setStatusTip(tr("Configuration"));
    connect(reloadConfigAct, SIGNAL(triggered()), this, SLOT(reloadCfg()));

	reloadPythonAct = new QAction(tr("&Python"), this);
    reloadPythonAct->setShortcut(tr("Ctrl+P"));
    reloadPythonAct->setStatusTip(tr("Python"));
    connect(reloadPythonAct, SIGNAL(triggered()), this, SLOT(reloadPyt()));

	reloadScriptsAct = new QAction(tr("&Scripts"), this);
    reloadScriptsAct->setShortcut(tr("Ctrl+S"));
    reloadScriptsAct->setStatusTip(tr("Scripts"));
    connect(reloadScriptsAct, SIGNAL(triggered()), this, SLOT(reloadScp()));

	// Server
	serverSaveAct = new QAction(tr("Save &World"), this);
    serverSaveAct->setShortcut(tr("Ctrl+W"));
    serverSaveAct->setStatusTip(tr("Save World"));
    connect(serverSaveAct, SIGNAL(triggered()), this, SLOT(saveworld()));

	serverUsersAct = new QAction(tr("List &Users"), this);
    serverUsersAct->setShortcut(tr("Ctrl+U"));
    serverUsersAct->setStatusTip(tr("List Users"));
    connect(serverUsersAct, SIGNAL(triggered()), this, SLOT(listusers()));

	// Scripting - Profiler
	scriptingPythonInterpreter = new QAction( tr("Interactive Interpreter"), this );
	scriptingPythonInterpreter->setStatusTip( tr("Opens a window with an interactive Python interpreter") );
	connect( scriptingPythonInterpreter, SIGNAL(triggered()), this, SLOT(newInterpreterWindow()) );
	scriptingProfilerStart = new QAction( tr("Start"), this );
	connect( scriptingProfilerStart, SIGNAL(triggered()), PyProfiler::instance(), SLOT(start()), Qt::QueuedConnection);

	scriptingProfilerStop = new QAction( tr("Stop"), this );
	connect( scriptingProfilerStop, SIGNAL(triggered()), PyProfiler::instance(), SLOT(stop()), Qt::QueuedConnection);


	// Help
	helpHPAct = new QAction(tr("Wolfpack &HomePage"), this);
    helpHPAct->setShortcut(tr("Ctrl+H"));
    helpHPAct->setStatusTip(tr("Wolfpack HomePage"));
    connect(helpHPAct, SIGNAL(triggered()), this, SLOT(homepage()));

	helpAboutAct = new QAction(tr("&About"), this);
    helpAboutAct->setShortcut(tr("Ctrl+A"));
    helpAboutAct->setStatusTip(tr("About"));
    connect(helpAboutAct, SIGNAL(triggered()), this, SLOT(about()));
}
/*
	The Actions
*/
void MainWindow::exportDefs()
{
	cDefinitionExporter exporter;
	exporter.generate("categories.db");
}

void MainWindow::closeWP()
{
	close();
}

void MainWindow::reloadAcc()
{
	Server::instance()->queueAction( RELOAD_ACCOUNTS );
}

void MainWindow::reloadCfg()
{
	Server::instance()->queueAction( RELOAD_CONFIGURATION );
}

void MainWindow::reloadPyt()
{
	Server::instance()->queueAction( RELOAD_PYTHON );
}

void MainWindow::reloadScp()
{
	Server::instance()->queueAction( RELOAD_SCRIPTS );
}

void MainWindow::saveworld()
{
	Server::instance()->queueAction( SAVE_WORLD );
}

void MainWindow::listusers()
{
	// Vars
	cUOSocket* mSock;
	unsigned int i;

	// We simply do our thread safety manually here
	Network::instance()->lock();

	// Generate a list of Users
	i = 0;

	QList<cUOSocket*> sockets = Network::instance()->sockets();
	foreach ( mSock, sockets )
	{
		if ( mSock->player() )
			Console::instance()->send( QString( "%1) %2 [%3]\n" ).arg( ++i ).arg( mSock->player()->name() ).arg( QString::number( mSock->player()->serial(), 16 ) ) );
	}

	Network::instance()->unlock();

	Console::instance()->send( tr( "Total Users Online: %1\n" ).arg( i ) );
}

#if defined( Q_OS_WIN )
#include <windows.h>
#endif

void MainWindow::homepage()
{
#if defined ( Q_OS_WIN )
	ShellExecuteA(NULL, "open", "http://www.wpdev.org", NULL, NULL, SW_SHOWNORMAL);
#else
	QMessageBox::information( this, tr("Wolfpack homepage"), tr("Wolfpack doesn't know how to start your default browser\nPlease access http://www.wpdev.org/ manually") );
#endif
}

void MainWindow::about()
{
}

void MainWindow::profilerStopped()
{
	ProfilerWindow* p = new ProfilerWindow( this );
	p->show();
}

void MainWindow::newInterpreterWindow()
{
	PythonInteractiveWindow* p = new PythonInteractiveWindow( this );
	p->show();
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

	QApplication::postEvent( mainWindow, new QWolfpackConsoleEvent( QWolfpackConsoleEvent::SendEvent, sMessage ) );
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
	QApplication::postEvent( mainWindow, new QWolfpackConsoleEvent( QWolfpackConsoleEvent::RollbackCharsEvent, count ) );
}

void cConsole::changeColor( enConsoleColors color )
{
	QRgb qcolor = 0;
	switch ( color )
	{
	case WPC_GREEN:
		qcolor = qRgb( 0x00, 0xFF, 0x00 );
		break;

	case WPC_RED:
		qcolor = qRgb( 0xFF, 0x00, 0x00 );
		break;

	case WPC_YELLOW:
		qcolor = qRgb( 0x00, 0xFF, 0xFF );
		break;

	case WPC_BROWN:
		qcolor = qRgb( 204, 204, 153 );
		break;

	case WPC_NORMAL:
		qcolor = qRgb( 0xAF, 0xAF, 0xAF );
		break;

	case WPC_WHITE:
		qcolor = qRgb( 0xFF, 0xFF, 0xFF );
		break;
	}
	QApplication::postEvent( mainWindow, new QWolfpackConsoleEvent( QWolfpackConsoleEvent::ChangeColorEvent, (int)qcolor ) );
}

void cConsole::setConsoleTitle( const QString& title )
{
	QApplication::postEvent( mainWindow, new QWolfpackConsoleEvent( QWolfpackConsoleEvent::SetTitleEvent, title ) );
}

void cConsole::setAttributes( bool bold, bool italic, bool underlined, unsigned char r, unsigned char g, unsigned char b, unsigned char size, enFontType font )
{
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
	QApplication::postEvent( mainWindow, new QWolfpackConsoleEvent( f ) );
	QApplication::postEvent( mainWindow, new QWolfpackConsoleEvent( QWolfpackConsoleEvent::ChangeColorEvent, qRgb(r, g, b) ) );
}

void cConsole::notifyServerState( enServerState newstate )
{
	QApplication::postEvent( mainWindow, new QWolfpackConsoleEvent( QWolfpackConsoleEvent::NotifyState, newstate ) );
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

