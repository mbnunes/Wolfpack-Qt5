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

#include "targetrequests.h"
#include "mapstuff.h"
#include "regions.h"
#include "wpdefmanager.h"
#include "items.h"
#include "itemsmgr.h"

void cSetPrivLvlTarget::responsed( UOXSOCKET socket, PKGx6C targetInfo )
{
	if( !isCharSerial( targetInfo.Tserial ) )
		return;

	P_CHAR pc = FindCharBySerial( targetInfo.Tserial );
	if( pc == NULL )
		return;

	pc->setPrivLvl( plevel_ );
	sysmessage( socket, QString("PrivLvl set to : %1").arg(plevel_) );
};

void cAddNpcTarget::responsed( UOXSOCKET socket, PKGx6C targetInfo )
{
	if( targetInfo.TxLoc == -1 || targetInfo.TyLoc == -1 || targetInfo.TzLoc == -1 )
		return;

	QStringList arguments = QStringList::split( " ", npc_ );

	if( arguments.size() == 1 ) // script section string
		Npcs->createScriptNpc( socket, NULL, npc_, targetInfo.TxLoc, targetInfo.TyLoc, targetInfo.TzLoc + Map->TileHeight( targetInfo.model ) );
	else if( arguments.size() == 2 ) // 2 partial hex numbers for art-id ?
	{
		P_CHAR pc = Npcs->MemCharFree ();
		if ( pc == NULL )
			return;
		pc->Init();
		pc->name = "Dummy";
		pc->id1=(unsigned char)arguments[0].toShort();
		pc->id2=(unsigned char)arguments[1].toShort();
		pc->xid = pc->id();
		pc->setSkin(0);
		pc->setXSkin(0);
		pc->setPriv(0x10);
		pc->pos.x=targetInfo.TxLoc;
		pc->pos.y=targetInfo.TyLoc;
		pc->dispz = pc->pos.z = targetInfo.TzLoc + Map->TileHeight(targetInfo.model);
		mapRegions->Add(pc); // add it to da regions ...
		pc->isNpc();
		updatechar(pc);
	}
};

void cBuildMultiTarget::responsed( UOXSOCKET socket, PKGx6C targetInfo )
{
	if( targetInfo.TxLoc == -1 || targetInfo.TyLoc == -1 || targetInfo.TzLoc == -1 )
		return;
	
	QDomElement* DefSection = DefManager->getSection( WPDT_MULTI, multisection_ );
	if( DefSection->isNull() )
		return;

	if( DefSection->attribute( "type" ) == "house" )
	{
		cHouse* pHouse = new cHouse;
		cItemsManager::getInstance()->registerItem( pHouse );
		
		pHouse->build( *DefSection, targetInfo.TxLoc, targetInfo.TyLoc, targetInfo.TzLoc, senderserial_, deedserial_ );
	}
	else if( DefSection->attribute( "type" ) == "boat" )
	{
	}
};


