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
#include "wolfpack.h"
#include "items.h"
#include "basechar.h"
#include "persistentbroker.h"
#include "dbdriver.h"
#include "basics.h"
#include "globals.h"

#include <math.h>
#include <qstring.h>
#include <qshared.h>

cVariant::Private::Private()
{
    typ = cVariant::Invalid;
}

cVariant::Private::Private( Private* d )
{

    switch( d->typ )
	{
	case cVariant::Invalid:
	    break;
	case cVariant::String:
	    value.ptr = new QString( *((QString*)d->value.ptr) );
	    break;
	case cVariant::Int:
	    value.i = d->value.i;
	    break;
	case cVariant::Double:
	    value.d = d->value.d;
	    break;
	case cVariant::BaseChar:
		value.ptr = d->value.ptr;
	case cVariant::Item:
		value.ptr = d->value.ptr;
	case cVariant::Coord:
		value.ptr = d->value.ptr;
	default:
	    Q_ASSERT( 0 );
	}

    typ = d->typ;
}

cVariant::Private::~Private()
{
    clear();
}

void cVariant::Private::clear()
{
    switch( typ )
	{
	case cVariant::String:
	    delete (QString*)value.ptr;
	    break;
	case cVariant::Invalid:
	case cVariant::Int:
	case cVariant::Double:
	case cVariant::BaseChar:
	case cVariant::Item:
		break;
	case cVariant::Coord:
		delete (Coord_cl*)value.ptr;
	    break;
	}

    typ = cVariant::Invalid;
}

/*!
  Constructs an invalid variant.
*/
cVariant::cVariant()
{
    d = new Private;
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
    if ( d->deref() )
	delete d;
}

/*!
  Constructs a copy of the variant, \a p, passed as the argument to this
  constructor. Usually this is a deep copy, but a shallow copy is made
  if the stored data type is explicitly shared, as e.g. QImage is.
*/
cVariant::cVariant( const cVariant& p )
{
    d = new Private;
    *this = p;
}

/*!
  Constructs a new variant with a string value, \a val.
*/
cVariant::cVariant( const QString& val )
{
    d = new Private;
    d->typ = String;
    d->value.ptr = new QString( val );
}

/*!
  Constructs a new variant with an integer value, \a val.
*/
cVariant::cVariant( int val )
{
    d = new Private;
    d->typ = Int;
    d->value.i = val;
}

/*!
  Constructs a new variant with a floating point value, \a val.
*/
cVariant::cVariant( double val )
{
    d = new Private;
    d->typ = Double;
    d->value.d = val;
}

cVariant::cVariant( long int val )
{
	d = new Private;
	d->typ = Long;
	d->value.d = val;
}

/*!
  Constructs a new variant with a cBaseChar* value, \a val.
*/
cVariant::cVariant( cBaseChar *val )
{
    d = new Private;
    d->typ = BaseChar;
    d->value.ptr = val;
}

/*!
  Constructs a new variant with a cChar* value, \a val.
*/
cVariant::cVariant( cItem *val )
{
    d = new Private;
    d->typ = Item;
    d->value.ptr = val;
}

/*!
  Constructs a new variant with a Coord_cl value, \a val.
*/
cVariant::cVariant( Coord_cl val )
{
    d = new Private;
    d->typ = Coord;
    d->value.ptr = new Coord_cl( val );
}

/*!
  Assigns the value of the variant \a variant to this variant.

  This is a deep copy of the variant, but note that if the variant
  holds an explicitly shared type such as QImage, a shallow copy
  is performed.
*/
cVariant& cVariant::operator= ( const cVariant& variant )
{
    cVariant& other = (cVariant&)variant;

    other.d->ref();
    if ( d->deref() )
	delete d;

    d = other.d;

    return *this;
}

/*!
  \internal
*/
void cVariant::detach()
{
    if ( d->count == 1 )
	return;

    d->deref();
    d = new Private( d );
}

/*!
  Returns the name of the type stored in the variant.
  The returned strings describe the C++ datatype used to store the
  data: for example, "QFont", "QString", or "QValueList<cVariant>".
  An Invalid variant returns 0.
*/
const char* cVariant::typeName() const
{
    return typeToName( d->typ );
}

