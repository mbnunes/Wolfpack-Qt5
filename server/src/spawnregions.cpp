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
#include <math.h>

using namespace std;

// Custom position classes
class cSpawnPoint : public cSpawnPosition {
private:
	Coord_cl pos;
public:
	cSpawnPoint(const Coord_cl &pos) {
		this->pos = pos;
		points_ = 1;
	}

	Coord_cl findSpot() {
		return pos;
	}

	bool inBounds(const Coord_cl &pos) {
		return this->pos == pos;
	}
};

class cSpawnRectangle : public cSpawnPosition {
private:
	int x1, y1, x2, y2;
	bool fixedZ;
	signed char z;
	unsigned char map;
public:
	cSpawnRectangle(const Coord_cl &from, const Coord_cl &to, unsigned char map, bool fixedZ = false, signed char z = 0) {
		this->x1 = from.x;
		this->y1 = from.y;
		this->x2 = to.x;
		this->y2 = to.y;
		this->map = map;
		this->fixedZ = fixedZ;
		this->z = z;

		if (x1 > x2) {
			std::swap(x1, x2);
		}

		if (y1 > y2) {
			std::swap(y1, y2);
		}

		int xDiff = abs(x2 - x1) + 1;
		int yDiff = abs(y2 - y1) + 1;

		// Calculate number of points
		points_ = xDiff * yDiff;
	}

	Coord_cl findSpot() {
		// Simply select one random point within the rectangle
		Coord_cl pos = Coord_cl(RandomNum(x1, x2), RandomNum(y1, y2), z, map);

		// If a fixed z value should not be used, make sure to
		// find a good one.
		if (!fixedZ) {
			pos.z = Maps::instance()->mapElevation(pos);
		}

		return pos;
	}

	bool inBounds(const Coord_cl &pos) {
		return (pos.map == map && pos.x >= x1 && pos.x <= x2 && pos.y >= y1 && pos.y <= y2);
	}
};

class cSpawnCircle : public cSpawnPosition {
private:
	int x,y;
	int radius;
	bool fixedZ;
	signed char z;
	unsigned char map;
public:
	cSpawnCircle(const Coord_cl &center, int radius, unsigned char map, bool fixedZ = false, signed char z = 0) {
		this->x = center.x;
		this->y = center.y;
		this->radius = radius;
		this->map = map;
		this->fixedZ = fixedZ;
		this->z = z;

		// Calculate the surface of the circle
		// 2 * pi * r
		const float pi = 3.1415926535897932384626433832795;
		points_ = (int)(pi * (float)radius * (float)radius);
	}

	bool inBounds(const Coord_cl &pos) {
		// Calculate the distance to the center and
		// check if it's smaller than the radius
		float xDiff = (float)abs(pos.x - x);
		float yDiff = (float)abs(pos.y - y);

		float distance = sqrt(xDiff * xDiff + yDiff * yDiff);

		return distance <= (float)radius;
	}

	Coord_cl findSpot() {
		Coord_cl pos;

		// now get a point on this circle around the m_npc
		float rnddist = (float)RandomNum(0, radius);
		float rndphi = (float)RandomNum(0, 100) / 100.0f * 2.0f * 3.14159265358979323846f;

		pos.x = x + (unsigned short)floor(cos(rndphi) * rnddist);
		pos.y = y + (unsigned short)floor(sin(rndphi) * rnddist);
		pos.map = map;

		// If a fixed z value should not be used, make sure to
		// find a good one.
		if (!fixedZ) {
			pos.z = Maps::instance()->mapElevation(pos);
		} else {
			pos.z = z;
		}

		return pos;
	}
};

/*****************************************************************************
	cSpawnRegion member functions
*****************************************************************************/

