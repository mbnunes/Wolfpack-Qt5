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

#if !defined(__SPAWNREGIONS_H__)
#define __SPAWNREGIONS_H__ 

#include "baseregion.h"

class cSpawnRegion : public cBaseRegion
{
public:
	cSpawnRegion( const QDomElement &Tag )
	{
		this->Init();
		this->name_ = Tag.parentNode().toElement().attribute( "id" );
		this->applyDefinition( Tag );
	}
	virtual ~cSpawnRegion() {;}

	void	Init( void );
	void	Add( UI32 serial );

	void	reSpawn( void );
	void	deSpawn( void );
	void	reSpawnToMax( void );

	void	checkForDeleted( void );
	void	checkTimer( void );

	bool	findValidSpot( Coord_cl &pos );
private:
	virtual void processNode( const QDomElement &Tag );

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

	vector< UI08 >			z_;				// Height, if not specified, z will be chosen
};

class cAllSpawnRegions : public cAllBaseRegions
{
private:
	static cAllSpawnRegions instance;
public:
	cAllSpawnRegions() {;}
	~cAllSpawnRegions();

	virtual void	Load( void );
	void	Check( void );

	cSpawnRegion*	region( QString regName );

	void	reSpawn( void );
	void	reSpawnToMax( void );
	void	deSpawn( void );

	static cAllSpawnRegions *getInstance( void ) { return &instance; }
};

#endif