//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001 by holders identified in authors.txt
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
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================

#ifndef __CUSTOMTAGS_H
#define __CUSTOMTAGS_H

#include "platform.h"
#include "typedefs.h"
#include "defines.h"
#include "persistentobject.h"

// Library include
#include <map>
#include <vector>
#include "qstring.h"
#include "qstringlist.h"
#include "qmap.h"

// Forward Declarations
class cItem;
class cChar;
class QString;
class cVariant;

class cVariant
{
public:
    enum Type
	{
		Invalid = 0,
		String,
		Int,
		Double
	};

	cVariant();
	~cVariant();
	
	cVariant( const cVariant& p );
    cVariant( const QString& );
	cVariant( int );
    cVariant( double );

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

    QString& asString();
    int& asInt();
    double& asDouble();

    static const char* typeToName( Type typ );
    static Type nameToType( const char* name );

	bool		isString( void );

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

class cCustomTags : public PersistentObject
{
public:
	cCustomTags(): changed( false ) {;}
	~cCustomTags() {;}

	void del( SERIAL key );
	void save( SERIAL key );
	void load( SERIAL key );

	// implements cSerializable
	virtual QString		objectID() const  { return "cCustomTags";}

	cVariant	get( QString key ) 
	{ 
		std::map< QString, cVariant >::iterator it = tags_.find( key );
		if( it != tags_.end() )
			return it->second;
		else
			return cVariant();
	}

	void		set( QString key, cVariant value ) 
	{
		std::map< QString, cVariant >::iterator iter = tags_.find( key );

		if( iter != tags_.end() )
		{
			if( !value.isValid() )
			{
				tags_.erase( iter );
				changed = true;
			}
			else if( iter->second != value )
			{
				iter->second = value;
				changed = true;
			}
		}
		else
		{
			tags_.insert(std::make_pair(key, value)); 
			changed = true;
		}
	}

	void		remove( QString key )
	{
		std::map< QString, cVariant >::iterator iter = tags_.find( key );
		
		if( iter != tags_.end() )
		{
			tags_.erase( iter );
			changed = true;
		}
	}

	UI32		size( void ) { return this->tags_.size(); }

	QStringList getKeys( void )
	{
		QStringList keys_;
		std::map< QString, cVariant >::iterator it = this->tags_.begin();
		while( it != this->tags_.end() )
		{
			keys_.push_back( it->first );
			it++;
		}
		return keys_;
	}

	std::vector< cVariant > getValues( void )
	{
		std::vector< cVariant > values_;
		std::map< QString, cVariant >::iterator it = this->tags_.begin();
		while( it != this->tags_.end() )
		{
			values_.push_back( it->second );
			it++;
		}
		return values_;
	}

private:
	std::map< QString, cVariant > tags_;
	bool changed;
};

#endif
