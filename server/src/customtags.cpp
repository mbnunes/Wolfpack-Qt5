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

#include "customtags.h"
#include "items.h"
#include "basechar.h"
#include "persistentbroker.h"
#include "dbdriver.h"
#include "basics.h"
#include "globals.h"
#include "world.h"

#include <math.h>
#include <qstring.h>
#include <qshared.h>

/*!
  Constructs an invalid variant.
*/
cVariant::cVariant()
{
	typ = cVariant::Invalid;
}

/*!
  Destroys the cVariant and the contained object.

  Note that subclasses that reimplement clear() should reimplement
  the destructor to call clear().  This destructor calls clear(), but
  because it is the destructor, cVariant::clear() is called rather than
  a subclass's clear().
*/
cVariant::~cVariant()
{
	clear();
}

cVariant& cVariant::operator= ( const cVariant &v )
{
	typ = v.typ;

	// For non pointer types we can simply use the union
    switch( typ )
	{
	case cVariant::String:
	    value.ptr = new QString( v.toString() );
	    break;
	case cVariant::Coord:
		value.ptr = new Coord_cl( v.toCoord() );
	    break;
	default:
		memcpy( &value, &v.value, sizeof( value ) );
		break;
	}

	return *this;
}

bool cVariant::operator==( const cVariant &v ) const
{
	if( typ == v.typ )
	{
		switch( typ )
		{
		case cVariant::String:
			return *(QString*)value.ptr == *(QString*)v.value.ptr;
			
		case cVariant::Coord:
			return *(Coord_cl*)value.ptr == *(Coord_cl*)v.value.ptr;

		case Int:
			return value.i == v.value.i;

		case Double:
			return value.d == v.value.d;

		default:
			return true;
		}
	}

	return false;
}

bool cVariant::operator!=( const cVariant &v ) const
{
	return !operator==( v );
}

cVariant::cVariant( const cVariant &v )
{
	*this = v;
}

/*!
  Constructs a new variant with a string value, \a val.
*/
cVariant::cVariant( const QString& val )
{
    typ = String;
    value.ptr = new QString( val );
}

/*!
  Constructs a new variant with an integer value, \a val.
*/
cVariant::cVariant( int val )
{
    typ = Int;
    value.i = val;
}

/*!
  Constructs a new variant with a floating point value, \a val.
*/
cVariant::cVariant( double val )
{
    typ = Double;
    value.d = val;
}

cVariant::cVariant( long int val )
{
	typ = Long;
	value.d = val;
}

/*!
  Constructs a new variant with a cBaseChar* value, \a val.
*/
cVariant::cVariant( cBaseChar *val )
{
    typ = BaseChar;
    value.ptr = val;
}

/*!
  Constructs a new variant with a cItem* value, \a val.
*/
cVariant::cVariant( cItem *val )
{
    typ = Item;
    value.ptr = val;
}

/*!
  Constructs a new variant with a Coord_cl value, \a val.
*/
cVariant::cVariant( Coord_cl val )
{
    typ = Coord;
    value.ptr = new Coord_cl( val );
}

/*!
  Returns the name of the type stored in the variant.
  The returned strings describe the C++ datatype used to store the
  data: for example, "QFont", "QString", or "QValueList<cVariant>".
  An Invalid variant returns 0.
*/
const char* cVariant::typeName() const
{
    return typeToName( typ );
}

/*! Convert this variant to type Invalid and free up any resources
  used.
*/
void cVariant::clear()
{
    switch( typ )
	{
	case cVariant::String:
	    delete (QString*)value.ptr;
	    break;
	case cVariant::Coord:
		delete (Coord_cl*)value.ptr;
	    break;
	}

    typ = cVariant::Invalid;
}

static const int ntypes = 7;
static const char* const type_map[ntypes] =
{
    0,
    "String",
    "Int",
    "Double",
	"BaseChar",
	"Item",
	"Coord"
};

/*!
  Converts the enum representation of the storage type, \a typ, to its
  string representation.
*/
const char* cVariant::typeToName( Type typ )
{
    if ( typ >= ntypes )
	return 0;
    return type_map[typ];
}


