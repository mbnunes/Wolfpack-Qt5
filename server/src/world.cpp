//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
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

// Wolfpack Includes
#include "pfactory.h"
#include "world.h"
#include "wpconsole.h"
#include "globals.h"
#include "srvparams.h"
#include "dbdriver.h"
#include "progress.h"
#include "iserialization.h"
#include "persistentbroker.h"
#include "utilsys.h" // What the heck do we need this file for ?!
#include "accounts.h"
#include "inlines.h"
#include "basechar.h"
#include "player.h"
#include "npc.h"
#include "sqlite/sqlite.h"

// Postprocessing stuff, can be deleted later on
#include "maps.h"
#include "sectors.h"
#include "territories.h"

// Objects ( => Factory later on )
#include "uobject.h"
#include "items.h"
#include "multis.h"

// Python Includes
#include "python/utilities.h"
#include "python/tempeffect.h"

// Library Includes
#include <list>

// UNCOMMENT THIS IF YOU WANT TO USE A HASHMAP 
//#define WP_USE_HASH_MAP

// Important compile switch
#if defined(WP_USE_HASH_MAP)
#include <hash_map>
typedef std::hash_map< SERIAL, P_ITEM > ItemMap;
typedef std::hash_map< SERIAL, P_CHAR > CharMap;
#else
#include <map>
typedef std::map< SERIAL, P_ITEM > ItemMap;
typedef std::map< SERIAL, P_CHAR > CharMap;
#endif

/*****************************************************************************
  cWorldPrivate member functions
 *****************************************************************************/

#define ITEM_SPACE 0x40000000

class cWorldPrivate
{
public:
	// Choose here whether we want to have std::map or std::hash_map
	ItemMap items;
	CharMap chars;
    
	// Pending for deletion
	std::list< cUObject* > pendingObjects;

	void purgePendingObjects() 
	{
		std::list< cUObject* >::const_iterator it;
		for( it = pendingObjects.begin(); it != pendingObjects.end(); ++it )
		{	
			World::instance()->unregisterObject( *it );
			delete *it;
		}

		pendingObjects.clear();
	}
};

/*****************************************************************************
  cWorld member functions
 *****************************************************************************/

/*!
  \class cWorld world.h

  \brief The cWorld class provides a container of all cUObjects, sorted in two
  major groups: Items and Characters.

  \ingroup mainclass

  cWorld is responsible for maintaining all Ultima Online objects, retrievable
  by their serial number. It also provides loading and saving services to those
  objects. cWorld is a Singleton, accessible thru the symbol World::instance().
*/

/*!
	Constructs the world container.
*/
cWorld::cWorld()
{
	// Create our private implementation
	p = new cWorldPrivate;

	_charCount = 0;
	_itemCount = 0;
	lastTooltip = 0;
	_lastCharSerial = 0;
	_lastItemSerial = ITEM_SPACE;
}

/*!
	Destructs the world container and claims back the memory of it's contained objects
*/
cWorld::~cWorld()
{
	// Free pending objects
	p->purgePendingObjects();

	// Destroy our private implementation
	delete p;
}


static void quickdelete( P_ITEM pi ) throw()
{
	// Minimal way of deleting an item
	pi->SetOwnSerial( -1 );

	persistentBroker->addToDeleteQueue( "items", QString( "serial = '%1'" ).arg( pi->serial() ) );

	// Also delete all items inside if it's a container.
	cItem::ContainerContent container(pi->content());
	cItem::ContainerContent::const_iterator it ( container.begin() );
	cItem::ContainerContent::const_iterator end( container.end() );
	for ( ; it != end; ++it )
		quickdelete( *it );

	// if it is within a multi, delete it from the multis vector
	if( pi->multis() != INVALID_SERIAL )
	{
		cMulti* pMulti = dynamic_cast< cMulti* >( FindItemBySerial( pi->multis() ) );
		if( pMulti )
			pMulti->removeItem( pi );
	}

	MapObjects::instance()->remove( pi );
	World::instance()->unregisterObject( pi );
}

