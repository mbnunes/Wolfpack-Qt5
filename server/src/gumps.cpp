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
#include "spawnregions.h"
#include "accounts.h"
#include "pagesystem.h"

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

void cGump::addHtmlGump( INT32 x, INT32 y, INT32 width, INT32 height, const QString &html, bool hasBack, bool canScroll )
{
	QString layout = "{htmlgump %1 %2 %3 %4 %5 %6 %7}";
	layout = layout.arg( x ).arg( y ).arg( width ).arg( height );
	layout = layout.arg( addRawText( html ) ).arg( hasBack ? 1 : 0 ).arg( canScroll ? 1 : 0 );
	layout_.push_back( layout );
}

void cGump::addCheckertrans( INT32 x, INT32 y, INT32 width, INT32 height )
{
	QString layout = "{checkertrans %1 %2 %3 %4}";
	layout = layout.arg( x ).arg( y ).arg( width ).arg( height );
	layout_.push_back( layout );
}

void cGump::addCroppedText( Q_INT32 textX, Q_INT32 textY, Q_UINT32 width, Q_UINT32 height, const QString &data, Q_UINT16 hue )
{
	QString layout = "{croppedtext %1 %2 %3 %4 %5 %6}";
	layout = layout.arg( textX ).arg( textY ).arg( width ).arg( height ).arg( hue ).arg( addRawText( data ) );
	layout_.push_back( layout );
}


cSpawnRegionInfoGump::cSpawnRegionInfoGump( cSpawnRegion* region )
{
	region_ = region;

	if( region )
	{
		QStringList allrectangles = region->rectangles();

		UINT32 page_ = 0;
		UINT32 numrects = allrectangles.size();
		UINT32 pages = ((UINT32)ceil( (double)numrects / 10.0f ));

		startPage();
		// Basic .INFO Header
		addResizeGump( 0, 40, 0xA28, 450, 420 ); //Background
		addGump( 105, 18, 0x58B ); // Fancy top-bar
		addGump( 182, 0, 0x589 ); // "Button" like gump
		addTilePic( 202, 23, 0x14eb ); // Type of info menu
		addText( 170, 90, tr( "Spawnregion Info" ), 0x530 );
		// Give information about the spawnregion
		addText( 50, 120, tr( "Name: %1" ).arg( region->name() ), 0x834 );
		addText( 50, 140, tr( "NPCs: %1 of %2" ).arg( region->npcs() ).arg( region->maxNpcs() ), 0x834 );
		addText( 50, 160, tr( "Items: %1 of %2" ).arg( region->items() ).arg( region->maxItems() ), 0x834 );
		addText( 50, 180, tr( "Coordinates: %1" ).arg( allrectangles.size() ), 0x834 );

		// OK button
		addButton( 50, 410, 0xF9, 0xF8, 0 ); // Only Exit possible

		for( page_ = 1; page_ <= pages; page_++ )
		{
			startPage( page_ );

			UINT32 i;
			UINT32 right = page_ * 10 - 1;
			UINT32 left = page_ * 10 - 10;
			if( numrects <= right )
				right = numrects-1;

			QStringList rectangles = QStringList();
			QStringList::const_iterator it = allrectangles.at( left );
			while( it != allrectangles.at( right+1 ) )
			{
				rectangles.push_back( (*it) );
				it++;
			}
			UINT32 thisrects = rectangles.size();
			

			
			for( i = 0; i < thisrects; i++ )
			{
				addText( 50, 200 + i * 20, tr( "Rectangle %1: %2" ).arg( i+1+left ).arg( rectangles[i] ), 0x834 );
			}


			addText( 310, 410, tr( "Page %1 of %2" ).arg( page_ ).arg( pages ), 0x834 );
			if( page_ > 1 ) // previous page
				addPageButton( 270, 410, 0x0FC, 0x0FC, page_-1 );

			if( page_ < pages ) // next page
				addPageButton( 290, 410, 0x0FA, 0x0FA, page_+1 );
		}
	}
}

void cSpawnRegionInfoGump::handleResponse( cUOSocket* socket, gumpChoice_st choice )
{
	if( choice.button == 0 )
		return;

	if( region_ )
	{
		cSpawnRegionInfoGump* pGump = new cSpawnRegionInfoGump( region_ );
		socket->send( pGump );
	}
}

