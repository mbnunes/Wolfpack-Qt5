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

#ifndef __CUSTOMTAGS_H
#define __CUSTOMTAGS_H

#include "platform.h"
#include "typedefs.h"
#include "defines.h"
#include "persistentobject.h"

// Library include
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

// Forward Declarations
class cItem;
class cBaseChar;
class QString;
class cVariant;
class Coord_cl;

class cVariant
{
public:
	// Static NULL instance
	static const cVariant null;

	enum Type
	{
		Invalid			= 0,
		String,
		Int,
		Long,
		Double,
		BaseChar,
		Item,
		Coord
	};

	cVariant();
	~cVariant();

	cVariant( const cVariant& v );
	cVariant( const QString& );
	cVariant( int );
	cVariant( unsigned int);
	cVariant( cBaseChar* );
	cVariant( cItem* );
	cVariant( Coord_cl );
	cVariant( double );
	cVariant( long int );

	Type type() const;
	const char* typeName() const;

	bool canCast( Type ) const;
	bool cast( Type );

	bool isValid() const;

	void clear();

	const QString toString() const;
	int toInt( bool* ok = 0 ) const;
	double toDouble( bool* ok = 0 ) const;
	cBaseChar* toChar() const;
	cItem* toItem() const;
	Coord_cl toCoord() const;

	cVariant& operator=( const cVariant& );
	bool operator==( const cVariant& ) const;
	bool operator!=( const cVariant& ) const;

	QString& asString();
	int& asInt();
	double& asDouble();

	static const char* typeToName( Type typ );
	static Type nameToType( const char* name );

	bool isString();
private:
	Type typ;

	union
	{
		int i;
		double d;
		void* ptr;
	} value;
};

// Inline methods
inline cVariant::Type cVariant::type() const
{
	return typ;
}

inline bool cVariant::isValid() const
{
	return ( typ != Invalid );
}

class cCustomTags
{
public:
	cCustomTags() : tags_( 0 ), changed( false )
	{
	}
	cCustomTags( const cCustomTags& );
	virtual ~cCustomTags();

	void del( SERIAL key );
	void save( SERIAL key );
	void load( SERIAL key );

	const cVariant& get( const QString& key ) const;
	bool has( const QString& key ) const;
	void set( const QString& key, const cVariant& value );
	void remove( const QString& key );

	UI32 size()
	{
		return tags_ ? this->tags_->size() : 0;
	}

	QStringList getKeys() const;

	QValueList<cVariant> getValues();

	bool getChanged() const
	{
		return changed;
	}

	void setChanged( bool ch )
	{
		changed = ch;
	}

	cCustomTags& operator=( const cCustomTags& );
	bool operator==( const cCustomTags& ) const;
	bool operator!=( const cCustomTags& ) const;

private:
	QMap<QString, cVariant>* tags_;
	bool changed : 1;
};

#endif
