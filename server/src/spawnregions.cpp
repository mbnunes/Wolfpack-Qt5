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

#include "spawnregions.h"

#include "definitions.h"
#include "items.h"
#include "defines.h"
#include "muls/maps.h"
#include "walking.h"
#include "log.h"
#include "npc.h"
#include "world.h"
#include "basics.h"
#include "console.h"
#include "sectors.h"
#include "serverconfig.h"
#include "inlines.h"
#include "scriptmanager.h"
#include "python/pyspawnregion.h"

using namespace std;

/*****************************************************************************
	cSpawnRegion member functions
*****************************************************************************/

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

void cSpawnRegion::add(cUObject *object)
{
	if (object->isItem()) {
		items_.append(object);
	} else if (object->isChar()) {
		npcs_.append(object);
	}
}

void cSpawnRegion::remove(cUObject *object)
{
	if (object->isItem()) {
		items_.remove(object);
	} else if (object->isChar()) {
		npcs_.remove(object);
	}
}

void cSpawnRegion::processNode( const cElement* Tag )
{
	QString TagName(Tag->name());
	QString Value(Tag->value());

	//<npcs>
	//  <npc mult="2">npcsection</npc> (mult inserts 2 same sections into the list so the probability rises!
	//	<npc><random list="npcsectionlist" /></npc>
	//  <getlist id="npcsectionlist" />
	//</npcs>
	if ( TagName == "npcs" )
	{
		for ( unsigned int i = 0; i < Tag->childCount(); ++i )
		{
			const cElement* childNode = Tag->getChild( i );

			if ( childNode->name() == "npc" )
			{
				UI32 mult = childNode->getAttribute( "mult" ).toInt();
				if ( mult < 1 )
					mult = 1;

				for ( UI32 i = 0; i < mult; i++ )
					npcSections_.push_back( childNode->value() );
			}
			else if ( childNode->name() == "getlist" )
			{
				QString listSect;
				if ( childNode->hasAttribute( "id" ) )
					listSect = childNode->getAttribute( "id" );
				else
					listSect = childNode->value();

				QStringList NpcList = Definitions::instance()->getList( listSect );
				QStringList::const_iterator it(NpcList.begin());
				for ( ; it != NpcList.end(); ++it )
					this->npcSections_.push_back( *it );
			}
		}
	}

	//<items>
	//  <item>itemsection</item>
	//	<item><random list="itemsectionlist" /></item>
	//  <list id="itemsectionlist" />
	//</items>
	else if ( TagName == "items" )
	{
		for ( unsigned int i = 0; i < Tag->childCount(); ++i )
		{
			const cElement* childNode = Tag->getChild( i );

			if ( childNode->name() == "item" )
			{
				UI32 mult = childNode->getAttribute( "mult" ).toInt();
				if ( mult < 1 )
					mult = 1;

				for ( UI32 i = 0; i < mult; i++ )
					this->itemSections_.push_back( childNode->value() );
			}
			else if ( childNode->name() == "getlist" )
			{
				QString listSect;
				if ( childNode->hasAttribute( "id" ) )
					listSect = childNode->getAttribute( "id" );
				else
					listSect = childNode->value();

				QStringList itemList = Definitions::instance()->getList( listSect );
				QStringList::const_iterator it(itemList.begin());
				for ( ; it != itemList.end(); ++it )
					this->itemSections_.push_back( *it );
			}
		}
	}

	// <maxnpcamount>10</maxnpcamount>
	else if ( TagName == "maxnpcamount" )
		this->maxNpcAmt_ = Value.toUShort();

	// <maxitemamount>5</maxitemamount>
	else if ( TagName == "maxitemamount" )
		this->maxItemAmt_ = Value.toUShort();

	// <npcspercycle>3</npcspercycle>
	else if ( TagName == "npcspercycle" )
		this->npcsPerCycle_ = Value.toUShort();

	// <itemspercycle>3</itemspercycle>
	else if ( TagName == "itemspercycle" )
		this->itemsPerCycle_ = Value.toUShort();

	// <mintime>10</mintime>
	else if ( TagName == "mintime" )
		this->minTime_ = Value.toInt();

	// <maxtime>20</maxtime>
	else if ( TagName == "maxtime" )
		this->maxTime_ = Value.toInt();

	// <rectangle x1="0" x2="1000" y1="0" y2="500" z="5" />
	else if ( TagName == "rectangle" && Tag->hasAttribute( "x1" ) && Tag->hasAttribute( "x2" ) && Tag->hasAttribute( "y1" ) && Tag->hasAttribute( "y2" ) )
	{
		cBaseRegion::processNode( Tag );

		if ( Tag->hasAttribute( "z" ) )
			this->z_.push_back( Tag->getAttribute( "z" ).toUShort() );
		else
			this->z_.push_back( 255 );
	}
	else if ( TagName == "region" )
	{
		cSpawnRegion* toinsert_ = new cSpawnRegion( Tag );
		this->subregions_.push_back( toinsert_ );
		pair<QString, cSpawnRegion*> toInsert( Tag->getAttribute( "id" ), toinsert_ );
		SpawnRegions::instance()->insert( toInsert );
	}
	else
		cBaseRegion::processNode( Tag );
}

