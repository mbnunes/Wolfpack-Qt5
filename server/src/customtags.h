//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001-2003 by holders identified in authors.txt
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================

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
    enum Type
	{
		Invalid = 0,
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
	
	cVariant( const cVariant& p );
    cVariant( const QString& );
	cVariant( int );
	cVariant( cBaseChar* );
	cVariant( cItem* );
	cVariant( Coord_cl );
    cVariant( double );
	cVariant( long int );

    cVariant& operator= ( const cVariant& );
    bool operator==( const cVariant& ) const;
    bool operator!=( const cVariant& ) const;

    Type type() const;
    const char* typeName() const;

    bool canCast( Type ) const;
    bool cast( Type );

    bool isValid() const;

    void clear();

    const QString toString() const;
    int toInt( bool * ok=0 ) const;
    double toDouble( bool * ok=0 ) const;
	cBaseChar *toChar() const;
	cItem *toItem() const;
	Coord_cl toCoord() const;

    QString& asString();
    int& asInt();
    double& asDouble();

    static const char* typeToName( Type typ );
    static Type nameToType( const char* name );

	bool		isString();
private:
    void detach();

    class Private : public QShared
    {
    public:
        Private();
        Private( Private* );
        ~Private();

        void clear();

        Type typ;
        union
        {
		    int i;
		    double d;
			void *ptr;
        } value;
    };

    Private* d;
};

// Inline methods
inline cVariant::Type cVariant::type() const
{
    return d->typ;
}

inline bool cVariant::isValid() const
{
    return (d->typ != Invalid);
}

class cCustomTags
{
public:
	cCustomTags(): changed( false ), tags_( 0 ) {}
	cCustomTags( const cCustomTags& );
	virtual ~cCustomTags();

	void del( SERIAL key );
	void save( SERIAL key );
	void load( SERIAL key );

	cVariant	get( const QString& key );
	void		set( const QString& key, const cVariant& value );
	void		remove( const QString& key );

	UI32		size( void ) { return tags_ ? this->tags_->size() : 0; }

	QStringList getKeys( void );

	QValueList< cVariant > getValues( void );

	bool getChanged( void )
	{
		return changed;
	}

	void setChanged( bool ch )
	{
		changed = ch;
	}

	cCustomTags& operator=( const cCustomTags& );


private:
	QMap< QString, cVariant > *tags_;
	bool changed;
};

#endif
