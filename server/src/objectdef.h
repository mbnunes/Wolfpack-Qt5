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
* Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
*/

#if !defined( __OBJECTDEF_H__ )
#define __OBJECTDEF_H__

#ifndef QT_H
#include <qglobal.h>
#include <qapplication.h>
#endif // QT_H

/*!
	Since we are not using QObject due to memory footprint, moc won't create some nice functions
	either, we need some macro magic to get them ( or disabling some ) without doing too much
	mess
*/

#ifndef QT_NO_TRANSLATION

inline QString tr( const QString& text, const char* comment = 0, const char* context = "@default" )
{
	return qApp->translate( context, text.latin1(), comment );
}

# ifndef QT_NO_TEXTCODEC
// full set of tr functions
#  define WP_TR_FUNCTIONS(classname) \
	static QString tr( const char* s, const char* c = 0 ) \
	{ \
		if ( qApp ) \
			return qApp->translate( #classname, s, c, QApplication::DefaultCodec ); \
		else \
			return QString::fromLatin1( s ); \
	} \
	\
	static QString trUtf8( const char* s, const char* c = 0 ) \
	{ \
		if ( qApp ) \
			return qApp->translate( #classname, s, c, QApplication::UnicodeUTF8 ); \
		else \
			return QString::fromUtf8( s ); \
	}

# else
// no QTextCodec, no utf8
#  define WP_TR_FUNCTIONS \
	static QString tr( const char* s, const char* c = 0 ) \
	{ \
		if ( qApp ) \
			return qApp->translate( #classname, s, c, QApplication::DefaultCodec ); \
		else \
			return QString::fromLatin1( s ); \
	}

# endif
#else
# define WP_TR_FUNCTIONS(classname)
# define tr(x) QString(x)
#endif

#define OBJECTDEF(classname) \
public: \
	/*virtual const char *className() const { return #classname; }*/ \
	WP_TR_FUNCTIONS(classname) \
private:

#endif // __OBJECTDEF_H__

