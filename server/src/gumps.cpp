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


#include "accounts.h"
#include "globals.h"
#include "gumps.h"
#include "srvparams.h"
#include "wpdefmanager.h"
#include "network.h"
#include "network/uosocket.h"
#include "spawnregions.h"
#include "accounts.h"
#include "pagesystem.h"
#include "territories.h"
#include "basechar.h"
#include "player.h"
#include "world.h"
#include "inlines.h"

// System Includes
#include <math.h>

cGump::cGump() : serial_( INVALID_SERIAL ), type_( 1 ), 
x_( 50 ), y_( 50 ), noMove_( false ), noClose_( false ), 
noDispose_( false )
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
	layout_.push_back( QString( "{gumppictiled %1 %2 %4 %5 %3%6}" ).arg( gumpX ).arg( gumpY ).arg( gumpId ).arg( width ).arg( height ).arg( ( hue != -1 ) ? QString( " hue=%1" ).arg( hue ) : QString("") ) ); 
}

void cGump::addHtmlGump( INT32 x, INT32 y, INT32 width, INT32 height, const QString &html, bool hasBack, bool canScroll )
{
	QString layout = "{htmlgump %1 %2 %3 %4 %5 %6 %7}";
	layout = layout.arg( x ).arg( y ).arg( width ).arg( height );
	layout = layout.arg( addRawText( html ) ).arg( hasBack ? 1 : 0 ).arg( canScroll ? 1 : 0 );
	layout_.push_back( layout );
}

void cGump::addXmfHtmlGump( INT32 x, INT32 y, INT32 width, INT32 height, UINT32 clilocid, bool hasBack , bool canScroll )
{
	QString layout = "{xmfhtmlgump %1 %2 %3 %4 %5 %6 %7}";
	layout = layout.arg( x ).arg( y ).arg( width ).arg( height );
	layout = layout.arg( clilocid ).arg( hasBack ? 1 : 0 ).arg( canScroll ? 1 : 0 );
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


void cGump::handleResponse( cUOSocket* socket, const gumpChoice_st& choice )
{
	Q_UNUSED(socket);
	Q_UNUSED(choice);
#pragma message(__FILE__ Reminder "This is a good candidate for pure virtual")
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

void cSpawnRegionInfoGump::handleResponse( cUOSocket* socket, const gumpChoice_st& choice )
{
	if( choice.button == 0 )
		return;

	if( region_ )
	{
		cSpawnRegionInfoGump* pGump = new cSpawnRegionInfoGump( region_ );
		socket->send( pGump );
	}
}

cTagsInfoGump::cTagsInfoGump( const cUObject* object ) : object_( const_cast<cUObject*>(object) )
{
	if( object )
	{
		QStringList allkeys = object->getTags();

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
				addText( 50, 120 + i * 20, tr( "Tag \"%1\": %2" ).arg( keys[i] ).arg( object->getTag( keys[i] ).toString() ), 0x834 );
			}


			addText( 310, 410, tr( "Page %1 of %2" ).arg( page_ ).arg( pages ), 0x834 );
			if( page_ > 1 ) // previous page
				addPageButton( 270, 410, 0x0FC, 0x0FC, page_-1 );

			if( page_ < pages ) // next page
				addPageButton( 290, 410, 0x0FA, 0x0FA, page_+1 );
		}
	}
}

