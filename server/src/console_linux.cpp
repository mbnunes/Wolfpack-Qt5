
#include "console.h"

class cConsoleThread : public QThread
{
protected:
	virtual void run()
	{
		try
		{
			while( serverState < SHUTDOWN )
			{
				char c = getch();
				
				if( c > 0 && serverState == RUNNING )
				{
					Console::instance()->handleCommand( QChar( c ) );			
				}
				else
				{	
					Sleep( 100 );
				}
			}
		}
		// If there is any error: Quit.
		// It's better to have no console input
		// than a deadlocking server.
		catch( ... )
		{
		}
	}
};

cConsoleThread *thread = 0;

void cConsole::start()
{
	thread = new cConsoleThread;
	thread->start();
}

void cConsole::poll()
{
	// Normally we would check if there is a command in the command queue and execute it
}

void cConsole::stop()
{
	thread->wait();
	delete thread;
}


void cConsole::setConsoleTitle( const QString& data )
{
#if defined(Q_OS_WIN32)
	SetConsoleTitle( data.latin1() );
#endif
}

//=========================================================================================
// Change the console Color
void cConsole::ChangeColor( WPC_ColorKeys Color )
{
#if defined(Q_OS_UNIX)
QString cb = "\e[0m";
		switch( Color )
		{
		case WPC_GREEN: cb = "\e[1;32m";
			break;
		case WPC_RED:	cb = "\e[1;31m";
			break;
		case WPC_YELLOW:cb = "\e[1;33m";
			break;
		case WPC_NORMAL:cb = "\e[0m";
			break;
		case WPC_WHITE:	cb = "\e[1;37m";
			break;
		default: cb = "\e[0m";

		}
		send( cb );
#elif defined(Q_OS_WIN32)
		HANDLE ConsoleHandle = GetStdHandle( STD_OUTPUT_HANDLE );
		UI16 ColorKey = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE;

		switch( Color )
		{
		case WPC_GREEN:
			ColorKey = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
			break;

		case WPC_RED:
			ColorKey = FOREGROUND_RED | FOREGROUND_INTENSITY;
			break;

		case WPC_YELLOW:
			ColorKey = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
			break;

		case WPC_NORMAL:
			ColorKey = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE;
			break;

		case WPC_WHITE:
			ColorKey = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
			break;

		default:
			break;
		}

		SetConsoleTextAttribute( ConsoleHandle, ColorKey );
#endif
}

//========================================================================================
// Send a message to the console
void cConsole::send(const QString &sMessage)
{
	if( outputstrm != NULL )
	{
		(*outputstrm) << sMessage.latin1();
		flush( *outputstrm );
	}

	if( sMessage.contains( "\n" ) )
	{
#if defined(Q_OS_UNIX) && 0 
		sMessage.replace("\e[0m", "");
	  	sMessage.replace("\e[1;32m", "");
		sMessage.replace("\e[1;31m", "");
		sMessage.replace("\e[1;33m", "");
		sMessage.replace("\e[1;37m", "");
#endif
		incompleteLine_.append( sMessage ); // Split by \n
		QStringList lines = QStringList::split( "\n", incompleteLine_, true );

		// Insert all except the last element
		for( int i = 0; i < lines.count()-1; ++i )
			linebuffer_.push_back( lines[i] );

		incompleteLine_ = lines[ lines.count() - 1 ];
	}
	else
	{
		incompleteLine_.append( sMessage );
	}
}
