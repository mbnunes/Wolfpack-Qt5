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

#if !defined(__WPDEFAULTSCRIPT_H_)
#define __WPDEFAULTSCRIPT_H_

#include "typedefs.h"
#include "platform.h"

// Library Includes
#include "qstring.h"
#include "qdom.h"

class cUObject;
class Coord_cl;

class WPDefaultScript  
{
private:
	QString Name; // Important!
public:
	// Scripting Type (i.e. Python or "Default")
	virtual const QString Type( void ) {
		return "default";
	}

	WPDefaultScript() {}
	virtual ~WPDefaultScript() {}

	// Methods for loading, unloading and reloading the scripts
	// these methods are unused for the default script
	// as it basically does nothing
	virtual void load( const QDomElement &Data ) {}
	virtual void unload( void ) {}

	// We need an identification value for the scripts
	void setName( QString Value ) { Name = Value; }
	QString getName( void ) { return Name; }

	// This event will be caled in both directions(!!)
	// That means first onUse for the character is called
	// and afterwards it's called for the item
	// That way you can add special behaviour
	virtual bool onUse( P_CHAR User, P_ITEM Used ) { return false; }

	virtual bool onSingleClick( P_ITEM Item, P_CHAR Viewer ) { return false; }
	virtual bool onSingleClick( P_CHAR Character, P_CHAR Viewer ) { return false; }

	virtual bool onCollideItem( P_CHAR Character, P_ITEM Obstacle ) { return false; }
	virtual bool onCollideChar( P_CHAR Character, P_CHAR Obstacle ) { return false; }
	virtual bool onWalk( P_CHAR Character, UI08 Direction, UI08 onSequence ) { return false; }

	// if this events returns true (handeled) then we should not display the text
	virtual bool onTalk( P_CHAR Character, char speechType, UI16 speechColor, UI16 speechFont, const QString &Text, const QString &Lang ) { return false; }

	virtual bool onTalkToNPC( P_CHAR Talker, P_CHAR Character, const QString &Text ) { return false; }
	virtual bool onTalkToItem( P_CHAR Talker, P_ITEM Item, const QString &Text ) { return false; }

	virtual bool onWarModeToggle( P_CHAR Character, bool War ) { return false; }

	// Is the Client version already known to us here ???
	virtual bool onConnect( UOXSOCKET Socket, const QString &IP, const QString &Username, const QString &Password ) { return false; }

	virtual bool onDisconnect( UOXSOCKET Socket, QString IP ) { return false; }

	virtual bool onEnterWorld( P_CHAR Character ) { return false; }

	virtual bool onHelp( P_CHAR Character ) { return false; }

	virtual bool onChat( P_CHAR Character ) { return false; }

	virtual bool onSkillUse( P_CHAR Character, UI08 Skill ) { return false; }

	virtual bool onContextEntry( P_CHAR pChar, cUObject *pObject, UINT16 id ) { return false; }

	virtual bool onShowContextMenu( P_CHAR pChar, cUObject *pObject ) { return false; }
	
	virtual bool onBeginCast( P_CHAR pMage, UINT8 spell, UINT8 type ) { return false; }
	virtual bool onEndCast( P_CHAR pMage, UINT8 spell, UINT8 type ) { return false; }
	virtual bool onSpellTarget( P_CHAR pMage, UINT8 spell, UINT8 type, cUObject *pObject, const Coord_cl &pos, UINT16 model ) { return false; }
	virtual bool onSpellSuccess( P_CHAR pMage, UINT8 spell, UINT8 type, cUObject *pObject, const Coord_cl &pos, UINT16 model ) { return false; }
	virtual bool onSpellFailure( P_CHAR pMage, UINT8 spell, UINT8 type, cUObject *pObject, const Coord_cl &pos, UINT16 model ) { return false; }
};

#endif