cCharInfoGump::cCharInfoGump( cChar* pChar )
{
	char_ = pChar;

	if( char_ )
	{
		UINT32 page_ = 0;
		UINT32 pages = 4;

		startPage();
		addResizeGump( 0, 40, 0xA28, 450, 350 ); //Background
		addGump( 105, 18, 0x58B ); // Fancy top-bar
		addGump( 182, 0, 0x589 ); // "Button" like gump
		addTilePic( 202, 23, creatures[ char_->id() ].icon ); // Type of info menu
		addText( 178, 90, tr( "Char Properties" ), 0x530 );

		// Apply button
		addButton( 50, 340, 0xEF, 0xF0, 1 ); 
		// OK button
		addButton( 120, 340, 0xF9, 0xF8, 0 ); 
		// Cancel button
		addButton( 190, 340, 0xF3, 0xF1, -1 ); 

		page_++;
		startPage( page_ );

		addResizeGump( 195, 120, 0xBB8, 215, 20 );
		addResizeGump( 195, 140, 0xBB8, 215, 20 );
		addResizeGump( 195, 160, 0xBB8, 215, 20 );
		addResizeGump( 195, 180, 0xBB8, 215, 20 );
		addResizeGump( 195, 200, 0xBB8, 215, 20 );
		addResizeGump( 195, 220, 0xBB8, 215, 20 );
		addResizeGump( 195, 240, 0xBB8, 215, 20 );
		addResizeGump( 195, 260, 0xBB8, 215, 20 );
		addResizeGump( 195, 280, 0xBB8, 215, 20 );
		addResizeGump( 195, 300, 0xBB8, 215, 20 );

		addText( 50, 120, tr( "Name:" ), 0x834 );
		addInputField( 200, 120, 200, 16,  1, QString( "%1" ).arg( pChar->name.c_str() ), 0x834 );
		addText( 50, 140, tr( "Title:" ), 0x834 );
		addInputField( 200, 140, 200, 16,  2, QString( "%1" ).arg( pChar->title() ), 0x834 );
		addText( 50, 160, tr( "Body:" ), 0x834 );
		addInputField( 200, 160, 200, 16,  3, QString( "0x%1" ).arg( QString::number( pChar->id(), 16 ) ), 0x834 );
		addText( 50, 180, tr( "Skin:" ), 0x834 );
		addInputField( 200, 180, 200, 16,  4, QString( "0x%1" ).arg( QString::number( pChar->skin(), 16 ) ), 0x834 );
		addText( 50, 200, tr( "Strength:" ), 0x834 );
		addInputField( 200, 200, 200, 16,  5, QString( "%1" ).arg( pChar->st ), 0x834 );
		addText( 50, 220, tr( "Dexterity:" ), 0x834 );
		addInputField( 200, 220, 200, 16,  6, QString( "%1" ).arg( pChar->realDex() ), 0x834 );
		addText( 50, 240, tr( "Intelligence:" ), 0x834 );
		addInputField( 200, 240, 200, 16,  7, QString( "%1" ).arg( pChar->in ), 0x834 );
		addText( 50, 260, tr( "Hitpoints:" ), 0x834 );
		addInputField( 200, 260, 200, 16,  8, QString( "%1" ).arg( pChar->hp ), 0x834 );
		addText( 50, 280, tr( "Stamina:" ), 0x834 );
		addInputField( 200, 280, 200, 16,  9, QString( "%1" ).arg( pChar->stm ), 0x834 );
		addText( 50, 300, tr( "Mana:" ), 0x834 );
		addInputField( 200, 300, 200, 16, 10, QString( "%1" ).arg( pChar->mn ), 0x834 );

		addText( 310, 340, tr( "Page %1 of %2" ).arg( page_ ).arg( pages ), 0x834 );
		// next page
		addPageButton( 290, 340, 0x0FA, 0x0FA, page_+1 );

		page_++;
		startPage( page_ );

		addResizeGump( 195, 120, 0xBB8, 215, 20 );
		addResizeGump( 195, 140, 0xBB8, 215, 20 );
		addResizeGump( 195, 160, 0xBB8, 215, 20 );
		addResizeGump( 195, 180, 0xBB8, 215, 20 );
		addResizeGump( 195, 200, 0xBB8, 215, 20 );
		addResizeGump( 195, 220, 0xBB8, 215, 20 );
		addResizeGump( 195, 240, 0xBB8, 215, 20 );
		addResizeGump( 195, 260, 0xBB8, 215, 20 );
//		addResizeGump( 195, 280, 0xBB8, 215, 20 );
		addResizeGump( 195, 300, 0xBB8, 215, 20 );

		addText( 50, 120, tr( "Spawnregion:" ), 0x834 );
		addInputField( 200, 120, 200, 16, 11, QString( "%1" ).arg( pChar->spawnregion() ), 0x834 );
		addText( 50, 140, tr( "Karma:" ), 0x834 );
		addInputField( 200, 140, 200, 16, 12, QString( "%1" ).arg( pChar->karma ), 0x834 );
		addText( 50, 160, tr( "Fame:" ), 0x834 );
		addInputField( 200, 160, 200, 16, 13, QString( "%1" ).arg( pChar->fame ), 0x834 );
		addText( 50, 180, tr( "Kills:" ), 0x834 );
		addInputField( 200, 180, 200, 16, 14, QString( "%1" ).arg( pChar->kills ), 0x834 );
		addText( 50, 200, tr( "Deaths:" ), 0x834 );
		addInputField( 200, 200, 200, 16, 15, QString( "%1" ).arg( pChar->deaths ), 0x834 );
		addText( 50, 220, tr( "Defense:" ), 0x834 );
		addInputField( 200, 220, 200, 16, 16, QString( "%1" ).arg( pChar->def ), 0x834 );
		addText( 50, 240, tr( "Dead:" ), 0x834 );
		addInputField( 200, 240, 200, 16, 17, QString( "%1" ).arg( pChar->dead ), 0x834 );
		addText( 50, 260, tr( "Position (x,y,z,map):" ), 0x834 );
		addInputField( 200, 260, 200, 16, 18, QString("%1,%2,%3,%4").arg( pChar->pos.x ).arg( pChar->pos.y ).arg( pChar->pos.z ).arg( pChar->pos.map ), 0x834 );
		addText( 50, 280, tr( "Serial:" ), 0x834 );
		addText( 200, 280, QString( "%1" ).arg( pChar->serial ), 0x834 );
		addText( 50, 300, tr( "Hunger:" ), 0x834 );
		addInputField( 200, 300, 200, 16, 20, QString( "%1" ).arg( pChar->hunger() ), 0x834 );

		addText( 310, 340, tr( "Page %1 of %2" ).arg( page_ ).arg( pages ), 0x834 );
		// prev page
		addPageButton( 270, 340, 0x0FC, 0x0FC, page_-1 );
		// next page
		addPageButton( 290, 340, 0x0FA, 0x0FA, page_+1 );

		page_++;
		startPage( page_ );

		addResizeGump( 195, 120, 0xBB8, 215, 20 );
		addResizeGump( 195, 140, 0xBB8, 215, 20 );
		addResizeGump( 195, 160, 0xBB8, 215, 20 );
		addResizeGump( 195, 180, 0xBB8, 215, 20 );
		addResizeGump( 195, 200, 0xBB8, 215, 20 );
		addResizeGump( 195, 220, 0xBB8, 215, 20 );
		addResizeGump( 195, 240, 0xBB8, 215, 20 );
		addResizeGump( 195, 260, 0xBB8, 215, 20 );
		addResizeGump( 195, 280, 0xBB8, 215, 20 );
		addResizeGump( 195, 300, 0xBB8, 215, 20 );

		addText( 50, 120, tr( "Npc Wander:" ), 0x834 );
		addInputField( 200, 120, 200, 16, 21, QString( "%1" ).arg( pChar->npcWander ), 0x834 );
		addText( 50, 140, tr( "fx1:" ), 0x834 );
		addInputField( 200, 140, 200, 16, 22, QString( "%1" ).arg( pChar->fx1 ), 0x834 );
		addText( 50, 160, tr( "fy1:" ), 0x834 );
		addInputField( 200, 160, 200, 16, 23, QString( "%1" ).arg( pChar->fy1 ), 0x834 );
		addText( 50, 180, tr( "fx2:" ), 0x834 );
		addInputField( 200, 180, 200, 16, 24, QString( "%1" ).arg( pChar->fx2 ), 0x834 );
		addText( 50, 200, tr( "fy2:" ), 0x834 );
		addInputField( 200, 200, 200, 16, 25, QString( "%1" ).arg( pChar->fy2 ), 0x834 );
		addText( 50, 220, tr( "fz:" ), 0x834 );
		addInputField( 200, 220, 200, 16, 26, QString( "%1" ).arg( pChar->fz1 ), 0x834 );
		addText( 50, 240, tr( "Direction:" ), 0x834 );
		addInputField( 200, 240, 200, 16, 27, QString( "%1" ).arg( pChar->dir ), 0x834 );
		addText( 50, 260, tr( "Strength modifier:" ), 0x834 );
		addInputField( 200, 260, 200, 16, 28, QString( "%1" ).arg( pChar->st2 ), 0x834 );
		addText( 50, 280, tr( "Dexterity modifier:" ), 0x834 );
		addInputField( 200, 280, 200, 16, 29, QString( "%1" ).arg( pChar->decDex() ), 0x834 );
		addText( 50, 300, tr( "Intelligence modifier:" ), 0x834 );
		addInputField( 200, 300, 200, 16, 30, QString( "%1" ).arg( pChar->in2 ), 0x834 );

		addText( 310, 340, tr( "Page %1 of %2" ).arg( page_ ).arg( pages ), 0x834 );
		// prev page
		addPageButton( 270, 340, 0x0FC, 0x0FC, page_-1 );
		// next page
		addPageButton( 290, 340, 0x0FA, 0x0FA, page_+1 );

		page_++;
		startPage( page_ );

		addResizeGump( 195, 120, 0xBB8, 215, 20 );
		addResizeGump( 195, 140, 0xBB8, 215, 20 );
		addResizeGump( 195, 160, 0xBB8, 215, 20 );
		addResizeGump( 195, 180, 0xBB8, 215, 20 );
		addResizeGump( 195, 200, 0xBB8, 215, 20 );

		addText( 50, 120, tr( "Speech color:" ), 0x834 );
		addInputField( 200, 120, 200, 16, 31, QString( "0x%1" ).arg( QString::number( pChar->saycolor, 16 ) ), 0x834 );
		addText( 50, 140, tr( "Emote color:" ), 0x834 );
		addInputField( 200, 140, 200, 16, 32, QString( "0x%1" ).arg( QString::number( pChar->emotecolor, 16 ) ), 0x834 );
		addText( 50, 160, tr( "Speech:" ), 0x834 );
		addInputField( 200, 160, 200, 16, 33, QString( "%1" ).arg( pChar->speech ), 0x834 );
		addText( 50, 180, tr( "Carve:" ), 0x834 );
		addInputField( 200, 260, 200, 16, 34, QString( "%1" ).arg( pChar->carve() ), 0x834 );
		addText( 50, 200, tr( "Loot:" ), 0x834 );
		addInputField( 200, 280, 200, 16, 35, QString( "%1" ).arg( pChar->lootList() ), 0x834 );

		addText( 310, 340, tr( "Page %1 of %2" ).arg( page_ ).arg( pages ), 0x834 );
		// prev page
		addPageButton( 270, 340, 0x0FC, 0x0FC, page_-1 );
	}
}

