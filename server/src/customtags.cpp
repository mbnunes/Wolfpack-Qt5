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

#include "customtags.h"
#include "junk.h"
#include "chars.h"
#include "items.h"

#include "qstring.h"
#include "qcstring.h"
#include "qshared.h"
#include "qbitarray.h"

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
	case cVariant::CString:
	    // QCString is explicit shared
	    value.ptr = new QCString( *((QCString*)d->value.ptr) );
	    break;
	case cVariant::StringList:
	    value.ptr = new QStringList( *((QStringList*)d->value.ptr) );
	    break;
	case cVariant::Map:
	    value.ptr = new QMap<QString,cVariant>( *((QMap<QString,cVariant>*)d->value.ptr) );
	    break;
	case cVariant::List:
	    value.ptr = new QValueList<cVariant>( *((QValueList<cVariant>*)d->value.ptr) );
	    break;
	case cVariant::ByteArray:
	    value.ptr = new QByteArray( *((QByteArray*)d->value.ptr) );
	    break;
	case cVariant::BitArray:
	    value.ptr = new QBitArray( *((QBitArray*)d->value.ptr) );
	    break;
	case cVariant::Int:
	    value.i = d->value.i;
	    break;
	case cVariant::UInt:
	    value.u = d->value.u;
	    break;
	case cVariant::Bool:
	    value.b = d->value.b;
	    break;
	case cVariant::Double:
	    value.d = d->value.d;
	    break;
	case cVariant::Character:
		value.ptr = (P_CHAR) d->value.ptr;
		break;
	case cVariant::Item:
		value.ptr = (P_ITEM) d->value.ptr;
		break;
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
	case cVariant::CString:
	    delete (QCString*)value.ptr;
	    break;
	case cVariant::StringList:
	    delete (QStringList*)value.ptr;
	    break;
	case cVariant::Map:
	    delete (QMap<QString,cVariant>*)value.ptr;
	    break;
	case cVariant::List:
	    delete (QValueList<cVariant>*)value.ptr;
	    break;
	case cVariant::ByteArray:
	    delete (QByteArray*)value.ptr;
	    break;
	case cVariant::BitArray:
	    delete (QBitArray*)value.ptr;
	    break;
	case cVariant::Invalid:
	case cVariant::Int:
	case cVariant::UInt:
	case cVariant::Bool:
	case cVariant::Double:
	case cVariant::Character:
	case cVariant::Item:
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
  Constructs a new variant with a C-string value, \a val.

  If you want to modify the QCString after you've passed it to this
  constructor, we recommend passing a deep copy (see
  QCString::copy()).
*/
cVariant::cVariant( const QCString& val )
{
    d = new Private;
    d->typ = CString;
    d->value.ptr = new QCString( val );
}

/*!
  Constructs a new variant with a C-string value of \a val if \a val
  is non-null.  The variant creates a deep copy of \a val.

  If \a val is null, the resulting variant has type Invalid.
*/
cVariant::cVariant( const char* val )
{
    d = new Private;
    if ( val == 0 )
	return;
    d->typ = CString;
    d->value.ptr = new QCString( val );
}

/*!
  Constructs a new variant with a string list value, \a val.
*/
cVariant::cVariant( const QStringList& val )
{
    d = new Private;
    d->typ = StringList;
    d->value.ptr = new QStringList( val );
}

/*!
  Constructs a new variant with a map of cVariants, \a val.
*/
cVariant::cVariant( const QMap<QString,cVariant>& val )
{
    d = new Private;
    d->typ = Map;
    d->value.ptr = new QMap<QString,cVariant>( val );
}

/*!
  Constructs a new variant with a bytearray value, \a val.
*/
cVariant::cVariant( const QByteArray& val )
{
    d = new Private;
    d->typ = ByteArray;
    d->value.ptr = new QByteArray( val );
}