cSpawnRegion::cSpawnRegion(const cElement *tag) {
	maxNpcAmt_ = 0;
	maxItemAmt_ = 0;
	npcsPerCycle_ = 1;
	itemsPerCycle_ = 1;
	minTime_ = 0;
	maxTime_ = 600;
	nextTime_ = 0;
	id_ = tag->getAttribute("id");
	positions_.setAutoDelete(true);
	checkFreeSpot_ = false;
	npcNodesTotal_ = 0;
	itemNodesTotal_ = 0;
	active_ = true;

	// Load the spawnregion
	applyDefinition(tag);

	// No Positions, No NPCs, No Items -> Inactive
	if (active_ && (positions_.isEmpty() || (npcNodes_.isEmpty() && itemNodes_.isEmpty()) || countPoints() == 0)) {
		Console::instance()->log(LOG_WARNING, tr("Active spawnregion '%1' has been flagged inactive because its incomplete.\n").arg(id_));
		active_ = false;
	}
}

cSpawnRegion::~cSpawnRegion() {
}

bool cSpawnRegion::isValidSpot(const Coord_cl &pos) {
	// Check all sub positions
	cSpawnPosition *position;
	for (position = positions_.first(); position; position = positions_.next()) {
		if (position->inBounds(pos)) {
			return true;
		}
	}
	return false;
}

void cSpawnRegion::add( cUObject* object )
{
	if ( object->isItem() )
	{
		items_.append( object );
	}
	else if ( object->isChar() )
	{
		npcs_.append( object );
	}
}

void cSpawnRegion::remove( cUObject* object )
{
	if ( object->isItem() )
	{
		items_.remove( object );
	}
	else if ( object->isChar() )
	{
		npcs_.remove( object );
	}
}