void cCharInfoGump::handleResponse( cUOSocket* socket, gumpChoice_st choice )
{
	if( choice.button == -1 )
		return;

	if( char_ )
	{
		std::map< UINT16, QString >::iterator it = choice.textentries.begin();
		while( it != choice.textentries.end() )
		{
			switch( it->first )
			{
			case 1:
				char_->name = it->second.latin1();
				break;
			case 2:
				char_->setTitle( it->second );
				break;
			case 3:
				char_->setId( hex2dec( it->second ).toUShort() );
				break;
			case 4:
				char_->setSkin( hex2dec( it->second ).toUShort() );
				break;
			case 5:
				char_->st = hex2dec( it->second ).toShort();
				break;
			case 6:
				char_->setDex( hex2dec( it->second ).toShort() );
				break;
			case 7:
				char_->in = hex2dec( it->second ).toShort();
				break;
			case 8:
				char_->hp = hex2dec( it->second ).toShort();
				break;
			case 9:
				char_->stm = hex2dec( it->second ).toShort();
				break;
			case 10:
				char_->mn = hex2dec( it->second ).toShort();
				break;
			case 11:
				char_->setSpawnregion( it->second );
				break;
			case 12:
				char_->karma = hex2dec( it->second ).toInt();
				break;
			case 13:
				char_->fame = hex2dec( it->second ).toInt();
				break;
			case 14:
				char_->kills = hex2dec( it->second ).toUInt();
				break;
			case 15:
				char_->deaths = hex2dec( it->second ).toUInt();
				break;
			case 16:
				char_->def = hex2dec( it->second ).toUInt();
				break;
			case 17:
				char_->dead = ( it->second == "true" || hex2dec( it->second ).toUInt() > 0 );
				break;
			case 18:
				{
					UINT16 x,y;
					INT8 z;

					QStringList sects = QStringList::split( " ", it->second );
					if( sects.count() > 0 )
					{
						QStringList coords = QStringList::split( ",", sects[0] );
						if( coords.count() >= 1 )
							char_->pos.x = coords[0].toUShort();
						if( coords.count() >= 2 )
							char_->pos.y = coords[1].toUShort();
						if( coords.count() >= 3 )
							char_->pos.z = coords[2].toShort();
						if( coords.count() >= 4 )
							char_->pos.map = coords[3].toUShort();

						if( sects.count() >= 3 && sects[1] == "map" && coords.count() < 4 )
						{
							char_->pos.map = sects[2].toUShort();
						}
					}
					char_->removeFromView( false );
					char_->moveTo( char_->pos );
					char_->resend( false );
					socket->resendPlayer();
					socket->resendWorld();
				}
				break;
			case 20:
				char_->setHunger( hex2dec( it->second ).toInt() );
				break;
			case 21:
				char_->npcWander = hex2dec( it->second ).toUShort();
				break;
			case 22:
				char_->fx1 = hex2dec( it->second ).toInt();
				break;
			case 23:
				char_->fy1 = hex2dec( it->second ).toInt();
				break;
			case 24:
				char_->fx2 = hex2dec( it->second ).toInt();
				break;
			case 25:
				char_->fy2 = hex2dec( it->second ).toInt();
				break;
			case 26:
				char_->fz1 = hex2dec( it->second ).toInt();
				break;
			case 27:
				char_->dir = hex2dec( it->second ).toUShort();
				break;
			case 28:
				char_->st2 = hex2dec( it->second ).toShort();
				break;
			case 29:
				char_->setDecDex( hex2dec( it->second ).toShort() );
				break;
			case 30:
				char_->in2 = hex2dec( it->second ).toShort();
				break;
			case 31:
				char_->saycolor = hex2dec( it->second ).toUShort();
				break;
			case 32:
				char_->emotecolor = hex2dec( it->second ).toUShort();
				break;
			case 33:
				char_->speech = hex2dec( it->second ).toUShort();
				break;
			case 34:
				char_->setCarve( it->second );
				break;
			case 35:
				char_->setLootList( it->second );
				break;
			}
			it++;
		}

		if( choice.button == 0 )
			return;
		else
		{
			cCharInfoGump* pGump = new cCharInfoGump( char_ );
			socket->send( pGump );
		}
	}
}