/*!
  Converts the string representation of the storage type gven in \a
  name, to its enum representation.

  If the string representation cannot be converted to any enum
  representation, the variant is set to \c Invalid.
*/
cVariant::Type cVariant::nameToType( const char* name )
{
    for ( int i = 0; i < ntypes; i++ ) {
	if ( !qstrcmp( type_map[i], name ) )
	    return (Type) i;
    }
    return Invalid;
}

/*! \fn Type cVariant::type() const

  Returns the storage type of the value stored in the variant. Usually
  it's best to test with canCast() whether the variant can deliver the
  data type you are interested in.
*/

/*! \fn bool cVariant::isValid() const

  Returns TRUE if the storage type of this variant is not
  cVariant::Invalid; otherwise returns FALSE.
*/

/*!
  Returns the variant as a QString if the variant has type()
  String, CString, ByteArray, Int, Uint, Bool, Double, Date, Time, or DateTime,
  or QString::null otherwise.

  \sa asString()
*/
const QString cVariant::toString() const
{
    if ( typ == Int )
		return QString::number( value.i );

	if ( typ == Long )
		return QString::number( value.d );
    
	if ( typ == Double )
		return QString::number( toDouble() );

	if ( typ == BaseChar )
	{
		P_CHAR pChar = static_cast< P_CHAR >( value.ptr );
		if( pChar )
			return "0x" + QString::number( (unsigned int)pChar->serial(), 16 );
		else
			return "0x" + QString::number( (unsigned int)INVALID_SERIAL, 16 );
	}

	if ( typ == Item )
	{		
		P_ITEM pItem = static_cast< P_ITEM >( value.ptr );
		if( pItem )
			return "0x" + QString::number( (unsigned int)pItem->serial(), 16 );
		else
			return "0x" + QString::number( (unsigned int)INVALID_SERIAL, 16 );
	}

	if ( typ == Coord )
	{
		Coord_cl *pos = static_cast< Coord_cl* >( value.ptr );
		return QString( "%1,%2,%3,%4" ).arg( pos->x ).arg( pos->y ).arg( pos->z ).arg( pos->map );
	}

    if ( typ != String )
		return QString::null;
    return *((QString*)value.ptr);
}

/*!
  Returns the variant as an int if the variant has type()
  String, CString, Int, UInt, Double, Bool or KeySequence; or 0 otherwise.

  If \a ok is non-null, \a *ok is set to TRUE if the value could be
  converted to an int and FALSE otherwise.

  \sa asInt() canCast()
*/
int cVariant::toInt( bool * ok ) const
{
    if( typ == String )
		return hex2dec( *( (QString*)value.ptr ) ).toInt( ok );
    
	if ( ok )
		*ok = canCast( Int );

    if( typ == Int )
		return value.i;

	if( typ == Long )
		return value.d;

    if ( typ == Double )
		return (int)value.d;

	if ( typ == BaseChar )
	{
		P_CHAR pChar = static_cast< P_CHAR >( value.ptr );
		return pChar ? pChar->serial() : INVALID_SERIAL;
	}

	if ( typ == Item )
	{		
		P_ITEM pItem = static_cast< P_ITEM >( value.ptr );
		return pItem ? pItem->serial() : INVALID_SERIAL;
	}

    return 0;
}

/*!
  Returns the variant as a double if the variant has type()
  String, CString, Double, Int, UInt, or Bool; or 0.0 otherwise.

  If \a ok is non-null, \a *ok is set to TRUE if the value could be
  converted to a double and FALSE otherwise.

  \sa asDouble()
*/
double cVariant::toDouble( bool * ok ) const
{
    if( typ == String )
		return ((QString*)value.ptr)->toDouble( ok );
    
	if ( ok )
		*ok = canCast( Double );

    if ( typ == Double )
		return value.d;

    if ( typ == Int )
		return (double)value.i;

	if ( typ == Long )
		return (double)value.d;

	if ( typ == BaseChar )
	{
		P_CHAR pChar = static_cast< P_CHAR >( value.ptr );
		return pChar ? (double)pChar->serial() : (double)INVALID_SERIAL;
	}

	if ( typ == Item )
	{		
		P_ITEM pItem = static_cast< P_ITEM >( value.ptr );
		return pItem ? (double)pItem->serial() : (double)INVALID_SERIAL;
	}

    return 0.0;
}

