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


#include "wolfpack.h"
#include "im.h"
#include "prototypes.h"
#include "globals.h"
#include "SndPkg.h"
#include "gumps.h"
#include "guildstones.h"
#include "srvparams.h"
#include "menuactions.h"
#include "wpdefmanager.h"
#include "classes.h"
#include "network.h"
#include "scriptc.h"
#include "network/uosocket.h"
#include "gumpsmgr.h"
#include "spawnregions.h"

#include "debug.h"

#undef  DBGFILE
#define DBGFILE "gumps.cpp"

void entrygump(int s, SERIAL serial, unsigned char type, char index, short int maxlength, char *text1)
{
	short int length;
	char textentry1[12]="\xAB\x01\x02\x01\x02\x03\x04\x00\x01\x12\x34";
	char textentry2[9]="\x01\x01\x00\x00\x12\x34\x12\x34";
	
	sprintf((char*)temp, "(%i chars max)", maxlength);
	length=11+strlen((char*)text1)+1+8+strlen((char*)temp)+1;
	textentry1[1]=length>>8;
	textentry1[2]=length%256;
	LongToCharPtr(serial, (unsigned char*)textentry1+3);
	textentry1[7]=type;
	textentry1[8]=index;
	textentry1[9]=(strlen(text1)+1)>>8;
	textentry1[10]=(strlen(text1)+1)%256;
	Xsend(s, textentry1, 11);
	Xsend(s, text1, strlen(text1)+1);
	textentry2[4]=maxlength>>8;
	textentry2[5]=maxlength%256;
	textentry2[6]=(strlen((char*)temp)+1)>>8;
	textentry2[7]=(strlen((char*)temp)+1)%256;
	Xsend(s, textentry2, 8);
	Xsend(s, temp, strlen((char*)temp)+1);
}

// Parses an Action node
bool DisposeAction( UOXSOCKET Socket, QDomElement Action )
{
	if( Action.nodeName() == "useitem" )
		return useItem( Socket, Action );

	else if( Action.nodeName() == "hasitem" )
		return hasItem( Socket, Action );

	else if( Action.nodeName() == "makeitem" )
		return makeItem( Socket, Action );

	else if( Action.nodeName() == "checkskill" )
		return checkSkill( Socket, Action );

	else if( Action.nodeName() == "serverstatus" )
		return serverStatus( Socket, Action );

	return true;
}

// REWRITTEN BY DARKSTORM
// \x7D, this packet is the reply
// to the craft-menus
void MenuChoice( UOXSOCKET Socket ) 
{ 
	P_CHAR pc_currchar = currchar[ Socket ];

	UI16 Menu = ( buffer[ Socket ][5] << 8 ) + buffer[ Socket ][6];
	UI16 Choice = ( buffer[ Socket ][7] << 8 ) + buffer[ Socket ][8];

	// Guildstone Menus
	// Maybe we should change that to 0xFE so we can maintain a given style
	if ( (UI08)( Menu >> 8 ) == 0xFE )
	{
		// Get the Guildstone item
		cGuildStone* pStone = dynamic_cast<cGuildStone*>( FindItemBySerial( pc_currchar->guildstone() ) );
		
		if ( pStone != NULL )
			pStone->GumpChoice( Socket, Menu, Choice );
		// ok, ok, not too much sphere ;)
		// else
		//	sysmessage( s, "Unexpected target info" );

		// If it's a Guildstone Menu i dont think we need to process anyhing else
		return;
	}

	// we have nothing to do
	if( Choice == 0 )
		return;

	// Retrieve our menu
	Choice--;
	QString MenuID;
	MenuID.sprintf( "%i", Menu );

	QDomElement *MenuNode = DefManager->getSection( WPDT_MENU, MenuID );

	if( MenuNode->isNull() )
	{
		sysmessage( Socket, "No such menu: %i", Menu );
		return;
	}

	// Get the Entry which was selected
	if( Choice+1 > MenuNode->childNodes().count() ) 
	{
		sysmessage( Socket, "Invalid Choice (%i) selected for Menu %i", Choice, Menu );
		return;
	}
	
	QDomElement Entry = MenuNode->childNodes().item( Choice ).toElement();

	if( Entry.isNull() )
	{
		sysmessage( Socket, "Invalid Choice (%i) selected for Menu %i", Choice, Menu );
		return;
	}

	// Simple processing: Enter a new Submenu
	if( Entry.nodeName() == "submenu" )
	{
		if( !Entry.attributes().contains( "id" ) )
		{
			sysmessage( Socket, "Invalid Submenu found in Menu %i", Menu );
			return;
		}

		QString SubMenu = Entry.attributeNode( "id" ).nodeValue();
		ShowMenu( Socket, SubMenu.toUShort() );
		return;
	}

	// We now have only one choice left (actions)
	if( Entry.nodeName() != "actions" )
	{
		sysmessage( Socket, "Invalid Entry (%s) selected in Menu %i", Entry.nodeName().latin1(), Menu );
		return;
	}

	// Walk all required actions
	// if one of them fails then quit the
	// action-chain
	for( UI32 i = 0; i < Entry.childNodes().count(); i++ )
	{
		if( !Entry.childNodes().item( i ).isElement() )
			continue;

		QDomElement Action = Entry.childNodes().item( i ).toElement();

		if( Action.isNull() )
			continue;

		if( !DisposeAction( Socket, Action ) )
			return;
	}

	// DO WE NEED THAT ANYMORE !?!
	// A maximum of 256 Item-menus ->
	// 0xFFxx -> Ss
	/*if( (UI08)( Menu >> 8 ) == 0xFF )
	{
		if ( im_choice( s, Menu, Choice ) == 0 ) 
			return;
	}*/

	// Polymorph:
	// Magic->Polymorph(s,POLYMORPHMENUOFFSET,sub); 
	
	// Potions:
	// Skills->DoPotion(s, main-1246, sub, FindItemBySerial(calcserial(addid1[s], addid2[s], addid3[s], addid4[s])));

	// Tracking:
	//	if(!Skills->CheckSkill(currchar[s], TRACKING, 0, 1000))
	// {
	//	sysmessage(s,"You fail your attempt at tracking.");
	//	return;
	// }
	// Skills->TrackingMenu(s,sub-1);
}

