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

#include "wpdefaultscript.h"

void WPDefaultScript::load( const QDomElement &Data )
{
	return;
}

void WPDefaultScript::unload( void )
{
	return;
}

void WPDefaultScript::reload( void )
{
	return;
}

void WPDefaultScript::setName( QString Value )
{
	Name = Value;
}

QString WPDefaultScript::getName( void )
{
	return Name;
}

bool WPDefaultScript::onUse( P_CHAR User, P_ITEM Used )
{
	return false;
}

// Two Versions
bool WPDefaultScript::onShowItemName( P_ITEM Item, P_CHAR Viewer )
{
	return false;
}

bool WPDefaultScript::onShowCharName( P_CHAR Character, P_CHAR Viewer )
{
	return false;
}

void WPDefaultScript::onCollideItem( P_CHAR Character, P_ITEM Obstacle )
{
}

void WPDefaultScript::onCollideChar( P_CHAR Character, P_CHAR Obstacle )
{
}

bool WPDefaultScript::onWalk( P_CHAR Character, UI08 Direction, UI08 onSequence )
{
	return true;
}


// if this events returns true (handeled) then we should not display the text
bool WPDefaultScript::onTalk( P_CHAR Character, QString Text )
{
	return true;
}


void WPDefaultScript::onTalkToNPC( P_CHAR Talker, P_CHAR Character, const QString &Text )
{
}

void WPDefaultScript::onTalkToItem( P_CHAR Talker, P_ITEM Item, const QString &Text )
{
}


void WPDefaultScript::onWarModeToggle( P_CHAR Character, bool War )
{
}


// Is the Client version already known to us here ???
bool WPDefaultScript::onConnect( UOXSOCKET Socket, const QString &IP, const QString &Username, const QString &Password )
{
	return false;
}


bool WPDefaultScript::onDisconnect( UOXSOCKET Socket, QString IP )
{
	return false;
}


void WPDefaultScript::onEnterWorld( P_CHAR Character )
{
}


bool WPDefaultScript::onHelp( P_CHAR Character )
{
	return false;
}


bool WPDefaultScript::onChat( P_CHAR Character )
{
	return false;
}


bool WPDefaultScript::onSkillUse( P_CHAR Character, UI08 Skill )
{
	return false;
}
