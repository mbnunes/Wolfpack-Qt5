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
//	Wolfpack Homepage: http://www.wpdev.sf.net/
//========================================================================================

// Platform Specifics
#include "platform.h"

// Wolfpack includes
#include "coord.h"
#include "uobject.h"
#include "iserialization.h"
#include "globals.h"
#include "regions.h"
#include "junk.h"
#include "defines.h"
#include "wpdefaultscript.h"
#include "wpscriptmanager.h"

// Debug includes and defines
#undef  DBGFILE
#define DBGFILE "uobject.cpp"

cUObject::cUObject()
{
	init();
}

cUObject::cUObject( cUObject &src )
{
	// Copy Events
	this->setEvents( src.getEvents() );

	this->serial = src.serial;
	this->multis = src.multis;
	this->name = src.name;
	this->free = src.free;
}

cUObject::~cUObject()
{
}

void cUObject::init()
{
	this->serial = INVALID_SERIAL;
	this->multis = INVALID_SERIAL;
	this->free = false;
}

void cUObject::moveTo( const Coord_cl& newpos )
{
	bool regionChanged = false;
	if ( cRegion::GetCell(pos) != cRegion::GetCell(newpos) )
	{
		mapRegions->Remove( this );
		regionChanged = true;
	}
	pos = newpos;
	if ( regionChanged )
		mapRegions->Add( this );
}

/*!
 * Provides persistence for instances of UOBject
 *
 * @param &archive : an ISerialization descendent.
 *
 * @return void  : none.
 */
void cUObject::Serialize(ISerialization &archive)
{
	QString events;

	if (archive.isReading())
	{
		archive.read("name", name);
		archive.read("serial", serial);
		archive.read("multis", multis);
		archive.read("pos.x", pos.x);
		archive.read("pos.y", pos.y);
		archive.read("pos.z", pos.z);
		archive.read("pos.map", pos.map);
		archive.read("pos.plane", pos.plane);
		archive.read("events", events );

		eventList_ = QStringList::split( ",", events );
	}
	else if (archive.isWritting())
	{
		archive.write("name", name);
		archive.write("serial", serial);
		archive.write("multis", multis);
		archive.write("pos.x", pos.x);
		archive.write("pos.y", pos.y);
		archive.write("pos.z", pos.z);
		archive.write("pos.map", pos.map);
		archive.write("pos.plane", pos.plane);

		events = eventList_.join( "," );
		archive.write( "events", events );
	}
	cSerializable::Serialize( archive );
}

inline string cUObject::objectID()
{
	return string("UOBJECT");
}

// Method for setting a list of WPDefaultScripts
void cUObject::setEvents( std::vector< WPDefaultScript* > List )
{
	scriptChain.clear();
	eventList_.clear();

	// "Reset" the events
	if( List.size() == 0 )
		return;

	// Walk the List and add the events
	for( UI08 i = 0; i < List.size(); i++ )
		if( List[ i ] != NULL )
		{
			scriptChain.push_back( List[ i ] );
			eventList_.push_back( List[ i ]->getName() );
		}
}

// Gets a vector of all assigned events
const std::vector< WPDefaultScript* > &cUObject::getEvents( void )
{
	/*std::vector< WPDefaultScript* > List;

	for( UI08 i = 0; i < scriptChain.size(); i++ )
		List.push_back( scriptChain[ i ] );*/

	return scriptChain;
}

// Checks if the object has a specific event
bool cUObject::hasEvent( QString Name )
{
	bool hasEvent = false;

	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->getName() == Name )
			hasEvent = true;

	return hasEvent;
}

void cUObject::addEvent( WPDefaultScript *Event )
{
	if( hasEvent( Event->getName() ) )
		return;

	scriptChain.push_back( Event );
	eventList_.push_back( Event->getName() );
}

void cUObject::removeEvent( QString Name )
{
	std::vector< WPDefaultScript* >::iterator myIterator;

	for( myIterator = scriptChain.begin(); myIterator != scriptChain.end(); ++myIterator )
	{
		if( (*myIterator)->getName() == Name )
			scriptChain.erase( myIterator );
	}
 
	// I hope this works
	eventList_.remove( Name );
}

/****************************
 * 
 * Scripting events
 *
 ****************************/

bool cUObject::onUse( cUObject *Target )
{
	// If we dont have any events assigned just skip processing
	if( scriptChain.empty() )
		return false;

	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
	{
		// If we're the Character pass us as the second param
		// if not as the first
		bool Handeled = false;

		if( this->objectID() != "CHARACTER" )
			Handeled = scriptChain[ i ]->onUse( (P_CHAR)Target, (P_ITEM)this );
		else
			Handeled = scriptChain[ i ]->onUse( (P_CHAR)this, (P_ITEM)Target );

		if( Handeled )
			return true;
	}

	return false;
}

bool cUObject::onCollide( cUObject* Obstacle )
{
	// If we dont have any events assigned just skip processing
	if( scriptChain.empty() )
		return false;

	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
	{
		// Items cannot collide with items
		if( this->objectID() != "CHARACTER" ) // Item, so obstacle has to be character
			scriptChain[ i ]->onCollideItem( (P_CHAR)Obstacle, (P_ITEM)this );
		else
			if( Obstacle->objectID() == "ITEM" )
				if( scriptChain[ i ]->onCollideItem( (P_CHAR)this, (P_ITEM)Obstacle ) )
					return true;

			else // Character, Character
				if( scriptChain[ i ]->onCollideChar( (P_CHAR)this, (P_CHAR)Obstacle ) )
					return true;
	}

	return false;
}

// Returns the list of events
QString cUObject::eventList( void )
{
	return eventList_.join( "," );
}

// If the scripts are reloaded call that for each and every existing object
void cUObject::recreateEvents( void )
{
	// Walk the eventList and recreate 
	QStringList::const_iterator myIter;

	scriptChain.clear();

	for( myIter = eventList_.begin(); myIter != eventList_.end(); ++myIter )
	{
		WPDefaultScript *myScript = ScriptManager->find( *myIter );

		// Script not found
		if( myScript == NULL )
			continue;

		scriptChain.push_back( myScript );
	}
}
