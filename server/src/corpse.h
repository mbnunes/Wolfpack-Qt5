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
//	Wolfpack Homepage: http://www.wpdev.sf.net/
//========================================================================================

#ifndef __CORPSE_H__
#define __CORPSE_H__

#include "items.h"

class cUOSocket;

class cCorpse: public cItem
{
private:
	UINT16 bodyId_; // Body id of the corpse
	UINT16 hairStyle_; // Style of the hair
	UINT16 hairColor_; // Color of the hair
	UINT16 beardStyle_; // Beardstyle
	UINT16 beardColor_; // Color of the beard
	map< UINT8, SERIAL > equipment_; // Serials of the old equipment
	// The meaning of this is that even if the items are inside of the corpse
	// they're displayed as equipment
public:
	cCorpse();

	void setBodyId( UINT16 data ) { bodyId_ = data; }
	void setHairStyle( UINT16 data ) { hairStyle_ = data; }
	void setHairColor( UINT16 data ) { hairColor_ = data; }
	void setBeardStyle( UINT16 data ) { beardStyle_ = data; }
	void setBeardColor( UINT16 data ) { beardColor_ = data; }

	UINT16 bodyId() { return bodyId_; }
	UINT16 hairStyle() { return hairStyle_; }
	UINT16 hairColor() { return hairColor_; }
	UINT16 beardStyle() { return beardStyle_; }
	UINT16 beardColor() { return beardColor_; }

	void addEquipment( UINT8 layer, SERIAL serial );

	// abstract cSerializable
	virtual QString objectID( void ) const { return "CORPSE"; }
	virtual void Serialize( ISerialization &archive );

	// abstract cDefinable
	virtual void processNode( const QDomElement &Tag );

	// override update
	virtual void update( cUOSocket *mSock = NULL );
};

#endif
