//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
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

#if !defined (__UOBJECT_H__)
#define __UOBJECT_H__

#include "platform.h"
#include "typedefs.h"
#include "coord.h"
#include "persistentobject.h"
#include "definable.h"
#include "singleton.h"
#include "customtags.h"
#include "factory.h"

// System includes
#include <vector>
#include <map>

// Library includes
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

// Forward class declarations
class Coord_cl;
class cPythonScript;
class cUOSocket;
class QSqlQuery;
class cItem;
class cUOTxTooltipList;

struct stError;

class cUObject : public PersistentObject, public cDefinable
{
private:
	uchar changed_:1;
	cCustomTags tags_;

protected:
	QString bindmenu_;
	uint tooltip_;
	QString name_;
	Coord_cl pos_;
	SERIAL serial_;
	SERIAL multis_;	
	uchar dir_:3;

	// Things for building the SQL string
	static void buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions );
	void changed( uint );
	
	enum eChanged // Each bit controls different state
	{
//		SAVE = 1,
		TOOLTIP = 2,
		UNUSED = 4,
		UNUSED2 = 8
	};

public:
	const char *objectID() const
	{
        return "cUObject";
	}

	// EventHandling functions
	cPythonScript **getEvents()
	{
		return scriptChain; 
	}

	// Forwarder for the tags
	const cVariant &getTag( const QString& key ) const;
	bool hasTag( const QString& key ) const;
	void setTag( const QString& key, const cVariant& value );
	void removeTag( const QString& key );
	void clearTags();
	QStringList getTags() const;

	void clearEvents();
	void addEvent( cPythonScript *Event );
	void removeEvent( const QString& Name );
	bool hasEvent( const QString& Name ) const;
	void recreateEvents();
	
	// Returns the list of events
	const QString &eventList() const
	{
		return eventList_; 
	}	
	
	// New Load (Query: result, offset: current field offset)
	void load( char **, UINT16& );
	void save();
	bool del();

	bool inRange( cUObject *object, UINT32 range ) const;
	void removeFromView( bool clean = true );

	// Multiple quick-effect methods
	void effect( UINT16 id, UINT8 speed = 10, UINT8 duration = 5, UINT16 hue = 0, UINT16 renderMode = 0 ); // Moving with this character
	void effect( UINT16 id, cUObject *target, bool fixedDirection = true, bool explodes = false, UINT8 speed = 10, UINT16 hue = 0, UINT16 renderMode = 0 );
	void effect( UINT16 id, const Coord_cl &target, bool fixedDirection = true, bool explodes = false, UINT8 speed = 10, UINT16 hue = 0, UINT16 renderMode = 0 );
	void lightning( unsigned short hue = 0 );

	// Events
	virtual bool onCreate( const QString &definition );
	virtual void createTooltip(cUOTxTooltipList &tooltip, cPlayer *player);
	
	bool free;

// Methods
public:
	cUObject();
	cUObject( cUObject& ); // Copy constructor
	virtual ~cUObject() {};

	void moveTo( const Coord_cl&, bool noRemove = false );
	unsigned int	dist(cUObject* d) const;
	QString			bindmenu()		const { return bindmenu_; }
	QString			name()			const { return name_;		}
	Coord_cl		pos()			const { return pos_;		}
	SERIAL			serial()		const { return serial_;	}
	SERIAL			multis()		const { return multis_;	}
	UINT32			getTooltip()	const { return tooltip_; }
	uchar			direction()		const { return dir_;  }

	void setBindmenu( const QString& d )	{ bindmenu_ = d; changed_ = true;	}
	void setName( const QString& d )		{ name_ = d; changed_ = true; changed( TOOLTIP );		}	
	void setPos( const Coord_cl& d )		{ pos_ = d;	changed_ = true;		}
	void setMultis( const SERIAL d )		{ multis_ = d; changed_ = true;		}
	virtual void setSerial( SERIAL d )		{ serial_ = d; changed_ = true;	}
	void setTooltip( const UINT32 d )		{ tooltip_ = d; }
	void	setDirection( uchar d )			{ dir_ = d; changed_ = true;}

	virtual void sendTooltip( cUOSocket* mSock );

	bool isItem() { return (serial_ != INVALID_SERIAL && serial_ > 0 && serial_ >= 0x40000000); }
	bool isChar() { return (serial_ != INVALID_SERIAL && serial_ > 0 && serial_ <  0x40000000); }

	void processNode( const cElement *Tag );

	virtual void talk( const QString &message, UI16 color = 0xFFFF, UINT8 type = 0, bool autospam = false, cUOSocket* socket = NULL ) = 0;
	virtual stError *setProperty( const QString &name, const cVariant &value );
	virtual stError *getProperty( const QString &name, cVariant &value ) const;
	virtual void flagUnchanged() { changed_ = false; }
	void resendTooltip();

	char direction( cUObject* ) const;

protected:
	cPythonScript **scriptChain;	// NULL Terminated Array
	QString eventList_; // Important for recreating the scriptChain on reloading

	void init();
};


class cUObjectFactory : public Factory<cUObject, QString>
{
public:
	void registerSqlQuery( const QString &type, const QString &query )
	{
		sql_queries.insert( std::make_pair( type, query ) );
		sql_keys.push_back( type );
	}

	QString findSqlQuery( const QString &type ) const
	{
		std::map< QString, QString >::const_iterator iter = sql_queries.find( type );

		if( iter == sql_queries.end() )
			return QString::null;
		else
			return iter->second;
	}

	QStringList objectTypes() const
	{
		return sql_keys;
	}

private:
	std::map< QString, QString > sql_queries;
	QStringList sql_keys;
};

typedef SingletonHolder< cUObjectFactory > UObjectFactory;

struct stError 
{
	INT8 code;
	QString text;
};

#define PROPERTY_ERROR( errno, data ) { stError *errRet = new stError; errRet->code = errno; errRet->text = data; return errRet; }

#define GET_PROPERTY( id, getter ) if( name == id ) {\
	value = cVariant( getter ); \
	return 0; \
}

#define SET_STR_PROPERTY( id, setter ) if( name == id ) {\
	QString text = value.toString(); \
	if( text == QString::null )	\
		PROPERTY_ERROR( -2, "String expected" ) \
	setter = text; \
	return 0; \
	}

#define SET_INT_PROPERTY( id, setter ) if( name == id ) {\
	bool ok; \
	INT32 data = value.toInt( &ok ); \
	if( !ok ) \
		PROPERTY_ERROR( -2, "Integer expected" ) \
	setter = data; \
	return 0; \
	}

#define SET_FLOAT_PROPERTY( id, setter ) if( name == id ) {\
	bool ok; \
	float data = static_cast<QString>( value.toString() ).toFloat( &ok ); \
	if( !ok ) \
		PROPERTY_ERROR( -2, "Float expected" ) \
	setter = data; \
	return 0; \
	}

#define SET_BOOL_PROPERTY( id, setter ) if( name == id ) {\
	bool ok; \
	INT32 data = value.toInt( &ok ); \
	if( !ok ) \
		PROPERTY_ERROR( -2, "Boolean expected" ) \
	setter = data == 0 ? false : true; \
	return 0; \
	}

#define SET_CHAR_PROPERTY( id, setter ) if( name == id ) {\
	setter = value.toChar(); \
	return 0; \
	}

#define SET_ITEM_PROPERTY( id, setter ) if( name == id ) {\
	setter = value.toItem(); \
	return 0; \
	}

#endif // __UOBJECT_H__
