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

#include "spawnregions.h"
#include "wpdefmanager.h"
#include "globals.h"
#include "chars.h"
#include "charsmgr.h"
#include "items.h"
#include "itemsmgr.h"
#include "defines.h"
#include "mapstuff.h"
#include "walking2.h"

#include "junk.h" // needed for objects Npcs and Items

// cSpawnRegions

cSpawnRegion::cSpawnRegion( const QDomElement &Tag )
{
	this->Init();
	this->name_ = Tag.parentNode().toElement().attribute( "id" );
	this->applyDefinition( Tag );
}

void cSpawnRegion::Init( void )
{
	name_ = QString();
	npcSections_ = QStringList();
	itemSections_ = QStringList();
	maxNpcAmt_ = 0;
	maxItemAmt_ = 0;
	npcsPerCycle_ = 0;
	itemsPerCycle_ = 0;
	minTime_ = 0;
	maxTime_ = 0;
	nextTime_ = 0;
	x1_ = 0;
	x2_ = 0;
	y1_ = 0;
	y2_ = 0;
	z_ = 255;
}

void cSpawnRegion::Add( UI32 serial )
{
	if( serial >= 0x40000000 )
		this->itemSerials_.push_back( serial );
	else
		this->npcSerials_.push_back( serial );
}

void cSpawnRegion::processNode( const QDomElement &Tag )
{
	QString TagName = Tag.nodeName();
	QString Value = this->getNodeValue( Tag );

	//<npcs>
	//  <npc>npcsection</npc>
	//	<npc><random list="npcsectionlist" /></npc>
	//  <list id="npcsectionlist" />
	//</npcs>
	if( TagName == "npcs" )
	{
		QDomNode childNode = Tag.firstChild();
		while( !childNode.isNull() )
		{
			if( !childNode.isElement() )
				this->npcSections_.push_back( Value );
			else if( childNode.nodeName() == "npc" )
				this->npcSections_.push_back( this->getNodeValue( childNode.toElement() ) );
			else if( childNode.nodeName() == "list" && childNode.attributes().contains( "id" ) )
			{
				QStringList NpcList = DefManager->getList( childNode.toElement().attribute( "id" ) );
				QStringList::iterator it = NpcList.begin();
				while( it != NpcList.end() )
				{
					this->npcSections_.push_back( *it );
					it++;
				}
			}

			childNode.nextSibling();
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
				this->itemSections_.push_back( this->getNodeValue( childNode.toElement() ) );
			else if( childNode.nodeName() == "list" && childNode.attributes().contains( "id" ) )
			{
				QStringList itemList = DefManager->getList( childNode.toElement().attribute( "id" ) );
				QStringList::iterator it = itemList.begin();
				while( it != itemList.end() )
				{
					this->itemSections_.push_back( *it );
					it++;
				}
			}

			childNode.nextSibling();
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
	else if( TagName == "maxnpcamount" )
		this->maxTime_ = Value.toInt();

	// <region x1="0" x2="1000" y1="0" y2="500" z="5" />
	else if( TagName == "region" && 
		Tag.attributes().contains( "x1" ) && Tag.attributes().contains( "x2" ) && 
		Tag.attributes().contains( "y1" ) && Tag.attributes().contains( "y2" ) )
	{
		this->x1_ = Tag.attribute( "x1" ).toUShort();
		this->x2_ = Tag.attribute( "x2" ).toUShort();
		this->y1_ = Tag.attribute( "y1" ).toUShort();
		this->y2_ = Tag.attribute( "y2" ).toUShort();

		if( Tag.attributes().contains( "z" ) )
			this->z_ = Tag.attribute( "z" ).toUShort();
	}
}

bool cSpawnRegion::findValidSpot( Coord_cl &pos )
{
	UI32 i = 0;
	while( i < 100 )
	{
		pos.x = RandomNum( this->x1_, this->x2_ );
		pos.y = RandomNum( this->y1_, this->y2_ );
		if( this->z_ != 255 )
			pos.z = this->z_;
		else
			pos.z = Map->MapElevation( pos );

		if( Map->CanMonsterMoveHere( pos ) )
			return true;
		i++;
	}
	clConsole.send( QString("WOLFPACK: A problem has occured in spawnregion %1. Couldn't find valid spot!\n").arg(this->name_).latin1() );
	return false;
}
	
// do one spawn and reset the timer
void cSpawnRegion::reSpawn( void )
{
	UI16 i = 0;
	for( i = 0; i < this->npcsPerCycle_; i++ )
	{
		if( (this->npcSerials_.size()+1) < this->maxNpcAmt_ )
		{
			// spawn a random npc
			// first find a valid position for the npc
			Coord_cl pos;
			if( this->findValidSpot( pos ) )
			{
				QString NpcSect = this->npcSections_[ RandomNum( 0, this->npcSections_.size() ) ];
				P_CHAR pc = Npcs->createScriptNpc( -1, NULL, NpcSect, pos.x, pos.y, pos.z );
				if( pc != NULL )
					this->npcSerials_.push_back( pc->serial );
			}
		}
	}

	for( i = 0; i < this->itemsPerCycle_; i++ )
	{
		if( (this->itemSerials_.size()+1) < this->maxItemAmt_ )
		{
			// spawn a random item
			// first find a valid position for the item
			Coord_cl pos;
			if( this->findValidSpot( pos ) )
			{
				QString ItemSect = this->itemSections_[ RandomNum( 0, this->itemSections_.size() ) ];
				P_ITEM pi = Items->createScriptItem( ItemSect );
				if( pi != NULL )
				{
					pi->pos = pos;
					this->itemSerials_.push_back( pi->serial );
				}
			}
		}
	}

	this->nextTime_ = uiCurrentTime + RandomNum( this->minTime_, this->maxTime_ ) * MY_CLOCKS_PER_SEC;
}

// delete all spawns and reset the timer
void cSpawnRegion::deSpawn( void )
{
	std::vector< UI32 >::iterator it = this->npcSerials_.begin();

	while( it != this->npcSerials_.end() )
	{
		Npcs->DeleteChar( FindCharBySerial( *it ) );
		this->npcSerials_.erase( it );
		it++;
	}

	while( it != this->itemSerials_.end() )
	{
		Items->DeleItem( FindItemBySerial( *it ) );
		this->itemSerials_.erase( it );
		it++;
	}

	this->nextTime_ = uiCurrentTime + RandomNum( this->minTime_, this->maxTime_ ) * MY_CLOCKS_PER_SEC;
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
	iterator it = this->begin();
	while( it != this->end() )
	{
		delete it->second; // delete the cSpawnRegion objects from the stack!
		it++;
	}
}

void cAllSpawnRegions::Load( void )
{
	QStringList DefSections = DefManager->getSections( WPDT_SPAWNREGION );

	QStringList::iterator it = DefSections.begin();
	while( it != DefSections.end() )
	{
		QDomElement* DefSection = DefManager->getSection( WPDT_SPAWNREGION, *it );

		pair< QString, cSpawnRegion* > toInsert( *it, new cSpawnRegion( *DefSection ) );
		this->insert( toInsert );

		it++;
	}

	AllCharsIterator iter_char;
	for( iter_char.Begin(); !iter_char.atEnd(); iter_char++ )
	{
		P_CHAR pc = iter_char.GetData();
		
		iterator iter_spreg = this->find( pc->spawnregion() );
		if( iter_spreg != this->end() )
			iter_spreg->second->Add( pc->serial );
	}

	AllItemsIterator iter_item;
	for( iter_item.Begin(); !iter_item.atEnd(); iter_item++ )
	{
		P_ITEM pi = iter_item.GetData();
		
		iterator iter_spreg = this->find( pi->spawnregion() );
		if( iter_spreg != this->end() )
			iter_spreg->second->Add( pi->serial );
	}
}

void cAllSpawnRegions::Check( void )
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

void cAllSpawnRegions::deSpawn( void )
{
	iterator it = this->begin();
	while( it != this->end() )
	{
		it->second->deSpawn();
		it++;
	}
}

