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

#include "customtags.h"
#include "items.h"
#include "basechar.h"
#include "persistentbroker.h"
#include "dbdriver.h"
#include "basics.h"

#include "world.h"

#include <math.h>
#include <qstring.h>

/*!
	\class cVariant customtags.h
	\brief The cVariant class acts like a union for the most common Wolfpack data types.

	\ingroup objectmodel
	\ingroup misc
	\mainclass

	Because C++ forbids unions from including types that have
	non-default constructors or destructors, most interesting Wolfpack
	classes cannot be used in unions. Without cVariant, this would be
	a problem for Python properties, Custom tags, etc.

	The methods named toT() (for any supported T, see the \c Type
	documentation for a list) are const. If you ask for the stored
	type, they return a copy of the stored object. If you ask for a
	type that can be generated from the stored type, toT() copies and
	converts and leaves the object itself unchanged. If you ask for a
	type that cannot be generated from the stored type, the result
	depends on the type (see the function documentation for details).

	The asT() functions are not const. They do conversion like the
	toT() methods, set the variant to hold the converted value, and
	return a reference to the new contents of the variant.

	Here is some example code to demonstrate the use of cVariant:

	\code
		cVariant val;
		if( PyString_Check( value ) )
			val = cVariant( PyString_AsString( value ) );
		else if( PyInt_Check( value ) )
			val = cVariant( PyInt_AsLong( value ) );
		else if( checkWpItem( value ) )
			val = cVariant( getWpItem( value ) );
		else if( checkWpChar( value ) )
			val = cVariant( getWpChar( value ) );
		else if( checkWpCoord( value ) )
			val = cVariant( getWpCoord( value ) );
		else if( PyFloat_Check( value ) )
			val = cVariant( PyFloat_AsDouble( value ) );
	\endcode

*/

/*!
  Constructs an invalid variant.
*/
cVariant::cVariant()
{
	typ = InvalidType;
}

cVariant::cVariant( const cVariant& v )
{
	*this = v;
}

/*!
  Constructs a new variant with a string value, \a val.
*/
cVariant::cVariant( const QString& val )
{
	typ = StringType;
	value.ptr = new QString( val );
}

/*!
  Constructs a new variant with an integer value, \a val.
*/
cVariant::cVariant( int val )
{
	typ = IntType;
	value.i = val;
}

/*!
  Constructs a new variant with an integer value, \a val.
*/
cVariant::cVariant( unsigned int val )
{
	typ = IntType;
	value.i = ( int ) val;
}

/*!
  Constructs a new variant with a floating point value, \a val.
*/
cVariant::cVariant( double val )
{
	typ = DoubleType;
	value.d = val;
}

cVariant::cVariant( long int val )
{
	typ = LongType;
	value.d = val;
}

/*!
  Constructs a new variant with a cBaseChar* value, \a val.
*/
cVariant::cVariant( cBaseChar* val )
{
	typ = BaseCharType;
	value.ptr = val;
}

/*!
  Constructs a new variant with a cItem* value, \a val.
*/
cVariant::cVariant( cItem* val )
{
	typ = ItemType;
	value.ptr = val;
}

/*!
  Constructs a new variant with a Coord value, \a val.
*/
cVariant::cVariant( const Coord& val )
{
	typ = CoordType;
	value.ptr = new Coord( val );
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
	cVariant::clear();
}

/*!
	Assigns the value of the variant \a variant to this variant.
	This is a deep copy of the variant.
*/
cVariant& cVariant::operator=( const cVariant& v )
{
	typ = v.typ;

	// For non pointer types we can simply use the union
	switch ( typ )
	{
	case StringType:
		value.ptr = new QString( v.toString() );
		break;
	case CoordType:
		value.ptr = new Coord( v.toCoord() );
		break;
	default:
		memcpy( &value, &v.value, sizeof( value ) );
		break;
	}

	return *this;
}

