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

#ifndef __WPPYTHONSCRIPT_H__
#define __WPPYTHONSCRIPT_H__

#include "../wpdefaultscript.h"
#include "Python.h"

class cUORxTarget;

class WPPythonScript : public WPDefaultScript  
{
private:
	PyObject *codeModule; // This object stores the compiled Python Module

public:
	virtual const QString Type( void ) {
		return "python";
	};
	
	virtual ~WPPythonScript() {};

	virtual void load( const QDomElement &Data );
	virtual void unload( void );

	// Normal Events
	bool onServerstart();

	bool onUse( P_CHAR User, P_ITEM Used );
	bool onSingleClick( P_ITEM Item, P_CHAR Viewer );
	bool onSingleClick( P_CHAR Character, P_CHAR Viewer );

	bool onCollideItem( P_CHAR Character, P_ITEM Obstacle );
	bool onCollideChar( P_CHAR Character, P_CHAR Obstacle );
	bool onWalk( P_CHAR Character, UI08 Direction, UI08 Sequence );
	bool onCreate( cUObject *object, const QString &definition );

	// if this events returns true (handeled) then we should not display the text
	bool onTalk( P_CHAR Character, char speechType, UI16 speechColor, UI16 speechFont, const QString &Text, const QString &Lang );
	bool onWarModeToggle( P_CHAR Character, bool War );
	bool onLogin( P_CHAR pChar );
	bool onLogout( P_CHAR pChar );
	bool onHelp( P_CHAR Character );
	bool onChat( P_CHAR Character );
	bool onSkillUse( P_CHAR Character, UI08 Skill );
	bool onShowPaperdoll( P_CHAR pChar, P_CHAR pOrigin );

	bool onContextEntry( P_CHAR pChar, cUObject *pObject, UINT16 id );
	bool onShowContextMenu( P_CHAR pChar, cUObject *pObject );

	bool onShowToolTip( P_CHAR pChar, cUObject *pObject, cUOTxTooltipList* tooltip );

	bool onSpeech( cUObject *listener, P_CHAR talker, const QString &text, const QValueVector< UINT16 >& keywords );

	// Drop/Pickup events
	bool onDropOnChar( P_CHAR pChar, P_ITEM pItem );
	bool onDropOnItem( P_ITEM pCont, P_ITEM pItem );
	bool onDropOnGround( P_ITEM pItem, const Coord_cl &pos );
	bool onPickup( P_CHAR pChar, P_ITEM pItem );

	bool onCommand( cUOSocket *socket, const QString &name, const QString &args );

	// Magic System (This should be reduced eventually. It's a bit much)
	// But as soon as the flag-system is introduced for python-script 
	// It shouldn't be that much.
	bool onBeginCast( P_CHAR pMage, UINT8 spell, UINT8 type );
	bool onEndCast( P_CHAR pMage, UINT8 spell, UINT8 type );
	bool onSpellCheckTarget( P_CHAR pMage, UINT8 spell, UINT8 type, cUORxTarget *target );
	bool onSpellSuccess( P_CHAR pMage, UINT8 spell, UINT8 type, cUORxTarget* );
	bool onSpellFailure( P_CHAR pMage, UINT8 spell, UINT8 type, cUORxTarget* );
};

#endif // __WPPYTHONSCRIPT_H__