bool cSpawnRegion::findValidSpot(Coord_cl& pos) {
	// Try up to 100 times.
	for(unsigned int i = 0; i < 100; ++i)
	{
		int rndRectNum = RandomNum( 0, this->rectangles_.size() - 1 );
		pos.x = RandomNum( this->rectangles_[rndRectNum].x1, this->rectangles_[rndRectNum].x2 );
		pos.y = RandomNum( this->rectangles_[rndRectNum].y1, this->rectangles_[rndRectNum].y2 );
		if ( this->z_[rndRectNum] != 255 )
			pos.z = this->z_[rndRectNum];
		else
			pos.z = Maps::instance()->mapElevation( pos );
		pos.map = rectangles_[rndRectNum].map;

		if (Movement::instance()->canLandMonsterMoveHere(pos)) {
			if (!Config::instance()->dontStackSpawnedObjects()) {
				return true;
			}

			// Check if there are spawned items or npcs at the position.
			cCharSectorIterator *chariterator = SectorMaps::instance()->findChars(pos, 0);

			bool blocked = false;
			P_CHAR pChar;
			for (pChar = chariterator->first(); pChar; pChar = chariterator->next()) {
				if (pChar->spawnregion()) {
					blocked = true;
					break;
				}
			}

			if (blocked) {
				continue;
			}

			cItemSectorIterator *itemiterator = SectorMaps::instance()->findItems(pos, 0);

			P_ITEM pItem;
			for (pItem = itemiterator->first(); pItem; pItem = itemiterator->next()) {
				if (pItem->spawnregion()) {
					blocked = true;
					break;
				}
			}

			if (blocked) {
				continue;
			}

			return true;
		}
	}

	Console::instance()->log( LOG_WARNING, tr( "A problem has occured in spawnregion %1. Couldn't find valid spot." ).arg( this->name_ ) );
	return false;
}

void cSpawnRegion::spawnSingleNPC()
{
	Coord_cl pos;
	if (findValidSpot(pos))
	{
		QString NpcSect = this->npcSections_[RandomNum( 1, this->npcSections_.size() ) - 1];
		P_NPC pc = cNPC::createFromScript( NpcSect, pos );
		if (pc)
		{
			pc->setSpawnregion(this);
			if (pc->wanderType() == enFreely)
			{
				pc->setWanderType(enWanderSpawnregion);
			}
			pc->update();
			onSpawn( pc );
		}
	}
}

void cSpawnRegion::spawnSingleItem()
{
	Coord_cl pos;
	if ( findValidSpot( pos ) )
	{
		QString ItemSect = this->itemSections_[RandomNum( 1, this->itemSections_.size() ) - 1];
		P_ITEM pi = cItem::createFromScript( ItemSect );
		if ( pi )
		{
			pi->moveTo(pos, true);
			pi->setSpawnregion(this);
			pi->update();
			onSpawn( pi );
		}
	}
}

void cSpawnRegion::onSpawn(cUObject* obj)
{
	cPythonScript* global = ScriptManager::instance()->getGlobalHook( EVENT_CREATE );

	if ( global )
	{
		PyObject* args = Py_BuildValue( "NN", PyGetSpawnRegionObject(this), PyGetObjectObject(obj) );

		global->callEventHandler( EVENT_SPAWN, args );

		Py_DECREF( args );
	}
}

// do one spawn and reset the timer
void cSpawnRegion::reSpawn( void )
{
	unsigned int i = 0;
	for (i = 0; i < npcsPerCycle_; ++i)
		if (npcs() < maxNpcAmt_)
			spawnSingleNPC(); // spawn a random npc

	for ( i = 0; i < this->itemsPerCycle_; i++ )
		if ( items() < this->maxItemAmt_ )
			spawnSingleItem(); // spawn a random item

	this->nextTime_ = Server::instance()->time() + RandomNum( this->minTime_, this->maxTime_ ) * MY_CLOCKS_PER_SEC;
}