void ShowMenu( UOXSOCKET Socket, UI16 Menu ) // Menus for item creation
{
	P_CHAR pc_currchar = currchar[ Socket ];

	QString MenuID;
	MenuID.sprintf( "%d", Menu );

	// Get our node
	QDomNode *NodePtr = DefManager->getSection( WPDT_MENU, MenuID );
	QDomElement Node = NodePtr->toElement();
	
	if( Node.isNull() )
	{
		sysmessage( Socket, "There is no such menu: %i\n", Menu );
		return;
	}

	// If the menu Requires special clearance make sure
	// it requirements are met
	if( Node.attributes().contains( "menupriv" ) )
	{
		bool Passed = false;

		// If we're on account 0 you can always access the menu
		if( pc_currchar->menupriv() == -1 ) 
			Passed = true;
		else
		{
			QStringList MenuPrivs = QStringList::split( ",", Node.attributeNode( "menupriv" ).nodeValue() );

			// No Priv requirements -> pass!
			if( MenuPrivs.empty() )
				Passed = true;

			// Check if our character meets one of the requirements
			for( UI08 i = 0; i < MenuPrivs.count(); i++ )
				if( pc_currchar->menupriv() == MenuPrivs[ i ].toInt() )
					Passed = true;
		}

		if( !Passed )
		{
			sysmessage( Socket, "You do not meet the requirements to display this menu" );
			return;
		}
	}

	// Get the Menu Name
	if( !Node.attributes().contains( "name" ) )
	{
		sysmessage( Socket, "An error occured while parsing menu: %i\n", Menu );
		return;
	}

	QString MenuName = Node.attributes().namedItem( "name" ).toAttr().nodeValue();
	QByteArray ByteArray;

	// we've got 
	// a 9 byte header, 1 byte stringlength and xx string
	ByteArray.resize( 9 + 1 + MenuName.length() + 1 );
	ByteArray.fill( 0 );

	// Basic Packet information
	// Packet: x7C
	// BYTE cmd = x7C
	// BYTE[2] blockSize x0000
	// BYTE[4] dialogID x01020304
	// BYTE[2] menuid x0064
	ByteArray[ 0 ] = 0x7C; // Packet ID

	// Skip 2 bytes for the packet length
	ByteArray[ 3 ] = (UI08)( pc_currchar->serial >> 24 );
	ByteArray[ 4 ] = (UI08)( pc_currchar->serial >> 16 );
	ByteArray[ 5 ] = (UI08)( pc_currchar->serial >> 8 );
	ByteArray[ 6 ] = (UI08)( pc_currchar->serial );

	// Menu-ID
	UI16 RealID = MenuID.toULong();

	ByteArray[ 7 ] = (UI08)( RealID >> 8 );
	ByteArray[ 8 ] = (UI08)( RealID );

	// BYTE length of question
	// BYTE[length of question] question text
	ByteArray[ 9 ] = (UI08)( MenuName.length() );

        UI08 i;

	for( i = 0; i < MenuName.length(); i++ )
	{
		ByteArray[ 10 + i ] = MenuName.at( i ).latin1();
	}

	// Current Position = Length() - 1
	int CurrentIndex = ByteArray.count();
	UI08 Responses = 0;

	// BYTE[2] model id # of shown item (if grey menu -- then always 0x00 as msb)
	// BYTE[2] unknown2 (00 00 check or not?)
	// BYTE response text length
	// BYTE[response text length] response text
	for( i = 0; i < Node.childNodes().count(); i++ )
	{
		QDomNode SubNode = Node.childNodes().item( i );

		if( !SubNode.isElement() )
			continue;

		if( ( SubNode.toElement().tagName() != "submenu" ) && ( SubNode.toElement().tagName() != "actions" ) )
			continue;

		// Display the item __ONLY__ if the user meets the requirements
		// I.e. Skill or Resources
		//
		// ^^^^ This is DISABLED as it uses up too much CPU time
		//      to check for the requirements each time
		UI16 ModelID = 0;
		QString ResponseText;

		// Small Icon in the horizontal scrolling menu
		if( SubNode.toElement().attributes().contains( "model" ) )
			ModelID = SubNode.toElement().attributeNode( "model" ).nodeValue().toUShort( NULL, 16 );
		
		// Response Text
		if( SubNode.toElement().attributes().contains( "name" ) )
			ResponseText = SubNode.toElement().attributeNode( "name" ).nodeValue();

		// Resize the ByteArray to meet the new conditions
		// Current Length + 2 model-id + 2 unknown + 1 textlength + textlength
		ByteArray.resize( ByteArray.size() + 4 + 1 + ResponseText.length() );
		
		// Model ID
		ByteArray[ CurrentIndex++ ] = (UI08)( ModelID >> 8 );
		ByteArray[ CurrentIndex++ ] = (UI08)( ModelID );

		// Model Hue
		QString Hue = "0";

		if( SubNode.toElement().attributes().contains( "hue" ) )
			Hue = SubNode.toElement().attributeNode( "hue" ).nodeValue();

		UI16 RealHue = Hue.toUShort( NULL, 16 );

		ByteArray[ CurrentIndex++ ] = (UI08)( RealHue >> 8 );
		ByteArray[ CurrentIndex++ ] = (UI08)( RealHue );

		// "Copy" in the Response Text
		ByteArray[ CurrentIndex++ ] = (UI08)( ResponseText.length() );

		for( UI08 j = 0; j < ResponseText.length(); j++ )
		{
			ByteArray[ CurrentIndex++ ] = ResponseText.at( j ).latin1();
		}

		Responses++;
	}

	// Number of Responses
	ByteArray[ (int)( 9 + 1 + MenuName.length() ) ] = Responses;

	// Set the packet length
	UI16 PacketLength = ByteArray.count();
	ByteArray[ 1 ] = (UI08)( PacketLength >> 8 );
	ByteArray[ 2 ] = (UI08)( PacketLength % 256 );

	Xsend( Socket, ByteArray.data(), ByteArray.count() );
}

