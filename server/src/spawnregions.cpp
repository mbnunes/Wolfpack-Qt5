//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
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


/*
BE AWARE, the cAllSpawnRegion object is a hybrid of cAllBaseObject and std::map !
I. make sure you establish the recursive structure of the topregion/subregions
and
II. make sure to hold the map up to date !
*/
#include "spawnregions.h"
#include "wpdefmanager.h"
#include "globals.h"
#include "items.h"
#include "defines.h"
#include "maps.h"
#include "walking.h"
#include "log.h"
#include "chars.h"
#include "npc.h"
#include "world.h"
#include "basics.h"
#include "console.h"

using namespace std;

// cSpawnRegions

void cSpawnRegion::init( void )
{
	cBaseRegion::init();
	maxNpcAmt_ = 0;
	maxItemAmt_ = 0;
	npcsPerCycle_ = 1;
	itemsPerCycle_ = 1;
	minTime_ = 0;
	maxTime_ = 600;
	nextTime_ = 0;
}

void cSpawnRegion::add( UI32 serial )
{
	if( isItemSerial(serial) )
		this->itemSerials_.push_back( serial );
	else
		this->npcSerials_.push_back( serial );
}

void cSpawnRegion::processNode( const cElement *Tag )
{
	QString TagName = Tag->name();
	QString Value = Tag->getValue();

	//<npcs>
	//  <npc mult="2">npcsection</npc> (mult inserts 2 same sections into the list so the probability rises!
	//	<npc><random list="npcsectionlist" /></npc>
	//  <getlist id="npcsectionlist" />
	//</npcs>
	if( TagName == "npcs" )
	{
		for( unsigned int i = 0; i < Tag->childCount(); ++i )
		{
			const cElement* childNode = Tag->getChild( i ); 

			if( childNode->name() == "npc" )
			{
				UI32 mult = childNode->getAttribute( "mult" ).toInt();
				if( mult < 1 )
					mult = 1;

				for( UI32 i = 0; i < mult; i++ )
					npcSections_.push_back( childNode->getValue() );
			}
			else if( childNode->name() == "getlist" )
			{
				QString listSect;
				if( childNode->hasAttribute( "id" ) )
					listSect = childNode->getAttribute( "id" );
				else
					listSect = childNode->getValue();

				QStringList NpcList = DefManager->getList( listSect );
				QStringList::iterator it = NpcList.begin();
				while( it != NpcList.end() )
				{
					this->npcSections_.push_back( *it );
					it++;
				}
			}
		}
	}

	//<items>
	//  <item>itemsection</item>
	//	<item><random list="itemsectionlist" /></item>
	//  <list id="itemsectionlist" />
	//</items>
	else if( TagName == "items" )
	{
		for( unsigned int i = 0; i < Tag->childCount(); ++i )
		{
			const cElement *childNode = Tag->getChild( i );

			if( childNode->name() == "item" )
			{
				UI32 mult = childNode->getAttribute( "mult" ).toInt();
				if( mult < 1 )
					mult = 1;

				for( UI32 i = 0; i < mult; i++ )
					this->itemSections_.push_back( childNode->getValue() );
			}
			else if( childNode->name() == "getlist" )
			{
				QString listSect;
				if( childNode->hasAttribute( "id" ) )
					listSect = childNode->getAttribute( "id" );
				else
					listSect = childNode->getValue();

				QStringList itemList = DefManager->getList( listSect );
				QStringList::iterator it = itemList.begin();
				while( it != itemList.end() )
				{
					this->itemSections_.push_back( *it );
					it++;
				}
			}
		}
	}

	// <maxnpcamount>10</maxnpcamount>
	else if( TagName == "maxnpcamount" )
		this->maxNpcAmt_ = Value.toUShort();

	// <maxitemamount>5</maxitemamount>
	else if( TagName == "maxitemamount" )
		this->maxItemAmt_ = Value.toUShort();

	// <npcspercycle>3</npcspercycle>
	else if( TagName == "npcspercycle" )
		this->npcsPerCycle_ = Value.toUShort();

	// <itemspercycle>3</itemspercycle>
	else if( TagName == "itemspercycle" )
		this->itemsPerCycle_ = Value.toUShort();

	// <mintime>10</mintime>
	else if( TagName == "mintime" )
		this->minTime_ = Value.toInt();

	// <maxtime>20</maxtime>
	else if( TagName == "maxtime" )
		this->maxTime_ = Value.toInt();

	// <rectangle x1="0" x2="1000" y1="0" y2="500" z="5" />
	else if( TagName == "rectangle" && 
		Tag->hasAttribute( "x1" ) && Tag->hasAttribute( "x2" ) && 
		Tag->hasAttribute( "y1" ) && Tag->hasAttribute( "y2" ) )
	{
		cBaseRegion::processNode( Tag );

		if( Tag->hasAttribute( "z" ) )
			this->z_.push_back( Tag->getAttribute( "z" ).toUShort() );
		else
			this->z_.push_back( 255 );
	}
	
	else if( TagName == "region" )
	{
		cSpawnRegion* toinsert_ = new cSpawnRegion( Tag );
		this->subregions_.push_back( toinsert_ );
		pair< QString, cSpawnRegion* > toInsert( Tag->getAttribute( "id" ), toinsert_ );
		SpawnRegions::instance()->insert( toInsert );
	}

	else
		cBaseRegion::processNode( Tag );
}