void cSpawnRegion::processNode( const cElement *tag )
{
	QString name = tag->name();
	QString value = tag->value();

	if ( name == "npc" ) {
		bool ok;
		unsigned int value = tag->getAttribute("frequency", "1").toUInt(&ok);

		if (!ok || value < 1) {
			Console::instance()->log(LOG_WARNING, tr("Spawnregion '%1' has a npc tag with an invalid frequency attribute '%2'.\n").arg(id_).arg(tag->getAttribute("frequency")));
			value = 1;
		}

		npcNodes_.append(tag); // Append to npc nodes
		npcNodeFrequencies_.append(value);
		npcNodesTotal_ += value;
	} else if ( name == "item" ) {
		bool ok;
		unsigned int value = tag->getAttribute("frequency", "1").toUInt(&ok);

		if (!ok || value < 1) {
			Console::instance()->log(LOG_WARNING, tr("Spawnregion '%1' has an item tag with an invalid frequency attribute '%2'.\n").arg(id_).arg(tag->getAttribute("frequency")));
			value = 1;
		}

		itemNodes_.append(tag); // Append to item nodes
		itemNodeFrequencies_.append(value);
		itemNodesTotal_ += value;
	}

	else if ( name == "checkfreespot" )
		this->checkFreeSpot_ = true;

	else if ( name == "nocheckfreespot" )
		this->checkFreeSpot_ = false;

	// <maxnpcamount value="10" />
	else if ( name == "maxnpcamount" && tag->hasAttribute("value")  )
		this->maxNpcAmt_ = tag->getAttribute("value").toUInt();
		
	else if ( name == "maxnpcamount" )
		this->maxNpcAmt_ = value.toUShort();

	// <maxitemamount value="5 " />
	else if ( name == "maxitemamount" && tag->hasAttribute("value")  )
		this->maxItemAmt_ = tag->getAttribute("value").toUInt();
		
	else if ( name == "maxitemamount" )
		this->maxItemAmt_ = value.toUShort();		

	// <npcspercycle value="3 " />
	else if ( name == "npcspercycle" && tag->hasAttribute("value" ) )
		this->npcsPerCycle_ = tag->getAttribute("value").toUInt();
		
	else if ( name == "npcspercycle" )
		this->npcsPerCycle_ = value.toUShort();

	// <itemspercycle value="3" />
	else if ( name == "itemspercycle" && tag->hasAttribute("value" ) )
		this->itemsPerCycle_ = tag->getAttribute("value").toUInt();
		
	else if ( name == "itemspercycle" )
		this->itemsPerCycle_ = value.toUShort();

	// <delay min="xx" max="xx" />
	// <delay value="" />
	else if ( name == "delay" && tag->hasAttribute("value") ) {
		unsigned int delay = tag->getAttribute("value").toUInt();
		minTime_ = delay;
		maxTime_ = delay;
	}

	else if ( name == "delay" && tag->hasAttribute("min") && tag->hasAttribute("max") ) {
		minTime_ = tag->getAttribute("min").toUInt();
		maxTime_ = tag->getAttribute("max").toUInt();
	}

	// <active />
	else if ( name == "active" )
		this->active_ = true;

	// <inactive />
	else if ( name == "inactive" )
		this->active_ = false;

	// Add this to a spawngroup
	else if ( name == "group" )
		this->groups_.append(value);

	// <rectangle from="0,1000" to="0,1000" map="" z="" />
	else if ( name == "rectangle" && tag->hasAttribute("from") && tag->hasAttribute("to") && tag->hasAttribute("map") ) {
		// Parse from/to
		Coord_cl from, to;
		if (!parseCoordinates(tag->getAttribute("from"), from, true)) {
			Console::instance()->log(LOG_WARNING, tr("Spawnregion '%1' has a rectangle with an invalid from attribute '%2'.\n").arg(id_).arg(tag->getAttribute("from")));
			return;
		}

		if (!parseCoordinates(tag->getAttribute("to"), to, true)) {
			Console::instance()->log(LOG_WARNING, tr("Spawnregion '%1' has a rectangle with an invalid from attribute '%2'.\n").arg(id_).arg(tag->getAttribute("from")));
			return;
		}

		bool ok;

		unsigned char map = tag->getAttribute("map").toUShort(&ok);

		if (!ok) {
			Console::instance()->log(LOG_WARNING, tr("Spawnregion '%1' has a rectangle with an invalid map attribute '%2'.\n").arg(id_).arg(tag->getAttribute("map")));
			return;
		}

		if (!Maps::instance()->hasMap(map)) {
			Console::instance()->log(LOG_WARNING, tr("Ignoring rectangle with unknown map %1 for spawnregion '%2'.\n").arg(map).arg(id_));
			return;
		}

		// Fixed Z Level?
		bool fixedZ = false;
		signed char z = 0;

		if (tag->hasAttribute("z")) {
			z = tag->getAttribute("z").toShort(&ok);
			if (!ok) {
				Console::instance()->log(LOG_WARNING, tr("Spawnregion '%1' has a rectangle with an invalid z attribute '%2'.\n").arg(id_).arg(tag->getAttribute("z")));
				return;
			}
		}

		positions_.append(new cSpawnRectangle(from, to, map, fixedZ, z)); // Append to position
	}

	// <point pos="x,y,z,map" />
	else if ( name == "point" && tag->hasAttribute( "pos" ) ) {
		Coord_cl pos;
		if (!parseCoordinates(tag->getAttribute("pos"), pos)) {
			Console::instance()->log(LOG_WARNING, tr("Spawnregion '%1' has a point with an invalid pos attribute '%2'.\n").arg(id_).arg(tag->getAttribute("pos")));
			return;
		}

		positions_.append(new cSpawnPoint(pos));
	}

	// <circle center="x,y" radius="" map="" [z=""] />
	else if ( name == "circle" && tag->hasAttribute( "center" ) && tag->hasAttribute( "radius" ) && tag->hasAttribute( "map" ) ) {
		// Parse center
		Coord_cl center;
		if (!parseCoordinates(tag->getAttribute("center"), center, true)) {
			Console::instance()->log(LOG_WARNING, tr("Spawnregion '%1' has a circle with an invalid center attribute '%2'.\n").arg(id_).arg(tag->getAttribute("center")));
			return;
		}

		bool ok;
		unsigned char map = tag->getAttribute("map").toUShort(&ok);

		if (!ok) {
			Console::instance()->log(LOG_WARNING, tr("Spawnregion '%1' has a circle with an invalid map attribute '%2'.\n").arg(id_).arg(tag->getAttribute("map")));
			return;
		}

		unsigned int radius = tag->getAttribute("radius").toUInt(&ok);

		if (!ok) {
			Console::instance()->log(LOG_WARNING, tr("Spawnregion '%1' has a circle with an invalid radius attribute '%2'.\n").arg(id_).arg(tag->getAttribute("radius")));
			return;
		}

		if (!Maps::instance()->hasMap(map)) {
			Console::instance()->log(LOG_WARNING, tr("Ignoring circle with unknown map %1 for spawnregion '%2'.\n").arg(map).arg(id_));
			return;
		}

		// Fixed Z Level?
		bool fixedZ = false;
		signed char z = 0;

		if (tag->hasAttribute("z")) {
			z = tag->getAttribute("z").toShort(&ok);
			if (!ok) {
				Console::instance()->log(LOG_WARNING, tr("Spawnregion '%1' has a circle with an invalid z attribute '%2'.\n").arg(id_).arg(tag->getAttribute("z")));
				return;
			}
		}

		positions_.append(new cSpawnCircle(center, radius, map, fixedZ, z)); // Append to position
	}

	// Inherit from another spawnregion
	else if ( name == "inherit" && tag->hasAttribute("id") ) {
		const cElement *node = Definitions::instance()->getDefinition(WPDT_SPAWNREGION, tag->getAttribute("id"));
		if (node) {
			applyDefinition(node);
		} else {
			Console::instance()->log(LOG_WARNING, tr("Spawnregion '%1' is inheriting from unknown spawnregion '%2'.\n").arg(id_).arg(tag->getAttribute("id")));
		}
		return;
	}
}