/*! Convert this variant to type Invalid and free up any resources
  used.
*/
void cVariant::clear()
{
    if ( d->count > 1 )
    {
	d->deref();
	d = new Private;
	return;
    }

    d->clear();
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
    if ( d->typ == Int )
		return QString::number( d->value.i );

	if ( d->typ == Long )
		return QString::number( d->value.d );
    
	if ( d->typ == Double )
		return QString::number( toDouble() );

	if ( d->typ == BaseChar )
	{
		P_CHAR pChar = static_cast< P_CHAR >( d->value.ptr );
		if( pChar )
			return "0x" + QString::number( (unsigned int)pChar->serial(), 16 );
		else
			return "0x" + QString::number( (unsigned int)INVALID_SERIAL, 16 );
	}

	if ( d->typ == Item )
	{		
		P_ITEM pItem = static_cast< P_ITEM >( d->value.ptr );
		if( pItem )
			return "0x" + QString::number( (unsigned int)pItem->serial(), 16 );
		else
			return "0x" + QString::number( (unsigned int)INVALID_SERIAL, 16 );
	}

	if ( d->typ == Coord )
	{
		Coord_cl *pos = static_cast< Coord_cl* >( d->value.ptr );
		return QString( "%1,%2,%3,%4" ).arg( pos->x ).arg( pos->y ).arg( pos->z ).arg( pos->map );
	}

    if ( d->typ != String )
		return QString::null;
    return *((QString*)d->value.ptr);
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
    if( d->typ == String )
		return hex2dec( *( (QString*)d->value.ptr ) ).toInt( ok );
    
	if ( ok )
		*ok = canCast( Int );

    if( d->typ == Int )
		return d->value.i;

	if( d->typ == Long )
		return d->value.d;

    if ( d->typ == Double )
		return (int)d->value.d;

	if ( d->typ == BaseChar )
	{
		P_CHAR pChar = static_cast< P_CHAR >( d->value.ptr );
		return pChar ? pChar->serial() : INVALID_SERIAL;
	}

	if ( d->typ == Item )
	{		
		P_ITEM pItem = static_cast< P_ITEM >( d->value.ptr );
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
    if( d->typ == String )
		return ((QString*)d->value.ptr)->toDouble( ok );
    
	if ( ok )
		*ok = canCast( Double );

    if ( d->typ == Double )
		return d->value.d;

    if ( d->typ == Int )
		return (double)d->value.i;

	if ( d->typ == Long )
		return (double)d->value.d;

	if ( d->typ == BaseChar )
	{
		P_CHAR pChar = static_cast< P_CHAR >( d->value.ptr );
		return pChar ? (double)pChar->serial() : (double)INVALID_SERIAL;
	}

	if ( d->typ == Item )
	{		
		P_ITEM pItem = static_cast< P_ITEM >( d->value.ptr );
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
	if( d->typ == BaseChar )
		return (P_CHAR)d->value.ptr;

	if( d->typ == String )
		return FindCharBySerial( hex2dec( *( (QString*)d->value.ptr ) ).toUInt() );

	if( d->typ == Int )
		return FindCharBySerial( d->value.i );

	if( d->typ == Long )
		return FindCharBySerial( d->value.d );

	if( d->typ == Double )
		return FindCharBySerial( floor( d->value.d ) );

	return 0;
}

/*!
  Returns the variant as an Item if the variant has type()
  String, Double, Int; or NULL otherwise.
*/
cItem *cVariant::toItem() const
{
	if( d->typ == Item )
		return (P_ITEM)d->value.ptr;

	if( d->typ == String )
		return FindItemBySerial( hex2dec( *( (QString*)d->value.ptr ) ).toUInt() );

	if( d->typ == Int )
		return FindItemBySerial( d->value.i );

	if( d->typ == Long )
		return FindItemBySerial( d->value.d );

	if( d->typ == Double )
		return FindItemBySerial( floor( d->value.d ) );

	return 0;
}

/*!
  Returns the variant as a Coordinate if the variant has type()
  String; or NULL otherwise.
*/
Coord_cl cVariant::toCoord() const
{
	if( d->typ == Coord )
		return *( (Coord_cl*)d->value.ptr );

	// Parse Coord
	if( d->typ == String )
	{
		Coord_cl pos;
		if( parseCoordinates( *( (QString*)d->value.ptr ), pos ) )
			return pos;
	}

	return Coord_cl( 0, 0, 0, 0 );
}

#define Q_VARIANT_AS( f ) Q##f& cVariant::as##f() { \
   if ( d->typ != f ) *this = cVariant( to##f() ); else detach(); return *((Q##f*)d->value.ptr);}

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
    detach();
    if ( d->typ != Int ) {
	int i = toInt();
	d->clear();
 	d->value.i = i;
	d->typ = Int;
    }
    return d->value.i;
}

/*!
  Returns the variant's value as double reference.
*/
double& cVariant::asDouble()
{
    if ( d->typ != Double ) {
	double dbl = toDouble();
	d->clear();
	d->value.d = dbl;
	d->typ = Double;
    }
    return d->value.d;
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
    if ( d->typ == t )
		return TRUE;
    if ( t == Int && ( d->typ == Int || d->typ == Long || d->typ == BaseChar || d->typ == Item || d->typ == String || d->typ == Double ) )
		return TRUE;
    if ( t == Double && ( d->typ == BaseChar || d->typ == Item || d->typ == Long || d->typ == String || d->typ == Int ) )
		return TRUE;
    if ( t == String && ( d->typ == BaseChar || d->typ == Item || d->typ == Long || d->typ == Int || d->typ == Double ) )
		return TRUE;
	if ( t == BaseChar && ( d->typ == BaseChar || d->typ == Int || d->typ == Double || d->typ == String || d->typ == Long ) )
		return TRUE;
	if ( t == Item && ( d->typ == Item || d->typ == Int || d->typ == Double || d->typ == String || d->typ == Long ) )
		return TRUE;
	if ( t == Coord && ( d->typ == String || d->typ == Coord ) )
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

/*!  Compares this cVariant with \a v and returns TRUE if they are
  equal; otherwise returns FALSE.
*/

bool cVariant::operator==( const cVariant &v ) const
{
    if ( !v.canCast( type() ) )
	return FALSE;
    switch( d->typ ) {
    case String:
	return v.toString() == toString();
    case Int:
	return v.toInt() == toInt();
    case Double:
	return v.toDouble() == toDouble();
    case Invalid:
	break;
    }
    return FALSE;
}

/*!  Compares this cVariant with \a v and returns TRUE if they are
  not equal; otherwise returns FALSE.
*/

bool cVariant::operator!=( const cVariant &v ) const
{
    return !( v == *this );
}



/*****************************************************************************
  cCustomTags member functions
 *****************************************************************************/

cCustomTags::cCustomTags( const cCustomTags& d )
{
	tags_ = d.tags_;
	changed = true;
}

cCustomTags& cCustomTags::operator=( const cCustomTags& tags)
{
	changed = true;
	tags_ = tags.tags_;
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

	QMap< QString, cVariant >::const_iterator it( tags_.begin() );
	if ( tags_.count() > 6 )
		persistentBroker->lockTable("tags");
	for( ; it != tags_.end(); ++it )
	{
		// Erase invalid tags.
		if( !it.data().isValid() )
		{
			continue;
		}

		// Save the Variant type and value
		QString type = it.data().typeName();
		QString value = it.data().toString();
		QString name = it.key();

		persistentBroker->executeQuery( QString( "INSERT INTO tags SET serial = '%1', type = '%2', value = '%3', name = '%4'" ).arg( key ).arg( type ).arg( __escapeReservedCharacters( value ) ).arg( __escapeReservedCharacters( name ) ) );
	}
	if ( tags_.count() > 6 )
		persistentBroker->unlockTable("tags");

	changed = false;
}

void cCustomTags::load( SERIAL key )
{
	tags_.clear();

	cDBResult result = persistentBroker->query( QString( "SELECT serial,name,type,value FROM tags WHERE serial = '%1'" ).arg( key ) );

	while( result.fetchrow() )
	{
		QString name = result.getString( 1 );
		QString type = result.getString( 2 );
		QString value = result.getString( 3 );

		if( type == "String" )
			tags_.insert( name, cVariant( value ) );
		else if( type == "Int" )
			tags_.insert( name, cVariant( value.toInt() ) );
		else if( type == "Double" )
			tags_.insert( name, cVariant( value.toDouble() ) );
	}

	result.free();

	changed = false;
}

cVariant cCustomTags::get( const QString& key ) 
{ 
	QMap< QString, cVariant >::iterator it = tags_.find( key );
	if( it != tags_.end() )
		return it.data();
	else
		return cVariant();
}

void cCustomTags::set( const QString& key, const cVariant& value ) 
{
	QMap< QString, cVariant >::iterator iter = tags_.find( key );
	
	if( iter != tags_.end() )
	{
		if( !value.isValid() )
		{
			tags_.erase( iter );
			changed = true;
		}
		else if( iter.data() != value )
		{
			iter.data() = value;
			changed = true;
		}
	}
	else
	{
		tags_.insert( key, value ); 
		changed = true;
	}
}

void cCustomTags::remove( const QString& key )
{
	QMap< QString, cVariant >::iterator iter( tags_.find( key ) );
	
	if( iter != tags_.end() )
	{
		tags_.erase( iter );
		changed = true;
	}
}

QStringList cCustomTags::getKeys( void )
{
	return tags_.keys();
}

QValueList< cVariant > cCustomTags::getValues( void )
{
	return tags_.values();
}

