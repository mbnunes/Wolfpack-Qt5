/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2005 by holders identified in AUTHORS.txt
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

#if !defined(__CORPSE_H__)
#define __CORPSE_H__

#include "items.h"

// Library Includes
#include <qstring.h>

// System Includes
#include <qmap.h>
//Added by qt3to4:
#include <Q3CString>

// Forward declarations
class cUOSocket;

class cCorpse : public cItem
{
private:
	bool changed_;
	static unsigned char classid;

protected:
	Q_UINT16 bodyId_; // Body id of the corpse
	unsigned char direction_; // Direction the corpse is facing.
	uint murdertime_; // When the people has been killed
	SERIAL murderer_; // Who was the murderer
	Q3CString charbaseid_;

	QMap<Q_UINT8, SERIAL> equipment_; // Serials of the old equipment
	// The meaning of this is that even if the items are inside of the corpse
	// they're displayed as equipment
public:
	static void buildSqlString( const char* objectid, QStringList& fields, QStringList& tables, QStringList& conditions );

	unsigned char getClassid()
	{
		return cCorpse::classid;
	}

	static void setClassid( unsigned char id )
	{
		cCorpse::classid = id;
	}

	cCorpse( bool init = false );
	void setBodyId( Q_UINT16 data );
	void setMurderer( SERIAL data );
	void setMurderTime( uint data );

	void setDirection( unsigned char data );
	void setCharBaseid( const Q3CString& data );
	bool corpse() const
	{
		return true;
	}

	unsigned int decayDelay();

	Q_UINT16 bodyId() const;
	unsigned char direction() const;
	const Q3CString& charBaseid() const;
	SERIAL murderer() const;
	unsigned int murderTime() const;

	void addEquipment( Q_UINT8 layer, SERIAL serial );
	SERIAL getEquipment( Q_UINT8 layer );

	const char* objectID() const
	{
		return "cCorpse";
	}

	virtual void flagUnchanged()
	{
		cCorpse::changed_ = false; cItem::flagUnchanged();
	}

	// DB Serialization
	void load( char**, Q_UINT16& );
	void save();
	bool del();
	void load( cBufferedReader& reader, unsigned int version );
	void save( cBufferedWriter& reader, unsigned int version );

	// abstract cDefinable
	virtual void processNode( const cElement* Tag );

	// override update
	virtual void update( cUOSocket* mSock = 0 );

	virtual stError* setProperty( const QString& name, const cVariant& value );
	PyObject* getProperty( const QString& name );
	void createTooltip( cUOTxTooltipList& tooltip, cPlayer* player );
};

// Inline members
inline void cCorpse::setBodyId( Q_UINT16 data )
{
	bodyId_ = data; changed_ = true;
}

inline void cCorpse::setMurderer( SERIAL data )
{
	murderer_ = data; changed_ = true;
}

inline void cCorpse::setMurderTime( uint data )
{
	murdertime_ = data; changed_ = true;
}

inline void cCorpse::setDirection( unsigned char data )
{
	direction_ = data;
}

inline void cCorpse::setCharBaseid( const Q3CString& baseid )
{
	charbaseid_ = baseid;
}

inline Q_UINT16 cCorpse::bodyId() const
{
	return bodyId_;
}

inline const Q3CString& cCorpse::charBaseid() const
{
	return charbaseid_;
}

inline unsigned char cCorpse::direction() const
{
	return direction_;
}

inline SERIAL cCorpse::murderer() const
{
	return murderer_;
}

inline unsigned int cCorpse::murderTime() const
{
	return murdertime_;
}

#endif // __CORPSE_H__