bool cSpawnRegion::findValidSpot( Coord_cl &pos )
{
	UI32 i = 0;
	while( i < 100 )
	{
		int rndRectNum = RandomNum( 0, this->rectangles_.size()-1 );
		pos.x = RandomNum( this->rectangles_[rndRectNum].x1, this->rectangles_[rndRectNum].x2 );
		pos.y = RandomNum( this->rectangles_[rndRectNum].y1, this->rectangles_[rndRectNum].y2 );
		if( this->z_[rndRectNum] != 255 )
			pos.z = this->z_[rndRectNum];
		else
			pos.z = Map->mapElevation( pos );

		pos.map = rectangles_[rndRectNum].map;

		if( Movement::instance()->canLandMonsterMoveHere( pos ) )
			return true;
		i++;
	}

	Console::instance()->log( LOG_WARNING, QString( "A problem has occured in spawnregion %1. Couldn't find valid spot." ).arg( this->name_ ) );
	return false;
}
	
// do one spawn and reset the timer
void cSpawnRegion::reSpawn( void )
{
	this->checkForDeleted();

	UI16 i = 0;
	for( i = 0; i < this->npcsPerCycle_; i++ )
	{
		if( this->npcSerials_.size() < this->maxNpcAmt_ )
		{
			// spawn a random npc
			// first find a valid position for the npc
			Coord_cl pos;
			if( this->findValidSpot( pos ) )
			{
				QString NpcSect = this->npcSections_[ RandomNum( 1, this->npcSections_.size() ) - 1 ];
				P_NPC pc = cCharStuff::createScriptNpc( NpcSect, pos );
				if( pc != NULL )
				{
					this->npcSerials_.push_back( pc->serial() );
					pc->setSpawnregion( this->name_ );
				}
			}
		}
	}

	for( i = 0; i < this->itemsPerCycle_; i++ )
	{
		if( this->itemSerials_.size() < this->maxItemAmt_ )
		{
			// spawn a random item
			// first find a valid position for the item
			Coord_cl pos;
			if( this->findValidSpot( pos ) )
			{
				QString ItemSect = this->itemSections_[ RandomNum( 1, this->itemSections_.size() ) - 1 ];
				P_ITEM pi = cItem::createFromScript( ItemSect );
				if( pi != NULL )
				{
					pi->moveTo( pos );
					this->itemSerials_.push_back( pi->serial() );
//					pi->setSpawnRegion( this->name_ );
				}
			}
		}
	}

	this->nextTime_ = uiCurrentTime + RandomNum( this->minTime_, this->maxTime_ ) * MY_CLOCKS_PER_SEC;
}

void cSpawnRegion::reSpawnToMax( void )
{
	this->checkForDeleted();

	while( this->npcSerials_.size() < this->maxNpcAmt_ )
	{
		// spawn a random npc
		// first find a valid position for the npc
		Coord_cl pos;
		if( this->findValidSpot( pos ) )
		{
			QString NpcSect = this->npcSections_[ RandomNum( 1, static_cast<uint>(this->npcSections_.size()) ) - 1 ];
			P_NPC pc = cCharStuff::createScriptNpc( NpcSect, pos );
			if( pc != NULL )
			{
				this->npcSerials_.push_back( pc->serial() );
				pc->setSpawnregion( this->name_ );
				pc->update();
			}
		}
	}

	while( this->npcSerials_.size() < this->maxNpcAmt_ )
	{
		// spawn a random item
		// first find a valid position for the item
		Coord_cl pos;
		if( this->findValidSpot( pos ) )
		{
			QString ItemSect = this->itemSections_[ RandomNum( 1, this->itemSections_.size() ) - 1 ];
			P_ITEM pi = cItem::createFromScript( ItemSect );
			if( pi != NULL )
			{
				pi->setPos( pos );
				this->itemSerials_.push_back( pi->serial() );
//				pi->setSpawnRegion( this->name_ );
			}
		}
	}

	this->nextTime_ = uiCurrentTime + RandomNum( this->minTime_, this->maxTime_ ) * MY_CLOCKS_PER_SEC;
}

