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
class QCString;
class QBitArray;
class cVariant;
class ISerialization;
template <class T> class QValueList;
template <class T> class QValueListConstIterator;
template <class T> class QValueListNode;
template <class Key, class T> class QMap;
template <class Key, class T> class QMapConstIterator;

class cVariant
{
public:
    enum Type {	Invalid,	Map,	List,	String,		StringList,		Int,	UInt,	Bool,
				Double,		CString,		ByteArray,	BitArray,		Character,		Item
    };

	cVariant();
	~cVariant();
	cVariant( const cVariant& p );
    cVariant( const QString& );
    cVariant( const QCString& );
    cVariant( const char* );
    cVariant( const QStringList& );
    cVariant( const QByteArray& );
    cVariant( const QBitArray& );
    cVariant( const QValueList<cVariant>& );
    cVariant( const QMap<QString,cVariant>& );
	cVariant( const P_CHAR );
	cVariant( const P_ITEM );

    cVariant( int );
    cVariant( uint );
    cVariant( bool, int );
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
    const QCString toCString() const;
    const QStringList toStringList() const;
    const QByteArray toByteArray() const;
    const QBitArray toBitArray() const;
    int toInt( bool * ok=0 ) const;
    uint toUInt( bool * ok=0 ) const;
    bool toBool() const;
    double toDouble( bool * ok=0 ) const;
    const QValueList<cVariant> toList() const;
    const QMap<QString,cVariant> toMap() const;
	P_CHAR toCharacter() const;
	P_ITEM toItem() const;

    QValueListConstIterator<QString> stringListBegin() const;
    QValueListConstIterator<QString> stringListEnd() const;
    QValueListConstIterator<cVariant> listBegin() const;
    QValueListConstIterator<cVariant> listEnd() const;
    QMapConstIterator<QString,cVariant> mapBegin() const;
    QMapConstIterator<QString,cVariant> mapEnd() const;
    QMapConstIterator<QString,cVariant> mapFind( const QString& ) const;
    QString& asString();
    QCString& asCString();
    QStringList& asStringList();
    QByteArray& asByteArray();
    QBitArray& asBitArray();
    int& asInt();
    uint& asUInt();
    bool& asBool();
    double& asDouble();
    QValueList<cVariant>& asList();
    QMap<QString,cVariant>& asMap();
	P_CHAR		asCharacter( void );
	P_ITEM		asItem( void );
	SERIAL		asSerial( void );

    static const char* typeToName( Type typ );
    static Type nameToType( const char* name );

    void load( ISerialization& );
    void save( ISerialization& ) const;

	bool		isString( void );
	bool		isCharacter( void );
	bool		isItem( void );
	bool		isSerial( void );


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
	    uint u;
	    int i;
	    bool b;
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

inline QValueListConstIterator<QString> cVariant::stringListBegin() const
{
    if ( d->typ != StringList )
	return QValueListConstIterator<QString>();
    return ((const QStringList*)d->value.ptr)->begin();
}

inline QValueListConstIterator<QString> cVariant::stringListEnd() const
{
    if ( d->typ != StringList )
	return QValueListConstIterator<QString>();
    return ((const QStringList*)d->value.ptr)->end();
}

inline QValueListConstIterator<cVariant> cVariant::listBegin() const
{
    if ( d->typ != List )
	return QValueListConstIterator<cVariant>();
    return ((const QValueList<cVariant>*)d->value.ptr)->begin();
}

inline QValueListConstIterator<cVariant> cVariant::listEnd() const
{
    if ( d->typ != List )
	return QValueListConstIterator<cVariant>();
    return ((const QValueList<cVariant>*)d->value.ptr)->end();
}

inline QMapConstIterator<QString,cVariant> cVariant::mapBegin() const
{
    if ( d->typ != Map )
	return QMapConstIterator<QString,cVariant>();
    return ((const QMap<QString,cVariant>*)d->value.ptr)->begin();
}

inline QMapConstIterator<QString,cVariant> cVariant::mapEnd() const
{
    if ( d->typ != Map )
	return QMapConstIterator<QString,cVariant>();
    return ((const QMap<QString,cVariant>*)d->value.ptr)->end();
}

inline QMapConstIterator<QString,cVariant> cVariant::mapFind( const QString& key ) const
{
    if ( d->typ != Map )
	return QMapConstIterator<QString,cVariant>();
    return ((const QMap<QString,cVariant>*)d->value.ptr)->find( key );
}

class cCustomTags
{
public:
	cCustomTags() {;}
	~cCustomTags() {;}

	cVariant	get( QString key ) { return this->tags_[ key ]; }
	void		set( QString key, cVariant value ) 
	{
		if( !value.isValid() )
			this->tags_.erase( key );
		else
			this->tags_.insert(std::make_pair(key, value)); 
	}
	void		remove( QString key ) { this->tags_.erase( key ); }

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
};

#endif