/*!
	Compares this QVariant with \a v and returns TRUE if they are
	equal; otherwise returns FALSE.
*/
bool cVariant::operator==( const cVariant& v ) const
{
	if ( typ == v.typ )
	{
		switch ( typ )
		{
		case StringType:
			return *( QString * ) value.ptr == *( QString * ) v.value.ptr;

		case CoordType:
			return *( Coord * ) value.ptr == *( Coord * ) v.value.ptr;

		case IntType:
			return value.i == v.value.i;

		case DoubleType:
			return value.d == v.value.d;

		default:
			return true;
		}
	}

	return false;
}

/*!
	Compares this QVariant with \a v and returns TRUE if they are not
	equal; otherwise returns FALSE.
*/
bool cVariant::operator!=( const cVariant& v ) const
{
	return !operator == ( v );
}

/*!
	Returns the name of the type stored in the variant.
	The returned strings describe the C++ datatype used to store the
	data: for example, "QFont", "QString", or "QValueList<cVariant>".
	An InvalidType variant returns 0.
*/
const char* cVariant::typeName() const
{
	return typeToName( typ );
}

/*!
	Convert this variant to type InvalidType and free up any resources used.
*/
void cVariant::clear()
{
	switch ( typ )
	{
	case cVariant::StringType:
		delete ( QString * ) value.ptr;
		break;
	case cVariant::CoordType:
		delete ( Coord * ) value.ptr;
		break;
	default:
		break;
	}

	typ = cVariant::InvalidType;
}