/*!
  Returns the variant as a Character if the variant has type()
  String, Double, Int; or NULL otherwise.

  \sa toChar()
*/
cBaseChar *cVariant::toChar() const
{
	if( typ == BaseChar )
		return (P_CHAR)value.ptr;

	if( typ == String )
		return FindCharBySerial( hex2dec( *( (QString*)value.ptr ) ).toUInt() );

	if( typ == Int )
		return FindCharBySerial( value.i );

	if( typ == Long )
		return FindCharBySerial( value.d );

	if( typ == Double )
		return FindCharBySerial( floor( value.d ) );

	return 0;
}

/*!
  Returns the variant as an Item if the variant has type()
  String, Double, Int; or NULL otherwise.
*/
cItem *cVariant::toItem() const
{
	if( typ == Item )
		return (P_ITEM)value.ptr;

	if( typ == String )
		return FindItemBySerial( hex2dec( *( (QString*)value.ptr ) ).toUInt() );

	if( typ == Int )
		return FindItemBySerial( value.i );

	if( typ == Long )
		return FindItemBySerial( value.d );

	if( typ == Double )
		return FindItemBySerial( floor( value.d ) );

	return 0;
}

/*!
  Returns the variant as a Coordinate if the variant has type()
  String; or NULL otherwise.
*/
Coord_cl cVariant::toCoord() const
{
	if( typ == Coord )
		return *( (Coord_cl*)value.ptr );

	// Parse Coord
	if( typ == String )
	{
		Coord_cl pos;
		if( parseCoordinates( *( (QString*)value.ptr ), pos ) )
			return pos;
	}

	return Coord_cl( 0, 0, 0, 0 );
}