cGump::cGump() : noMove_( false ), noClose_( false ), 
noDispose_( false ), x_( -1 ), y_( -1 ), serial_( INVALID_SERIAL ), 
type_( 1 ) 
{
}

// New Single gump implementation, written by darkstorm
Q_UINT32 cGump::addRawText( const QString &data )
{
	// Do we already have the text?
	if( !text_.contains( data ) ) 
		text_.push_back( data );

	return text_.findIndex( data );
}

void cGump::addButton( Q_INT32 buttonX, Q_INT32 buttonY, Q_UINT16 gumpUp, Q_UINT16 gumpDown, Q_INT32 returnCode )
{
	QString button = QString( "{button %1 %2 %3 %4 1 0 %5}" ).arg( buttonX ).arg( buttonY ).arg( gumpUp ).arg( gumpDown ).arg( returnCode );
	layout_.push_back( button );
}

void cGump::addPageButton( Q_INT32 buttonX, Q_INT32 buttonY, Q_UINT16 gumpUp, Q_UINT16 gumpDown, Q_INT32 pageId )
{
	QString button = QString( "{button %1 %2 %3 %4 0 %5 0}" ).arg( buttonX ).arg( buttonY ).arg( gumpUp ).arg( gumpDown ).arg( pageId );
	layout_.push_back( button );
}