cItemInfoGump::cItemInfoGump( cItem* pItem )
{
	item_ = pItem;

	if( item_ )
	{
		UINT32 page_ = 0;
		UINT32 pages = 5;

		startPage();
		addResizeGump( 0, 40, 0xA28, 450, 350 ); //Background
		addGump( 105, 18, 0x58B ); // Fancy top-bar
		addGump( 182, 0, 0x589 ); // "Button" like gump
		addTilePic( 202, 23, item_->id() ); // Type of info menu
		addText( 180, 90, tr( "Item Properties" ), 0x530 );

		// Apply button
		addButton( 50, 340, 0xEF, 0xF0, 1 ); 
		// OK button
		addButton( 120, 340, 0xF9, 0xF8, 0 ); 
		// Cancel button
		addButton( 190, 340, 0xF3, 0xF1, -1 ); 

		page_++;
		startPage( page_ );

		addResizeGump( 195, 120, 0xBB8, 215, 20 );
		addResizeGump( 195, 140, 0xBB8, 215, 20 );
		addResizeGump( 195, 160, 0xBB8, 215, 20 );
//		addResizeGump( 195, 180, 0xBB8, 215, 20 ); serial is readonly
		addResizeGump( 195, 200, 0xBB8, 215, 20 );
		addResizeGump( 195, 220, 0xBB8, 215, 20 );
		addResizeGump( 195, 240, 0xBB8, 215, 20 );
		addResizeGump( 195, 260, 0xBB8, 215, 20 );
		addResizeGump( 195, 280, 0xBB8, 215, 20 );
		addResizeGump( 195, 300, 0xBB8, 215, 20 );

		addText( 50, 120, tr( "Name:" ), 0x834 );
		addInputField( 200, 120, 200, 16,  1, QString( "%1" ).arg( pItem->name() ), 0x834 );
		addText( 50, 140, tr( "Name (identified):" ), 0x834 );
		addInputField( 200, 140, 200, 16,  2, QString( "%1" ).arg( pItem->name2() ), 0x834 );
		addText( 50, 160, tr( "ID:" ), 0x834 );
		addInputField( 200, 160, 200, 16,  3, QString( "0x%1" ).arg( QString::number( pItem->id(), 16 ) ), 0x834 );
		addText( 50, 180, tr( "Serial:" ), 0x834 );
		addText( 200, 180, QString( "%1" ).arg( pItem->serial ), 0x834 );
		addText( 50, 200, tr( "Position (x,y,z,map):" ), 0x834 );
		addInputField( 200, 200, 200, 16,  5, QString("%1,%2,%3,%4").arg( pItem->pos.x ).arg( pItem->pos.y ).arg( pItem->pos.z ).arg( pItem->pos.map ), 0x834 );
		addText( 50, 220, tr( "Color:" ), 0x834 );
		addInputField( 200, 220, 200, 16,  6, QString( "0x%1" ).arg( QString::number( pItem->color(), 16 ) ), 0x834 );
		addText( 50, 240, tr( "Amount:" ), 0x834 );
		addInputField( 200, 240, 200, 16,  7, QString( "%1" ).arg( pItem->amount() ), 0x834 );
		addText( 50, 260, tr( "Weight:" ), 0x834 );
		addInputField( 200, 260, 200, 16,  8, QString( "%1" ).arg( pItem->weight() ), 0x834 );
		addText( 50, 280, tr( "Layer:" ), 0x834 );
		addInputField( 200, 280, 200, 16,  9, QString( "%1" ).arg( pItem->layer() ), 0x834 );
		addText( 50, 300, tr( "Good:" ), 0x834 );
		addInputField( 200, 300, 200, 16, 10, QString( "%1" ).arg( pItem->good ), 0x834 );

		addText( 310, 340, tr( "Page %1 of %2" ).arg( page_ ).arg( pages ), 0x834 );
		// next page
		addPageButton( 290, 340, 0x0FA, 0x0FA, page_+1 );

		page_++;
		startPage( page_ );

		addResizeGump( 195, 120, 0xBB8, 215, 20 );
		addResizeGump( 195, 140, 0xBB8, 215, 20 );
		addResizeGump( 195, 160, 0xBB8, 215, 20 );
		addResizeGump( 195, 180, 0xBB8, 215, 20 );
		addResizeGump( 195, 200, 0xBB8, 215, 20 );
		addResizeGump( 195, 220, 0xBB8, 215, 20 );
		addResizeGump( 195, 240, 0xBB8, 215, 20 );
		addResizeGump( 195, 260, 0xBB8, 215, 20 );
		addResizeGump( 195, 280, 0xBB8, 215, 20 );
		addResizeGump( 195, 300, 0xBB8, 215, 20 );

		addText( 50, 120, tr( "Type:" ), 0x834 );
		addInputField( 200, 120, 200, 16, 11, QString( "%1" ).arg( pItem->type() ), 0x834 );
		addText( 50, 140, tr( "Type 2:" ), 0x834 );
		addInputField( 200, 140, 200, 16, 12, QString( "%1" ).arg( pItem->type2() ), 0x834 );
		addText( 50, 160, tr( "Max. hitpoints:" ), 0x834 );
		addInputField( 200, 160, 200, 16, 13, QString( "%1" ).arg( pItem->maxhp() ), 0x834 );
		addText( 50, 180, tr( "Hitpoints:" ), 0x834 );
		addInputField( 200, 180, 200, 16, 14, QString( "%1" ).arg( pItem->hp() ), 0x834 );
		addText( 50, 200, tr( "Speed:" ), 0x834 );
		addInputField( 200, 200, 200, 16, 15, QString( "%1" ).arg( pItem->speed() ), 0x834 );
		addText( 50, 220, tr( "Low damage:" ), 0x834 );
		addInputField( 200, 220, 200, 16, 16, QString( "%1" ).arg( pItem->lodamage() ), 0x834 );
		addText( 50, 240, tr( "High damage:" ), 0x834 );
		addInputField( 200, 240, 200, 16, 17, QString( "%1" ).arg( pItem->hidamage() ), 0x834 );
		addText( 50, 260, tr( "Str (required):" ), 0x834 );
		addInputField( 200, 260, 200, 16, 18, QString( "%1" ).arg( pItem->st ), 0x834 );
		addText( 50, 280, tr( "Dex (required):" ), 0x834 );
		addInputField( 200, 280, 200, 16, 19, QString( "%1" ).arg( pItem->dx ), 0x834 );
		addText( 50, 300, tr( "Int (required):" ), 0x834 );
		addInputField( 200, 300, 200, 16, 20, QString( "%1" ).arg( pItem->in ), 0x834 );

		addText( 310, 340, tr( "Page %1 of %2" ).arg( page_ ).arg( pages ), 0x834 );
		// prev page
		addPageButton( 270, 340, 0x0FC, 0x0FC, page_-1 );
		// next page
		addPageButton( 290, 340, 0x0FA, 0x0FA, page_+1 );

		page_++;
		startPage( page_ );

		addResizeGump( 195, 120, 0xBB8, 215, 20 );
		addResizeGump( 195, 140, 0xBB8, 215, 20 );
		addResizeGump( 195, 160, 0xBB8, 215, 20 );
		addResizeGump( 195, 180, 0xBB8, 215, 20 );
		addResizeGump( 195, 200, 0xBB8, 215, 20 );
		addResizeGump( 195, 220, 0xBB8, 215, 20 );
		addResizeGump( 195, 240, 0xBB8, 215, 20 );
		addResizeGump( 195, 260, 0xBB8, 215, 20 );
		addResizeGump( 195, 280, 0xBB8, 215, 20 );
		addResizeGump( 195, 300, 0xBB8, 215, 20 );

		addText( 50, 120, tr( "Dyable:" ), 0x834 );
		addInputField( 200, 120, 200, 16, 21, QString( "%1" ).arg( pItem->dye ), 0x834 );
		addText( 50, 140, tr( "Decay:" ), 0x834 );
		addInputField( 200, 140, 200, 16, 22, QString( "%1" ).arg( pItem->priv & 0x01 ? "true" : "false" ), 0x834 );
		addText( 50, 160, tr( "Dispellable/newbie:" ), 0x834 );
		addInputField( 200, 160, 200, 16, 23, QString( "%1" ).arg( pItem->priv & 0x04 ? "true" : "false" ), 0x834 );
		addText( 50, 180, tr( "Movable:" ), 0x834 );
		addInputField( 200, 180, 200, 16, 24, QString( "%1" ).arg( pItem->magic ), 0x834 );
		addText( 50, 200, tr( "Visible:" ), 0x834 );
		addInputField( 200, 200, 200, 16, 25, QString( "%1" ).arg( pItem->visible ), 0x834 );
		addText( 50, 220, tr( "Rank:" ), 0x834 );
		addInputField( 200, 220, 200, 16, 26, QString( "%1" ).arg( pItem->rank ), 0x834 );
		addText( 50, 240, tr( "Value:" ), 0x834 );
		addInputField( 200, 240, 200, 16, 27, QString( "%1" ).arg( pItem->value ), 0x834 );
		addText( 50, 260, tr( "Restock:" ), 0x834 );
		addInputField( 200, 260, 200, 16, 28, QString( "%1" ).arg( pItem->restock ), 0x834 );
		addText( 50, 280, tr( "Poisoned:" ), 0x834 );
		addInputField( 200, 280, 200, 16, 29, QString( "%1" ).arg( pItem->poisoned ), 0x834 );
		addText( 50, 300, tr( "Spawnregion:" ), 0x834 );
		addInputField( 200, 300, 200, 16, 30, QString( "%1" ).arg( pItem->spawnregion() ), 0x834 );

		addText( 310, 340, tr( "Page %1 of %2" ).arg( page_ ).arg( pages ), 0x834 );
		// prev page
		addPageButton( 270, 340, 0x0FC, 0x0FC, page_-1 );
		// next page
		addPageButton( 290, 340, 0x0FA, 0x0FA, page_+1 );

		page_++;
		startPage( page_ );

		addResizeGump( 195, 120, 0xBB8, 215, 20 );
		addResizeGump( 195, 140, 0xBB8, 215, 20 );
		addResizeGump( 195, 160, 0xBB8, 215, 20 );
		addResizeGump( 195, 180, 0xBB8, 215, 20 );
		addResizeGump( 195, 200, 0xBB8, 215, 20 );
		addResizeGump( 195, 220, 0xBB8, 215, 20 );
		addResizeGump( 195, 240, 0xBB8, 215, 20 );
		addResizeGump( 195, 260, 0xBB8, 215, 20 );
		addResizeGump( 195, 280, 0xBB8, 215, 20 );
		addResizeGump( 195, 300, 0xBB8, 215, 20 );

		addText( 50, 120, tr( "Disabled:" ), 0x834 );
		addInputField( 200, 120, 200, 16, 31, QString( "%1" ).arg( pItem->disabled ), 0x834 );
		addText( 50, 140, tr( "Time unused:" ), 0x834 );
		addInputField( 200, 140, 200, 16, 32, QString( "%1" ).arg( pItem->time_unused ), 0x834 );
		addText( 50, 160, tr( "Creator:" ), 0x834 );
		addInputField( 200, 160, 200, 16, 33, QString( "%1" ).arg( pItem->creator.c_str() ), 0x834 );
		addText( 50, 180, tr( "Made with skill no.:" ), 0x834 );
		addInputField( 200, 180, 200, 16, 34, QString( "%1" ).arg( pItem->madewith ), 0x834 );
		addText( 50, 200, tr( "Morex:" ), 0x834 );
		addInputField( 200, 200, 200, 16, 35, QString( "%1" ).arg( pItem->morex ), 0x834 );
		addText( 50, 220, tr( "Morey:" ), 0x834 );
		addInputField( 200, 220, 200, 16, 36, QString( "%1" ).arg( pItem->morey ), 0x834 );
		addText( 50, 240, tr( "Morez:" ), 0x834 );
		addInputField( 200, 240, 200, 16, 37, QString( "%1" ).arg( pItem->morez ), 0x834 );
		addText( 50, 260, tr( "More1:" ), 0x834 );
		addInputField( 200, 260, 200, 16, 38, QString( "%1" ).arg( pItem->more1 ), 0x834 );
		addText( 50, 280, tr( "More2:" ), 0x834 );
		addInputField( 200, 280, 200, 16, 39, QString( "%1" ).arg( pItem->more2 ), 0x834 );
		addText( 50, 300, tr( "More3:" ), 0x834 );
		addInputField( 200, 300, 200, 16, 40, QString( "%1" ).arg( pItem->more3 ), 0x834 );

		addText( 310, 340, tr( "Page %1 of %2" ).arg( page_ ).arg( pages ), 0x834 );
		// prev page
		addPageButton( 270, 340, 0x0FC, 0x0FC, page_-1 );
		// next page
		addPageButton( 290, 340, 0x0FA, 0x0FA, page_+1 );

		page_++;
		startPage( page_ );

		addResizeGump( 195, 120, 0xBB8, 215, 20 );
		addResizeGump( 195, 140, 0xBB8, 215, 20 );
		addResizeGump( 195, 160, 0xBB8, 215, 20 );
		addResizeGump( 195, 180, 0xBB8, 215, 20 );
		addResizeGump( 195, 200, 0xBB8, 215, 20 );

		addText( 50, 120, tr( "More4:" ), 0x834 );
		addInputField( 200, 120, 200, 16, 41, QString( "%1" ).arg( pItem->more4 ), 0x834 );
		addText( 50, 140, tr( "Moreb1:" ), 0x834 );
		addInputField( 200, 140, 200, 16, 42, QString( "%1" ).arg( pItem->moreb1() ), 0x834 );
		addText( 50, 160, tr( "Moreb2:" ), 0x834 );
		addInputField( 200, 160, 200, 16, 43, QString( "%1" ).arg( pItem->moreb2() ), 0x834 );
		addText( 50, 180, tr( "Moreb3:" ), 0x834 );
		addInputField( 200, 180, 200, 16, 44, QString( "%1" ).arg( pItem->moreb3() ), 0x834 );
		addText( 50, 200, tr( "Moreb4:" ), 0x834 );
		addInputField( 200, 200, 200, 16, 45, QString( "%1" ).arg( pItem->moreb4() ), 0x834 );

		addText( 310, 340, tr( "Page %1 of %2" ).arg( page_ ).arg( pages ), 0x834 );
		// prev page
		addPageButton( 270, 340, 0x0FC, 0x0FC, page_-1 );
	}
}