void cSpawnRegion::reSpawnToMax( void )
{
	while ( npcs() < maxNpcAmt_ )
		spawnSingleNPC();

	while (items() < maxItemAmt_)
		spawnSingleItem();

	this->nextTime_ = Server::instance()->time() + RandomNum( this->minTime_, this->maxTime_ ) * MY_CLOCKS_PER_SEC;
}

// delete all spawns and reset the timer
void cSpawnRegion::deSpawn( void )
{
	QPtrList<cUObject> items = items_; // Copy
	QPtrList<cUObject> npcs = npcs_; // Copy
	cUObject *object;

	for (object = items.first(); object; object = items.next()) {
		object->remove();
	}

	for (object = npcs.first(); object; object = npcs.next()) {
		object->remove();
	}

	nextTime_ = Server::instance()->time() + RandomNum(minTime_, maxTime_) * MY_CLOCKS_PER_SEC;
}

// check the timer and if expired do reSpawn
void cSpawnRegion::checkTimer( void )
{
	if ( this->nextTime_ <= Server::instance()->time() )
		this->reSpawn();
}


/*****************************************************************************
	cAllSpawnRegions member functions
*****************************************************************************/


void cAllSpawnRegions::check( void )
{
	iterator it( this->begin() );
	while ( it != this->end() )
	{
		it->second->checkTimer();
		++it;
	}
}

void cAllSpawnRegions::reSpawn( void )
{
	iterator it( this->begin() );
	while ( it != this->end() )
	{
		it->second->reSpawn();
		++it;
	}
}

void cAllSpawnRegions::reSpawnToMax( void )
{
	iterator it( this->begin() );
	while ( it != this->end() )
	{
		it->second->reSpawnToMax();
		++it;
	}
}

void cAllSpawnRegions::deSpawn( void )
{
	iterator it( this->begin() );
	while ( it != this->end() )
	{
		it->second->deSpawn();
		++it;
	}
}

cSpawnRegion* cAllSpawnRegions::region( const QString& regName )
{
	iterator it( find( regName ) );
	if ( it != this->end() )
		return it->second;
	else
		return 0;
}

void cAllSpawnRegions::load()
{
	this->clear(); // clear the std::map

	QStringList DefSections = Definitions::instance()->getSections( WPDT_SPAWNREGION );

	QStringList::iterator it = DefSections.begin();
	while ( it != DefSections.end() )
	{
		const cElement* DefSection = Definitions::instance()->getDefinition( WPDT_SPAWNREGION, *it );

		cSpawnRegion* toinsert_ = new cSpawnRegion( DefSection );
		this->insert( make_pair( *it, toinsert_ ) );
		++it;
	}
	cComponent::load();
}

void cAllSpawnRegions::reload()
{
	// Save a list of all objects and their spawnregions
	// So the references can be recreated later.
	QMap<QString, QPtrList<cUObject> > objects;

	cItemIterator iItems;
	for (P_ITEM pItem = iItems.first(); pItem; pItem = iItems.next()) {
		cSpawnRegion *region = pItem->spawnregion();
		if (region)
		{
			if (!objects.contains(region->name()))
			{
				objects[region->name()].setAutoDelete(false);
			}

			objects[region->name()].append(pItem);
			pItem->setSpawnregion(0); // Remove from spawnregion before pointer gets invalid
		}
	}

	cCharIterator iChars;
	for (P_CHAR pChar = iChars.first(); pChar; pChar = iChars.next()) {
		cSpawnRegion *region = pChar->spawnregion();
		if (region)
		{
			if (!objects.contains(region->name()))
			{
				objects[region->name()].setAutoDelete(false);
			}

			objects[region->name()].append(pChar);
			pChar->setSpawnregion(0); // Remove from spawnregion before pointer gets invalid
		}
	}

	unload();
	load();

	QMap<QString, QPtrList<cUObject> >::iterator it;
	for (it = objects.begin(); it != objects.end(); ++it)
	{
		cSpawnRegion *region = this->region(it.key());
		if (region)
		{
			cUObject *object;
			QPtrList<cUObject> &list = it.data();
			for (object = list.first(); object; object = list.next())
			{
				object->setSpawnregion(region);
			}
		}
	}
}

void cAllSpawnRegions::unload()
{
	iterator it( begin() );
	for ( ; it != end(); ++it )
		delete it->second;

	clear();
	cComponent::unload();
}