static const int ntypes = 8;
static const char* const type_map[ntypes] =
{
	0, "String", "Int", "Long", "Double", "BaseChar", "Item", "Coord"
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
	representation, the variant is set to \c InvalidType.
*/
cVariant::Type cVariant::nameToType( const char* name )
{
	for ( int i = 0; i < ntypes; i++ )
	{
		if ( !qstrcmp( type_map[i], name ) )
			return ( Type ) i;
	}
	return InvalidType;
}

/*! \fn Type cVariant::type() const

	Returns the storage type of the value stored in the variant. Usually
	it's best to test with canCast() whether the variant can deliver the
	data type you are interested in.
*/

/*! \fn bool cVariant::isValid() const

	Returns TRUE if the storage type of this variant is not
	cVariant::InvalidType; otherwise returns FALSE.
*/

/*!
	Returns the variant as a QString if the variant has type()
	StringType, CString, ByteArray, IntType, Uint, Bool, DoubleType, Date, Time, or DateTime,
	or QString::null otherwise.

	\sa asString()
*/
const QString cVariant::toString() const
{
	if ( typ == IntType )
		return QString::number( value.i );

	if ( typ == LongType )
		return QString::number( value.d );

	if ( typ == DoubleType )
		return QString::number( toDouble() );

	if ( typ == BaseCharType )
	{
		P_CHAR pChar = static_cast<P_CHAR>( value.ptr );
		if ( pChar )
			return "0x" + QString::number( ( unsigned int ) pChar->serial(), 16 );
		else
			return "0x" + QString::number( ( unsigned int ) INVALID_SERIAL, 16 );
	}

	if ( typ == ItemType )
	{
		P_ITEM pItem = static_cast<P_ITEM>( value.ptr );
		if ( pItem )
			return "0x" + QString::number( ( unsigned int ) pItem->serial(), 16 );
		else
			return "0x" + QString::number( ( unsigned int ) INVALID_SERIAL, 16 );
	}

	if ( typ == CoordType )
	{
		Coord* pos = static_cast<Coord*>( value.ptr );
		return QString( "%1,%2,%3,%4" ).arg( pos->x ).arg( pos->y ).arg( pos->z ).arg( pos->map );
	}

	if ( typ != StringType )
		return QString::null;

	return *( ( QString * ) value.ptr );
}

/*!
	Returns the variant as an int if the variant has type()
	StringType, CString, IntType, UInt, DoubleType, Bool or KeySequence; or 0 otherwise.

	If \a ok is non-null, \a *ok is set to TRUE if the value could be
	converted to an int and FALSE otherwise.

	\sa asInt() canCast()
*/
int cVariant::toInt( bool* ok ) const
{
	if ( typ == StringType )
		return hex2dec( *( ( QString * ) value.ptr ) ).toInt( ok );

	if ( ok )
		*ok = canCast( IntType );

	if ( typ == IntType )
		return value.i;

	if ( typ == LongType )
		return ( int ) value.d;

	if ( typ == DoubleType )
		return ( int ) value.d;

	if ( typ == BaseCharType )
	{
		P_CHAR pChar = static_cast<P_CHAR>( value.ptr );
		return pChar ? pChar->serial() : INVALID_SERIAL;
	}

	if ( typ == ItemType )
	{
		P_ITEM pItem = static_cast<P_ITEM>( value.ptr );
		return pItem ? pItem->serial() : INVALID_SERIAL;
	}

	return 0;
}

/*!
	Returns the variant as a double if the variant has type()
	StringType, CString, DoubleType, IntType, UInt, or Bool; or 0.0 otherwise.

	If \a ok is non-null, \a *ok is set to TRUE if the value could be
	converted to a double and FALSE otherwise.

	\sa asDouble()
*/
double cVariant::toDouble( bool* ok ) const
{
	if ( typ == StringType )
		return ( ( QString * ) value.ptr )->toDouble( ok );

	if ( ok )
		*ok = canCast( DoubleType );

	if ( typ == DoubleType )
		return value.d;

	if ( typ == IntType )
		return ( double ) value.i;

	if ( typ == LongType )
		return ( double ) value.d;

	if ( typ == BaseCharType )
	{
		P_CHAR pChar = static_cast<P_CHAR>( value.ptr );
		return pChar ? ( double ) pChar->serial() : ( double ) INVALID_SERIAL;
	}

	if ( typ == ItemType )
	{
		P_ITEM pItem = static_cast<P_ITEM>( value.ptr );
		return pItem ? ( double ) pItem->serial() : ( double ) INVALID_SERIAL;
	}

	return 0.0;
}

/*!
	Returns the variant as a Character if the variant has type()
	StringType, DoubleType, IntType; or NULL otherwise.

	\sa toChar()
*/
cBaseChar* cVariant::toChar() const
{
	if ( typ == BaseCharType )
		return ( P_CHAR ) value.ptr;

	if ( typ == StringType )
		return FindCharBySerial( hex2dec( *( ( QString * ) value.ptr ) ).toUInt() );

	if ( typ == IntType )
		return FindCharBySerial( value.i );

	if ( typ == LongType )
		return FindCharBySerial( ( unsigned int ) value.d );

	if ( typ == DoubleType )
		return FindCharBySerial( ( unsigned int ) floor( value.d ) );

	return 0;
}

/*!
	Returns the variant as an ItemType if the variant has type()
	StringType, DoubleType, IntType; or NULL otherwise.
*/
cItem* cVariant::toItem() const
{
	if ( typ == ItemType )
		return ( P_ITEM ) value.ptr;

	if ( typ == StringType )
		return FindItemBySerial( hex2dec( *( ( QString * ) value.ptr ) ).toUInt() );

	if ( typ == IntType )
		return FindItemBySerial( value.i );

	if ( typ == LongType )
		return FindItemBySerial( ( unsigned int ) value.d );

	if ( typ == DoubleType )
		return FindItemBySerial( ( unsigned int ) floor( value.d ) );

	return 0;
}

/*!
  Returns the variant as a Coordinate if the variant has type()
  StringType; or NULL otherwise.
*/
Coord cVariant::toCoord() const
{
	if ( typ == CoordType )
		return *( ( Coord * ) value.ptr );

	// Parse Coord
	if ( typ == StringType )
	{
		Coord pos;
		if ( parseCoordinates( *( ( QString * ) value.ptr ), pos ) )
			return pos;
	}

	return Coord( 0, 0, 0, 0 );
}

/*! \fn QString& cVariant::asString()

	Tries to convert the variant to hold a string value. If that
	is not possible the variant is set to an empty string.

	Returns a reference to the stored string.

	\sa toString()
*/
QString& cVariant::asString()
{
	if ( typ != StringType )
		*this = cVariant( toString() );
	return *( ( QString * ) value.ptr );
}

/*!
	Returns the variant's value as int reference.
*/
int& cVariant::asInt()
{
	if ( typ != IntType )
	{
		int i = toInt();
		clear();
		value.i = i;
		typ = IntType;
	}
	return value.i;
}

/*!
	Returns the variant's value as double reference.
*/
double& cVariant::asDouble()
{
	if ( typ != DoubleType )
	{
		double dbl = toDouble();
		clear();
		value.d = dbl;
		typ = DoubleType;
	}
	return value.d;
}

/*!
	Returns TRUE if the variant's type can be cast to the requested
	type, \a t. Such casting is done automatically when calling the
	toInt(), toBool(), ... or asInt(), asBool(), ... methods.

	The following casts are done automatically:
	\list
	\i DoubleType => StringType, IntType
	\i IntType => StringType, DoubleType
	\i StringType => IntType, DoubleType
	\endlist
*/
bool cVariant::canCast( Type t ) const
{
	if ( typ == t )
		return TRUE;
	if ( t == IntType && ( typ == IntType || typ == LongType || typ == BaseCharType || typ == ItemType || typ == StringType || typ == DoubleType ) )
		return TRUE;
	if ( t == DoubleType && ( typ == BaseCharType || typ == ItemType || typ == LongType || typ == StringType || typ == IntType ) )
		return TRUE;
	if ( t == StringType && ( typ == BaseCharType || typ == ItemType || typ == LongType || typ == IntType || typ == DoubleType ) )
		return TRUE;
	if ( t == BaseCharType && ( typ == BaseCharType || typ == IntType || typ == DoubleType || typ == StringType || typ == LongType ) )
		return TRUE;
	if ( t == ItemType && ( typ == ItemType || typ == IntType || typ == DoubleType || typ == StringType || typ == LongType ) )
		return TRUE;
	if ( t == CoordType && ( typ == StringType || typ == CoordType ) )
		return TRUE;

	return FALSE;
}

/*!
	Casts the variant to the requested type.  If the cast cannot be
	done, the variant is set to the default value of the requested type
	(e.g. an empty string if the requested type \a t is
	cVariant::StringType, an empty point array if the requested type \a t is
	cVariant::PointArray, etc).  Returns TRUE if the current type of the
	variant was successfully casted; otherwise returns FALSE.

	\sa canCast()
*/

bool cVariant::cast( Type t )
{
	switch ( t )
	{
	case StringType:
		asString();
		break;
	case IntType:
		asInt();
		break;
	case DoubleType:
		asDouble();
		break;
	default:
		( *this ) = cVariant();
	}
	return canCast( t );
}

void cVariant::serialize( cBufferedWriter& writer, unsigned int /*version*/ )
{
	writer.writeByte( typ );
	char skipper[8];

	switch ( typ )
	{
	case InvalidType:
		writer.writeRaw( skipper, 8 );
		break;

	case StringType:
		if ( value.ptr )
		{
			writer.writeUtf8( *( QString * ) value.ptr );
		}
		else
		{
			writer.writeUtf8( QString::null );
		}
		writer.writeRaw( skipper, 4 );
		break;

	case IntType:
		writer.writeInt( value.i );
		writer.writeRaw( skipper, 4 );
		break;

	case LongType:
		writer.writeInt( value.i );
		writer.writeRaw( skipper, 4 );
		break;

	case DoubleType:
		writer.writeDouble( value.d );
		break;

	case BaseCharType:
		if ( value.ptr )
		{
			writer.writeInt( ( ( P_CHAR ) value.ptr )->serial() );
		}
		else
		{
			writer.writeInt( INVALID_SERIAL );
		}
		writer.writeRaw( skipper, 4 );
		break;

	case ItemType:
		if ( value.ptr )
		{
			writer.writeInt( ( ( P_ITEM ) value.ptr )->serial() );
		}
		else
		{
			writer.writeInt( INVALID_SERIAL );
		}
		writer.writeRaw( skipper, 4 );
		break;

	case CoordType:
		writer.writeShort( ( ( Coord * ) ( value.ptr ) )->x );
		writer.writeShort( ( ( Coord * ) ( value.ptr ) )->y );
		writer.writeByte( ( ( Coord * ) ( value.ptr ) )->z );
		writer.writeByte( ( ( Coord * ) ( value.ptr ) )->map );
		writer.writeRaw( skipper, 2 );
		break;
	}
}

void cVariant::serialize( cBufferedReader& reader, unsigned int /*version*/ )
{
	// Only invalid can be loaded
	if ( typ != InvalidType )
	{
		return;
	}

	unsigned char type = reader.readByte();
	typ = ( Type ) type;
	switch ( typ )
	{
	case InvalidType:
		reader.readInt();
		reader.readInt();
		break;

	case StringType:
		value.ptr = new QString( reader.readUtf8() );
		reader.readInt();
		break;

	case IntType:
		value.i = reader.readInt();
		reader.readInt();
		break;

	case LongType:
		value.i = reader.readInt();
		reader.readInt();
		break;

	case DoubleType:
		value.d = reader.readDouble();
		break;

	case BaseCharType:
		value.ptr = World::instance()->findChar( reader.readInt() );
		reader.readInt();
		break;

	case ItemType:
		value.ptr = World::instance()->findItem( reader.readInt() );
		reader.readInt();
		break;

	case CoordType:
		value.ptr = new Coord;
		( ( Coord * ) ( value.ptr ) )->x = reader.readShort();
		( ( Coord * ) ( value.ptr ) )->y = reader.readShort();
		( ( Coord * ) ( value.ptr ) )->z = reader.readByte();
		( ( Coord * ) ( value.ptr ) )->map = reader.readByte();
		reader.readShort();
		break;
	}
}

/*****************************************************************************
  cCustomTags member functions
 *****************************************************************************/
const cVariant cVariant::null;

/*!
	Copy constructor, accepts \a d cCustomTags and
	creates a copy of it.
*/
cCustomTags::cCustomTags( const cCustomTags& d )
{
	if ( d.tags_ )
		tags_ = new QMap<QString, cVariant>( *d.tags_ );
	else
		tags_ = 0;

	changed = true;
}

/*!
	Destructor, free the memory resources allocated by the custom tag
*/
cCustomTags::~cCustomTags()
{
	if ( tags_ )
		delete tags_;
}

/*!
	Provides assign semantics for custom tags.
*/
cCustomTags& cCustomTags::operator=( const cCustomTags& d )
{
	changed = true;
	if ( d.tags_ )
		tags_ = new QMap<QString, cVariant>( *d.tags_ );
	else
		tags_ = 0;

	return *this;
}

/*!
	Removes the tags from the given serial \a key object from
	the persistent storage
*/
void cCustomTags::del( SERIAL key )
{
	PersistentBroker::instance()->addToDeleteQueue( "tags", QString( "serial = '%1'" ).arg( key ) );
}

/*!
	Saves the custom tags into the persistent storage, under
	the object identified by the \a key serial.
*/
void cCustomTags::save( SERIAL key )
{
	if ( !changed )
		return;

	PersistentBroker::instance()->executeQuery( QString( "DELETE FROM tags WHERE serial = '%1'" ).arg( key ) );

	if ( !tags_ )
	{
		changed = false;
		return;
	}

	QMap<QString, cVariant>::const_iterator it( tags_->begin() );

	for ( ; it != tags_->end(); ++it )
	{
		// Erase invalid tags.
		if ( !it.data().isValid() )
		{
			continue;
		}

		// Save the Variant type and value
		QString name = it.key();
		QString type = it.data().typeName();
		QString value = it.data().toString();

		PersistentBroker::instance()->executeQuery( QString( "REPLACE INTO tags VALUES(%1,'%2','%3','%4')" ).arg( key ).arg( PersistentBroker::instance()->quoteString( name ) ).arg( type ).arg( PersistentBroker::instance()->quoteString( value ) ) );
	}

	changed = false;
}

/*!
	Loads Custom tags under the \a key serial from the persistent storage
*/
void cCustomTags::load( SERIAL key )
{
	if ( tags_ )
		tags_->clear();

	cDBResult result = PersistentBroker::instance()->query( QString( "SELECT name,type,value FROM tags WHERE serial = '%1'" ).arg( key ) );

	while ( result.fetchrow() )
	{
		QString name = result.getString( 0 );
		QString type = result.getString( 1 );
		QString value = result.getString( 2 );

		if ( !tags_ )
			tags_ = new QMap<QString, cVariant>;

		if ( type == "StringType" )
			tags_->insert( name, cVariant( value ) );
		else if ( type == "IntType" )
			tags_->insert( name, cVariant( value.toInt() ) );
		else if ( type == "DoubleType" )
			tags_->insert( name, cVariant( value.toDouble() ) );
	}

	result.free();

	changed = false;
}

/*!
	Tests if the \a key tag exists
*/
bool cCustomTags::has( const QString& key ) const
{
	if ( tags_ )
	{
		if ( tags_->find( key ) != tags_->end() )
			return true;
	}

	return false;
}

/*!
	Retrieves the value of the given \a key tag
	\sa cVariant
*/
const cVariant& cCustomTags::get( const QString& key ) const
{
	if ( tags_ )
	{
		QMap<QString, cVariant>::iterator it = tags_->find( key );
		if ( it != tags_->end() )
			return it.data();
	}

	return cVariant::null;
}


void cCustomTags::set( const QString& key, const cVariant& value )
{
	if ( !tags_ )
		tags_ = new QMap<QString, cVariant>;

	QMap<QString, cVariant>::iterator iter = tags_->find( key );

	if ( iter != tags_->end() )
	{
		if ( !value.isValid() )
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
	if ( tags_ )
	{
		QMap<QString, cVariant>::iterator iter( tags_->find( key ) );

		if ( iter != tags_->end() )
		{
			tags_->erase( iter );
			changed = true;
		}

		if ( tags_->count() == 0 )
		{
			delete tags_;
			tags_ = 0;
		}
	}
}

QStringList cCustomTags::getKeys( void ) const
{
	if ( tags_ )
	{
		return tags_->keys();
	}

	return QStringList();
}

QValueList< cVariant > cCustomTags::getValues( void )
{
	if ( tags_ )
	{
		return tags_->values();
	}

	return QValueList<cVariant>();
}

bool cCustomTags::operator==( const cCustomTags& cmp ) const
{
	if ( !tags_ && !cmp.tags_ )
	{
		return true;
	}

	// if either is null, they differ.
	if ( !tags_ || !cmp.tags_ )
	{
		return false;
	}

	// Check if all keys of Map1 are in Map2.
	QMap<QString, cVariant>::const_iterator it;
	QMap<QString, cVariant>::const_iterator cit;
	for ( it = tags_->begin(); it != tags_->end(); ++it )
	{
		cit = cmp.tags_->find( it.key() );

		if ( cit == cmp.tags_->end() )
		{
			return false;
		}

		if ( cit.data() != it.data() )
		{
			return false;
		}
	}

	// Maybe the comparee has some tags we don't have.
	for ( it = cmp.tags_->begin(); it != cmp.tags_->end(); ++it )
	{
		cit = tags_->find( it.key() );

		if ( cit == tags_->end() )
		{
			return false;
		}

		if ( cit.data() != it.data() )
		{
			return false;
		}
	}

	return true;
}

bool cCustomTags::operator!=( const cCustomTags& cmp ) const
{
	return !( this->operator == ( cmp ) );
}

void cCustomTags::save( SERIAL serial, cBufferedWriter& writer )
{
	if ( tags_ )
	{
		QMap<QString, cVariant>::iterator it( tags_->begin() );

		for ( ; it != tags_->end(); ++it )
		{
			// Erase invalid tags.
			if ( !it.data().isValid() )
			{
				continue;
			}

			// Save serial and name
			writer.writeByte( 0xFE );
			unsigned int length = writer.position();
			writer.writeInt( serial );
			writer.writeUtf8( it.key() );
			it.data().serialize( writer, writer.version() );
			length = writer.position() - length;

			writer.setSkipSize( 0xFE, length );
		}
	}
}