void cItemInfoGump::handleResponse( cUOSocket* socket, gumpChoice_st choice )
{
	if( choice.button == -1 )
		return;

	if( item_ )
	{
		std::map< UINT16, QString >::iterator it = choice.textentries.begin();
		while( it != choice.textentries.end() )
		{
			switch( it->first )
			{
			case 1:
				item_->setName( it->second );
				break;
			case 2:
				item_->setName2( it->second );
				break;
			case 3:
				item_->setId( hex2dec( it->second ).toUShort() );
				break;
			case 4: // serial is readonly
				break;
			case 5:
				{
					UINT16 x,y;
					INT8 z;

					QStringList sects = QStringList::split( " ", it->second );
					if( sects.count() > 0 )
					{
						QStringList coords = QStringList::split( ",", sects[0] );
						if( coords.count() >= 1 )
							item_->pos.x = coords[0].toUShort();
						if( coords.count() >= 2 )
							item_->pos.y = coords[1].toUShort();
						if( coords.count() >= 3 )
							item_->pos.z = coords[2].toShort();
						if( coords.count() >= 4 )
							item_->pos.map = coords[3].toUShort();

						if( sects.count() >= 3 && sects[1] == "map" && coords.count() < 4 )
						{
							item_->pos.map = sects[2].toUShort();
						}
					}
					item_->moveTo( item_->pos );
					item_->update();
				}
				break;
			case 6:
				item_->setColor( hex2dec( it->second ).toUShort() );
				break;
			case 7:
				item_->setAmount( hex2dec( it->second ).toUShort() );
				break;
			case 8:
				item_->setWeight( hex2dec( it->second ).toShort() );
				break;
			case 9:
				item_->setLayer( hex2dec( it->second ).toShort() );
				break;
			case 10:
				item_->good = hex2dec( it->second ).toInt();
				break;
			case 11:
				item_->setType( hex2dec( it->second ).toUInt() );
				break;
			case 12:
				item_->setType2( hex2dec( it->second ).toUInt() );
				break;
			case 13:
				item_->setMaxhp( hex2dec( it->second ).toShort() );
				break;
			case 14:
				item_->setHp( hex2dec( it->second ).toShort() );
				break;
			case 15:
				item_->setSpeed( hex2dec( it->second ).toShort() );
				break;
			case 16:
				item_->setLodamage( hex2dec( it->second ).toShort() );
				break;
			case 17:
				item_->setHidamage( hex2dec( it->second ).toShort() );
				break;
			case 18:
				item_->st = hex2dec( it->second ).toShort();
				break;
			case 19:
				item_->dx = hex2dec( it->second ).toShort();
				break;
			case 20:
				item_->in = hex2dec( it->second ).toShort();
				break;
			case 21:
				item_->dye = hex2dec( it->second ).toUShort();
				break;
			case 22:
				if( it->second == "true" || hex2dec( it->second ).toUInt() )
					item_->priv |= 0x01;
				else
					item_->priv &= 0xFE;
				break;
			case 23:
				if( it->second == "true" || hex2dec( it->second ).toUInt() )
					item_->priv |= 0x04;
				else
					item_->priv &= 0xFB;
				break;
			case 24:
				item_->magic = hex2dec( it->second ).toUShort();
				break;
			case 25:
				item_->visible = hex2dec( it->second ).toUShort();
				break;
			case 26:
				item_->rank = hex2dec( it->second ).toInt();
				break;
			case 27:
				item_->value = hex2dec( it->second ).toInt();
				break;
			case 28:
				item_->restock = hex2dec( it->second ).toInt();
				break;
			case 29:
				item_->poisoned = hex2dec( it->second ).toUInt();
				break;
			case 30:
				item_->setSpawnRegion( it->second );
				break;
			case 31:
				item_->disabled = hex2dec( it->second ).toUInt();
				break;
			case 32:
				item_->time_unused = hex2dec( it->second ).toUInt();
				break;
			case 33:
				if( it->second.length() > 0 )
					item_->creator = it->second.latin1();
				break;
			case 34:
				item_->madewith = hex2dec( it->second ).toInt();
				break;
			case 35:
				item_->morex = hex2dec( it->second ).toUInt();
				break;
			case 36:
				item_->morey = hex2dec( it->second ).toUInt();
				break;
			case 37:
				item_->morez = hex2dec( it->second ).toUInt();
				break;
			case 38:
				item_->more1 = hex2dec( it->second ).toUShort();
				break;
			case 39:
				item_->more2 = hex2dec( it->second ).toUShort();
				break;
			case 40:
				item_->more3 = hex2dec( it->second ).toUShort();
				break;
			case 41:
				item_->more4 = hex2dec( it->second ).toUShort();
				break;
			case 42:
				item_->setMoreb1( hex2dec( it->second ).toUShort() );
				break;
			case 43:
				item_->setMoreb2( hex2dec( it->second ).toUShort() );
				break;
			case 44:
				item_->setMoreb3( hex2dec( it->second ).toUShort() );
				break;
			case 45:
				item_->setMoreb4( hex2dec( it->second ).toUShort() );
				break;
			}
			it++;
		}

		if( choice.button == 0 )
			return;
		else
		{
			cItemInfoGump* pGump = new cItemInfoGump( item_ );
			socket->send( pGump );
		}
	}
}