void cGump::addGump( Q_INT32 gumpX, Q_INT32 gumpY, Q_UINT16 gumpId, Q_INT16 hue ) 
{
	layout_.push_back( QString( "{gumppic %1 %2 %3%4}" ).arg( gumpX ).arg( gumpY ).arg( gumpId ).arg( ( hue != -1 ) ? QString( " hue=%1" ).arg( hue ) : QString("") ) ); 
}

void cGump::addTiledGump( Q_INT32 gumpX, Q_INT32 gumpY, Q_INT32 width, Q_INT32 height, Q_UINT16 gumpId, Q_INT16 hue ) 
{
	layout_.push_back( QString( "{gumppictiled %1 %2 %3 %4 %5%6}" ).arg( gumpX ).arg( gumpY ).arg( gumpId ).arg( width ).arg( height ).arg( ( hue != -1 ) ? QString( " hue=%1" ).arg( hue ) : QString("") ) ); 
}

cSpawnRegionInfoGump::cSpawnRegionInfoGump( cSpawnRegion* region, UINT32 page )
{
	type_ = 3;
	region_ = region;
	page_ = page;

	if( region )
	{
		QStringList allrectangles = region->rectangles();

		UINT32 i;
		UINT32 right = page_ * 10 - 1;
		UINT32 left = page_ * 10 - 10;
		UINT32 numrects = allrectangles.size();
		UINT32 pages = ((UINT32)floor( numrects / 10 ))+1;
		if( numrects < right )
			right = numrects-1;

		QStringList rectangles = QStringList();
		QStringList::const_iterator it = allrectangles.at( left );
		while( it != allrectangles.at( right+1 ) )
		{
			rectangles.push_back( (*it) );
			it++;
		}
		numrects = rectangles.size();
			
		// Basic .INFO Header
		addResizeGump( 0, 40, 0xA28, 450, 220 + numrects * 20 ); //Background
		addGump( 105, 18, 0x58B ); // Fancy top-bar
		addGump( 182, 0, 0x589 ); // "Button" like gump
		addTilePic( 202, 23, 0x14eb ); // Type of info menu

		addText( 155, 90, tr( "Spawnregion Info - Page %1 / %2" ).arg( page_ ).arg( pages ), 0x530 );

		// Give information about the spawnregion
		addText( 50, 120, tr( "Name: %1" ).arg( region->name() ), 0x834 );
		addText( 50, 140, tr( "NPCs: %1 of %2" ).arg( region->npcs() ).arg( region->maxNpcs() ), 0x834 );
		addText( 50, 160, tr( "Items: %1 of %2" ).arg( region->items() ).arg( region->maxItems() ), 0x834 );
		addText( 50, 180, tr( "Coordinates: %1" ).arg( allrectangles.size() ), 0x834 );
			
		for( i = 0; i < numrects; i++ )
		{
			addText( 50, 200 + i * 20, tr( "Rectangle %1: %2" ).arg( i+1+left ).arg( rectangles[i] ), 0x834 );
		}

		// OK button
		addButton( 50, 210 + numrects * 20, 0x481, 0x483, 0 ); 
		addText( 90, 210 + numrects * 20, tr( "Close" ), 0x834 );

		if( page_ > 1 ) // previous page
			addButton( 320, 210 + numrects * 20, 0x0FC, 0x0FC, page_-1 );

		if( page_ < pages ) // next page
			addButton( 340, 210 + numrects * 20, 0x0FA, 0x0FA, page_+1 );
	}
}

void cSpawnRegionInfoGump::handleResponse( cUOSocket* socket, gumpChoice_st choice )
{
	if( choice.button == 0 )
		return;

	if( region_ )
	{
		cSpawnRegionInfoGump* pGump = new cSpawnRegionInfoGump( region_, choice.button );
		socket->send( pGump );
	}
}