// Counts the total number of points this spawnregion has
unsigned int cSpawnRegion::countPoints() {
	cSpawnPosition *position;
	unsigned int total = 0;
	for (position = positions_.first(); position; position = positions_.next()) {
		total += position->points();
	}
	return total;
}

bool cSpawnRegion::findValidSpot(Coord_cl& result, int tries) {
	if (tries == -1) {
		tries = 20; // Try 20 times (should be a config option instead)
	} else if (tries == 0) {
		return false; // We exceeded the maximum number of tries
	}

	unsigned int points = countPoints(); // Count the number of available points

	if (points == 0) {
		return false; // No points available
	}

	unsigned int chosen = RandomNum(0, points - 1); // Chose one random point
	unsigned int offset = 0; // Initialize the offset
	cSpawnPosition *position; // Current partition

	// Search for a random position from out positions list.
	for (position = positions_.first(); position; position = positions_.next()) {
		offset += position->points(); // Increase the offset

		// Is this the correct partition?
		if (chosen < offset) {
			// Find a random position within the partition
			Coord_cl rndPos = position->findSpot();

			// See if the spot is valid.
			if (!Movement::instance()->canLandMonsterMoveHere(rndPos)) {
				return findValidSpot(result, tries - 1); // Invalid spot, search for another one
			}

			// See if there already are items or characters at the given position
			if (checkFreeSpot_) {
				cUObject *pItem;
				for (pItem = items_.first(); pItem; pItem = items_.next()) {
					if (pItem->pos() == rndPos) {
						return findValidSpot(result, tries - 1); // Invalid spot, search for another one
					}
				}

				cUObject *pChar;
				for (pChar = npcs_.first(); pChar; pChar = npcs_.next()) {
					if (pChar->pos() == rndPos) {
						return findValidSpot(result, tries - 1); // Invalid spot, search for another one
					}
				}
			}

			result = rndPos;
			return true;
		}
	}

	return false;
}

