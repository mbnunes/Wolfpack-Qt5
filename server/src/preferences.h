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
 * Wolfpack Homepage: http://wpdev.sf.net/
 */

#ifndef PREFERENCES_H
#define PREFERENCES_H

class QDomElement;
class QString;
class PreferencesPrivate;

class Preferences
{
public:
	// constructor
	Preferences( const QString& filename, const QString& format, const QString& version );
	// destructor
	virtual ~Preferences();

	// preference file information
	const QString& file();
	const QString& format();
	const QString& version();
	// did file open successfully?
	bool fileState();
	// is this a proper preferences file for format?
	bool formatState();

	// boolean data storage
	bool getBool( const QString& group, const QString& key, bool def = false, bool create = false );
	void setBool( const QString& group, const QString& key, bool value );
	// integer data storage
	long getNumber( const QString& group, const QString& key, long def, bool create = false );
	void setNumber( const QString& group, const QString& key, long value );
	// double data storage
	double getDouble( const QString& group, const QString& key, double def = 0.0, bool create = false );
	void setDouble( const QString& group, const QString& key, double value );
	// string data storage
	QString getString( const QString& group, const QString& key, const QString& def, bool create = false );
	void setString( const QString& group, const QString& key, const QString& value );

	// remove a key/value from the preferences
	void removeKey( const QString& group, const QString& key );
	// remove the current group from the preferences
	void removeGroup( const QString& group );
	bool containKey( const QString& group, const QString& key ) const;
	bool containGroup( const QString& group ) const;

	// flush the preferences out to file
	void flush();
	virtual void clear();

protected:
	// serialization
	void readData();
	void writeData();
	void processGroup( const QDomElement& group );

	virtual QString getGroupDoc( const QString& group );
	virtual QString getEntryDoc( const QString& group, const QString& entry );

private:
	PreferencesPrivate* d;
};

#endif // PREFERENCES