#define Q_VARIANT_AS( f ) Q##f& cVariant::as##f() { \
   if ( typ != f ) *this = cVariant( to##f() ); return *((Q##f*)value.ptr);}

Q_VARIANT_AS(String)

/*! \fn QString& cVariant::asString()

  Tries to convert the variant to hold a string value. If that
  is not possible the variant is set to an empty string.

  Returns a reference to the stored string.

  \sa toString()
*/

/*!
  Returns the variant's value as int reference.
*/
int& cVariant::asInt()
{
    if ( typ != Int ) {
	int i = toInt();
	clear();
 	value.i = i;
	typ = Int;
    }
    return value.i;
}

/*!
  Returns the variant's value as double reference.
*/
double& cVariant::asDouble()
{
    if ( typ != Double ) {
	double dbl = toDouble();
	clear();
	value.d = dbl;
	typ = Double;
    }
    return value.d;
}

/*!
  Returns TRUE if the variant's type can be cast to the requested
  type, \a t. Such casting is done automatically when calling the
  toInt(), toBool(), ... or asInt(), asBool(), ... methods.

  The following casts are done automatically:
  \list
  \i Double => String, Int
  \i Int => String, Double
  \i String => Int, Double
  \endlist
*/
bool cVariant::canCast( Type t ) const
{
    if ( typ == t )
		return TRUE;
    if ( t == Int && ( typ == Int || typ == Long || typ == BaseChar || typ == Item || typ == String || typ == Double ) )
		return TRUE;
    if ( t == Double && ( typ == BaseChar || typ == Item || typ == Long || typ == String || typ == Int ) )
		return TRUE;
    if ( t == String && ( typ == BaseChar || typ == Item || typ == Long || typ == Int || typ == Double ) )
		return TRUE;
	if ( t == BaseChar && ( typ == BaseChar || typ == Int || typ == Double || typ == String || typ == Long ) )
		return TRUE;
	if ( t == Item && ( typ == Item || typ == Int || typ == Double || typ == String || typ == Long ) )
		return TRUE;
	if ( t == Coord && ( typ == String || typ == Coord ) )
		return TRUE;

    return FALSE;
}

/*!
  Casts the variant to the requested type.  If the cast cannot be
  done, the variant is set to the default value of the requested type
  (e.g. an empty string if the requested type \a t is
  cVariant::String, an empty point array if the requested type \a t is
  cVariant::PointArray, etc).  Returns TRUE if the current type of the
  variant was successfully casted; otherwise returns FALSE.

  \sa canCast()
*/

bool cVariant::cast( Type t )
{
    switch ( t ) {
    case cVariant::String:
	asString();
	break;
    case cVariant::Int:
	asInt();
	break;
    case cVariant::Double:
	asDouble();
	break;
    default:
    case cVariant::Invalid:
	(*this) = cVariant();
    }
    return canCast( t );
}

/*****************************************************************************
  cCustomTags member functions
 *****************************************************************************/

cCustomTags::cCustomTags( const cCustomTags& d )
{
	if( d.tags_ )
		tags_ = new QMap< QString, cVariant >( *d.tags_ );
	else
		tags_ = 0;		

	changed = true;
}

cCustomTags& cCustomTags::operator=( const cCustomTags& tags )
{
	changed = true;
	if( tags.tags_ )
		tags_ = new QMap< QString, cVariant >( *tags.tags_ );
	else
		tags_ = 0;

	return *this;
}

void cCustomTags::del( SERIAL key )
{
	persistentBroker->addToDeleteQueue( "tags", QString( "serial = '%1'" ).arg( key ) );
}

void cCustomTags::save( SERIAL key )
{
	if( !changed )
		return;

	persistentBroker->executeQuery( QString( "DELETE FROM tags WHERE serial = '%1'" ).arg( key ) );

	if( !tags_ )
	{
		changed = false;
		return;
	}

	QMap< QString, cVariant >::const_iterator it( tags_->begin() );

	for( ; it != tags_->end(); ++it )
	{
		// Erase invalid tags.
		if( !it.data().isValid() )
		{
			continue;
		}

		// Save the Variant type and value
		QString name = it.key();
		QString type = it.data().typeName();
		QString value = it.data().toString();		

		persistentBroker->executeQuery( QString( "REPLACE INTO tags VALUES(%1,'%2','%3','%4')" ).arg( key ).arg( persistentBroker->quoteString( name ) ).arg( type ).arg( persistentBroker->quoteString( value ) ) );
	}

	changed = false;
}

void cCustomTags::load( SERIAL key )
{
	if( tags_ )
		tags_->clear();

	cDBResult result = persistentBroker->query( QString( "SELECT name,type,value FROM tags WHERE serial = '%1'" ).arg( key ) );

	while( result.fetchrow() )
	{
		QString name = result.getString( 0 );
		QString type = result.getString( 1 );
		QString value = result.getString( 2 );

		if( !tags_ )
			tags_ = new QMap< QString, cVariant >;

		if( type == "String" )
			tags_->insert( name, cVariant( value ) );
		else if( type == "Int" )
			tags_->insert( name, cVariant( value.toInt() ) );
		else if( type == "Double" )
			tags_->insert( name, cVariant( value.toDouble() ) );
	}

	result.free();

	changed = false;
}

bool cCustomTags::has( const QString &key ) const
{
	if( tags_ )
	{
		if( tags_->find( key ) != tags_->end() )
			return true;
	}
	
	return false;
}

const cVariant &cCustomTags::get( const QString& key ) const
{
	if( tags_ )
	{
		QMap< QString, cVariant >::iterator it = tags_->find( key );
		if( it != tags_->end() )
			return it.data();
	}

	return cVariant::null;
}

void cCustomTags::set( const QString& key, const cVariant& value )
{
	if( !tags_ )
		tags_ = new QMap< QString, cVariant >;

	QMap< QString, cVariant >::iterator iter = tags_->find( key );
	
	if( iter != tags_->end() )
	{
		if( !value.isValid() )
		{
			tags_->erase( iter );
			changed = true;
		}
		else
		{
			iter.data() = value;
			changed = true;
		}
	}
	else
	{
		tags_->insert( key, value ); 
		changed = true;
	}
}

void cCustomTags::remove( const QString& key )
{
	if( tags_ )
	{
		QMap< QString, cVariant >::iterator iter( tags_->find( key ) );
		
		if( iter != tags_->end() )
		{
			tags_->erase( iter );
			changed = true;
		}

		if( tags_->count() == 0 )
		{
			delete tags_;
			tags_ = 0;
		}
	}
}

QStringList cCustomTags::getKeys( void ) const
{
	if( tags_ )
	{
		return tags_->keys();
	}
	
	return QStringList();
}

QValueList< cVariant > cCustomTags::getValues( void )
{
	if( tags_ )
	{
		return tags_->values();
	}

	return QValueList< cVariant >();
}

cCustomTags::~cCustomTags()
{
	if( tags_ )
		delete tags_;
}

const cVariant cVariant::null;