cTagsInfoGump::cTagsInfoGump( cUObject* object )
{
	object_ = object;

	if( object )
	{
		QStringList allkeys = object->tags.getKeys();

		UINT32 page_ = 0;
		UINT32 numkeys = allkeys.size();
		UINT32 pages = ((UINT32)ceil( (double)numkeys / 10.0f ));

		startPage();
		// Basic .INFO Header
		addResizeGump( 0, 40, 0xA28, 450, 420 ); //Background
		addGump( 105, 18, 0x58B ); // Fancy top-bar
		addGump( 182, 0, 0x589 ); // "Button" like gump
		addTilePic( 202, 23, 0x14eb ); // Type of info menu
		addText( 190, 90, tr( "Tags Info" ), 0x530 );

		// OK button
		addButton( 50, 410, 0xF9, 0xF8, 0 ); // Only Exit possible

		for( page_ = 1; page_ <= pages; page_++ )
		{
			startPage( page_ );

			UINT32 i;
			UINT32 right = page_ * 19 - 1;
			UINT32 left = page_ * 19 - 19;
			if( numkeys <= right )
				right = numkeys-1;

			QStringList keys = QStringList();
			QStringList::const_iterator it = allkeys.at( left );
			while( it != allkeys.at( right+1 ) )
			{
				keys.push_back( (*it) );
				it++;
			}
			UINT32 thiskeys = keys.size();
			

			
			for( i = 0; i < thiskeys; i++ )
			{
				addText( 50, 120 + i * 20, tr( "Tag \"%1\": %2" ).arg( keys[i] ).arg( object->tags.get( keys[i] ).asString() ), 0x834 );
			}


			addText( 310, 410, tr( "Page %1 of %2" ).arg( page_ ).arg( pages ), 0x834 );
			if( page_ > 1 ) // previous page
				addPageButton( 270, 410, 0x0FC, 0x0FC, page_-1 );

			if( page_ < pages ) // next page
				addPageButton( 290, 410, 0x0FA, 0x0FA, page_+1 );
		}
	}
}

void cTagsInfoGump::handleResponse( cUOSocket* socket, gumpChoice_st choice )
{
	if( choice.button == 0 )
		return;

	if( object_ )
	{
		cTagsInfoGump* pGump = new cTagsInfoGump( object_ );
		socket->send( pGump );
	}
}

cWhoMenuGump::cWhoMenuGump( UINT32 page )
{
	if( page == 0 )
		return;

	page_ = page;
	QStringList charNames;
	QStringList accNames;
	QStringList IPs;
	std::vector< UINT32 > offsets;
	UINT32 offset = 0;

	sockets_.clear();

	for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		cChar* pChar = mSock->player();
		if( pChar )
		{
			charNames.push_back( pChar->name.c_str() );
			accNames.push_back( pChar->account()->login() );
			IPs.push_back( mSock->ip() );
			sockets_.push_back( mSock );
			offsets.push_back( offset );
		}
		offset++;
	}

	UINT32 numsocks = charNames.size();
	UINT32 pages = ((UINT32)ceil( (double)numsocks / 10.0f ));

	startPage();
	addBackground( 0xE10, 380, 360 ); //Background
	addCheckertrans( 15, 15, 350, 330 );
	addGump( 130, 18, 0xFA8 );
	addText( 165, 20, tr( "Who Menu" ), 0x530 );
	
	// X button
	addText( 70, 320, "Close", 0x834 );
	addButton( 30, 320, 0xFB1, 0xFB3, 0 ); 

	startPage( 1 );

	INT32 i;
	INT32 right = page_ * 10 - 1;
	INT32 left = page_ * 10 - 10;
	if( numsocks <= right )
		right = numsocks-1;

	for( i = left; i <= right; i++ )
	{
		addButton( 20, 60 + (i-left) * 22, 0xFA5, 0xFA7, offsets[i]+3 ); 
		addText( 50, 60 + (i-left) * 22, QString( "%1(%2)" ).arg( charNames[i] ).arg( accNames[i] ), 0x834 );
		addText( 240, 60 + (i-left) * 22, QString( "%1" ).arg( IPs[i] ), 0x834 );
	}

	addText( 280, 320, tr( "Page %1 of %2" ).arg( page_ ).arg( pages ), 0x834 );
	if( page_ > 1 ) // previous page
		addButton( 240, 320, 0x0FC, 0x0FC, 1 );

	if( page_ < pages ) // next page
		addButton( 260, 320, 0x0FA, 0x0FA, 2 );
}
  
void cWhoMenuGump::handleResponse( cUOSocket *socket, gumpChoice_st choice )
{
	if( choice.button == 0 )
		return;
	else
	{
		switch( choice.button )
		{
		case 1:
			{
				cWhoMenuGump* pGump = new cWhoMenuGump( page_-1 );
				socket->send( pGump );
			}
			break;
		case 2:
			{
				cWhoMenuGump* pGump = new cWhoMenuGump( page_+1 );
				socket->send( pGump );
			}
			break;
		default:
			{
				cSocketInfoGump* pGump = new cSocketInfoGump( sockets_[ choice.button-3 ] );
				socket->send( pGump );
			}
			break;
		}
	}
}

cSocketInfoGump::cSocketInfoGump( cUOSocket* socket )
{
	if( !socket )
		return;

	socket_ = socket;
	P_CHAR pChar = socket->player();

	bool contains = false;
	for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		if( mSock = socket )
			contains = true;
	}

	if( contains && pChar )
	{
		startPage();
		
		addBackground( 0xE10, 440, 340 ); //Background
		addResizeGump( 195, 260, 0xBB8, 205, 20 );
		addCheckertrans( 15, 15, 410, 310 );
		addGump( 160, 18, 0xFA2 );
		addText( 195, 20, tr( "Socket Menu" ), 0x530 );
	
		// X button
		addText( 70, 300, "Close", 0x834 );
		addButton( 30, 300, 0xFB1, 0xFB3, 0 ); 

		startPage( 1 );

		addText( 50, 60, tr( "Char name:" ), 0x834 );
		addText( 250, 60, QString( "%1" ).arg( pChar->name.c_str() ), 0x834 );
		addText( 50, 80, tr( "IP:" ), 0x834 );
		addText( 250, 80, QString( "%1" ).arg( socket->ip() ), 0x834 );
		addText( 50, 100, tr( "Position:" ), 0x834 );
		addText( 250, 100, QString("%1,%2,%3 map %4").arg( pChar->pos.x ).arg( pChar->pos.y ).arg( pChar->pos.z ).arg( pChar->pos.map ), 0x834 );
		addText( 50, 120, tr( "Region:" ), 0x834 );
		addText( 250, 120, QString( "%1" ).arg( pChar->region ), 0x834 );
		addText( 50, 140, tr( "Account / ACL:" ), 0x834 );
		addText( 250, 140, QString( "%1 / %2" ).arg( pChar->account()->login() ).arg( pChar->account()->acl() ), 0x834 );

		addButton( 20, 180, 0xFA5, 0xFA7, 1 ); 
		addText( 50, 180, tr( "Go to position" ), 0x834 );
		addButton( 20, 200, 0xFA5, 0xFA71, 2 ); 
		addText( 50, 200, tr( "Bring char" ), 0x834 );
		addButton( 20, 220, 0xFA5, 0xFA7, 3 ); 
		addText( 50, 220, tr( "Jail char" ), 0x834 );
		addButton( 20, 260, 0xFBD, 0xFBF, 6 ); 
		addText( 50, 260, tr( "Send message:" ), 0x834 );
		addInputField( 200, 260, 190, 16, 1, tr( "<msg>" ), 0x834 );

		addButton( 220, 180, 0xFAB, 0xFAD, 5 ); 
		addText( 250, 180, tr( "Show char info gump" ), 0x834 );
		addButton( 220, 200, 0xFA5, 0xFA7, 7 ); 
		addText( 250, 200, tr( "Disconnect" ), 0x834 );
		addButton( 220, 220, 0xFA5, 0xFA7, 4 ); 
		addText( 250, 220, tr( "Forgive char" ), 0x834 );
	}
}

