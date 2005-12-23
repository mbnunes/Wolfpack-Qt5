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

#if !defined( __QWPEVENTS_H__ )
#define __QWPEVENTS_H__

#include <QEvent>
#include <QString>
#include <QFont>

class QWPConsoleSendEvent : public QEvent
{
	QString m;
public:
	QWPConsoleSendEvent( const QString& msg ) : QEvent( (QEvent::Type)(QEvent::User + 1) ), m( msg )
	{

	}

	QString message() const { return m;	}
};

class QWPConsoleTitleEvent : public QEvent
{
	QString title_;
public:
	QWPConsoleTitleEvent( const QString& title ) : QEvent( (QEvent::Type)(QEvent::User + 2) ), title_( title )
	{

	}

	QString title() const { return title_;	}
};

class QWPConsoleChangeColorEvent : public QEvent
{
	QColor color_;
public:
	QWPConsoleChangeColorEvent( const QColor& color ) : QEvent( (QEvent::Type)(QEvent::User + 3) ), color_( color )
	{

	}

	QColor color() const { return color_;	}
};

class QWPConsoleChangeFontEvent : public QEvent
{
	QFont format_;
public:
	QWPConsoleChangeFontEvent( const QFont& format ) : QEvent( (QEvent::Type)(QEvent::User + 4) ), format_( format )
	{

	}

	QFont format() const { return format_;	}
};


#endif // __QWPEVENTS_H__