void cWorld::load()
{
	clConsole.send( "Loading World...\n" );

	persistentBroker->connect( SrvParams->databaseHost(), SrvParams->databaseName(), SrvParams->databaseUsername(), SrvParams->databasePassword() );

	ISerialization* archive = cPluginFactory::serializationArchiver( "xml" );

	QString objectID;
	register unsigned int i = 0;

	QStringList types = UObjectFactory::instance()->objectTypes();

	for( uint j = 0; j < types.count(); ++j )
	{
		QString type = types[j];
		
		cDBResult res = persistentBroker->query( QString( "SELECT COUNT(*) FROM uobjectmap WHERE type = '%1'" ).arg( type ) );

		// Find out how many objects of this type are available		
		if( !res.isValid() )
			throw persistentBroker->lastError();			

		res.fetchrow();
		UINT32 count = res.getInt( 0 );
		res.free();

		if ( count == 0 )
			continue; // Move on...

		clConsole.send( "\n"+tr("Loading ") + QString::number( count ) + tr(" objects of type ") + type );

		res = persistentBroker->query( UObjectFactory::instance()->findSqlQuery( type ) );

		// Error Checking		
		if( !res.isValid() )
			throw persistentBroker->lastError();

		//UINT32 sTime = getNormalizedTime();
		cUObject *object;
		progress_display progress( count );

		// Fetch row-by-row
		persistentBroker->driver()->setActiveConnection( CONN_SECOND );
		while( res.fetchrow() )
		{
			unsigned short offset = 0;
			char **row = res.data();

			// do something with data
			object = UObjectFactory::instance()->createObject( type );			
			object->load( row, offset );

			++progress;
		}

		res.free();
		persistentBroker->driver()->setActiveConnection();

		//clConsole.send( "Loaded %i objects in %i msecs\n", progress.count(), getNormalizedTime() - sTime );
	}

	// Load Pages
	// cPagesManager::getInstance()->load();

	// Load Temporary Effects
	archive = cPluginFactory::serializationArchiver( "xml" );

	archive->prepareReading( "effects" );
	
	if ( archive->size() )
	{
		clConsole.send( QString( "Loading %1 Temp. Effects...\n" ).arg( archive->size() ) );
		progress_display progress( archive->size() );
		
		for ( i = 0; i < archive->size(); ++progress, ++i)
		{
			archive->readObjectID(objectID);
			
			cTempEffect* pTE = NULL;
			
			if( objectID == "HIDECHAR" )
				pTE = new cDelayedHideChar( INVALID_SERIAL );
			
			else if( objectID == "cPythonEffect" )
				pTE = new cPythonEffect;
			
			else
			{
				clConsole.log( LOG_FATAL, tr( "An unknown temporary Effect class was found: %1" ).arg( objectID ) );
				continue; // Skip the class, not a good habit but at the moment the user couldn't really debug the error
			}
			
			archive->readObject( pTE );
			TempEffects::instance()->insert( pTE );
		}
	}

	archive->close();
	delete archive;

	clConsole.PrepareProgress( tr("Postprocessing") );

	P_ITEM pi;	
	QPtrList< cItem > deleteItems;

	cItemIterator iter;
	for( pi = iter.first(); pi; pi = iter.next() )
	{
		SERIAL contserial = reinterpret_cast<SERIAL>(pi->container());

		// 1. Handle the Container Value
		if( isItemSerial( contserial ) )
		{
			P_ITEM pCont = FindItemBySerial( contserial );

			if( pCont )
			{
				pCont->addItem( pi, false, false, true );
			}
			else
			{
				// Queue this item up for deletion
				deleteItems.append( pi );
				continue; // Skip further processing
			}
		}
		else if( isCharSerial( contserial ) )
		{
			P_CHAR pCont = FindCharBySerial( contserial );

			if( pCont )
			{
				// NoRemove is important. 
				// It is faster *and* it prevents onEquip from being fired
				pCont->addItem( (cBaseChar::enLayer)pi->layer(), pi, false, true );
			}
			else
			{
				deleteItems.append( pi );
				continue; // Skip further processing
			}
		}
		else // Add to Map Regions
		{
			int max_x = Map->mapTileWidth(pi->pos().map) * 8;
			int max_y = Map->mapTileHeight(pi->pos().map) * 8;
			if ( pi->pos().x > max_x || pi->pos().y > max_y ) 
			{
				// these are invalid locations, delete them!
				deleteItems.append( pi );
			}
			else
				MapObjects::instance()->add(pi);
//			continue;
		}

		// If this item has a multiserial then add it to the multi
		if( isItemSerial( pi->multis() ) )
		{
			cMulti *pMulti = dynamic_cast< cMulti* >( FindItemBySerial( pi->multis() ) );

			if( pMulti )
				pMulti->addItem( pi );
		}

/*		// effect on dex ? like plate eg.
		if( pi->dx2() && pi->container() && pi->container()->isChar() )
		{
			P_CHAR pChar = dynamic_cast< P_CHAR >( pi->container() );

			if( pChar )
				pChar->setDexterity( pChar->dexterity + pi->dx2() );
		}*/
		pi->flagUnchanged(); // We've just loaded, nothing changes.
	}

	// Post Process Characters
	cCharIterator charIter;
	P_CHAR pChar;
	for( pChar = charIter.first(); pChar; pChar = charIter.next() )
	{
		P_NPC pNPC = dynamic_cast<P_NPC>(pChar);

		// Find Owner
		if( pNPC && pNPC->owner() )
		{
			SERIAL owner = pNPC->owner()->serial();
			
			P_PLAYER pOwner = dynamic_cast<P_PLAYER>(FindCharBySerial( owner ));
			if( pOwner )
			{
				pNPC->setOwner( pOwner );
				pOwner->addPet( pNPC, true );
			}
			else
			{
				clConsole.send( tr( "The owner of Serial 0x%1 is invalid: %2" ).arg( pNPC->serial(), 16 ).arg( owner, 16 ) );
				pNPC->setOwner( NULL );
			}
		}

		// Find Guarding
		if( pChar->guarding() )
		{
			SERIAL guarding = (SERIAL)pChar->guarding();

			P_CHAR pGuarding = FindCharBySerial( guarding );
			if( pGuarding )
			{
				pChar->setGuarding( pGuarding );
				pGuarding->addGuard( pChar, true );
			}
			else
			{
				clConsole.send( tr( "The guard target of Serial 0x%1 is invalid: %2" ).arg( pChar->serial(), 16 ).arg( guarding, 16 ) );
				pChar->setGuarding( NULL );
			}
		}

		if( isItemSerial( pChar->multis() ) )
		{
			cMulti *pMulti = dynamic_cast< cMulti* >( FindItemBySerial( pChar->multis() ) );

			if( pMulti )
				pMulti->addChar( pChar );
		}

		cTerritory *region = AllTerritories::instance()->region( pChar->pos().x, pChar->pos().y, pChar->pos().map );
		pChar->setRegion( region );

		// Now that we have our owner set correctly
		// do the charflags
		setcharflag( pChar );
		pChar->flagUnchanged(); // We've just loaded, nothing changes
	}

	clConsole.ProgressDone();

	clConsole.PrepareProgress( "Deleting lost items" );

	// Do we have to delete items?
	for( P_ITEM pItem = deleteItems.first(); pItem; pItem = deleteItems.next() )
		quickdelete( pItem );

	clConsole.ProgressDone();

	if( deleteItems.count() > 0 )
	{
		clConsole.send( QString::number( deleteItems.count() ) + " deleted due to invalid container or position.\n" );
		deleteItems.clear();
	}

	persistentBroker->disconnect();

	clConsole.send("World Loading ");
	clConsole.ChangeColor( WPC_GREEN );
	clConsole.send( "Completed\n" );
	clConsole.ChangeColor( WPC_NORMAL );
}

