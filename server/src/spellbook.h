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

#ifndef __SPELLBOOK_H__
#define __SPELLBOOK_H__

#include "items.h"
#include "qstring.h"

class cSpellBook: public cItem
{
private:
	UINT32 spells1_;
	UINT32 spells2_;

	UINT32 getBitmask( UINT8 spell );
public:
	class cSpellBook() {}
	virtual void Init( bool mkser = true );

	/*!
		Method for adding a spell to this spellbook.
		The id of the spell has to be passed to the 
		script.
	*/
	void addSpell( UINT8 spell );
	void removeSpell( UINT8 spell );
	bool hasSpell( UINT8 spell );
	UINT8 spellCount();

	UINT32 spells1() const;
	UINT32 spells2() const;
	void setSpells1( UINT32 data );
	void setSpells2( UINT32 data );

	// abstract cSerializable
	virtual QString objectID( void ) const { return "SPELLBOOK"; }
	virtual void Serialize( ISerialization &archive );

	// abstract cDefinable
	virtual void processNode( const QDomElement &Tag );

	// Opening a spellbook is done here
	virtual bool onUse( cUObject *Target );
	virtual bool onSingleClick( P_CHAR Viewer );
};

// Inline members
inline void cSpellBook::setSpells1( UINT32 data ) 
{ 
	spells1_ = data;
}

inline void cSpellBook::setSpells2( UINT32 data ) 
{ 
	spells2_ = data;
}

inline UINT32 cSpellBook::spells1() const
{ 
	return spells1_;
}

inline UINT32 cSpellBook::spells2() const
{ 
	return spells2_;
}

#endif