// delete all spawns and reset the timer
void cSpawnRegion::deSpawn( void )
{
	std::vector< SERIAL >::iterator it = this->npcSerials_.begin();

	while( it != this->npcSerials_.end() )
	{
		cCharStuff::DeleteChar( FindCharBySerial( *it ) );
		it++;
	}
	npcSerials_.erase( npcSerials_.begin(), npcSerials_.end() );

	it = this->itemSerials_.begin();
	while( it != this->itemSerials_.end() )
	{
		FindItemBySerial( *it )->remove();
		it++;
	}
	itemSerials_.erase( itemSerials_.begin(), itemSerials_.end() );

	this->nextTime_ = uiCurrentTime + RandomNum( this->minTime_, this->maxTime_ ) * MY_CLOCKS_PER_SEC;
}

void cSpawnRegion::checkForDeleted( void )
{
	std::vector< SERIAL > foundSerials;
	std::vector< SERIAL >::iterator it = this->npcSerials_.begin();
	while( it != this->npcSerials_.end() )
	{
		P_NPC pChar = dynamic_cast<P_NPC>( FindCharBySerial( *it ) );
		if( pChar && !pChar->free && pChar->spawnregion() == name_ )
			foundSerials.push_back( (*it) );
		it++;
	}
	npcSerials_ = foundSerials;

	foundSerials.clear();
	it = this->itemSerials_.begin();
	while( it != this->itemSerials_.end() )
	{
		if( FindItemBySerial( *it ) )
			foundSerials.push_back( (*it) );
		it++;
	}
	itemSerials_ = foundSerials;
}

// check the timer and if expired do reSpawn
void cSpawnRegion::checkTimer( void )
{
	if( this->nextTime_ <= uiCurrentTime )
		this->reSpawn();
}


void cAllSpawnRegions::load( void )
{
	this->clear(); // clear the std::map

	QStringList DefSections = DefManager->getSections( WPDT_SPAWNREGION );

	QStringList::iterator it = DefSections.begin();
	while( it != DefSections.end() )
	{
		const cElement* DefSection = DefManager->getDefinition( WPDT_SPAWNREGION, *it );

		cSpawnRegion* toinsert_ = new cSpawnRegion( DefSection );
		this->insert( make_pair(*it, toinsert_) );
		if ( toinsert_->cBaseRegion::rectangles().empty() )
		{
			Console::instance()->log( LOG_WARNING, QString( "Top level spawnregion %1 lacks rectangle tag, ignoring region." ).arg( toinsert_->name() ) );
			delete toinsert_;
		}
		else
		{
			topregions.insert( toinsert_->cBaseRegion::rectangles()[0].map, toinsert_ );
		}

		++it;
	}
}

void cAllSpawnRegions::check( void )
{
	iterator it(this->begin());
	while( it != this->end() )
	{
		it->second->checkTimer();
		++it;
	}
}

void cAllSpawnRegions::reSpawn( void )
{
	iterator it(this->begin());
	while( it != this->end() )
	{
		it->second->reSpawn();
		++it;
	}
}

void cAllSpawnRegions::reSpawnToMax( void )
{
	iterator it(this->begin());
	while( it != this->end() )
	{
		it->second->reSpawnToMax();
		++it;
	}
}

void cAllSpawnRegions::deSpawn( void )
{
	iterator it(this->begin());
	while( it != this->end() )
	{
		it->second->deSpawn();
		++it;
	}
}

cSpawnRegion*	cAllSpawnRegions::region( const QString& regName )
{
	iterator it(find( regName ) );
	if( it != this->end() )
		return it->second;
	else
		return 0;
}

cSpawnRegion*	cAllSpawnRegions::region( UI16 posx, UI16 posy, UI08 map )
{
	QMap<uint, cBaseRegion*>::const_iterator it( topregions.find(map) );
	if ( it != topregions.end() )
		return dynamic_cast< cSpawnRegion* >(it.data()->region( posx, posy, map ));
	else
		return 0;
}

void cAllSpawnRegions::postWorldLoading()
{
	cCharIterator iChars;
	for( P_CHAR pChar = iChars.first(); pChar; pChar = iChars.next() )
	{
		if( pChar->objectType() == enNPC )
		{
			P_NPC pNPC = dynamic_cast< P_NPC >(pChar);
			QString srname = pNPC->spawnregion();
			if( !srname.isNull() )
			{
				cSpawnRegion* spawnregion = region( srname );
				if( spawnregion )
					spawnregion->add( pNPC->serial() );
			}
		}
	}

	cItemIterator iItems;
	for( P_ITEM pItem = iItems.first(); pItem; pItem = iItems.next() )
	{
		QString srname = pItem->spawnregion();
		if( !srname.isNull() )
		{
			cSpawnRegion* spawnregion = region( srname );
			if( spawnregion )
				spawnregion->add( pItem->serial() );
		}
	}
}