void cSpawnRegion::spawnSingleNPC()
{
	Coord_cl pos;
	if ( findValidSpot( pos ) )
	{
		// This is a little tricky.
		// There are some regions where one NPC should be spawned more often than others
		// So we treat every NPC section as a 1 point section and then select accordingly...
		// The frequency="" attribute is used for this
		cElement *node;
		unsigned int selected = RandomNum(0, npcNodesTotal_ - 1); // Random number
		unsigned int offset = 0; // Random offset
		unsigned int i = 0; // Index for the npcNodeFrequencies
		const cElement *tag = 0;

		// Search the selected element
		for (node = npcNodes_.first(); node; node = npcNodes_.next()) {
			unsigned int value = npcNodeFrequencies_[i++];
			offset += value;

			if (selected < offset) {
				tag = node;
				break;
			}
		}

		// We didn't find a npc
		if (!tag) {
			Console::instance()->log(LOG_ERROR, tr("Unable to find a valid npc definition for spawnregion '%1'.\n").arg(id_));
			return;
		}

		// Get the id of the NPC we want to spawn... (important -> basedef)
		QString id = tag->getAttribute("id");

		if (id.isEmpty() || id.isNull()) {
			Console::instance()->log(LOG_ERROR, tr("Npc tag for spawnregion '%1' lacks id attribute.\n").arg(id_));
			return;
		}

		const cElement *parent = Definitions::instance()->getDefinition(WPDT_NPC, id);

		if (!parent) {
			Console::instance()->log(LOG_ERROR, tr("Npc tag for spawnregion '%1' has invalid id attribute '%2'.\n").arg(id_).arg(id));
			return;
		}

		// Create the NPC
		P_NPC pChar = new cNPC;
		pChar->Init();
		pChar->setSpawnregion(this);
		pChar->setBaseid(id.latin1());
		pChar->moveTo( pos );

		pChar->applyDefinition( parent ); // Apply the definition from the id first

		// Apply these settings between the inherited npc and the custom settings in the spawnregion
		// file
		if (countPoints() == 1) {
			pChar->setWanderType( enHalt ); // Most likely a vendor spawn with only one point
		} else {
			pChar->setWanderType( enWanderSpawnregion );
			pChar->setWanderX1( pos.x );
			pChar->setWanderY1( pos.y );
		}

		pChar->applyDefinition( tag ); // Now apply the given tag

		// OrgBody and OrgSkin are often not set in the scripts
		pChar->setOrgBody( pChar->body() );
		pChar->setOrgSkin( pChar->skin() );
		pChar->setOrgName( pChar->name() );

		// Now we call onCreate
		cDelayedOnCreateCall* onCreateCall = new cDelayedOnCreateCall( pChar, id );
		Timers::instance()->insert( onCreateCall );

		pChar->resend(false); // Resend the NPC
		onSpawn( pChar ); // Call the onSpawn event
	} else {
		Console::instance()->log(LOG_ERROR, tr("Unable to find valid spot for spawnregion %1.\n").arg(id_));
	}
}