void cWorld::save()
{
	clConsole.send( "Saving World..." );

	try
	{
		persistentBroker->connect( SrvParams->databaseHost(), SrvParams->databaseName(), SrvParams->databaseUsername(), SrvParams->databasePassword() );
	}
	catch( QString &e )
	{
		clConsole.log( LOG_FATAL, QString( "Couldn't open the database: %1\n" ).arg( e ) );
		return;
	}

	unsigned int startTime = getNormalizedTime();

	// Try to Benchmark
	SrvParams->flush();

	// Flush old items
	persistentBroker->flushDeleteQueue();
	
	p->purgePendingObjects();

	//persistentBroker->executeQuery( "BEGIN;" );

	try
	{
		cItemIterator iItems;
		for( P_ITEM pItem = iItems.first(); pItem; pItem = iItems.next() )
			persistentBroker->saveObject( pItem );

		cCharIterator iChars;
		for( P_CHAR pChar = iChars.first(); pChar; pChar = iChars.next() )
			persistentBroker->saveObject( pChar );
	}
	catch( QString &e )
	{
		//persistentBroker->executeQuery( "ROLLBACK;" );

		clConsole.ChangeColor( WPC_RED );
		clConsole.send( " Failed" );
		clConsole.ChangeColor( WPC_NORMAL );

		clConsole.log( LOG_ERROR, "Saving failed: " + e );
		return;
	}

	//persistentBroker->executeQuery( "COMMIT;" );

	ISerialization *archive = cPluginFactory::serializationArchiver( "xml" );
	archive->prepareWritting( "effects" );
	TempEffects::instance()->serialize( *archive );
	archive->close();
	delete archive;

	// Save the accounts
	Accounts::instance()->save();

	uiCurrentTime = getNormalizedTime();
	
	clConsole.ChangeColor( WPC_GREEN );
	clConsole.send( " Done" );
	clConsole.ChangeColor( WPC_NORMAL );

	persistentBroker->disconnect();

	clConsole.send( QString( " [%1ms]\n" ).arg( getNormalizedTime() - startTime ) );

}

