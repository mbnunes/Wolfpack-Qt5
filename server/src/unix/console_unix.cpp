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

// Wolfpack includes
#include "../console.h"
#include "../getopts.h"
#include "../server.h"

// System Includes
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>


// Qt Includes
#include <QThread>
#include <QCoreApplication>

using namespace std;


void resetNonBlockingIo()
{
	termios term_caps;

	if ( tcgetattr( STDIN_FILENO, &term_caps ) < 0 )
		return;

	term_caps.c_lflag |= ICANON;
	term_caps.c_lflag |= ECHO;

	if ( tcsetattr( STDIN_FILENO, TCSANOW, &term_caps ) < 0 )
		return;
}

void setNonBlockingIo()
{
	termios term_caps;

	if ( tcgetattr( STDIN_FILENO, &term_caps ) < 0 )
		return;

	term_caps.c_lflag &= ~ICANON;
	term_caps.c_lflag &= ~ECHO;

	if ( tcsetattr( STDIN_FILENO, TCSANOW, &term_caps ) < 0 )
		return;

	setbuf( stdin, NULL );

	atexit( resetNonBlockingIo );
}

void signal_handler( int signal )
{
	switch ( signal )
	{
	case SIGHUP:
		Server::instance()->queueAction( RELOAD_SCRIPTS );
		break;
	case SIGUSR1:
		Server::instance()->queueAction( RELOAD_ACCOUNTS );
		break;
	case SIGUSR2:
		Server::instance()->queueAction( SAVE_WORLD );
		break;
	case SIGTERM:
		Server::instance()->cancel();
		break;
	default:
		break;
	}
}

class cConsoleThread : public QThread
{
protected:
	virtual void run()
	{
		try
		{
			setNonBlockingIo();

			signal( SIGHUP, &signal_handler ); // Reload Scripts
			signal( SIGUSR1, &signal_handler ); // Save World
			signal( SIGUSR2, &signal_handler ); // Reload Accounts
			signal( SIGTERM, &signal_handler ); // Terminate Server
			signal( SIGPIPE, SIG_IGN );			// Ignore SIGPIPE

			if ( !CommandLineOptions::instance()->value("isDaemon").toBool() )
			{
				while ( Server::instance()->getState() < SHUTDOWN )
				{
					// Do a select operation on the stdin handle and see
					// if there is any input waiting.
					fd_set consoleFds;
					FD_ZERO( &consoleFds );
					FD_SET( STDIN_FILENO, &consoleFds );

					timeval tvTimeout;
					tvTimeout.tv_sec = 0;
					tvTimeout.tv_usec = 1;

					if ( select( 1, &consoleFds, 0, 0, &tvTimeout ) > 0 )
					{
						char c = fgetc( stdin );

						if ( c > 0 && Server::instance()->getState() == RUNNING )
						{
							Console::instance()->queueCommand( QString( c ) );
						}
					}
					else
					{
						msleep( 100 );
					}
				}
			}
		}
		// If there is any error: Quit.
		// It's better to have no console input
		// than a deadlocking server.
		catch ( ... )
		{
		}
	}
};

cConsoleThread* thread = 0;

void daemonize()
{
	int pid, fd;

	pid = fork();

	switch ( pid )
	{
	case 0:
		// child
		setsid();
		for ( fd = getdtablesize(); fd >= 0; --fd )
			close( fd );

		if ( ( fd = open( "/dev/null", O_RDWR ) ) != -1 )
		{
			dup2( fd, 0 );
			dup2( fd, 1 );
		}
		signal(SIGTSTP, SIG_IGN); // Ignore tty signals
		signal(SIGTTOU, SIG_IGN);
		signal(SIGTTIN, SIG_IGN);

		break;

	case -1:
		perror( "fork error" );
		break;

	default:
		// we forked, so silently exit the parent
		exit( 0 );
	}
}

void pidfile_add( QString pidfile )
{
	FILE* pf;

	if ( ( pf = fopen( qPrintable(pidfile), "w+" ) ) != NULL )
	{
		fprintf( pf, "%i", getpid() );
		fclose( pf );
	}
	else
	{
		perror( "fopen" );
	}
}

void pidfile_del( QString pidfile )
{
	if ( unlink( qPrintable( pidfile ) ) == -1 )
		perror( "unlink" );
}

void cConsole::start()
{
	if ( CommandLineOptions::instance()->value("isDaemon").toBool() )
		daemonize();

	if ( CommandLineOptions::instance()->value("pidFile").isValid() )
		pidfile_add( CommandLineOptions::instance()->value("pidFile").toString() );

	thread = new cConsoleThread;
	thread->start();
}

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
	thread->wait();
	delete thread;
	if ( CommandLineOptions::instance()->value("pidFile").isValid() )
		pidfile_del( CommandLineOptions::instance()->value("pidFile").toString() );
}


void cConsole::setConsoleTitle( const QString& data )
{
	Q_UNUSED( data );
}

//=========================================================================================
// Change the console Color
void cConsole::changeColor( enConsoleColors Color )
{
	QString cb = "\e[0m";
	switch ( Color )
	{
	case WPC_GREEN:
		cb = "\e[1;32m";
		break;
	case WPC_RED:
		cb = "\e[1;31m";
		break;
	case WPC_YELLOW:
		cb = "\e[1;33m";
		break;
	case WPC_NORMAL:
		cb = "\e[0m";
		break;
	case WPC_WHITE:
		cb = "\e[1;37m";
		break;
	case WPC_BROWN:
		cb = "\e[0;33m";
		break;
	default:
		cb = "\e[0m";
		break;
	}
	send( cb );
}

void cConsole::rollbackChars( unsigned int count )
{
	for ( unsigned int i = 0; i < count; ++i )
	{
		fprintf( stdout, "\b" );
	}
	fflush( stdout );

	// Remove from the end of the linebuffer
	linebuffer_.truncate( linebuffer_.length() - count );
}

//========================================================================================
// Send a message to the console
void cConsole::send( const QString& sMessage )
{
	if ( sMessage.isNull() || sMessage.isEmpty() )
	{
		return;
	}

	// If a progress message is waiting, remove it.
	if ( !progress.isEmpty() )
	{
		QString temp = progress;
		progress = QString::null;
		rollbackChars( temp.length() );
		progress = temp;
	}

	// Replace %'s by %%'s
	QString newMessage = sMessage;
	newMessage.replace( "%", "%%" );

	fprintf( stdout, newMessage.toLocal8Bit() );
	fflush( stdout );

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

void cConsole::setAttributes( bool bold, bool italic, bool, unsigned char r, unsigned char g, unsigned char b, unsigned char, enFontType )
{
	Q_UNUSED( bold );
	Q_UNUSED( italic );
	Q_UNUSED( r );
	Q_UNUSED( g );
	Q_UNUSED( b );
}

void cConsole::notifyServerState( enServerState newstate )
{
	Q_UNUSED( newstate );
}


int main( int argc, char** argv )
{
	QCoreApplication app( argc, argv );
	QObject::connect( Server::instance(), SIGNAL(finished()), &app, SLOT(quit()) );
	Server::instance()->start();
	return app.exec();
}
