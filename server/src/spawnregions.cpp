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


/*
BE AWARE, the cAllSpawnRegion object is a hybrid of cAllBaseObject and std::map !
I. make sure you establish the recursive structure of the topregion/subregions
and
II. make sure to hold the map up to date !
*/
#include "spawnregions.h"
#include "wpdefmanager.h"
#include "globals.h"
#include "chars.h"
#include "charsmgr.h"
#include "items.h"
#include "itemsmgr.h"
#include "defines.h"
#include "maps.h"
#include "walking.h"
#include "utilsys.h"

#include "junk.h" // needed for objects Npcs and Items

using namespace std;

// cSpawnRegions

void cSpawnRegion::init( void )
{
	cBaseRegion::init();
	npcSections_ = QStringList();
	itemSections_ = QStringList();
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
	if( isItemSerial(serial) /*>= 0x40000000*/ )
		this->itemSerials_.push_back( serial );
	else
		this->npcSerials_.push_back( serial );
}

void cSpawnRegion::processNode( const QDomElement &Tag )
{
	QString TagName = Tag.nodeName();
	QString Value = this->getNodeValue( Tag );

	//<npcs>
	//  <npc mult="2">npcsection</npc> (mult inserts 2 same sections into the list so the probability rises!
	//	<npc><random list="npcsectionlist" /></npc>
	//  <getlist id="npcsectionlist" />
	//</npcs>
	if( TagName == "npcs" )
	{
		QDomNode childNode = Tag.firstChild();
		while( !childNode.isNull() )
		{
			if( !childNode.isElement() )
				this->npcSections_.push_back( Value );
			else if( childNode.nodeName() == "npc" )
			{
				UI32 mult = childNode.toElement().attribute( "mult" ).toInt();
				if( mult < 1 )
					mult = 1;

				for( UI32 i = 0; i < mult; i++ )
					this->npcSections_.push_back( this->getNodeValue( childNode.toElement() ) );
			}
			else if( childNode.nodeName() == "getlist" )
			{
				QString listSect;
				if( childNode.attributes().contains( "id" ) )
					listSect = childNode.toElement().attribute( "id" );
				else
					listSect = getNodeValue( childNode.toElement() );

				QStringList NpcList = DefManager->getList( listSect );
				QStringList::iterator it = NpcList.begin();
				while( it != NpcList.end() )
				{
					this->npcSections_.push_back( *it );
					it++;
				}
			}

			childNode = childNode.nextSibling();
		}
	}

	//<items>
	//  <item>itemsection</item>
	//	<item><random list="itemsectionlist" /></item>
	//  <list id="itemsectionlist" />
	//</items>
	else if( TagName == "items" )
	{
		QDomNode childNode = Tag.firstChild();
		while( !childNode.isNull() )
		{
			if( !childNode.isElement() )
				this->itemSections_.push_back( Value );
			else if( childNode.nodeName() == "item" )
			{
				UI32 mult = childNode.toElement().attribute( "mult" ).toInt();
				if( mult < 1 )
					mult = 1;

				for( UI32 i = 0; i < mult; i++ )
					this->itemSections_.push_back( this->getNodeValue( childNode.toElement() ) );
			}
			else if( childNode.nodeName() == "getlist" )
			{
				QString listSect;
				if( childNode.attributes().contains( "id" ) )
					listSect = childNode.toElement().attribute( "id" );
				else
					listSect = getNodeValue( childNode.toElement() );

				QStringList itemList = DefManager->getList( listSect );
				QStringList::iterator it = itemList.begin();
				while( it != itemList.end() )
				{
					this->itemSections_.push_back( *it );
					it++;
				}
			}

			childNode = childNode.nextSibling();
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
		Tag.attributes().contains( "x1" ) && Tag.attributes().contains( "x2" ) && 
		Tag.attributes().contains( "y1" ) && Tag.attributes().contains( "y2" ) )
	{
		cBaseRegion::processNode( Tag );

		if( Tag.attributes().contains( "z" ) )
			this->z_.push_back( Tag.attribute( "z" ).toUShort() );
		else
			this->z_.push_back( 255 );
	}
	
	else if( TagName == "region" )
	{
		cSpawnRegion* toinsert_ = new cSpawnRegion( Tag );
		this->subregions_.push_back( toinsert_ );
		pair< QString, cSpawnRegion* > toInsert( Tag.attribute( "id" ), toinsert_ );
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
			pos.z = Map->height( pos );

		if( Movement::instance()->canLandMonsterMoveHere( pos ) )
			return true;
		i++;
	}
	clConsole.send( QString("WOLFPACK: A problem has occured in spawnregion %1. Couldn't find valid spot!\n").arg(this->name_) );
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
				P_CHAR pc = cCharStuff::createScriptNpc( -1, NULL, NpcSect, pos.x, pos.y, pos.z );
				if( pc != NULL )
				{
					this->npcSerials_.push_back( pc->serial );
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
				P_ITEM pi = Items->createScriptItem( ItemSect );
				if( pi != NULL )
				{
					pi->pos = pos;
					this->itemSerials_.push_back( pi->serial );
					pi->setSpawnRegion( this->name_ );
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
			QString NpcSect = this->npcSections_[ RandomNum( 1, this->npcSections_.size() ) - 1 ];
			P_CHAR pc = cCharStuff::createScriptNpc( -1, NULL, NpcSect, pos.x, pos.y, pos.z );
			if( pc != NULL )
			{
				this->npcSerials_.push_back( pc->serial );
				pc->setSpawnregion( this->name_ );
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
			P_ITEM pi = Items->createScriptItem( ItemSect );
			if( pi != NULL )
			{
				pi->pos = pos;
				this->itemSerials_.push_back( pi->serial );
				pi->setSpawnRegion( this->name_ );
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
		Items->DeleItem( FindItemBySerial( *it ) );
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
		if( FindCharBySerial( *it ) )
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

// cAllSpawnRegions
cAllSpawnRegions::~cAllSpawnRegions( void )
{
	if( topregion_ )
		delete topregion_;
	// the destructor of cBaseRegion contains the deletion of its subregion!
	// so the regions will be deleted recursively from the stack by this one
	// operation!
}

void cAllSpawnRegions::load( void )
{
	UI32 starttime = getNormalizedTime();
	
	this->clear(); // clear the std::map

	QStringList DefSections = DefManager->getSections( WPDT_SPAWNREGION );
	clConsole.PrepareProgress( "Loading spawn regions" );

	QStringList::iterator it = DefSections.begin();
	while( it != DefSections.end() )
	{
		QDomElement* DefSection = DefManager->getSection( WPDT_SPAWNREGION, *it );

		cSpawnRegion* toinsert_ = new cSpawnRegion( *DefSection );
		pair< QString, cSpawnRegion* > toInsert( *it, toinsert_ );
		this->insert( toInsert );
		this->topregion_ = toinsert_;

		it++;
	}

	AllCharsIterator iter_char;
	for( iter_char.Begin(); !iter_char.atEnd(); iter_char++ )
	{
		P_CHAR pc = iter_char.GetData();
		
		iterator iter_spreg = this->find( pc->spawnregion() );
		if( iter_spreg != this->end() )
			iter_spreg->second->add( pc->serial );
	}

	AllItemsIterator iter_item;
	for( iter_item.Begin(); !iter_item.atEnd(); iter_item++ )
	{
		P_ITEM pi = iter_item.GetData();
		
		iterator iter_spreg = this->find( pi->spawnregion() );
		if( iter_spreg != this->end() )
			iter_spreg->second->add( pi->serial );
	}

	UI32 endtime = getNormalizedTime();
	clConsole.ProgressDone();
	clConsole.send( QString( "Loaded %1 spawnregions in %2 sec.\n" ).arg( DefSections.size() ).arg( (float)((float)endtime - (float)starttime) / MY_CLOCKS_PER_SEC ) );
}

void cAllSpawnRegions::check( void )
{
	iterator it = this->begin();
	while( it != this->end() )
	{
		it->second->checkTimer();
		it++;
	}
}

void cAllSpawnRegions::reSpawn( void )
{
	iterator it = this->begin();
	while( it != this->end() )
	{
		it->second->reSpawn();
		it++;
	}
}

void cAllSpawnRegions::reSpawnToMax( void )
{
	iterator it = this->begin();
	while( it != this->end() )
	{
		it->second->reSpawnToMax();
		it++;
	}
}

void cAllSpawnRegions::deSpawn( void )
{
	iterator it = this->begin();
	while( it != this->end() )
	{
		it->second->deSpawn();
		it++;
	}
}

cSpawnRegion*	cAllSpawnRegions::region( QString regName )
{
	cSpawnRegion* Region = NULL;
	if( this->find( regName ) != this->end() )
		Region = this->find( regName )->second;
	
	return Region;
}

cSpawnRegion*	cAllSpawnRegions::region( UI16 posx, UI16 posy )
{
	if( this->topregion_ )
		return dynamic_cast< cSpawnRegion* >(this->topregion_->region( posx, posy ));
	return NULL;
}
