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
#include "objectdef.h"

#include <map>

// Abstract class for spawn position
class cSpawnPosition {
protected:
	unsigned int points_; // The number of points
public:
	virtual unsigned int points() {
		return points_;
	}

	virtual Coord_cl findSpot() = 0;
	virtual bool inBounds(const Coord_cl &pos) = 0;
};

class cSpawnRegion : public cDefinable {
	OBJECTDEF(cSpawnRegion)
public:
	cSpawnRegion(const cElement* tag);
	~cSpawnRegion();

	// Manage spawned objects
	void add( cUObject* object );
	void remove( cUObject* object );

	void reSpawn( void );
	void deSpawn( void );
	void reSpawnToMax( void );
	void checkTimer( void );

	unsigned int countPoints();
	bool isValidSpot( const Coord_cl& pos );
	bool findValidSpot( Coord_cl& pos, int tries = -1 );

	// Getters
	const QString &id() const {
		return id_;
	}

	unsigned int npcs( void ) const {
		return npcs_.count();
	}

	unsigned int items( void ) const {
		return items_.count();
	}

	UI16 maxNpcs( void ) const
	{
		return maxNpcAmt_;
	}

	UI16 maxItems( void ) const
	{
		return maxItemAmt_;
	}

	QPtrList<cUObject> spawnedItems() const
	{
		return items_;
	}

	QPtrList<cUObject> spawnedNpcs() const
	{
		return npcs_;
	}

	const QStringList &groups() const {
		return groups_;
	}

	bool active() const {
		return active_;
	}

	inline void setActive(bool value) {
		active_ = value;
	}

	inline unsigned int nextTime() {
		return nextTime_;
	}

private:
	virtual void processNode( const cElement* Tag );
	void spawnSingleNPC();
	void spawnSingleItem();
	void onSpawn( cUObject* );

private:
	QString id_; // Spawnregion id
	QStringList groups_; // Spawngroups
	bool active_; // Is this spawnregion active?

	QPtrList<cSpawnPosition> positions_; // Spawn positions

	bool checkFreeSpot_; // The target spot has to be free.

	QPtrList<cUObject> items_; // List of spawned items
	QPtrList<cUObject> npcs_; // List of spawned npcs
    
	QPtrList<cElement> itemNodes_; // Pointers to definition elements for NPCs
	QPtrList<cElement> npcNodes_; // Pointers to definition elements for Items
	QValueList<unsigned int> itemNodeFrequencies_; // Frequencies for item nodes
	QValueList<unsigned int> npcNodeFrequencies_; // Frequencies for npc nodes
	unsigned int npcNodesTotal_; // The total spawn frequency value. Speedup property.
	unsigned int itemNodesTotal_; // The total spawn frequency value. Speedup property.

	UI16 maxNpcAmt_; // Max amount of characters to spawn
	UI16 maxItemAmt_; // Max amount of items to spawn

	UI16 npcsPerCycle_; // amount of characters to spawn per cycle
	UI16 itemsPerCycle_; // amount of items to spawn per cycle

	UI32 minTime_; // Minimum spawn time in sec
	UI32 maxTime_; // Maximum spawn time in sec
	UI32 nextTime_; // Next time for this region to spawn
};

class cAllSpawnRegions : public std::map<QString, cSpawnRegion*>, public cComponent
{
public:

	void load( void );
	void unload();
	void check( void );
	void reload();

	cSpawnRegion* region( const QString& regName );

	void reSpawn( void );
	bool reSpawnGroup( const QString &group );
	void reSpawnToMax( void );
	bool reSpawnToMaxGroup( const QString &group );
	void deSpawn( void );
	bool deSpawnGroup( const QString &group );

	UI16 npcs( void )
	{
		UI16 numNpcs = 0;
		std::map<QString, cSpawnRegion*>::iterator it = this->begin();
		while ( it != this->end() )
		{
			numNpcs += (*it).second->npcs();
			++it;
		}
		return numNpcs;
	}

	UI16 items( void )
	{
		UI16 numItems = 0;
		std::map<QString, cSpawnRegion*>::iterator it = this->begin();
		while ( it != this->end() )
		{
			numItems += (*it).second->items();
			++it;
		}
		return numItems;
	}

	UI16 maxNpcs( void )
	{
		UI16 numNpcs = 0;
		std::map<QString, cSpawnRegion*>::iterator it = this->begin();
		while ( it != this->end() )
		{
			numNpcs += (*it).second->maxNpcs();
			++it;
		}
		return numNpcs;
	}

	UI16 maxItems( void )
	{
		UI16 numItems = 0;
		std::map<QString, cSpawnRegion*>::iterator it = this->begin();
		while ( it != this->end() )
		{
			numItems += (*it).second->maxItems();
			++it;
		}
		return numItems;
	}
};

typedef SingletonHolder<cAllSpawnRegions> SpawnRegions;

#endif