/*!
  Constructs a new variant with a bitarray value, \a val.
*/
cVariant::cVariant( const QBitArray& val )
{
    d = new Private;
    d->typ = BitArray;
    d->value.ptr = new QBitArray( val );
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
  Constructs a new variant with an unsigned integer value, \a val.
*/
cVariant::cVariant( uint val )
{
    d = new Private;
    d->typ = UInt;
    d->value.u = val;
}

/*!
  Constructs a new variant with a boolean value, \a val. The integer argument
  is a dummy, necessary for compatibility with some compilers.
*/
cVariant::cVariant( bool val, int )
{ // this is the comment that does NOT name said compiler.
    d = new Private;
    d->typ = Bool;
    d->value.b = val;
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

/*!
  Constructs a new variant with a list value, \a val.
*/
cVariant::cVariant( const QValueList<cVariant>& val )
{
    d = new Private;
    d->typ = List;
    d->value.ptr = new QValueList<cVariant>( val );
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

static const int ntypes = 14;
static const char* const type_map[ntypes] =
{
    0,
    "QMap<QString,cVariant>",
    "QValueList<cVariant>",
    "QString",
    "QStringList",
    "int",
    "uint",
    "bool",
    "double",
    "QCString",
    "QByteArray",
    "QBitArray",
	"Character",
	"Item",
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

void cVariant::load( ISerialization& s )
{
/*    Q_UINT32 u;
    s.read("type", u);
    Type t = (Type)u;

    switch( t ) {
    case Invalid:
	d->typ = t;
	break;
#ifndef QT_NO_TEMPLATE_VARIANT
    case Map:
	{
	    QMap<QString,cVariant>* x = new QMap<QString,cVariant>;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case List:
	{
	    QValueList<cVariant>* x = new QValueList<cVariant>;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
#endif
    case Cursor:
	{
#ifndef QT_NO_CURSOR
	    QCursor* x = new QCursor;
	    s >> *x;
	    d->value.ptr = x;
#endif
	}
	break;
    case Bitmap:
	{
	    QBitmap* x = new QBitmap;
#ifndef QT_NO_IMAGEIO
	    s >> *x;
#endif
	    d->value.ptr = x;
	}
	break;
    case Region:
	{
	    QRegion* x = new QRegion;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case PointArray:
	{
	    QPointArray* x = new QPointArray;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case String:
	{
	    QString* x = new QString;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case CString:
	{
	    QCString* x = new QCString;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
#ifndef QT_NO_STRINGLIST
    case StringList:
	{
	    QStringList* x = new QStringList;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
#endif // QT_NO_STRINGLIST
    case Font:
	{
	    QFont* x = new QFont;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case Pixmap:
	{
	    QPixmap* x = new QPixmap;
#ifndef QT_NO_IMAGEIO
	    s >> *x;
#endif
	    d->value.ptr = x;
	}
	break;
    case Image:
	{
	    QImage* x = new QImage;
#ifndef QT_NO_IMAGEIO
	    s >> *x;
#endif
	    d->value.ptr = x;
	}
	break;
    case Brush:
	{
	    QBrush* x = new QBrush;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case Rect:
	{
	    QRect* x = new QRect;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case Point:
	{
	    QPoint* x = new QPoint;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case Size:
	{
	    QSize* x = new QSize;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case Color:
	{
	    QColor* x = new QColor;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
#ifndef QT_NO_PALETTE
    case Palette:
	{
	    QPalette* x = new QPalette;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case ColorGroup:
	{
	    QColorGroup* x = new QColorGroup;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
#endif
#ifndef QT_NO_ICONSET
    case IconSet:
	{
	    QPixmap* x = new QPixmap;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
#endif
    case Int:
	{
	    int x;
	    s >> x;
	    d->value.i = x;
	}
	break;
    case UInt:
	{
	    uint x;
	    s >> x;
	    d->value.u = x;
	}
	break;
    case Bool:
	{
	    Q_INT8 x;
	    s >> x;
	    d->value.b = x;
	}
	break;
    case Double:
	{
	    double x;
	    s >> x;
	    d->value.d = x;
	}
	break;
    case SizePolicy:
	{
	    int h,v;
	    Q_INT8 hfw;
	    s >> h >> v >> hfw;
	    d->value.ptr = new QSizePolicy( (QSizePolicy::SizeType)h,
					    (QSizePolicy::SizeType)v,
					    (bool) hfw);
	}
	break;
    case Date:
	{
	    QDate* x = new QDate;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case Time:
	{
	    QTime* x = new QTime;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case DateTime:
	{
	    QDateTime* x = new QDateTime;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case ByteArray:
	{
	    QByteArray* x = new QByteArray;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case BitArray:
	{
	    QBitArray* x = new QBitArray;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
#ifndef QT_NO_ACCEL
    case KeySequence:
	QKeySequence* x = new QKeySequence;
 	s >> *x;
	d->value.ptr = x;
	break;
#endif // QT_NO_ACCEL
    }
    d->typ = t;
	*/
}

void cVariant::save( ISerialization& s ) const
{
/*    s << (Q_UINT32)type();

    switch( d->typ ) {
    case Cursor:
	s << *((QCursor*)d->value.ptr);
	break;
    case Bitmap:
#ifndef QT_NO_IMAGEIO
	s << *((QBitmap*)d->value.ptr);
#endif
	break;
    case PointArray:
	s << *((QPointArray*)d->value.ptr);
	break;
    case Region:
	s << *((QRegion*)d->value.ptr);
	break;
#ifndef QT_NO_TEMPLATE_VARIANT
    case List:
	s << *((QValueList<cVariant>*)d->value.ptr);
	break;
    case Map:
	s << *((QMap<QString,cVariant>*)d->value.ptr);
	break;
#endif
    case String:
	s << *((QString*)d->value.ptr);
	break;
    case CString:
	s << *((QCString*)d->value.ptr);
	break;
#ifndef QT_NO_STRINGLIST
    case StringList:
	s << *((QStringList*)d->value.ptr);
	break;
#endif
    case Font:
	s << *((QFont*)d->value.ptr);
	break;
    case Pixmap:
#ifndef QT_NO_IMAGEIO
	s << *((QPixmap*)d->value.ptr);
#endif
	break;
    case Image:
#ifndef QT_NO_IMAGEIO
	s << *((QImage*)d->value.ptr);
#endif
	break;
    case Brush:
	s << *((QBrush*)d->value.ptr);
	break;
    case Point:
	s << *((QPoint*)d->value.ptr);
	break;
    case Rect:
	s << *((QRect*)d->value.ptr);
	break;
    case Size:
	s << *((QSize*)d->value.ptr);
	break;
    case Color:
	s << *((QColor*)d->value.ptr);
	break;
#ifndef QT_NO_PALETTE
    case Palette:
	s << *((QPalette*)d->value.ptr);
	break;
    case ColorGroup:
	s << *((QColorGroup*)d->value.ptr);
	break;
#endif
#ifndef QT_NO_ICONSET
    case IconSet:
	//### add stream operator to iconset
	s << ((QIconSet*)d->value.ptr)->pixmap();
	break;
#endif
    case Int:
	s << d->value.i;
	break;
    case UInt:
	s << d->value.u;
	break;
    case Bool:
	s << (Q_INT8)d->value.b;
	break;
    case Double:
	s << d->value.d;
	break;
    case SizePolicy:
	{
	    QSizePolicy p = toSizePolicy();
	    s << (int) p.horData() << (int) p.verData()
	      << (Q_INT8) p.hasHeightForWidth();
	}
	break;
    case Date:
	s << *((QDate*)d->value.ptr);
	break;
    case Time:
	s << *((QTime*)d->value.ptr);
	break;
    case DateTime:
	s << *((QDateTime*)d->value.ptr);
	break;
    case ByteArray:
	s << *((QByteArray*)d->value.ptr);
	break;
    case BitArray:
	s << *((QBitArray*)d->value.ptr);
	break;
    case KeySequence:
	s << *((QKeySequence*)d->value.ptr);
	break;
    case Invalid:
	s << QString(); // ### looks wrong.
	break;
    }
	*/
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

/*! \fn QValueListConstIterator<QString> cVariant::stringListBegin() const

  Returns an iterator to the first string in the list if the
  variant's type is StringList, or else a null iterator.
*/

/*! \fn QValueListConstIterator<QString> cVariant::stringListEnd() const

  Returns the end iterator for the list if the variant's type is
  StringList, or else a null iterator.
*/

/*! \fn QValueListConstIterator<cVariant> cVariant::listBegin() const

  Returns an iterator to the first item in the list if the
  variant's type is appropriate, or else a null iterator.
*/

/*! \fn QValueListConstIterator<cVariant> cVariant::listEnd() const

  Returns the end iterator for the list if the variant's type is
  appropriate, or else a null iterator.
*/

/*! \fn QMapConstIterator<QString, cVariant> cVariant::mapBegin() const

  Returns an iterator to the first item in the map, if the
  variant's type is appropriate, or else a null iterator.
*/

/*! \fn QMapConstIterator<QString, cVariant> cVariant::mapEnd() const

  Returns the end iterator for the map, if the variant's type is
  appropriate, or else a null iterator.
*/

/*! \fn QMapConstIterator<QString, cVariant> cVariant::mapFind( const QString& key ) const

  Returns an iterator to the item in the map with \a key as key, if the
  variant's type is appropriate and \a key is a valid key, or else a
  null iterator.
*/

/*!
  Returns the variant as a QString if the variant has type()
  String, CString, ByteArray, Int, Uint, Bool, Double, Date, Time, or DateTime,
  or QString::null otherwise.

  \sa asString()
*/
const QString cVariant::toString() const
{
    if ( d->typ == CString )
	return QString::fromLatin1( toCString() );
    if ( d->typ == Int )
	return QString::number( toInt() );
    if ( d->typ == UInt )
	return QString::number( toUInt() );
    if ( d->typ == Double )
	return QString::number( toDouble() );
    if ( d->typ == Bool )
	return QString::number( toInt() );
    if ( d->typ == ByteArray )
	return QString( *((QByteArray*)d->value.ptr) );
    if ( d->typ != String )
	return QString::null;
    return *((QString*)d->value.ptr);
}

/*!
  Returns the variant as a QCString if the variant has type()
  CString or String, or a 0 otherwise.

  \sa asCString()
*/
const QCString cVariant::toCString() const
{
    if ( d->typ == CString )
	return *((QCString*)d->value.ptr);
    if ( d->typ == String )
	return ((QString*)d->value.ptr)->latin1();

    return 0;
}


/*!
  Returns the variant as a QStringList if the variant has type()
  StringList or List of a type that can be converted to QString, or an
  empty list otherwise.

  Note that if you want to iterate over the list, you should
  iterate over a copy, e.g.
    \code
    QStringList list = myVariant.toStringList();
    QStringList::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

  \sa asStringList()
*/
const QStringList cVariant::toStringList() const
{
    if ( d->typ == StringList )
	return *((QStringList*)d->value.ptr);
    if ( d->typ == List ) {
	QStringList lst;
	QValueList<cVariant>::ConstIterator it = listBegin();
	QValueList<cVariant>::ConstIterator end = listEnd();
	while( it != end ) {
	    QString tmp = (*it).toString();
	    ++it;
	    lst.append( tmp );
	}
	return lst;
    }

    return QStringList();
}

/*!
  Returns the variant as a QMap<QString,cVariant> if the variant has type()
  Map, or an empty map otherwise.

  Note that if you want to iterate over the list, you should
  iterate over a copy, e.g.
    \code
    QMap<QString, cVariant> list = myVariant.toMap();
    QMap<QString, cVariant>::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

  \sa asMap()
*/
const QMap<QString, cVariant> cVariant::toMap() const
{
    if ( d->typ != Map )
	return QMap<QString,cVariant>();

    return *((QMap<QString,cVariant>*)d->value.ptr);
}

/*!
  Returns the variant as a QByteArray if the variant has type()
  ByteArray, or an empty bytearray otherwise.

  \sa asByteArray()
*/
const QByteArray cVariant::toByteArray() const
{
    if ( d->typ == ByteArray )
	return *((QByteArray*)d->value.ptr);
    return QByteArray();
}

/*!
  Returns the variant as a QBitArray if the variant has type()
  BitArray, or an empty bitarray otherwise.

  \sa asBitArray()
*/
const QBitArray cVariant::toBitArray() const
{
    if ( d->typ == BitArray )
	return *((QBitArray*)d->value.ptr);
    return QBitArray();
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
	return ((QString*)d->value.ptr)->toInt( ok );
    if ( d->typ == CString )
	return ((QCString*)d->value.ptr)->toInt( ok );
    if ( ok )
	*ok = canCast( UInt );
    if( d->typ == Int )
	return d->value.i;
    if( d->typ == UInt )
	return (int)d->value.u;
    if ( d->typ == Double )
	return (int)d->value.d;
    if ( d->typ == Bool )
	return (int)d->value.b;
	if ( d->typ == Character)
	return (int)((P_CHAR)(d->value.ptr))->serial;
	if ( d->typ == Item)
	return (int)((P_ITEM)(d->value.ptr))->serial;
    return 0;
}

/*!
  Returns the variant as an unsigned int if the variant has type()
  String, CString, UInt, Int, Double, or Bool; or 0 otherwise.

  If \a ok is non-null, \a *ok is set to TRUE if the value could be
  converted to a uint and FALSE otherwise.

  \sa asUInt()
*/
uint cVariant::toUInt( bool * ok ) const
{
    if( d->typ == String )
	return ((QString*)d->value.ptr)->toUInt( ok );
    if ( d->typ == CString )
	return ((QCString*)d->value.ptr)->toUInt( ok );
    if ( ok )
	*ok = canCast( UInt );
    if( d->typ == Int )
	return d->value.i;
    if( d->typ == UInt )
	return (int)d->value.u;
    if ( d->typ == Double )
	return (int)d->value.d;
    if ( d->typ == Bool )
	return (int)d->value.b;
	if ( d->typ == Character)
	return (uint)((P_CHAR)(d->value.ptr))->serial;
	if ( d->typ == Item)
	return (uint)((P_ITEM)(d->value.ptr))->serial;

    return 0;
}

/*!  Returns the variant as a bool if the variant has type() Bool.

  Returns TRUE if the variant has type Int, UInt or Double and its value
  is non-zero; otherwise returns FALSE.

  \sa asBool()
*/
bool cVariant::toBool() const
{
    if ( d->typ == Bool )
	return d->value.b;
    if ( d->typ == Double )
	return d->value.d != 0.0;
    if ( d->typ == Int )
	return d->value.i != 0;
    if ( d->typ == UInt )
	return d->value.u != 0;

    return FALSE;
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
    if ( d->typ == CString )
	return ((QCString*)d->value.ptr)->toDouble( ok );
    if ( ok )
	*ok = canCast( Double );
    if ( d->typ == Double )
	return d->value.d;
    if ( d->typ == Int )
	return (double)d->value.i;
    if ( d->typ == Bool )
	return (double)d->value.b;
    if ( d->typ == UInt )
	return (double)d->value.u;
    return 0.0;
}

/*!
  Returns the variant as a QValueList<cVariant> if the variant has type()
  List or StringList, or an empty list otherwise.

  Note that if you want to iterate over the list, you should
  iterate over a copy, e.g.
    \code
    QValueList<cVariant> list = myVariant.toList();
    QValueList<cVariant>::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

  \sa asList()
*/
const QValueList<cVariant> cVariant::toList() const
{
    if ( d->typ == List )
	return *((QValueList<cVariant>*)d->value.ptr);
    if ( d->typ == StringList ) {
	QValueList<cVariant> lst;
	QStringList::ConstIterator it = stringListBegin();
	QStringList::ConstIterator end = stringListEnd();
	for( ; it != end; ++it )
	    lst.append( cVariant( *it ) );
	return lst;
    }
    return QValueList<cVariant>();
}


#define Q_VARIANT_AS( f ) Q##f& cVariant::as##f() { \
   if ( d->typ != f ) *this = cVariant( to##f() ); else detach(); return *((Q##f*)d->value.ptr);}

Q_VARIANT_AS(String)
Q_VARIANT_AS(CString)
Q_VARIANT_AS(StringList)
Q_VARIANT_AS(ByteArray)
Q_VARIANT_AS(BitArray)

/*! \fn QString& cVariant::asString()

  Tries to convert the variant to hold a string value. If that
  is not possible the variant is set to an empty string.

  Returns a reference to the stored string.

  \sa toString()
*/

/*! \fn QCString& cVariant::asCString()

  Tries to convert the variant to hold a string value. If that
  is not possible the variant is set to an empty string.

  Returns a reference to the stored string.

  \sa toCString()
*/

/*! \fn QStringList& cVariant::asStringList()

  Tries to convert the variant to hold a QStringList value. If that
  is not possible the variant is set to an empty string list.

  Returns a reference to the stored string list.

  Note that if you want to iterate over the list, you should
  iterate over a copy, e.g.
    \code
    QStringList list = myVariant.asStringList();
    QStringList::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

  \sa toStringList()
*/

/*! \fn QFont& cVariant::asFont()

  Tries to convert the variant to hold a QFont. If that
  is not possible the variant is set to a default font.

  Returns a reference to the stored font.

  \sa toFont()
*/

/*! \fn QPixmap& cVariant::asPixmap()

  Tries to convert the variant to hold a pixmap value. If that
  is not possible the variant is set to a null pixmap.

  Returns a reference to the stored pixmap.

  \sa toPixmap()
*/

/*! \fn QImage& cVariant::asImage()

  Tries to convert the variant to hold an image value. If that
  is not possible the variant is set to a null image.

  Returns a reference to the stored image.

  \sa toImage()
*/

/*! \fn QBrush& cVariant::asBrush()

  Tries to convert the variant to hold a brush value. If that
  is not possible the variant is set to a default black brush.

  Returns a reference to the stored brush.

  \sa toBrush()
*/

/*! \fn QPoint& cVariant::asPoint()

  Tries to convert the variant to hold a point value. If that
  is not possible the variant is set to a null point.

  Returns a reference to the stored point.

  \sa toPoint()
*/

/*! \fn QRect& cVariant::asRect()

  Tries to convert the variant to hold a rectangle value. If that
  is not possible the variant is set to an empty rectangle.

  Returns a reference to the stored rectangle.

  \sa toRect()
*/

/*! \fn QSize& cVariant::asSize()

  Tries to convert the variant to hold a QSize value. If that
  is not possible the variant is set to an invalid size.

  Returns a reference to the stored size.

  \sa toSize() QSize::isValid()
*/

/*!  \fn QSizePolicy& cVariant::asSizePolicy()

  Tries to convert the variant to hold a QSizePolicy value.  If that
  fails, the variant is set to an arbitrary size policy.
*/


/*! \fn QColor& cVariant::asColor()

  Tries to convert the variant to hold a QColor value. If that
  is not possible the variant is set to an invalid color.

  Returns a reference to the stored color.

  \sa toColor() QColor::isValid()
*/

/*! \fn QPalette& cVariant::asPalette()

  Tries to convert the variant to hold a QPalette value. If that
  is not possible the variant is set to a palette with black colors only.

  Returns a reference to the stored palette.

  \sa toString()
*/

/*! \fn QColorGroup& cVariant::asColorGroup()

  Tries to convert the variant to hold a QColorGroup value. If that
  is not possible the variant is set to a color group with all colors
  set to black.

  Returns a reference to the stored color group.

  \sa toColorGroup()
*/

/*! \fn QIconSet& cVariant::asIconSet()

  Tries to convert the variant to hold a QIconSet value. If that
  is not possible the variant is set to an empty iconset.

  Returns a reference to the stored iconset.

  \sa toIconSet()
*/

/*! \fn QPointArray& cVariant::asPointArray()

  Tries to convert the variant to hold a QPointArray value. If that
  is not possible the variant is set to an empty point array.

  Returns a reference to the stored point array.

  \sa toPointArray()
*/

/*! \fn QBitmap& cVariant::asBitmap()

  Tries to convert the variant to hold a bitmap value. If that
  is not possible the variant is set to a null bitmap.

  Returns a reference to the stored bitmap.

  \sa toBitmap()
*/

/*! \fn QRegion& cVariant::asRegion()

  Tries to convert the variant to hold a QRegion value. If that
  is not possible the variant is set to a null region.

  Returns a reference to the stored region.

  \sa toRegion()
*/

/*! \fn QCursor& cVariant::asCursor()

  Tries to convert the variant to hold a QCursor value. If that
  is not possible the variant is set to a default arrow cursor.

  Returns a reference to the stored cursor.

  \sa toCursor()
*/

/*! \fn QDate& cVariant::asDate()

  Tries to convert the variant to hold a QDate value. If that
  is not possible then the variant is set to an invalid date.

  Returns a reference to the stored date.

  \sa toDate()
*/

/*! \fn QTime& cVariant::asTime()

  Tries to convert the variant to hold a QTime value. If that
  is not possible then the variant is set to an invalid time.

  Returns a reference to the stored time.

  \sa toTime()
*/

/*! \fn QDateTime& cVariant::asDateTime()

  Tries to convert the variant to hold a QDateTime value. If that
  is not possible then the variant is set to an invalid date/time.

  Returns a reference to the stored date/time.

  \sa toDateTime()
*/

/*! \fn QByteArray& cVariant::asByteArray()

  Tries to convert the variant to hold a QByteArray value. If that
  is not possible then the variant is set to an empty bytearray.

  Returns a reference to the stored bytearray.

  \sa toByteArray()
*/

/*! \fn QBitArray& cVariant::asBitArray()

  Tries to convert the variant to hold a QBitArray value. If that
  is not possible then the variant is set to an empty bitarray.

  Returns a reference to the stored bitarray.

  \sa toBitArray()
*/

/*! \fn QKeySequence& cVariant::asKeySequence()

  Tries to convert the variant to hold a QKeySequence value. If that
  is not possible then the variant is set to an empty key sequence.

  Returns a reference to the stored key sequence.

  \sa toKeySequence()
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
  Returns the variant's value as unsigned int reference.
*/
uint& cVariant::asUInt()
{
    detach();
    if ( d->typ != UInt ) {
	uint u = toUInt();
	d->clear();
	d->value.u = u;
	d->typ = UInt;
    }
    return d->value.u;
}

/*!
  Returns the variant's value as bool reference.
*/
bool& cVariant::asBool()
{
    detach();
    if ( d->typ != Bool ) {
	bool b = toBool();
	d->clear();
	d->value.b = b;
	d->typ = Bool;
    }
    return d->value.b;
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

#ifndef QT_NO_TEMPLATE_VARIANT
/*!
  Returns the variant's value as variant list reference.

  Note that if you want to iterate over the list, you should
  iterate over a copy, e.g.
    \code
    QValueList<cVariant> list = myVariant.asList();
    QValueList<cVariant>::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode
*/
QValueList<cVariant>& cVariant::asList()
{
    if ( d->typ != List )
	*this = cVariant( toList() );
    return *((QValueList<cVariant>*)d->value.ptr);
}

/*!
  Returns the variant's value as variant map reference.

  Note that if you want to iterate over the list, you should
  iterate over a copy, e.g.
    \code
    QMap<QString, cVariant> list = myVariant.asMap();
    QMap<QString, cVariant>::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode
*/
QMap<QString, cVariant>& cVariant::asMap()
{
    if ( d->typ != Map )
	*this = cVariant( toMap() );
    return *((QMap<QString,cVariant>*)d->value.ptr);
}
#endif

/*!
  Returns TRUE if the variant's type can be cast to the requested
  type, \a t. Such casting is done automatically when calling the
  toInt(), toBool(), ... or asInt(), asBool(), ... methods.

  The following casts are done automatically:
  \list
  \i Bool => Double, Int, UInt
  \i CString => String
  \i Date => String
  \i DateTime => String, Date, Time
  \i Double => String, Int, Bool, UInt
  \i Int => String, Double, Bool, UInt
  \i List => StringList (if the list contains strings or something
       that can be cast to a string)
  \i String => CString, Int, Uint, Double, Date, Time, DateTime
  \i StringList => List
  \i Time => String
  \i UInt => String, Double, Bool, Int
  \endlist
*/
bool cVariant::canCast( Type t ) const
{
    if ( d->typ == t )
	return TRUE;
    if ( t == Bool && ( d->typ == Double || d->typ == Int || d->typ == UInt ) )
	return TRUE;
    if ( t == Int && ( d->typ == String || d->typ == Double || d->typ == Bool || d->typ == UInt ) )
	return TRUE;
    if ( t == UInt && ( d->typ == String || d->typ == Double || d->typ == Bool || d->typ == Int ) )
	return TRUE;
    if ( t == Double && ( d->typ == String || d->typ == Int || d->typ == Bool || d->typ == UInt ) )
	return TRUE;
    if ( t == CString && d->typ == String )
	return TRUE;
    if ( t == String && ( d->typ == CString || d->typ == Int || d->typ == UInt || d->typ == Double ) )
	return TRUE;
    if ( t == List && d->typ == StringList )
	return TRUE;
    if ( t == StringList && d->typ == List ) {
	QValueList<cVariant>::ConstIterator it = listBegin();
	QValueList<cVariant>::ConstIterator end = listEnd();
	for( ; it != end; ++it ) {
	    if ( !(*it).canCast( String ) )
		return FALSE;
	}
	return TRUE;
    }
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
    case cVariant::Map:
	asMap();
	break;
    case cVariant::List:
	asList();
	break;
    case cVariant::String:
	asString();
	break;
    case cVariant::StringList:
	asStringList();
	break;
    case cVariant::Int:
	asInt();
	break;
    case cVariant::UInt:
	asUInt();
	break;
    case cVariant::Bool:
	asBool();
	break;
    case cVariant::Double:
	asDouble();
	break;
    case cVariant::CString:
	asCString();
	break;
    case cVariant::ByteArray:
	asByteArray();
	break;
    case cVariant::BitArray:
	asBitArray();
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
    case List:
	return v.toList() == toList();
    case Map: {
	if ( v.toMap().count() != toMap().count() )
	    return FALSE;
	QMap<QString, cVariant>::ConstIterator it = v.toMap().begin();
	QMap<QString, cVariant>::ConstIterator it2 = toMap().begin();
	for ( ; it != v.toMap().end(); ++it ) {
	    if ( *it != *it2 )
		return FALSE;
	}
	return TRUE;
    }
    case String:
	return v.toString() == toString();
    case CString:
	return v.toCString() == toCString();
    case StringList:
	return v.toStringList() == toStringList();
    case Int:
	return v.toInt() == toInt();
    case UInt:
	return v.toUInt() == toUInt();
    case Bool:
	return v.toBool() == toBool();
    case Double:
	return v.toDouble() == toDouble();
    case ByteArray:
	return v.toByteArray() == toByteArray();
    case BitArray:
	return v.toBitArray() == toBitArray();
	case Character:
	return v.toCharacter() == toCharacter();
	case Item:
	return v.toItem() == toItem();
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

