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

#if !defined(__CORPSE_H__)
#define __CORPSE_H__

#include "items.h"

// Library Includes
#include <qstring.h>

// System Includes
#include <qmap.h>

// Forward declarations
class cUOSocket;

class cCorpse : public cItem
{
private:
	static void buildSqlString( QStringList& fields, QStringList& tables, QStringList& conditions );
	bool changed_;
	static unsigned char classid;

protected:
	UINT16 bodyId_; // Body id of the corpse
	UINT16 hairStyle_; // Style of the hair
	UINT16 hairColor_; // Color of the hair
	UINT16 beardStyle_; // Beardstyle
	UINT16 beardColor_; // Color of the beard
	unsigned char direction_; // Direction the corpse is facing.
	uint murdertime_; // When the people has been killed
	SERIAL murderer_; // Who was the murderer
	QCString charbaseid_;

	QMap<UINT8, SERIAL> equipment_; // Serials of the old equipment
	// The meaning of this is that even if the items are inside of the corpse
	// they're displayed as equipment
public:
	unsigned char getClassid()
	{
		return cCorpse::classid;
	}

	cCorpse( bool init = false );
	void setBodyId( UINT16 data );
	void setHairStyle( UINT16 data );
	void setHairColor( UINT16 data );
	void setBeardStyle( UINT16 data );
	void setBeardColor( UINT16 data );
	void setMurderer( SERIAL data );
	void setMurderTime( uint data );

	void setDirection( unsigned char data );
	void setCharBaseid( const QCString& data );
	bool corpse() const
	{
		return true;
	}

	unsigned int decayDelay();

	UINT16 bodyId() const;
	UINT16 hairStyle() const;
	UINT16 hairColor() const;
	UINT16 beardStyle() const;
	UINT16 beardColor() const;
	unsigned char direction() const;
	const QCString& charBaseid() const;
	SERIAL murderer() const;
	unsigned int murderTime() const;

	void addEquipment( UINT8 layer, SERIAL serial );
	SERIAL getEquipment( UINT8 layer );

	const char* objectID() const
	{
		return "cCorpse";
	}

	virtual void flagUnchanged()
	{
		cCorpse::changed_ = false; cItem::flagUnchanged();
	}

	// DB Serialization
	static void registerInFactory();
	void load( char**, UINT16& );
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
inline void cCorpse::setBodyId( UINT16 data )
{
	bodyId_ = data; changed_ = true;
}

inline void cCorpse::setHairStyle( UINT16 data )
{
	hairStyle_ = data; changed_ = true;
}

inline void cCorpse::setHairColor( UINT16 data )
{
	hairColor_ = data; changed_ = true;
}

inline void cCorpse::setBeardStyle( UINT16 data )
{
	beardStyle_ = data; changed_ = true;
}

inline void cCorpse::setBeardColor( UINT16 data )
{
	beardColor_ = data; changed_ = true;
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

inline void cCorpse::setCharBaseid( const QCString& baseid )
{
	charbaseid_ = baseid;
}

inline UINT16 cCorpse::bodyId() const
{
	return bodyId_;
}

inline UINT16 cCorpse::hairStyle() const
{
	return hairStyle_;
}

inline UINT16 cCorpse::hairColor() const
{
	return hairColor_;
}

inline UINT16 cCorpse::beardStyle() const
{
	return beardStyle_;
}

inline UINT16 cCorpse::beardColor() const
{
	return beardColor_;
}

inline const QCString& cCorpse::charBaseid() const
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
