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
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#if !defined (__UOBJECT_H__)
#define __UOBJECT_H__

#include "exceptions.h"
#include "platform.h"
#include "typedefs.h"
#include "coord.h"
#include "persistentobject.h"
#include "definable.h"
#include "singleton.h"
#include "customtags.h"
#include "factory.h"
#include "spawnregions.h"
#include "pythonscript.h"
#include "world.h"

// System includes
#include <vector>
#include <map>

// Library includes
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

// Forward class declarations
class Coord;
class cPythonScript;
class cUOSocket;
class QSqlQuery;
class cItem;
class cUOTxTooltipList;
class cMulti;

class cBufferedReader;
class cBufferedWriter;

#pragma pack(1)
class cUObject : public cDefinable, public cPythonScriptable, public PersistentObject
{
protected:
	Coord pos_;
	SERIAL serial_;
	cMulti* multi_; // If we're in a Multi

public:
	// Indicates whether the object was deleted already.
	bool free : 1;

	const char* objectID() const
	{
		return "cUObject";
	}

	// Tag Management Methods
	const cVariant& getTag( const QString& key ) const;
	bool hasTag( const QString& key ) const;
	void setTag( const QString& key, const cVariant& value );
	void removeTag( const QString& key );
	void clearTags();
	QStringList getTags() const;

	// Event Management Methods
	void clearScripts();
	void addScript( cPythonScript* script, bool append = false );
	void removeScript( const QCString& name );
	virtual bool hasScript( const QCString& name );

	void freezeScriptChain();
	void unfreezeScriptChain();
	bool isScriptChainFrozen();
	void setScriptList( const QCString& scripts );
	QCString scriptList() const;
	inline cPythonScript** getScripts()
	{
		return scriptChain;
	}

	// Serialization Methods
	void load( char**, Q_UINT16& );
	void save();
	bool del();

	// Wrapper
	void load( cBufferedReader& reader ) = 0;
	void save( cBufferedWriter& reader );

	// "Real" ones
	void load( cBufferedReader& reader, unsigned int version );
	void save( cBufferedWriter& reader, unsigned int version );
	void postload( unsigned int version ) = 0;

	// Utility Methods
	void effect( Q_UINT16 id, Q_UINT8 speed = 10, Q_UINT8 duration = 5, Q_UINT16 hue = 0, Q_UINT16 renderMode = 0 ); // Moving with this character
	void effect( Q_UINT16 id, cUObject* target, bool fixedDirection = true, bool explodes = false, Q_UINT8 speed = 10, Q_UINT16 hue = 0, Q_UINT16 renderMode = 0 );
	void effect( Q_UINT16 id, const Coord& target, bool fixedDirection = true, bool explodes = false, Q_UINT8 speed = 10, Q_UINT16 hue = 0, Q_UINT16 renderMode = 0 );
	void lightning( unsigned short hue = 0 );
	bool inRange( cUObject* object, Q_UINT32 range ) const;
	void removeFromView( bool clean = true );
	virtual void sendTooltip( cUOSocket* mSock );
	bool isItem() const
	{
		return isItemSerial( serial_ );
	}
	bool isChar() const
	{
		return isCharSerial( serial_ );
	}
	virtual void talk( const QString& message, UI16 color = 0xFFFF, Q_UINT8 type = 0, bool autospam = false, cUOSocket* socket = NULL ) = 0;
	virtual void flagUnchanged()
	{
		changed_ = false;
	}
	void resendTooltip();
	unsigned char direction( cUObject* );
	virtual void remove();
	virtual void moveTo( const Coord& );
	unsigned int dist( cUObject* d ) const;

	// Event Methods
	virtual bool onCreate( const QString& definition );
	virtual bool onShowTooltip( P_PLAYER sender, cUOTxTooltipList* tooltip ); // Shows a tool tip for specific object
	virtual void createTooltip( cUOTxTooltipList& tooltip, cPlayer* player );

	// Constructors And Destructors
	cUObject();
	cUObject( const cUObject& );
	virtual ~cUObject();

	// Getter Methods
	virtual QCString bindmenu() = 0;
	virtual unsigned char getClassid() = 0;

	QString name() const
	{
		return name_;
	}
	const Coord& pos() const
	{
		return pos_;
	}
	SERIAL serial() const
	{
		return serial_;
	}
	Q_UINT32 getTooltip() const
	{
		return tooltip_;
	}
	inline cMulti* multi() const
	{
		return multi_;
	}

	inline cSpawnRegion* spawnregion() const
	{
		return spawnregion_;
	}

	// Setter Methods
	void setName( const QString& d )
	{
		name_ = d; changed_ = true; changed( TOOLTIP );
	}

	virtual void setSerial( SERIAL d )
	{
		serial_ = d; changed_ = true;
	}

	void setTooltip( const Q_UINT32 d )
	{
		tooltip_ = d;
	}
	inline void setMulti( cMulti* multi )
	{
		multi_ = multi; changed_ = true;
	}

	void setSpawnregion( cSpawnRegion* spawnregion );

	// Definable Methods
	void processNode( const cElement* Tag );
	stError* setProperty( const QString& name, const cVariant& value );
	PyObject* getProperty( const QString& name );

	// Call an event handler for this object and take both the normal
	// and the base script chain into account. This will also call the
	// global handler.
	virtual PyObject* callEvent( ePythonEvent event, PyObject* args = 0, bool ignoreErrors = false ) = 0;

	// Call a python event handler and return true if any of the
	// events in the call chain returns an object that evaluates
	// to true.
	virtual bool callEventHandler( ePythonEvent event, PyObject* args = 0, bool ignoreErrors = false ) = 0;

	// Check if any of the scripts assigned to this object can handle the given event,
	// this returns true even if there is a global handler for the event.
	virtual bool canHandleEvent( ePythonEvent event ) = 0;
private:
	uchar changed_ : 1;
protected:
	cCustomTags tags_;
	uint tooltip_;
	QString name_;
	cPythonScript** scriptChain; // NULL Terminated Array
	cSpawnRegion *spawnregion_;
	// Things for building the SQL string
	static void buildSqlString( const char* objectid, QStringList& fields, QStringList& tables, QStringList& conditions );

	enum eChanged
	{
		//SAVE = 1,
		TOOLTIP				= 2,
		UNUSED				= 4,
		UNUSED2				= 8
	};
	void changed( uint );
};
#pragma pack()

#endif // __UOBJECT_H__