void cTagsInfoGump::handleResponse( cUOSocket* socket, const gumpChoice_st& choice )
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
		P_PLAYER pChar = mSock->player();
		if( pChar )
		{
			charNames.push_back( pChar->name() );
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
  
void cWhoMenuGump::handleResponse( cUOSocket *socket, const gumpChoice_st& choice )
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
	P_PLAYER pChar = socket->player();

	bool contains = false;
	for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		if( mSock == socket )
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
		addText( 250, 60, QString( "%1" ).arg( pChar->name() ), 0x834 );
		addText( 50, 80, tr( "IP:" ), 0x834 );
		addText( 250, 80, QString( "%1" ).arg( socket->ip() ), 0x834 );
		addText( 50, 100, tr( "Position:" ), 0x834 );
		addText( 250, 100, QString("%1,%2,%3 map %4").arg( pChar->pos().x ).arg( pChar->pos().y ).arg( pChar->pos().z ).arg( pChar->pos().map ), 0x834 );
		addText( 50, 120, tr( "Region:" ), 0x834 );
		addText( 250, 120, QString( "%1" ).arg( pChar->region() ? pChar->region()->name() : QString("") ), 0x834 );
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

void cSocketInfoGump::handleResponse( cUOSocket* socket, const gumpChoice_st& choice )
{
	if( !socket_ )
		return;

	bool contains = false;
	for( cUOSocket *mSock = cNetwork::instance()->first(); mSock && !contains; mSock = cNetwork::instance()->next() )
	{
		if( mSock == socket_ )
			contains = true;
	}
	
	if( choice.button == 0 )
		return;
	else if( contains && socket_->player() )
	{
		P_PLAYER pChar = socket_->player();
		P_PLAYER mChar = socket->player();
		switch( choice.button )
		{
		case 1:
			// Check if the privileges are ok
			if( mChar && !mChar->account()->authorized( "command", "go" ) )
			{
				socket->sysMessage( tr( "Access to command 'go' was denied" ) );
			}
			else if( mChar )
			{
				UI08 map = mChar->pos().map;
				mChar->removeFromView( false );
				mChar->moveTo( pChar->pos() );
				mChar->resend( false, true );
				socket->resendPlayer( map == pChar->pos().map );
				socket->resendWorld();
			}
			break;
		case 2:
			// Check if the privileges are ok
			if( mChar && !mChar->account()->authorized( "command", "move" ) || pChar->account()->rank() >= mChar->account()->rank() )
			{
				socket->sysMessage( tr( "Access to command 'move' was denied" ) );
			}
			else if( mChar )
			{
				UI08 map = pChar->pos().map;
				pChar->removeFromView( false );
				pChar->moveTo( mChar->pos() );
				pChar->resend( false, true );
				socket->resendPlayer( map == pChar->pos().map );
				socket->resendWorld();
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
			if( mChar && !mChar->account()->authorized( "command", "info" ) )
			{
				socket->sysMessage( tr( "Access to command 'info' was denied" ) );
			}
			else
			{
//				cCharInfoGump* pGump = new cCharInfoGump( pChar );
//				socket->send( pGump );
			}
			break;
		case 6:
			{
				std::map< UINT16, QString >::const_iterator it = choice.textentries.find( 1 );
				if( it != choice.textentries.end() )
					socket_->sysMessage( it->second );

				socket->sysMessage( tr("Message sent.") );
			}
			break;
		case 7:
			if( pChar->account()->rank() >= mChar->account()->rank() )
			{
				socket->sysMessage( tr( "You're getting megalomaniac!" ) );
			}
			else
			{
				socket_->disconnect();
				socket->sysMessage( tr("Socket disconnected.") );
			}
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
		P_CHAR pChar = FindCharBySerial( (*it)->charSerial() );
		if( pChar && ptype <= (*it)->pageType() )
		{
			charNames.push_back( pChar->name() );
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
  
void cPagesGump::handleResponse( cUOSocket *socket, const gumpChoice_st& choice )
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
		P_PLAYER pChar = dynamic_cast<P_PLAYER>(FindCharBySerial( page->charSerial() ));
		if ( !pChar )
			return;
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
		addText( 200, 60, QString( "%1" ).arg( pChar->name() ), hue );
		addText( 50, 80, tr( "Account name:" ), hue );
		addText( 200, 80, QString( "%1" ).arg( pChar->account()->login() ), hue );
		addText( 50, 100, tr( "Char position:" ), hue );
		addText( 200, 100, QString("%1,%2,%3 map %4").arg( pChar->pos().x ).arg( pChar->pos().y ).arg( pChar->pos().z ).arg( pChar->pos().map ), hue );
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

void cPageInfoGump::handleResponse( cUOSocket* socket, const gumpChoice_st& choice )
{
	if( choice.button == 0 )
		return;
	else if( page_ && cPagesManager::getInstance()->contains( page_ ) )
	{
		P_PLAYER pChar = dynamic_cast<P_PLAYER>( FindCharBySerial( page_->charSerial() ) );
		
		if ( !pChar )
			return;

		cUOSocket* socket_ = pChar->socket();
		
		if( !socket_ )
			return;

		P_PLAYER mChar = socket->player();
		switch( choice.button )
		{
		case 1:
			// Check if the privileges are ok
			if( mChar && !mChar->account()->authorized( "command", "go" ) )
			{
				socket->sysMessage( tr( "Access to command 'go' was denied" ) );
			}
			else if( mChar )
			{
				mChar->removeFromView( false );
				mChar->moveTo( pChar->pos() );
				mChar->resend( false );
				socket->resendPlayer();
				socket->resendWorld();
			}
			break;
		case 2:
			// Check if the privileges are ok
			if( mChar && !mChar->account()->authorized( "command", "move" ) || pChar->account()->rank() >= mChar->account()->rank() )
			{
				socket->sysMessage( tr( "Access to command 'move' was denied" ) );
			}
			else if( mChar )
			{
				pChar->removeFromView( false );
				pChar->moveTo( mChar->pos() );
				pChar->resend( false );
				socket_->resendPlayer();
				socket_->resendWorld();
			}
			break;
		case 3:
			// Check if the privileges are ok
			if( mChar && !mChar->account()->authorized("command", "go" ) )
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
				std::map< UINT16, QString >::const_iterator it = choice.textentries.find( 1 );
				if( it != choice.textentries.end() )
					socket_->sysMessage( it->second );

				socket->sysMessage( tr("Message sent.") );
			}
			break;
		case 5:
			// Check if the privileges are ok
			if( mChar && !mChar->account()->authorized("command", "info" ) || pChar->account()->rank() >= mChar->account()->rank() )
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
	cPage* pPage = cPagesManager::getInstance()->find( pChar->serial() );

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

	addText( 50, 120, tr( "Message: %1" ).arg( ((pPage) ? pPage->pageTime() : QString("")) ), 0x834 );
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

	UINT32 i = 0; // categories[0] == none.. should not be option!
	UINT32 offset = 0;
	startGroup( 2 );
	addText( 50, 270, tr( "Categories:" ), 0x834 );
	while( i < categories.count() )
	{
		addRadioButton( 50, 290 + offset * 20, 0xD0, 0xD1, i+1, ((pPage) ? (category == (i)) : (i == 0) ) );
		addText( 80, 290 + offset * 20, categories[i], 0x834 );

		if( i+1 < categories.count() )
		{
			addRadioButton( 250, 290 + offset * 20, 0xD0, 0xD1, i+2, ((pPage) ? (category == (i+1)) : false ) );
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

void cHelpGump::handleResponse( cUOSocket* socket, const gumpChoice_st& choice )
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
		P_PLAYER pChar = dynamic_cast<P_PLAYER>(FindCharBySerial( char_ ));
		if( !pChar )
			return;

		QStringList lines;
		std::map< UINT16, QString >::const_iterator it = choice.textentries.begin();
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
			pPage->setPagePos( pChar->pos() );
		}
		else
		{
			pPage = new cPage( char_, PT_GM, content_, pChar->pos() );
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

		pPage->setPageCategory( choice.switches[1] );

		QString account = "";
		if( pChar->account() )
			account = pChar->account()->login();
		QString message = tr( "%1 Page from %2 [%3]: %4" ).arg( choice.switches[0] == 1 ? "GM" : "Counselor" ).arg( pChar->name() ).arg( account ).arg( lines.join( "\n" ) );

		cUOSocket *mSock = 0;
		for( mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
		{
			// Send a Message to this Character
			if( mSock->account() && mSock->account()->isPageNotify() )
				mSock->sysMessage( message );
		}
	}
}





