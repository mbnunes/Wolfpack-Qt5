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
#include <QMap>
#include <qstring.h>
#include <qstringlist.h>

#include "config.h"

class cConfigPrivate
{
public:
	bool dirty_;
	QString currentgroup_;
	QString file_;
	QString buffer_;
	bool filestate_;
	bool formatstate_;

	typedef QMap<QString, QString> PrefMap;
	QMap<QString, PrefMap> groups_;
};

/*****************************************************************************
  cConfig member functions
 *****************************************************************************/

QStringList cConfig::groups() const {
	QStringList result;
	QMap<QString, cConfigPrivate::PrefMap>::const_iterator it;
	for (it = d->groups_.begin(); it != d->groups_.end(); ++it) {
		result.append(it.key());
	}
	return result;
}

cConfig::cConfig() : d( new cConfigPrivate )
{
	d->dirty_ = false;
	d->filestate_ = false;
	d->formatstate_ = false;
}

cConfig::~cConfig()
{
	if ( d->dirty_ )
		writeData();
	delete d;
}

void cConfig::setFile(const QString &name) {
	d->file_ = name;
}

/*!
	Get a boolean value
*/
bool cConfig::getBool( const QString& group, const QString& key, bool def, bool create )
{
	QString buffer_ = getString( group, key, def ? "true" : "false", create );
	if ( buffer_.isEmpty() )
	{
		if ( create )
			setBool( group, key, def );
		return def;
	}
	if ( buffer_.contains( "true", Qt::CaseSensitive ) )
		return true;
	else
		return false;
}

/*!
	Set a boolean value
*/
void cConfig::setBool( const QString& group, const QString& key, bool value )
{
	d->groups_[group][key] = value ? "true" : "false";
	d->dirty_ = true;
}

/*!
	Get a number value
*/
long cConfig::getNumber( const QString& group, const QString& key, long def, bool create )
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
void cConfig::setNumber( const QString& group, const QString& key, long value )
{
	d->groups_[group][key] = QString::number( value );
	d->dirty_ = true;
}

/*!
	Get a double value
*/
double cConfig::getDouble( const QString& group, const QString& key, double def, bool create )
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
void cConfig::setDouble( const QString& group, const QString& key, double value )
{
	d->groups_[group][key] = QString::number( value );
	d->dirty_ = true;
}

/*!
	Get a string value
*/
QString cConfig::getString( const QString& group, const QString& key, const QString& def, bool create )
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
void cConfig::setString( const QString& group, const QString& key, const QString& value )
{
	d->groups_[group][key] = value;
	d->dirty_ = true;
}

bool cConfig::containGroup( const QString& group ) const
{
	return d->groups_.contains( group );
}

bool cConfig::containKey( const QString& group, const QString& key ) const
{
	if ( !containGroup( group ) )
		return false;
	return d->groups_[group].contains( key );
}

/*!
	Remove a value from the preferences
*/
void cConfig::removeKey( const QString& group, const QString& key )
{
	if ( containGroup( group ) )
		d->groups_[group].remove( key );
}

/*!
	Remove a group from the preferences, and all its options
*/
void cConfig::removeGroup( const QString& group )
{
	d->groups_.remove( group );
}

/*!
	Flush the preferences to file
*/
void cConfig::flush()
{
	if ( d->dirty_ )
	{
		writeData();
		d->dirty_ = false;
	}
}

void cConfig::clear()
{
	d->dirty_ = false;
	d->groups_.clear();
}

/*!
	Read data from the file
*/
void cConfig::readData()
{
	// open file
	QFile datafile( d->file_ );
	if ( !datafile.open( QIODevice::ReadOnly ) )
	{
		// error opening file
		qWarning( QString("Error: cannot open preferences file " + d->file_).toLatin1() );
		datafile.close();
		d->filestate_ = false;
		return;
	}
	d->filestate_ = true;

	// open dom document
	QDomDocument doc( "preferences" );
	if ( !doc.setContent( &datafile ) )
	{
		qWarning( QString("Error: " + d->file_ + " is not a proper preferences file").toLatin1() );
		datafile.close();
		d->formatstate_ = false;
		return;
	}

	QDomElement root = doc.documentElement();

	// get list of groups
	QDomNodeList nodes = root.elementsByTagName( "group" );

	// iterate through the groups
	QDomNodeList options;
	for ( int n = 0; n < nodes.count(); ++n )
	{
		if ( nodes.item( n ).isElement() && !nodes.item( n ).isComment() )
		{
			processGroup( nodes.item( n ).toElement() );
		}
	}
	d->formatstate_ = true;
}

