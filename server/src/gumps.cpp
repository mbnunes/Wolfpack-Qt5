/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2004 by holders identified in AUTHORS.txt
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In addition to that license, if you are running this program or modified
 * versions of it on a public system you HAVE TO make the complete source of
 * the version used by you available or provide people with a location to
 * download it.
 *
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#include "accounts.h"

#include "gumps.h"
#include "serverconfig.h"
#include "definitions.h"
#include "network/network.h"
#include "network/uosocket.h"
#include "spawnregions.h"
#include "accounts.h"
#include "territories.h"
#include "basechar.h"
#include "player.h"
#include "world.h"
#include "inlines.h"

// System Includes
#include <math.h>

cGump::cGump() : serial_( INVALID_SERIAL ), type_( 1 ), x_( 50 ), y_( 50 ), noMove_( false ), noClose_( false ), noDispose_( false )
{
}

// New Single gump implementation, written by darkstorm
Q_UINT32 cGump::addRawText( const QString& data )
{
	// Do we already have the text?
	if ( !text_.contains( data ) )
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
	layout_.push_back( QString( "{gumppic %1 %2 %3%4}" ).arg( gumpX ).arg( gumpY ).arg( gumpId ).arg( ( hue != -1 ) ? QString( " hue=%1" ).arg( hue ) : QString( "" ) ) );
}

void cGump::addTiledGump( Q_INT32 gumpX, Q_INT32 gumpY, Q_INT32 width, Q_INT32 height, Q_UINT16 gumpId, Q_INT16 hue )
{
	layout_.push_back( QString( "{gumppictiled %1 %2 %4 %5 %3%6}" ).arg( gumpX ).arg( gumpY ).arg( gumpId ).arg( width ).arg( height ).arg( ( hue != -1 ) ? QString( " hue=%1" ).arg( hue ) : QString( "" ) ) );
}

void cGump::addHtmlGump( Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height, const QString& html, bool hasBack, bool canScroll )
{
	QString layout( "{htmlgump %1 %2 %3 %4 %5 %6 %7}" );
	layout = layout.arg( x ).arg( y ).arg( width ).arg( height );
	layout = layout.arg( addRawText( html ) ).arg( hasBack ? 1 : 0 ).arg( canScroll ? 1 : 0 );
	layout_.push_back( layout );
}

void cGump::addXmfHtmlGump( Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height, Q_UINT32 clilocid, bool hasBack, bool canScroll )
{
	QString layout( "{xmfhtmlgump %1 %2 %3 %4 %5 %6 %7}" );
	layout = layout.arg( x ).arg( y ).arg( width ).arg( height );
	layout = layout.arg( clilocid ).arg( hasBack ? 1 : 0 ).arg( canScroll ? 1 : 0 );
	layout_.push_back( layout );
}

void cGump::addCheckertrans( Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height )
{
	QString layout( "{checkertrans %1 %2 %3 %4}" );
	layout = layout.arg( x ).arg( y ).arg( width ).arg( height );
	layout_.push_back( layout );
}

void cGump::addCroppedText( Q_INT32 textX, Q_INT32 textY, Q_UINT32 width, Q_UINT32 height, const QString& data, Q_UINT16 hue )
{
	QString layout( "{croppedtext %1 %2 %3 %4 %5 %6}" );
	layout = layout.arg( textX ).arg( textY ).arg( width ).arg( height ).arg( hue ).arg( addRawText( data ) );
	layout_.push_back( layout );
}


void cGump::handleResponse( cUOSocket* socket, const gumpChoice_st& choice )
{
	Q_UNUSED( socket );
	Q_UNUSED( choice );
}

cSpawnRegionInfoGump::cSpawnRegionInfoGump( cSpawnRegion* region )
{
	region_ = region;

	if ( region )
	{
		/*QStringList allrectangles = region->rectangles();
		*/
		uint page_ = 0;
		uint numrects = 0; /*allrectangles.size();*/
		uint pages = ( ( uint ) ceil( ( double ) numrects / 10.0f ) );

		startPage();
		// Basic .INFO Header
		addResizeGump( 0, 40, 0xA28, 450, 420 ); //Background
		addGump( 105, 18, 0x58B ); // Fancy top-bar
		addGump( 182, 0, 0x589 ); // "Button" like gump
		addTilePic( 202, 23, 0x14eb ); // Type of info menu
		addText( 170, 90, tr( "Spawnregion Info" ), 0x530 );
		// Give information about the spawnregion
		addText( 50, 120, tr( "Name: %1" ).arg( region->id() ), 0x834 );
		addText( 50, 140, tr( "NPCs: %1 of %2" ).arg( region->npcs() ).arg( region->maxNpcs() ), 0x834 );
		addText( 50, 160, tr( "Items: %1 of %2" ).arg( region->items() ).arg( region->maxItems() ), 0x834 );
		if ( region->active() )
		{
			addText( 50, 180, tr( "Status: Active" ), 0x834 );
		}
		else
		{
			addText( 50, 180, tr( "Status: Inactive" ), 0x834 );
		}
		addText( 50, 200, tr( "Groups: %1" ).arg( region->groups().join( ", " ) ), 0x834 );

		// Next Spawn
		unsigned int nextRespawn = 0;
		if ( region->nextTime() > Server::instance()->time() )
		{
			nextRespawn = ( region->nextTime() - Server::instance()->time() ) / 1000;
		}
		addText( 50, 220, tr( "Next Respawn: %1 seconds" ).arg( nextRespawn ), 0x834 );
		addText( 50, 240, tr( "Total Points: %1" ).arg( region->countPoints() ), 0x834 );
		addText( 50, 260, tr( "Delay: %1 to %2 seconds" ).arg( region->minTime() ).arg( region->maxTime() ), 0x834 );

		//addText( 50, 180, tr( "Coordinates: %1" ).arg( allrectangles.size() ), 0x834 );

		// OK button
		addButton( 50, 410, 0xF9, 0xF8, 0 ); // Only Exit possible

		/*for ( page_ = 1; page_ <= pages; page_++ )
			{
				startPage( page_ );
				uint i;
				uint right = page_ * 10 - 1;
				uint left = page_ * 10 - 10;
				if ( numrects <= right )
					right = numrects - 1;
				QStringList rectangles;
				QStringList::const_iterator it = allrectangles.at( left );
				while ( it != allrectangles.at( right + 1 ) )
				{
					rectangles.push_back( ( *it ) );
					it++;
				}
				uint thisrects = rectangles.size();
				for ( i = 0; i < thisrects; i++ )
				{
					addText( 50, 200 + i * 20, tr( "Rectangle %1: %2" ).arg( i + 1 + left ).arg( rectangles[i] ), 0x834 );
				}
				addText( 310, 410, tr( "Page %1 of %2" ).arg( page_ ).arg( pages ), 0x834 );
				if ( page_ > 1 ) // previous page
					addPageButton( 270, 410, 0x0FC, 0x0FC, page_ - 1 );
				if ( page_ < pages ) // next page
					addPageButton( 290, 410, 0x0FA, 0x0FA, page_ + 1 );
			}*/
	}
}

void cSpawnRegionInfoGump::handleResponse( cUOSocket* socket, const gumpChoice_st& choice )
{
	if ( choice.button == 0 )
		return;

	if ( region_ )
	{
		cSpawnRegionInfoGump* pGump = new cSpawnRegionInfoGump( region_ );
		socket->send( pGump );
	}
}