void cSocketInfoGump::handleResponse( cUOSocket* socket, gumpChoice_st choice )
{
	if( !socket_ )
		return;

	bool contains = false;
	for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		if( mSock = socket_ )
			contains = true;
	}
	
	if( choice.button == 0 )
		return;
	else if( contains && socket_->player() )
	{
		cChar* pChar = socket_->player();
		cChar* mChar = socket->player();
		switch( choice.button )
		{
		case 1:
			// Check if the privileges are ok
			if( mChar && !mChar->account()->authorized("command", "go" ))
			{
				socket->sysMessage( tr( "Access to command 'go' was denied" ) );
			}
			else if( mChar )
			{
				mChar->removeFromView( false );
				mChar->moveTo( pChar->pos );
				mChar->resend( false );
				socket->resendPlayer();
				socket->resendWorld();
			}
			break;
		case 2:
			// Check if the privileges are ok
			if( mChar && !mChar->account()->authorized("command", "go" ))
			{
				socket->sysMessage( tr( "Access to command 'go' was denied" ) );
			}
			else if( mChar )
			{
				pChar->removeFromView( false );
				pChar->moveTo( mChar->pos );
				pChar->resend( false );
				socket_->resendPlayer();
				socket_->resendWorld();
			}
			break;
		case 3:
			// TODO jail command
			break;
		case 4:
			// TODO forgive command
			break;
		case 5:
			// Check if the privileges are ok
			if( mChar && !mChar->account()->authorized("command", "info" ))
			{
				socket->sysMessage( tr( "Access to command 'info' was denied" ) );
			}
			else
			{
				cCharInfoGump* pGump = new cCharInfoGump( pChar );
				socket->send( pGump );
			}
			break;
		case 6:
			{
				std::map< UINT16, QString >::iterator it = choice.textentries.find( 1 );
				if( it != choice.textentries.end() )
					socket_->sysMessage( it->second );

				socket->sysMessage( tr("Message sent.") );
			}
			break;
		case 7:
			socket_->disconnect();
			socket->sysMessage( tr("Socket disconnected.") );
			break;
		}
	}
	else
		socket->sysMessage( tr("ERROR: Socket has disconnected or changed character in the meantime!") );
}

cPagesGump::cPagesGump( UINT32 page, WPPAGE_TYPE ptype )
{
	if( page == 0 )
		return;

	page_ = page;
	ptype_ = ptype;
	QStringList charNames;
	QStringList pageTimes;
	QStringList pageCategories;
	std::vector< WPPAGE_TYPE > pageTypes;
	std::vector< UINT32 > offsets;
	UINT32 offset = 0;

	QStringList categories = cPagesManager::getInstance()->categories();

	pagequeue_.clear();

	cPagesManager::iterator it = cPagesManager::getInstance()->begin();
	while( it != cPagesManager::getInstance()->end() )
	{
		cChar* pChar = FindCharBySerial( (*it)->charSerial() );
		if( pChar && ptype <= (*it)->pageType() )
		{
			charNames.push_back( pChar->name.c_str() );
			pageTimes.push_back( (*it)->pageTime() );
			pageCategories.push_back( categories[(*it)->pageCategory()-1] );
			pageTypes.push_back( (*it)->pageType() );
			offsets.push_back( offset );
			pagequeue_.push_back( (*it) );
		}
		it++;
		offset++;
	}

	UINT32 numsocks = charNames.size();
	UINT32 pages = ((UINT32)ceil( (double)numsocks / 10.0f ));

	startPage();
	addBackground( 0xE10, 480, 360 ); //Background
	addCheckertrans( 15, 15, 450, 330 );
	addGump( 180, 18, 0xFA8 );
	addText( 215, 20, tr( "Page Queue" ), 0x530 );
	
	// X button
	addText( 70, 320, "Close", 0x834 );
	addButton( 30, 320, 0xFB1, 0xFB3, 0 ); 

	startPage( 1 );

	INT32 right = page_ * 10 - 1;
	INT32 left = page_ * 10 - 10;
	if( numsocks <= right )
		right = numsocks-1;

	INT32 i = left;
	while( i <= right )
	{
		addButton( 20, 60 + (i-left) * 22, 0xFA5, 0xFA7, offsets[i]+3 );
		addText( 50, 60 + (i-left) * 22, QString( "%1" ).arg( charNames[i] ), 0x844 + 5 * pageTypes[i] );
		addText( 200, 60 + (i-left) * 22, QString( "%1" ).arg( pageCategories[i] ), 0x844 + 5 * pageTypes[i] );
		addText( 280, 60 + (i-left) * 22, QString( "%1" ).arg( pageTimes[i] ), 0x844 + 5 * pageTypes[i] );
		i++;
	}

	addText( 280, 320, tr( "Page %1 of %2" ).arg( page_ ).arg( pages ), 0x834 );
	if( page_ > 1 ) // previous page
		addButton( 240, 320, 0x0FC, 0x0FC, 1 );

	if( page_ < pages ) // next page
		addButton( 260, 320, 0x0FA, 0x0FA, 2 );
}
  
void cPagesGump::handleResponse( cUOSocket *socket, gumpChoice_st choice )
{
	if( choice.button == 0 )
		return;
	else
	{
		switch( choice.button )
		{
		case 1:
			{
				cPagesGump* pGump = new cPagesGump( page_-1, ptype_ );
				socket->send( pGump );
			}
			break;
		case 2:
			{
				cPagesGump* pGump = new cPagesGump( page_+1, ptype_ );
				socket->send( pGump );
			}
			break;
		default:
			{
				cPageInfoGump* pGump = new cPageInfoGump( pagequeue_[ choice.button-3 ] );
				socket->send( pGump );
			}
			break;
		}
	}
}

cPageInfoGump::cPageInfoGump( cPage* page )
{
	page_ = page;

	if( cPagesManager::getInstance()->contains( page ) )
	{
		cChar* pChar = FindCharBySerial( page->charSerial() );
		startPage();
		
		addBackground( 0xE10, 440, 440 ); //Background
		addResizeGump( 195, 360, 0xBB8, 205, 20 );
		addCheckertrans( 15, 15, 410, 410 );
		
		addGump( 165, 18, 0xFA2 );
		addText( 200, 20, tr( "Page Menu" ), 0x530 );
	
		// X button
		addText( 70, 400, "Close", 0x834 );
		addButton( 30, 400, 0xFB1, 0xFB3, 0 ); 

		QStringList categories = cPagesManager::getInstance()->categories();
		switch( page->pageType() )
		{
		case PT_GM:
			addText( 150, 400, QString( "GM Page, Category: %1").arg( categories[page->pageCategory()-1] ), 0x844 + 5 * page->pageType() );
			break;
		case PT_COUNSELOR:
			addText( 150, 400, QString( "Counselor Page, Category: %1").arg( categories[page->pageCategory()-1] ), 0x844 + 5 * page->pageType() );
			break;
		}

		startPage( 1 );

		UINT16 hue = 0x834;

		addText( 50, 60, tr( "Char name:" ), hue );
		addText( 200, 60, QString( "%1" ).arg( pChar->name.c_str() ), hue );
		addText( 50, 80, tr( "Account name:" ), hue );
		addText( 200, 80, QString( "%1" ).arg( pChar->account()->login() ), hue );
		addText( 50, 100, tr( "Char position:" ), hue );
		addText( 200, 100, QString("%1,%2,%3 map %4").arg( pChar->pos.x ).arg( pChar->pos.y ).arg( pChar->pos.z ).arg( pChar->pos.map ), hue );
		addText( 50, 120, tr( "Page sent from:" ), hue );
		addText( 200, 120, QString("%1,%2,%3 map %4").arg( page->pagePos().x ).arg( page->pagePos().y ).arg( page->pagePos().z ).arg( page->pagePos().map ), hue );
		addText( 50, 140, tr( "Date/time:" ), hue );
		addText( 200, 140, QString( "%1" ).arg( page->pageTime() ), hue );

		addText( 50, 160, tr( "Message:" ), hue );
		QString html = 
			QString("<body text=\"#0000FF\" leftmargin=\"0\" topmargin=\"0\" marginwidth=\"0\" marginheight=\"0\">%1</body>").arg( page_->content() );
		addResizeGump( 45, 180, 0xBB8, 345, 84 );
		addHtmlGump( 50, 180, 340, 80, html );

		addButton( 20, 280, 0xFA5, 0xFA7, 1 ); 
		addText( 50, 280, tr( "Go to char position" ), 0x834 );
		addButton( 20, 300, 0xFA5, 0xFA71, 2 ); 
		addText( 50, 300, tr( "Bring char" ), 0x834 );
		addButton( 20, 320, 0xFA5, 0xFA7, 3 ); 
		addText( 50, 320, tr( "Go to page position" ), 0x834 );
		addButton( 20, 360, 0xFBD, 0xFBF, 4 ); 
		addText( 50, 360, tr( "Send message:" ), 0x834 );
		addInputField( 200, 360, 190, 16, 1, tr( "<msg>" ), 0x834 );

		addButton( 220, 280, 0xFAB, 0xFAD, 5 ); 
		addText( 250, 280, tr( "Show socket info gump" ), 0x834 );
		addButton( 220, 300, 0xFA5, 0xFA7, 6 ); 
		addText( 250, 300, tr( "Move page on top" ), 0x834 );
		addButton( 220, 320, 0xFA5, 0xFA7, 7 ); 
		addText( 250, 320, tr( "Delete page" ), 0x834 );

	}
}

