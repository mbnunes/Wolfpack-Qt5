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

#ifndef __WPPYTHONSCRIPT_H__
#define __WPPYTHONSCRIPT_H__

#include "../wpdefaultscript.h"
#include "Python.h"

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
	virtual bool onUse( P_CHAR User, P_ITEM Used );

	virtual bool onShowItemName( P_ITEM Item, P_CHAR Viewer );
	virtual bool onShowCharName( P_CHAR Character, P_CHAR Viewer );

	virtual bool onCollideItem( P_CHAR Character, P_ITEM Obstacle );
	virtual bool onCollideChar( P_CHAR Character, P_CHAR Obstacle );
	virtual bool onWalk( P_CHAR Character, UI08 Direction, UI08 Sequence );

	// if this events returns true (handeled) then we should not display the text
	virtual bool onTalk( P_CHAR Character, char speechType, UI16 speechColor, UI16 speechFont, const QString &Text, const QString &Lang );
	virtual bool onTalkToNPC( P_CHAR Talker, P_CHAR Character, const QString &Text );
	virtual bool onTalkToItem( P_CHAR Talker, P_ITEM Item, const QString &Text );
	virtual bool onWarModeToggle( P_CHAR Character, bool War );

	// Is the Client version already known to us here ???
	//virtual bool onConnect( cUOSocket *Socket, const QString &IP, const QString &Username, const QString &Password );
	//virtual bool onDisconnect( cUOSocket *Socket, QString IP );
	//virtual bool onEnterWorld( P_CHAR Character );
	virtual bool onHelp( P_CHAR Character );
	virtual bool onChat( P_CHAR Character );
	virtual bool onSkillUse( P_CHAR Character, UI08 Skill );
};

#endif // __WPPYTHONSCRIPT_H__
