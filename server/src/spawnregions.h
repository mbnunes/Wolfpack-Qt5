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

#if !defined(__SPAWNREGIONS_H__)
#define __SPAWNREGIONS_H__

#include "baseregion.h"
#include "singleton.h"
#include "server.h"

#include <map>
#include <vector>

class cSpawnRegion : public cBaseRegion
{
public:
	cSpawnRegion( const cElement *Tag )
	{
		this->init();
		this->name_ = Tag->getAttribute( "id" );
		this->applyDefinition( Tag );
	}

	void	init( void );
	void	add( UI32 serial );

	void	reSpawn( void );
	void	deSpawn( void );
	void	reSpawnToMax( void );

	void	checkForDeleted( void );
	void	checkTimer( void );

	bool	findValidSpot( Coord_cl &pos );

	// Getters
	QString name( void )		{ return name_; }
	UI16	npcs( void )		{ return npcSerials_.size(); }
	UI16	items( void )		{ return itemSerials_.size(); }
	UI16	maxNpcs( void )		{ return maxNpcAmt_; }
	UI16	maxItems( void )	{ return maxItemAmt_; }

	QStringList	rectangles( void )
	{
		QStringList rectList;
		QValueVector< rect_st >::iterator it = this->rectangles_.begin();
		while( it != this->rectangles_.end() )
		{
			QString rect = QString( "%1,%2->%3,%4" ).arg( (*it).x1 ).arg( (*it).y1 ).arg( (*it).x2 ).arg( (*it).y2 );
			rectList.push_back( rect );
			++it;
		}
		return rectList;
	}
private:
	virtual void processNode( const cElement *Tag );

private:
	std::vector< SERIAL >		npcSerials_;	// serials of chars spawned by this area
	std::vector< SERIAL >		itemSerials_;	// serials of items spawned by this area

	QStringList				npcSections_;	// list of npc's sections
	QStringList				itemSections_;	// list of item's sections

	UI16					maxNpcAmt_;		// Max amount of characters to spawn
	UI16					maxItemAmt_;	// Max amount of items to spawn

	UI16					npcsPerCycle_;	// amount of characters to spawn per cycle
	UI16					itemsPerCycle_;	// amount of items to spawn per cycle

	UI32					minTime_;		// Minimum spawn time in sec
	UI32					maxTime_;		// Maximum spawn time in sec
	UI32					nextTime_;		// Next time for this region to spawn

	std::vector< UI08 >		z_;				// Height, if not specified, z will be chosen
};

class cAllSpawnRegions : public cAllBaseRegions, public std::map< QString, cSpawnRegion* >, public cComponent
{
public:

	void load( void );
	void unload();
	void check( void );
	void reload();

	cSpawnRegion*	region( const QString& regName );
	cSpawnRegion*	region( UI16 posx, UI16 posy, UI08 map );

	void	reSpawn( void );
	void	reSpawnToMax( void );
	void	deSpawn( void );

	UI16	npcs( void )
	{
		UI16 numNpcs = 0;
		std::map< QString, cSpawnRegion* >::iterator it = this->begin();
		while( it != this->end() )
		{
			numNpcs += it->second->npcs();
			++it;
		}
		return numNpcs;
	}

	UI16	items( void )
	{
		UI16 numItems = 0;
		std::map< QString, cSpawnRegion* >::iterator it = this->begin();
		while( it != this->end() )
		{
			numItems += it->second->items();
			++it;
		}
		return numItems;
	}

	UI16	maxNpcs( void )
	{
		UI16 numNpcs = 0;
		std::map< QString, cSpawnRegion* >::iterator it = this->begin();
		while( it != this->end() )
		{
			numNpcs += it->second->maxNpcs();
			++it;
		}
		return numNpcs;
	}

	UI16	maxItems( void )
	{
		UI16 numItems = 0;
		std::map< QString, cSpawnRegion* >::iterator it = this->begin();
		while( it != this->end() )
		{
			numItems += it->second->maxItems();
			++it;
		}
		return numItems;
	}
};

typedef SingletonHolder<cAllSpawnRegions> SpawnRegions;

#endif