void cConfig::processGroup( const QDomElement& group )
{
	QDomElement elem;
	QDomNodeList options;
	QString currentgroup_ = group.attribute( "name", "Default" );
	options = group.elementsByTagName( "option" );
	for ( int n = 0; n < options.count(); ++n )
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
void cConfig::writeData()
{
	QDomDocument doc(QString("preferences"));

	// create the root element
	QDomElement root = doc.createElement( doc.doctype().name() );

	// now do our options group by group
	QMap<QString, cConfigPrivate::PrefMap>::Iterator git;
	cConfigPrivate::PrefMap::Iterator pit;
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
			option.setAttribute( "value", pit.value() );
			group.appendChild( option );
		}
		root.appendChild( group );
	}
	doc.appendChild( root );

	// open file
	QFile datafile( d->file_ );
	if ( !datafile.open( QIODevice::WriteOnly ) )
	{
		// error opening file
		qWarning( QString("Error: Cannot open preferences file " + d->file_).toLatin1() );
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

const QString& cConfig::file()
{
	return d->file_;
};

bool cConfig::fileState()
{
	return d->filestate_;
}

bool cConfig::formatState()
{
	return d->formatstate_;
}

QString cConfig::getGroupDoc( const QString& group )
{
	Q_UNUSED( group );
	return QString::null;
}

QString cConfig::getEntryDoc( const QString& group, const QString& entry )
{
	Q_UNUSED( group );
	Q_UNUSED( entry );
	return QString::null;
}

void cConfig::load() {
	readData();

	// Cache the entries we can find
	logPath_ = getString("Logging", "Path", "./logs/");
	logMask_ = getNumber("Logging", "Mask", 0);
	logRotate_ = getNumber("Logging", "Rotate", true);
	packetLogging_ = getNumber("Logging", "Enable Packet Log", false);

	// Engine
	engineWidth_ = getNumber("Engine", "Width", 640);
	engineHeight_ = getNumber("Engine", "Height", 480);
	engineWindowed_ = getBool("Engine", "Windowed", true);
	engineWindowX_ = getNumber("Engine", "Window Pos X", -1);
	engineWindowY_ = getNumber("Engine", "Window Pos Y", -1);
	engineMaximized_ = getBool("Engine", "Maximized", false);

 	// General
	uoPath_ = getString("General", "Ultima Online Path", QString::null);
	useVerdata_ = getBool("General", "Use Verdata", true);
	screenshotsPath_ = getString("General", "Screenshots Path", "./screenshots/", true);
	disableSound_ = getBool("General", "Disable Sound", false, true);

	// Encryption
	encryptionEnableLogin_ = getBool("Encryption", "Encrypt Login", false, true);
	QString temp = getString("Encryption", "Login Key 1", "0", true);
	if (temp.startsWith("0x")) {
		encryptionLoginKey1_ = temp.right(temp.length() - 2).toULong(0, 16);
	} else {
		encryptionLoginKey1_ = temp.toULong();
	}

	temp = getString("Encryption", "Login Key 2", "0", true);
	if (temp.startsWith("0x")) {
		encryptionLoginKey2_ = temp.right(temp.length() - 2).toULong(0, 16);
	} else {
		encryptionLoginKey2_ = temp.toULong();
	}

	// Login
	loginHost_ = getString("Login", "Host", "127.0.0.1");
	loginPort_ = getNumber("Login", "Port", 2593);
	lastUsername_ = getString("Login", "Last Username", "");
	lastShardId_ = getNumber("Login", "Last Shard Id", 0);

	// Game
	gameHideStatics_ = getBool("Game", "Hide Statics", false);
	gameHideDynamics_ = getBool("Game", "Hide Dynamics", false);
	gameHideMobiles_ = getBool("Game", "Hide Mobiles", false);
	gameHideMap_ = getBool("Game", "Hide Map", false);
	gameHighlightStatics_ = getBool("Game", "Highlight Statics", false);
	gameHighlightMap_ = getBool("Game", "Highlight Map", false);
}

void cConfig::reload() {
	load(); // Nothing special here
}

void cConfig::save() {
	writeData();
}

cConfig *Config = 0; // Global Config Instance
