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

#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qmap.h>
#include <qstring.h>

#include "preferences.h"

class PreferencesPrivate
{
public:
	bool dirty_;
	QString currentgroup_;
	QString file_;
	QString format_;
	QString version_;
	QString buffer_;
	bool filestate_;
	bool formatstate_;

	typedef QMap<QString, QString> PrefMap;
	QMap<QString, PrefMap> groups_;
};

/*****************************************************************************
  Preferences member functions
 *****************************************************************************/

Preferences::Preferences( const QString& filename, const QString& format, const QString& version ) : d( new PreferencesPrivate )
{
	d->dirty_ = false;
	d->file_ = filename;
	d->format_ = format;
	d->version_ = version;
	d->filestate_ = false;
	d->formatstate_ = false;
}

Preferences::~Preferences()
{
	if ( d->dirty_ )
		writeData();
	delete d;
}

/*!
	Get a boolean value
*/
bool Preferences::getBool( const QString& group, const QString& key, bool def, bool create )
{
	QString buffer_ = getString( group, key, def ? "true" : "false", create );
	if ( buffer_.isEmpty() )
	{
		if ( create )
			setBool( group, key, def );
		return def;
	}
	if ( buffer_.contains( "true", false ) )
		return true;
	else
		return false;
}

/*!
	Set a boolean value
*/
void Preferences::setBool( const QString& group, const QString& key, bool value )
{
	d->groups_[group][key] = value ? "true" : "false";
	d->dirty_ = true;
}

/*!
	Get a number value
*/
long Preferences::getNumber( const QString& group, const QString& key, long def, bool create )
{
	QString buffer_ = getString( group, key, QString::number( def ), create );
	if ( buffer_.isEmpty() )
	{
		return def;
	}

	bool ok = false;
	long num = buffer_.toLong( &ok );
	return ok ? num : def;
}

/*!
	Set a number value
*/
void Preferences::setNumber( const QString& group, const QString& key, long value )
{
	d->groups_[group][key] = QString::number( value );
	d->dirty_ = true;
}

/*!
	Get a double value
*/
double Preferences::getDouble( const QString& group, const QString& key, double def, bool create )
{
	QString buffer_ = getString( group, key, QString::number( def ), create );
	if ( buffer_.isEmpty() )
	{
		return def;
	}

	bool ok = false;
	double num = buffer_.toDouble( &ok );
	return ok ? num : def;
}

/*!
	Set a double value
*/
void Preferences::setDouble( const QString& group, const QString& key, double value )
{
	d->groups_[group][key] = QString::number( value );
	d->dirty_ = true;
}

/*!
	Get a string value
*/
QString Preferences::getString( const QString& group, const QString& key, const QString& def, bool create )
{
	QString buffer_;
	if ( containKey( group, key ) )
	{
		buffer_ = d->groups_[group][key];
	}
	if ( buffer_.isEmpty() )
	{
		if ( create )
			setString( group, key, def );
		return def;
	}
	return buffer_;
}

/*!
	Set a string value
*/
void Preferences::setString( const QString& group, const QString& key, const QString& value )
{
	d->groups_[group][key] = value;
	d->dirty_ = true;
}

bool Preferences::containGroup( const QString& group ) const
{
	return d->groups_.contains( group );
}

bool Preferences::containKey( const QString& group, const QString& key ) const
{
	if ( !containGroup( group ) )
		return false;
	return d->groups_[group].contains( key );
}

/*!
	Remove a value from the preferences
*/
void Preferences::removeKey( const QString& group, const QString& key )
{
	if ( containGroup( group ) )
		d->groups_[group].remove( key );
}

/*!
	Remove a group from the preferences, and all its options
*/
void Preferences::removeGroup( const QString& group )
{
	d->groups_.remove( group );
}

/*!
	Flush the preferences to file
*/
void Preferences::flush()
{
	if ( d->dirty_ )
	{
		writeData();
		d->dirty_ = false;
	}
}

void Preferences::clear()
{
	d->dirty_ = false;
	d->groups_.clear();
}