void cPageInfoGump::handleResponse( cUOSocket* socket, gumpChoice_st choice )
{
	if( choice.button == 0 )
		return;
	else if( page_ && cPagesManager::getInstance()->contains( page_ ) )
	{
		cChar* pChar = FindCharBySerial( page_->charSerial() );
		cUOSocket* socket_ = pChar->socket();
		
		if( !pChar || !socket_ )
			return;

		cChar* mChar = socket->player();
		switch( choice.button )
		{
		case 1:
			// Check if the privileges are ok
			if( mChar && !mChar->account()->authorized("command", "go" ))
			{
				socket->sysMessage( tr( "Access to command 'go' was denied" ) );
			}
			else if( mChar )
			{
				mChar->removeFromView( false );
				mChar->moveTo( pChar->pos );
				mChar->resend( false );
				socket->resendPlayer();
				socket->resendWorld();
			}
			break;
		case 2:
			// Check if the privileges are ok
			if( mChar && !mChar->account()->authorized("command", "go" ))
			{
				socket->sysMessage( tr( "Access to command 'go' was denied" ) );
			}
			else if( mChar )
			{
				pChar->removeFromView( false );
				pChar->moveTo( mChar->pos );
				pChar->resend( false );
				socket_->resendPlayer();
				socket_->resendWorld();
			}
			break;
		case 3:
			// Check if the privileges are ok
			if( mChar && !mChar->account()->authorized("command", "go" ))
			{
				socket->sysMessage( tr( "Access to command 'go' was denied" ) );
			}
			else if( mChar )
			{
				mChar->removeFromView( false );
				mChar->moveTo( page_->pagePos() );
				mChar->resend( false );
				socket->resendPlayer();
				socket->resendWorld();
			}
			break;
		case 4:
			{
				std::map< UINT16, QString >::iterator it = choice.textentries.find( 1 );
				if( it != choice.textentries.end() )
					socket_->sysMessage( it->second );

				socket->sysMessage( tr("Message sent.") );
			}
			break;
		case 5:
			// Check if the privileges are ok
			if( mChar && !mChar->account()->authorized("command", "info" ))
			{
				socket->sysMessage( tr( "Access to command 'info' was denied" ) );
			}
			else
			{
				cSocketInfoGump* pGump = new cSocketInfoGump( pChar->socket() );
				socket->send( pGump );
			}
			break;
		case 6:
			cPagesManager::getInstance()->moveOnTop( page_ );
			break;
		case 7:
			cPagesManager::getInstance()->remove( page_ );
			return;
		}
		cPageInfoGump* pGump = new cPageInfoGump( page_ );
		socket->send( pGump );
	}
	else
		socket->sysMessage( tr("ERROR: Page has been deleted in the meantime!") );
}

cHelpGump::cHelpGump( SERIAL charSerial )
{
	P_CHAR pChar = FindCharBySerial( charSerial );
	if( !pChar )
		return;

	char_ = charSerial;
	QStringList categories = cPagesManager::getInstance()->categories();
	cPage* pPage = cPagesManager::getInstance()->find( pChar->serial );

	UINT32 category = 0;
	QStringList lines;
	if( pPage )
	{
		category = pPage->pageCategory();
		lines = QStringList::split( "<br>", pPage->content(), true );
	}
	else
	{
		lines.push_back( tr("<msg>") );
		lines.push_back( "" );
		lines.push_back( "" );
		lines.push_back( "" );
	}

	if( category > categories.count() )
		category = 0;

	UINT32 heightmod = ((UINT32)(categories.count() / 2) + ((pPage) ? 2 : 0));

	startPage();
	addResizeGump( 0, 40, 0xA28, 450, 330 + 20 * heightmod ); //Background
	addGump( 105, 18, 0x58B ); // Fancy top-bar
	addGump( 182, 0, 0x589 ); // "Button" like gump
	addGump( 193, 10, 0x15E9 ); // "Button" like gump
	addText( 190, 90, tr( "Help menu" ), 0x530 );

	addText( 50, 120, tr( "Message: %1" ).arg( ((pPage) ? pPage->pageTime() : "") ), 0x834 );
	addResizeGump( 45, 140, 0xBB8, 345, 84 );

	addInputField( 50, 140, 330, 16, 1, lines[0] , 0x834 );
	addInputField( 50, 160, 330, 16, 2, lines[1], 0x834 );
	addInputField( 50, 180, 330, 16, 3, lines[2], 0x834 );
	addInputField( 50, 200, 330, 16, 4, lines[3], 0x834 );

	startGroup( 1 );

	addRadioButton( 50, 230, 0xD0, 0xD1, 1, ((pPage) ? (pPage->pageType() == PT_GM) : true ) );
	addText( 80, 230, tr( "GM Page" ), 0x844 );

	addRadioButton( 250, 230, 0xD0, 0xD1, 2, ((pPage) ? (pPage->pageType() == PT_COUNSELOR) : false ) );
	addText( 280, 230, tr( "Counselor Page" ), 0x849 );

	UINT32 i = 1; // categories[0] == none.. should not be option!
	UINT32 offset = 0;
	startGroup( 2 );
	addText( 50, 270, tr( "Categories:" ), 0x834 );
	while( i < categories.count() )
	{
		addRadioButton( 50, 290 + offset * 20, 0xD0, 0xD1, 4 + i, ((pPage) ? (category == (i+1)) : (i == 1) ) );
		addText( 80, 290 + offset * 20, categories[i], 0x834 );

		if( i+1 < categories.count() )
		{
			addRadioButton( 250, 290 + offset * 20, 0xD0, 0xD1, 5 + i, ((pPage) ? (category == (i+2)) : false ) );
			addText( 280, 290 + offset * 20, categories[i+1], 0x834 );
		}
		offset++;
		i += 2;
	}

	if( pPage )
	{
		addButton( 50, 290 + (offset+1) * 20, 0xFA5, 0xFA7, 2 );
		addText( 100, 290 + (offset+1) * 20, "Delete my page", 0x834 );
	}

//	addButton( 20, 280, 0xFA5, 0xFA7, 1 ); 
//	addText( 60, 280, tr( "" ), 0x834 );

	// OK button
	addButton( 50, 320 + 20 * heightmod, 0xF9, 0xF8, 1 );
	// Cancel button
	addButton( 120, 320 + 20 * heightmod, 0xF3, 0xF1, 0 );

	startPage(1);
}

void cHelpGump::handleResponse( cUOSocket* socket, gumpChoice_st choice )
{
	if( choice.button == 0 ) // canceled
		return;

	cPage* pPage = cPagesManager::getInstance()->find( char_ );

	if( choice.button == 2 )
	{
		if( pPage )
		{
			cPagesManager::getInstance()->remove( pPage );
			socket->sysMessage( "Page successfully deleted." );
			return;
		}
	}
	else
	{
		P_CHAR pChar = FindCharBySerial( char_ );
		if( !pChar )
			return;

		QStringList lines;
		std::map< UINT16, QString >::iterator it = choice.textentries.begin();
		while( it != choice.textentries.end() )
		{
			lines.push_back( it->second );
			it++;
		}
		QString content_ = lines.join( "<br>" );

		if( pPage )
		{
			pPage->setPageTime();
			pPage->setContent( content_ );
			pPage->setPagePos( pChar->pos );
		}
		else
		{
			pPage = new cPage( char_, PT_GM, content_, pChar->pos );
			cPagesManager::getInstance()->push_back( pPage );
		}

		switch( choice.switches[0] )
		{
		case 1:
			pPage->setPageType( PT_GM );
			break;
		case 2:
			pPage->setPageType( PT_COUNSELOR );
			break;
		}
		pPage->setPageCategory( choice.switches[1] - 3 );
	}
}