void cSpawnRegion::spawnSingleItem()
{
	Coord_cl pos;
	if ( findValidSpot( pos ) )
	{
		// This is a little tricky.
		// There are some regions where one item should be spawned more often than others
		// So we treat every item section as a 1 point section and then select accordingly...
		// The frequency="" attribute is used for this
		cElement *node;
		unsigned int selected = RandomNum(0, itemNodesTotal_ - 1); // Random number
		unsigned int offset = 0; // Random offset
		unsigned int i = 0; // Index for the npcNodeFrequencies
		const cElement *tag = 0;

		// Search the selected element
		for (node = itemNodes_.first(); node; node = itemNodes_.next()) {
			unsigned int value = itemNodeFrequencies_[i++];
			offset += value;

			if (selected < offset) {
				tag = node;
				break;
			}
		}

		// We didn't find a item
		if (!tag) {
			Console::instance()->log(LOG_ERROR, tr("Unable to find a valid item definition for spawnregion '%1'.\n").arg(id_));
			return;
		}

		// Get the id of the item we want to spawn... (important -> basedef)
		QString id = tag->getAttribute("id");

		if (id.isEmpty() || id.isNull()) {
			Console::instance()->log(LOG_ERROR, tr("Item tag for spawnregion '%1' lacks id attribute.\n").arg(id_));
			return;
		}

		const cElement *parent = Definitions::instance()->getDefinition(WPDT_ITEM, id);

		if (!parent) {
			Console::instance()->log(LOG_ERROR, tr("Item tag for spawnregion '%1' has invalid id attribute '%2'.\n").arg(id_).arg(id));
			return;
		}

		// Create the item
		P_ITEM pItem = new cItem;
		pItem->Init();
		pItem->setSpawnregion(this);
		pItem->setBaseid(id.latin1());
		pItem->moveTo( pos );

		pItem->applyDefinition( parent ); // Apply the definition from the id first
		pItem->applyDefinition( tag ); // Now apply the given tag

		// Now we call onCreate
		cDelayedOnCreateCall* onCreateCall = new cDelayedOnCreateCall( pItem, id );
		Timers::instance()->insert( onCreateCall );

		pItem->update(); // Resend the NPC
		onSpawn( pItem ); // Call the onSpawn event
	}
}

void cSpawnRegion::onSpawn( cUObject* obj )
{
	cPythonScript* global = ScriptManager::instance()->getGlobalHook( EVENT_SPAWN );

	if ( global ) {
		PyObject* args = Py_BuildValue( "NN", PyGetSpawnRegionObject( this ), PyGetObjectObject( obj ) );
		global->callEventHandler( EVENT_SPAWN, args );
		Py_DECREF( args );
	}
}

// do one spawn and reset the timer
void cSpawnRegion::reSpawn( void )
{
	if (active_) {
		unsigned int i = 0;
		for ( i = 0; i < npcsPerCycle_; ++i )
			if ( npcs() < maxNpcAmt_ )
				spawnSingleNPC(); // spawn a random npc

		for ( i = 0; i < this->itemsPerCycle_; i++ )
			if ( items() < this->maxItemAmt_ )
				spawnSingleItem(); // spawn a random item

		this->nextTime_ = Server::instance()->time() + RandomNum( this->minTime_, this->maxTime_ ) * MY_CLOCKS_PER_SEC;
	}
}

void cSpawnRegion::reSpawnToMax( void )
{
	if (active_) {
		unsigned int failed = 0;

		while ( npcs() < maxNpcAmt_  && failed < 50) {
			unsigned int oldamount = npcs();
			spawnSingleNPC();
			if (npcs() == oldamount) {
				failed++;
			}
		}

		failed = 0;
		while ( items() < maxItemAmt_ ) {
			unsigned int oldamount = items();
			spawnSingleItem();
			if (npcs() == oldamount) {
				failed++;
			}
		}

		this->nextTime_ = Server::instance()->time() + RandomNum( this->minTime_, this->maxTime_ ) * MY_CLOCKS_PER_SEC;
	}
}

// delete all spawns and reset the timer
void cSpawnRegion::deSpawn( void )
{
	QPtrList<cUObject> items = items_; // Copy
	QPtrList<cUObject> npcs = npcs_; // Copy
	cUObject *object;

	for ( object = items.first(); object; object = items.next() )
	{
		object->remove();
	}

	for ( object = npcs.first(); object; object = npcs.next() )
	{
		object->remove();
	}

	nextTime_ = Server::instance()->time() + RandomNum( minTime_, maxTime_ ) * MY_CLOCKS_PER_SEC;
}

// check the timer and if expired do reSpawn
void cSpawnRegion::checkTimer( void )
{
	if (active_) {
		if ( this->nextTime_ <= Server::instance()->time() )
			this->reSpawn();
	}
}


/*****************************************************************************
	cAllSpawnRegions member functions
*****************************************************************************/