/*!
	Read data from the file
*/
void Preferences::readData()
{
	// open file
	QFile datafile( d->file_ );
	if ( !datafile.open( IO_ReadOnly ) )
	{
		// error opening file
		qWarning( "Error: cannot open preferences file " + d->file_ );
		datafile.close();
		d->filestate_ = false;
		return;
	}
	d->filestate_ = true;

	// open dom document
	QDomDocument doc( "preferences" );
	if ( !doc.setContent( &datafile ) )
	{
		qWarning( "Error: " + d->file_ + " is not a proper preferences file" );
		datafile.close();
		d->formatstate_ = false;
		return;
	}
	datafile.close();

	// check the doc type and stuff
	if ( doc.doctype().name() != "preferences" )
	{
		// wrong file type
		qWarning( "Error: " + d->file_ + " is not a valid preferences file" );
		d->formatstate_ = false;
		return;
	}
	QDomElement root = doc.documentElement();
	if ( root.attribute( "application" ) != d->format_ )
	{
		// right file type, wrong application
		qWarning( "Error: " + d->file_ + " is not a preferences file for " + d->format_ );
		d->formatstate_ = false;
		return;
	}
	// We don't care about application version...

	// get list of groups
	QDomNodeList nodes = root.elementsByTagName( "group" );

	// iterate through the groups
	QDomNodeList options;
	for ( uint n = 0; n < nodes.count(); ++n )
	{
		if ( nodes.item( n ).isElement() && !nodes.item( n ).isComment() )
		{
			processGroup( nodes.item( n ).toElement() );
		}
	}
	d->formatstate_ = true;
}

void Preferences::processGroup( const QDomElement& group )
{
	QDomElement elem;
	QDomNodeList options;
	QString currentgroup_ = group.attribute( "name", "Default" );
	options = group.elementsByTagName( "option" );
	for ( unsigned n = 0; n < options.count(); ++n )
	{
		if ( options.item( n ).isElement() )
		{
			if ( !options.item( n ).isComment() )
			{
				elem = options.item( n ).toElement();
				setString( currentgroup_, elem.attribute( "key" ), elem.attribute( "value" ) );
			}
		}
	}
}

/*!
	Write data out to the file
*/
void Preferences::writeData()
{
	QDomDocument doc( "preferences" );

	// create the root element
	QDomElement root = doc.createElement( doc.doctype().name() );
	root.setAttribute( "version", d->version_ );
	root.setAttribute( "application", d->format_ );

	// now do our options group by group
	QMap<QString, PreferencesPrivate::PrefMap>::Iterator git;
	PreferencesPrivate::PrefMap::Iterator pit;
	QDomElement group, option;
	for ( git = d->groups_.begin(); git != d->groups_.end(); ++git )
	{
		// comment the group
		QString commentText = getGroupDoc( git.key() );

		if ( commentText != QString::null )
		{
			root.appendChild( doc.createTextNode( "\n\n  " ) );
			root.appendChild( doc.createComment( "\n  " + commentText.replace( "\n", "\n  " ) + "\n  " ) );
			root.appendChild( doc.createTextNode( "\n  " ) );
		}

		// create a group element
		group = doc.createElement( "group" );
		group.setAttribute( "name", git.key() );
		// add in options
		for ( pit = ( *git ).begin(); pit != ( *git ).end(); ++pit )
		{
			QString commentText = getEntryDoc( git.key(), pit.key() );

			if ( commentText != QString::null )
			{
				group.appendChild( doc.createTextNode( "\n\n	" ) );
				group.appendChild( doc.createComment( " " + commentText.replace( "\n", "\n	  " ) + " " ) );
				group.appendChild( doc.createTextNode( "\n	" ) );
			}

			option = doc.createElement( "option" );
			option.setAttribute( "key", pit.key() );
			option.setAttribute( "value", pit.data() );
			group.appendChild( option );
		}
		root.appendChild( group );
	}
	doc.appendChild( root );

	// open file
	QFile datafile( d->file_ );
	if ( !datafile.open( IO_WriteOnly ) )
	{
		// error opening file
		qWarning( "Error: Cannot open preferences file " + d->file_ );
		d->filestate_ = false;
		return;
	}
	d->filestate_ = true;

	// write it out
	QTextStream textstream( &datafile );
	doc.save( textstream, 2 );
	datafile.close();
	d->formatstate_ = true;
}

const QString& Preferences::file()
{
	return d->file_;
};

const QString& Preferences::format()
{
	return d->format_;
}

bool Preferences::fileState()
{
	return d->filestate_;
}

bool Preferences::formatState()
{
	return d->formatstate_;
}

QString Preferences::getGroupDoc( const QString& group )
{
	Q_UNUSED( group );
	return QString::null;
}

QString Preferences::getEntryDoc( const QString& group, const QString& entry )
{
	Q_UNUSED( group );
	Q_UNUSED( entry );
	return QString::null;
}
