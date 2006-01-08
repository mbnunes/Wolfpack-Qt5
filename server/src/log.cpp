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

// Wolfpack Includes
#include "log.h"

#include "pythonscript.h"
#include "scriptmanager.h"

#include "serverconfig.h"
#include "network/uosocket.h"
#include "console.h"

// QT Includes
#include <QDateTime>
#include <QDir>
//Added by qt3to4:
#include <QByteArray>

cLog::cLog()
{
	currentday = 0xFF;
}

cLog::~cLog()
{
	if ( logfile.isOpen() )
	{
		logfile.close();
	}
}

bool cLog::checkLogFile()
{
	QDate today = QDate::currentDate();

	/*
		Try to open the logfile for today if:
		a) Our filedescriptor is invalid
		b) We don't have today anymore
		EXCEPT if the user sets LogRotate to false
	*/
	if ( !logfile.isOpen() || currentday != today.day() )
	{
		currentday = today.day();

		logfile.close(); // Just to be sure

		QString path = Config::instance()->logPath();

		if ( !path.endsWith( QChar( QDir::separator() ) ) )
			path.append( QDir::separator() );

		QDir d;
		if ( !d.exists( path ) )
		{
			d.mkdir( path );
		}

		QString filename;
		if ( Config::instance()->logRotate() )
			filename.sprintf( "wolfpack-%04u-%02u-%02u.log", today.year(), today.month(), today.day() );
		else
			filename = QString( "wolfpack.log" );

		logfile.setName( path + filename );

		if ( !logfile.open( QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text ) )
		{
			Console::instance()->send( tr( "Couldn't open logfile '%1'\n" ).arg( path + filename ) );
			return false;
		}
	}

	return true;
}

/*
	Log to the logfile only.
 */
void cLog::log( eLogLevel loglevel, cUOSocket* sock, const QString& string, bool timestamp )
{
	if ( !( Config::instance()->logMask() & loglevel ) )
	{
		return;
	}

	// -> Log Event
	cPythonScript *globalHook = ScriptManager::instance()->getGlobalHook(EVENT_LOG);
	if (globalHook && globalHook->canHandleEvent(EVENT_LOG)) {
		PyObject *args = Py_BuildValue("(iNNO)", (unsigned int)loglevel, PyGetSocketObject(sock), QString2Python(string), Py_None );
		bool result = globalHook->callEventHandler(EVENT_LOG, args);
		Py_DECREF(args);

		if (result) {
			return;
		}
	}

	if ( !checkLogFile() )
		return;

	// Timestamp the data
	QTime now = QTime::currentTime();

	QString prelude;

	if ( timestamp || loglevel == LOG_PYTHON )
	{
		prelude.sprintf( "%02u:%02u:", now.hour(), now.minute() );

		if ( sock )
			prelude.append( QString( "%1:" ).arg( sock->uniqueId(), 0, 16 ) );
	}

	// LogLevel
	switch ( loglevel )
	{
	case LOG_ERROR:
		prelude.append( "ERROR: " );
		break;

	case LOG_WARNING:
		prelude.append( "WARNING: " );
		break;

	case LOG_PYTHON:
		prelude.append( "PYTHON: " );
		break;

	default:
		prelude.append( " " );
	}

	QByteArray utfdata = string.utf8();
	utfdata.prepend( prelude.utf8() );

	logfile.writeBlock( utfdata );
	logfile.flush();
}

// Sends to the console and logs too
void cLog::print( eLogLevel loglevel, const QString& string, bool timestamp )
{
	// Send to the Console too
	print( loglevel, 0, string, timestamp );
}

void cLog::print( eLogLevel loglevel, cUOSocket* sock, const QString& string, bool timestamp )
{
	// send to the logfile
	log( loglevel, sock, string, timestamp );

	if ( timestamp )
	{
		// Timestamp the data
		QTime now = QTime::currentTime();
		QString prelude;
		prelude.sprintf( "%02u:%02u:", now.hour(), now.minute() );

		Console::instance()->changeColor( WPC_WHITE );
		Console::instance()->send( prelude );
		Console::instance()->changeColor( WPC_NORMAL );

		if ( sock )
			Console::instance()->send( QString( "%1:" ).arg( sock->uniqueId(), 0, 16 ) );
	}

	// LogLevel
	switch ( loglevel )
	{
	case LOG_ERROR:
		Console::instance()->changeColor( WPC_RED );
		Console::instance()->send( tr( "ERROR: " ) );
		Console::instance()->changeColor( WPC_NORMAL );
		break;

	case LOG_WARNING:
		Console::instance()->changeColor( WPC_YELLOW );
		Console::instance()->send( tr( "WARNING: " ) );
		Console::instance()->changeColor( WPC_NORMAL );
		break;

	case LOG_PYTHON:
		Console::instance()->changeColor( WPC_BROWN );
		break;

	default:
		Console::instance()->send( " " );
	}

	Console::instance()->send( string );

	if ( loglevel == LOG_PYTHON )
	{
		Console::instance()->changeColor( WPC_NORMAL );
	}
}