void cAllSpawnRegions::check( void )
{
	int respawned = 0;
	iterator it( this->begin() );
	while ( it != this->end() && respawned < 10 )
	{
		if ( it->second->active() && it->second->nextTime() <= Server::instance()->time() ) {
			it->second->reSpawn();
			respawned++;
		}
		++it;
	}
}

bool cAllSpawnRegions::reSpawnToMaxGroup( const QString &group ) {
	iterator it( this->begin() );
	bool found = false;
	while ( it != this->end() )
	{
		if (it->second->groups().contains(group)) {
			it->second->reSpawnToMax();
			found = true;
		}
		++it;
	}
	return found;
}

bool cAllSpawnRegions::reSpawnGroup( const QString &group ) {
	iterator it( this->begin() );
	bool found = false;
	while ( it != this->end() )
	{
		if (it->second->groups().contains(group)) {
			it->second->reSpawn();
			found = true;
		}
		++it;
	}
	return found;
}

bool cAllSpawnRegions::deSpawnGroup( const QString &group ) {
	iterator it( this->begin() );
	bool found = false;
	while ( it != this->end() )
	{
		if (it->second->groups().contains(group)) {
			it->second->deSpawn();
			found = true;
		}
		++it;
	}
	return found;
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
		if (it->second->active()) {
			it->second->reSpawnToMax();
		}
		++it;
	}
}

void cAllSpawnRegions::deSpawn( void )
{
	iterator it( this->begin() );
	while ( it != this->end() )
	{
		if (it->second->active()) {
			it->second->deSpawn();
		}
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

	// Don't load spawnregions if disabled
	if (!Config::instance()->getBool("General", "Disable Spawnregions", false, true)) {	
		QStringList DefSections = Definitions::instance()->getSections( WPDT_SPAWNREGION );

		QStringList::iterator it = DefSections.begin();
		while ( it != DefSections.end() )
		{
			const cElement* DefSection = Definitions::instance()->getDefinition( WPDT_SPAWNREGION, *it );

			cSpawnRegion* toinsert_ = new cSpawnRegion( DefSection );
			this->insert( make_pair( *it, toinsert_ ) );
			++it;
		}
	}

	cComponent::load();
}

void cAllSpawnRegions::reload()
{
	// Save a list of all objects and their spawnregions
	// So the references can be recreated later.
	QMap<QString, QPtrList<cUObject> > objects;

	cItemIterator iItems;
	for ( P_ITEM pItem = iItems.first(); pItem; pItem = iItems.next() )
	{
		cSpawnRegion *region = pItem->spawnregion();
		if ( region )
		{
			if ( !objects.contains( region->id() ) )
			{
				objects[region->id()].setAutoDelete( false );
			}

			objects[region->id()].append( pItem );
			pItem->setSpawnregion( 0 ); // Remove from spawnregion before pointer gets invalid
		}
	}

	cCharIterator iChars;
	for ( P_CHAR pChar = iChars.first(); pChar; pChar = iChars.next() )
	{
		cSpawnRegion *region = pChar->spawnregion();
		if ( region )
		{
			if ( !objects.contains( region->id() ) )
			{
				objects[region->id()].setAutoDelete( false );
			}

			objects[region->id()].append( pChar );
			pChar->setSpawnregion( 0 ); // Remove from spawnregion before pointer gets invalid
		}
	}

	unload();
	load();

	QMap<QString, QPtrList<cUObject> >::iterator it;
	for ( it = objects.begin(); it != objects.end(); ++it )
	{
		cSpawnRegion *region = this->region( it.key() );
		if ( region )
		{
			cUObject *object;
			QPtrList<cUObject> &list = it.data();
			for ( object = list.first(); object; object = list.next() )
			{
				object->setSpawnregion( region );
			}
		} else {
			cUObject *object;
			QPtrList<cUObject> &list = it.data();
			for ( object = list.first(); object; object = list.next() )
			{
				object->remove();
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