void cWorld::registerObject( cUObject *object )
{
	if( !object )
	{
		clConsole.log( LOG_ERROR, "Couldn't register a NULL object in the world." );
		return;
	}

	registerObject( object->serial(), object );
}

void cWorld::registerObject( SERIAL serial, cUObject *object )
{
	if( !object )
	{
		clConsole.log( LOG_ERROR, "Trying to register a null object in the World." );
		return;
	}

	// Check if the Serial really is correct
	if( isItemSerial( serial ) )
	{
		ItemMap::iterator it = p->items.find( serial - ITEM_SPACE );

		if( it != p->items.end() )
		{
			clConsole.log( LOG_ERROR, QString( "Trying to register an item with the Serial 0x%1 which is already in use." ).arg( serial, 16 ) );
			return;
		}

		// Insert the Item into our Registry
		P_ITEM pItem = dynamic_cast< P_ITEM >( object );

		if( !pItem )
		{
			clConsole.log( LOG_ERROR, QString( "Trying to register an object with an item serial (0x%1) which is no item." ).arg( serial, 16 ) );
			return;
		}

		p->items.insert( std::make_pair( serial - ITEM_SPACE, pItem ) );
		_itemCount++;
		
		if( serial > _lastItemSerial )
			_lastItemSerial = serial;
	}
	else if( isCharSerial( serial ) )
	{
		CharMap::iterator it = p->chars.find( serial );

		if( it != p->chars.end() )
		{
			clConsole.log( LOG_ERROR, QString( "Trying to register a character with the Serial 0x%1 which is already in use." ).arg( QString::number( serial, 16 ) ) );
			return;
		}

		// Insert the Character into our Registry
		P_CHAR pChar = dynamic_cast< P_CHAR >( object );

		if( !pChar )
		{
			clConsole.log( LOG_ERROR, QString( "Trying to register an object with a character serial (0x%1) which is no character." ).arg( QString::number( serial, 16 ) ) );
			return;
		}

		p->chars.insert( std::make_pair( serial, pChar ) );
		_charCount++;

		if( serial > _lastCharSerial )
			_lastCharSerial = serial;
	}
	else
	{
		clConsole.log( LOG_ERROR, QString( "Tried to register an object with an invalid Serial (0x%1) in the World." ).arg( QString::number( serial, 16 ) ) );
		return;
	}
}