cCharInfoGump::cCharInfoGump( cChar* pChar, UINT32 page )
{
	type_ = 4;
	char_ = pChar;
	page_ = page;
	UINT32 pages = 2;

	if( char_ )
	{
		if( page == 1 )
		{
			addResizeGump( 0, 40, 0xA28, 450, 350 ); //Background
			addGump( 105, 18, 0x58B ); // Fancy top-bar
			addGump( 182, 0, 0x589 ); // "Button" like gump
			addTilePic( 202, 23, 0x14eb ); // Type of info menu

			addText( 155, 90, tr( "Char Info - Page %1 / %2" ).arg( page_ ).arg( pages ), 0x530 );

			// Give information about the spawnregion
			addText( 50, 120, tr( "Name:" ), 0x834 );
			addText( 250, 120, QString( "%1" ).arg( pChar->name.c_str() ), 0x834 );
			addText( 50, 140, tr( "Title:" ), 0x834 );
			addText( 250, 140, QString( "%1" ).arg( pChar->title() ), 0x834 );
			addText( 50, 160, tr( "Body:" ), 0x834 );
			addText( 250, 160, QString( "%1" ).arg( pChar->id() ), 0x834 );
			addText( 50, 180, tr( "Skin:" ), 0x834 );
			addText( 250, 180, QString( "%1" ).arg( pChar->skin() ), 0x834 );
			addText( 50, 200, tr( "Strength:" ), 0x834 );
			addText( 250, 200, QString( "%1 / %2" ).arg( pChar->st2 ).arg( pChar->st ), 0x834 );
			addText( 50, 220, tr( "Dexterity:" ), 0x834 );
			addText( 250, 220, QString( "%1 / %2" ).arg( pChar->decDex() ).arg( pChar->realDex() ), 0x834 );
			addText( 50, 240, tr( "Intelligence:" ), 0x834 );
			addText( 250, 240, QString( "%1 / %2" ).arg( pChar->in2 ).arg( pChar->in2 ), 0x834 );
			addText( 50, 260, tr( "Hitpoints:" ), 0x834 );
			addText( 250, 260, QString( "%1" ).arg( pChar->hp ), 0x834 );
			addText( 50, 280, tr( "Stamina:" ), 0x834 );
			addText( 250, 280, QString( "%1" ).arg( pChar->stm ), 0x834 );
			addText( 50, 300, tr( "Mana:" ), 0x834 );
			addText( 250, 300, QString( "%1" ).arg( pChar->mn ), 0x834 );

			// OK button
			addButton( 50, 340, 0x481, 0x483, 0 ); 
			addText( 90, 340, tr( "Close" ), 0x834 );
			// next page
			addButton( 340, 340, 0x0FA, 0x0FA, page_+1 );
		}
		else if( page == 2 )
		{
			addResizeGump( 0, 40, 0xA28, 450, 350 ); //Background
			addGump( 105, 18, 0x58B ); // Fancy top-bar
			addGump( 182, 0, 0x589 ); // "Button" like gump
			addTilePic( 202, 23, 0x14eb ); // Type of info menu

			addText( 155, 90, tr( "Char Info - Page %1 / %2" ).arg( page_ ).arg( pages ), 0x530 );

			// Give information about the spawnregion
			addText( 50, 120, tr( "Spawnregion:" ), 0x834 );
			addText( 250, 120, QString( "%1" ).arg( pChar->spawnregion() ), 0x834 );
			addText( 50, 140, tr( "Karma:" ), 0x834 );
			addText( 250, 140, QString( "%1" ).arg( pChar->karma ), 0x834 );
			addText( 50, 160, tr( "Fame:" ), 0x834 );
			addText( 250, 160, QString( "%1" ).arg( pChar->fame ), 0x834 );
			addText( 50, 180, tr( "Kills:" ), 0x834 );
			addText( 250, 180, QString( "%1" ).arg( pChar->kills ), 0x834 );
			addText( 50, 200, tr( "Deaths:" ), 0x834 );
			addText( 250, 200, QString( "%1" ).arg( pChar->deaths ), 0x834 );
			addText( 50, 220, tr( "Defense:" ), 0x834 );
			addText( 250, 220, QString( "%1" ).arg( pChar->def ), 0x834 );
			addText( 50, 240, tr( "Npc Wander:" ), 0x834 );
			addText( 250, 240, QString( "%1" ).arg( pChar->npcWander ), 0x834 );
			addText( 50, 260, tr( "Carve:" ), 0x834 );
			addText( 250, 260, QString( "%1" ).arg( pChar->carve() ), 0x834 );
			addText( 50, 280, tr( "Loot:" ), 0x834 );
			addText( 250, 280, QString( "%1" ).arg( pChar->lootList() ), 0x834 );
			addText( 50, 300, tr( "Hunger:" ), 0x834 );
			addText( 250, 300, QString( "%1" ).arg( pChar->hunger() ), 0x834 );

			// OK button
			addButton( 50, 340, 0x481, 0x483, 0 ); 
			addText( 90, 340, tr( "Close" ), 0x834 );
			// previous page
			addButton( 320, 340, 0x0FC, 0x0FC, page_-1 );
		}
	}
}

void cCharInfoGump::handleResponse( cUOSocket* socket, gumpChoice_st choice )
{
	if( choice.button == 0 )
		return;

	if( char_ )
	{
		cCharInfoGump* pGump = new cCharInfoGump( char_, choice.button );
		socket->send( pGump );
	}
}

