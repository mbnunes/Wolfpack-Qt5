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
 * Wolfpack Homepage: http://wpdev.sf.net/
 */

#include "tracking.h"
#include "network/uosocket.h"
#include "items.h"
#include "skills.h"

#include "timers.h"
#include "sectors.h"
#include "gumps.h"
#include "basechar.h"
#include "player.h"
#include "basedef.h"
#include "world.h"
#include "inlines.h"

class cRefreshTracking: public cTimer
{
private:
	SERIAL tracker_;
	SERIAL target_;
public:
	cRefreshTracking( SERIAL tracker, SERIAL target )
	{
		tracker_ = tracker;
		target_ = target;
		expiretime = Server::instance()->time() + ( 5 * MY_CLOCKS_PER_SEC );
		serializable = false;
	}

	// Refresh the quest-arrow
	// Until our target expires
	virtual void Expire()
	{
		P_PLAYER pChar = dynamic_cast<P_PLAYER>(FindCharBySerial( tracker_ ));

		if( !pChar || !pChar->socket() )
			return;

		P_CHAR pTarget = FindCharBySerial( target_ );

		// Disable the quest-arrow
		if( !pTarget || pChar->trackingTime() <= Server::instance()->time() )
		{
			pChar->socket()->sendQuestArrow( false, 0, 0 );
			return;
		}

		pChar->socket()->sendQuestArrow( true, pTarget->pos().x, pTarget->pos().y );
		Timers::instance()->insert( new cRefreshTracking( tracker_, target_ ) );
	}
};

class cTrackingList: public cGump
{
public:
	virtual void handleResponse( cUOSocket *socket, const gumpChoice_st& choice )
	{
		P_PLAYER player = socket->player();

		if( !player )
			return;

		P_CHAR pChar = FindCharBySerial( choice.button );

		if( !pChar || pChar->dist( player ) > 32 )
			return;

		// Start the refresh-timer
		// Start the wearoff-timer
		player->setTrackingTime( Server::instance()->time() + ( 30 * MY_CLOCKS_PER_SEC ) );
		Timers::instance()->insert( new cRefreshTracking( player->serial(), choice.button ) );
	}

	cTrackingList( P_CHAR player, UINT8 type )
	{
		setType( 0xFE12ACDE );

		startPage( 0 );
		addBackground( 0x13BE, 440, 160 );
		addResizeGump( 10, 10, 0xA3C, 420, 75 );
		addResizeGump( 10, 85, 0xBB8, 420, 50 );

		UINT32 pCount = 0;
		UINT32 pAmount = 0;

		// 1: Animals
		// 2: Monsters
		// 3: Humans
		// 4: Players
		cCharSectorIterator *iter = SectorMaps::instance()->findChars( player->pos(), 18 );

		for( P_CHAR pChar = iter->first(); pChar; pChar = iter->next() )
		{
			// Do the neccesary checks
			bool passed = true;

			switch( type )
			{
			// Animals
			case 1:
				passed = !( pChar->objectType() != enNPC || pChar->body() == 0x190 || pChar->body() == 0x191 ); //|| pChar->npcaitype() == 2 );
				break;
			// Monsters
			case 2:
				passed = !( pChar->objectType() != enNPC || pChar->body() == 0x190 || pChar->body() == 0x191 ); //|| pChar->npcaitype() != 2 );
				break;
			// Human
			case 3:
				passed = !( pChar->objectType() != enNPC || ( pChar->body() != 0x190 && pChar->body() != 0x191 ) );
				break;
			case 4:
				passed = ( pChar->objectType() == enPlayer && dynamic_cast<P_PLAYER>(pChar)->socket() );
				break;
			};

			if( !passed )
				continue;

			// Checks passed, add the character
			if( pAmount == 0 ) // Start new page
			{
				// Add a button on the old page
				if( pCount > 0 )
				{
					addPageButton( 395, 137, 0x26af, 0x26b1, pCount );
				}

				startPage( ++pCount );

				// Add a button on the new page
				if (pCount > 1) {
					addPageButton( 365, 137, 0x26b5, 0x26b7, pCount-1 );
				}
			}

			if (pChar->figurine() != 0) {
				addTilePic((pAmount*100)+20, 20, pChar->figurine());
			}

			addButton( (pAmount*100)+20, 110, 0xFA5, 0xFA7, pChar->serial() );
			addCroppedText( (pAmount*100)+20, 90, 100, 40, pChar->name() );

			++pAmount;
		}

		delete iter;
	}
};

class cTrackingMenu: public cGump
{
public:
	cTrackingMenu()
	{
		setType( 0xFE12ACDE );
		startPage( 0 );
		addBackground( 0x13BE, 440, 135 );
		addResizeGump( 10, 10, 0xA3C, 420, 75 );
		addResizeGump( 10, 85, 0xBB8, 420, 25 );

		// Animals (0x2101)
		addTilePic( 20, 20, 0x2101 );
		addButton( 20, 110, 0xFA5, 0xFA7, 1 );
		addXmfHtmlGump( 20, 90, 100, 20, 0xF88E7 );

		// Monster (0x210a)
		addTilePic( 120, 20, 0x210a );
		addButton( 120, 110, 0xFA5, 0xFA7, 2 );
		addXmfHtmlGump( 120, 90, 100, 20, 0xF88E8 );

		// Humans (0x2106)
		addTilePic( 220, 20, 0x2106 );
		addButton( 220, 110, 0xFA5, 0xFA7, 3 );
		addXmfHtmlGump( 220, 90, 100, 20, 0xF88E9 );

		// Player (0x2107)
		addTilePic( 320, 20, 0x2107 );
		addButton( 320, 110, 0xFA5, 0xFA7, 4 );
		addXmfHtmlGump( 320, 90, 100, 20, 0xF88EA );
	}

	virtual void handleResponse( cUOSocket *socket, const gumpChoice_st& choice )
	{
		if( choice.button > 4 || choice.button < 1 )
			return;

		if( !socket->player() )
			return;

		cTrackingList *list = new cTrackingList( socket->player(), choice.button );
		socket->send( list );
	}
};

// Creates the tracking menu (Creature type)
void trackingMenu( cUOSocket *socket )
{
	P_CHAR pChar = socket->player();

	if( !pChar )
		return;

	socket->closeGump( 0xFE12ACDE, 0 );

	// If we fail a simple check we dont get the menu
	if( !pChar->checkSkill( TRACKING, 0, 250 ) )
	{
		socket->sysMessage( tr( "You seem to be unable to track the traces of creatures who have been here." ) );
		return;
	}

	cTrackingMenu *gump = new cTrackingMenu;
	socket->send( gump );
}