void cWorld::unregisterObject( cUObject *object )
{
	if( !object )
	{
		clConsole.log( LOG_ERROR, "Trying to unregister a null object from the world." );
		return;
	}

	unregisterObject( object->serial() );
}

void cWorld::unregisterObject( SERIAL serial )
{
	if( isItemSerial( serial ) )
	{
		ItemMap::iterator it = p->items.find( serial - ITEM_SPACE );

		if( it == p->items.end() )
		{
			clConsole.log( LOG_ERROR, QString( "Trying to unregister a non-existing item with the serial 0x%1." ).arg( serial, 0, 16 ) );
			return;
		}

		p->items.erase( it );
		_itemCount--;

		if( _lastItemSerial == serial )
			_lastItemSerial--;
	}
	else if( isCharSerial( serial ) )
	{
		CharMap::iterator it = p->chars.find( serial );

		if( it == p->chars.end() )
		{
			clConsole.log( LOG_ERROR, QString( "Trying to unregister a non-existing character with the serial 0x%1." ).arg( serial, 0, 16 ) );
			return;
		}

		p->chars.erase( it );
		_charCount--;

		if( _lastCharSerial == serial )
			_lastCharSerial--;
	}
	else
	{
		clConsole.log( LOG_ERROR, QString( "Trying to unregister an object with an invalid serial (0x%08x)." ).arg( serial ) );
		return;
	}
}

P_ITEM cWorld::findItem( SERIAL serial ) const
{
	if( !isItemSerial( serial ) )
		return 0;

	if( serial > _lastItemSerial )
		return 0;

	ItemMap::const_iterator it = p->items.find( serial - ITEM_SPACE );

	if( it == p->items.end() )
		return 0;

	return it->second;
}

P_CHAR cWorld::findChar( SERIAL serial ) const
{
	if( !isCharSerial( serial ) )
		return 0;

	if( serial > _lastCharSerial )
		return 0;

	CharMap::const_iterator it = p->chars.find( serial );

	if( it == p->chars.end() )
		return 0;

	return it->second;
}

P_OBJECT cWorld::findObject( SERIAL serial ) const
{
	if( isItemSerial( serial ) )
		return findItem( serial );
	else if( isCharSerial( serial ) )
		return findChar( serial );
	else
		return 0;
}

void cWorld::deleteObject( cUObject *object )
{
	if( !object )
	{
		clConsole.log( LOG_ERROR, "Tried to delete a null object from the worldsave." );
		return;
	}

	// Delete from Database
	object->del();

	// Mark it as Free
	object->free = true;

	p->pendingObjects.push_back( object );
}

// "Really" delete objects that are pending to be deleted.
void cWorld::purge()
{
	p->purgePendingObjects();
}

/*****************************************************************************
  cItemIterator member functions
 *****************************************************************************/

// Iterators
struct stItemIteratorPrivate
{
	ItemMap::const_iterator it;
};

cItemIterator::cItemIterator()
{
	p = new stItemIteratorPrivate;
}

cItemIterator::~cItemIterator()
{
	delete p;
}

P_ITEM cItemIterator::first()
{
	p->it = World::instance()->p->items.begin();
	return next();
}

P_ITEM cItemIterator::next()
{
	if( p->it == World::instance()->p->items.end() )
		return 0;

	if( p->it->second->free )
	{
		p->it++;
		return next();
	}

	return (p->it++)->second;
}

/*****************************************************************************
  cCharIterator member functions
 *****************************************************************************/

struct stCharIteratorPrivate
{
	CharMap::const_iterator it;
};

cCharIterator::cCharIterator()
{
	p = new stCharIteratorPrivate;
}

cCharIterator::~cCharIterator()
{
	delete p;
}

P_CHAR cCharIterator::first()
{
	p->it = World::instance()->p->chars.begin();
	return next();
}

P_CHAR cCharIterator::next()
{
	if( p->it == World::instance()->p->chars.end() )
		return 0;

	if( p->it->second->free )
	{
		p->it++;
		return next();
	}

	return (p->it++)->second;
}
