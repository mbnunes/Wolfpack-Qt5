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

// Wolfpack Includes
#include "world.h"
#include "console.h"

#include "serverconfig.h"
#include "dbdriver.h"
#include "progress.h"
#include "uotime.h"
#include "persistentbroker.h"
#include "accounts.h"
#include "inlines.h"
#include "guilds.h"
#include "basechar.h"
#include "network/network.h"
#include "player.h"
#include "npc.h"
#include "log.h"
#include "timing.h"
#include "scriptmanager.h"
#include "pythonscript.h"
#include "basics.h"
#include <sqlite.h>
#include <qfileinfo.h>
#include <qdir.h>

// Postprocessing stuff, can be deleted later on
#include "muls/maps.h"
#include "sectors.h"
#include "territories.h"

// Objects ( => Factory later on )
#include "uobject.h"
#include "items.h"
#include "multi.h"

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
typedef std::hash_map<SERIAL, P_ITEM> ItemMap;
typedef std::hash_map<SERIAL, P_CHAR> CharMap;
#else
#include <map>
typedef std::map<SERIAL, P_ITEM> ItemMap;
typedef std::map<SERIAL, P_CHAR> CharMap;
#endif

// Don't forget to change the version number before changing tableInfo!
//
// ONCE AGAIN, DON'T FORGET TO INCREASE THIS VALUE
#define DATABASE_VERSION 8
#define WP_DATABASE_VERSION "8"

// This is used for autocreating the tables
struct
{
	const char* name;
	const char* create;
} tableInfo[] =
{
{ "guilds", "CREATE TABLE guilds ( \
serial unsigned int(10) NOT NULL default '0', \
name varchar(255) NOT NULL default '', \
abbreviation varchar(6) NOT NULL default '', \
charta LONGTEXT NOT NULL, \
website varchar(255) NOT NULL default 'http://www.wpdev.org', \
alignment tinyint(2) NOT NULL default '0', \
leader unsigned int(10) NOT NULL default '0', \
founded int(11) NOT NULL default '0', \
guildstone unsigned int(10) NOT NULL default '0', \
PRIMARY KEY(serial) \
);" },
{ "guilds_members", "CREATE TABLE guilds_members ( \
guild unsigned int(10) NOT NULL default '0', \
player unsigned int(10) NOT NULL default '0', \
showsign unsigned tinyint(1) NOT NULL default '0', \
guildtitle varchar(255) NOT NULL default '', \
joined int(11) NOT NULL default '0', \
PRIMARY KEY(guild,player) \
);"},
{ "guilds_canidates", "CREATE TABLE guilds_canidates ( \
guild unsigned int(10) NOT NULL default '0', \
player unsigned int(10) NOT NULL default '0', \
PRIMARY KEY(guild,player) \
);"},
{ "settings", "CREATE TABLE settings ( \
option varchar(255) NOT NULL default '', \
value varchar(255) NOT NULL default '', \
PRIMARY KEY (option) \
);" },
{ "characters", "CREATE TABLE characters (\
serial unsigned int(10) NOT NULL default '0',\
name varchar(255) default NULL,\
title varchar(255) default NULL,\
creationdate varchar(19) default NULL,\
body unsigned smallint(5)  NOT NULL default '0',\
orgbody unsigned smallint(5)  NOT NULL default '0',\
skin unsigned smallint(5)  NOT NULL default '0',\
orgskin unsigned smallint(5)  NOT NULL default '0',\
saycolor unsigned smallint(5)  NOT NULL default '0',\
emotecolor unsigned smallint(5)  NOT NULL default '0',\
strength smallint(6) NOT NULL default '0',\
strengthmod smallint(6) NOT NULL default '0',\
dexterity smallint(6) NOT NULL default '0',\
dexteritymod smallint(6) NOT NULL default '0',\
intelligence smallint(6) NOT NULL default '0',\
intelligencemod smallint(6) NOT NULL default '0',\
maxhitpoints smallint(6) NOT NULL default '0',\
hitpoints smallint(6) NOT NULL default '0',\
maxstamina smallint(6) NOT NULL default '0',\
stamina smallint(6) NOT NULL default '0',\
maxmana smallint(6) default NULL,\
mana smallint(6) default NULL,\
karma int(11) NOT NULL default '0',\
fame int(11) NOT NULL default '0',\
kills unsigned int(10) NOT NULL default '0',\
deaths unsigned int(10) NOT NULL default '0',\
hunger unsigned int(10) NOT NULL default '0',\
poison tinyint(2) NOT NULL default '-1',\
murderertime unsigned int(10) NOT NULL default '0',\
criminaltime unsigned int(10) NOT NULL default '0',\
gender unsigned tinyint(1) NOT NULL default '0',\
propertyflags int(11)  NOT NULL default '0',\
murderer unsigned int(10) NOT NULL default '0',\
guarding unsigned int(10) NOT NULL default '0',\
hitpointsbonus smallint(6) NOT NULL default '0',\
staminabonus smallint(6) NOT NULL default '0',\
manabonus smallint(6) NOT NULL default '0',\
strcap tinyint(4)  NOT NULL default '125',\
dexcap tinyint(4)  NOT NULL default '125',\
intcap tinyint(4)  NOT NULL default '125',\
statcap tinyint(4)  NOT NULL default '225',\
baseid varchar(64) NOT NULL default '',\
direction unsigned tinyint(1) NOT NULL default '0',\
PRIMARY KEY (serial)\
);" },
{ "corpses", "CREATE TABLE corpses (\
serial unsigned int(10) NOT NULL default '0',\
bodyid unsigned smallint(5) NOT NULL default '0',\
hairstyle unsigned smallint(5) NOT NULL default '0',\
haircolor unsigned smallint(5) NOT NULL default '0',\
beardstyle unsigned smallint(5) NOT NULL default '0',\
beardcolor unsigned smallint(5) NOT NULL default '0',\
direction unsigned tinyint(1) NOT NULL default '0',\
charbaseid varchar(64) NOT NULL default '',\
murderer unsigned int(10) NOT NULL default '0',\
murdertime unsigned int(10) NOT NULL default '0',\
PRIMARY KEY (serial)\
);" },
{ "corpses_equipment", "CREATE TABLE corpses_equipment (\
serial unsigned int(10) NOT NULL default '0',\
layer unsigned tinyint(3)  NOT NULL default '0',\
item unsigned int(10) NOT NULL default '0',  \
PRIMARY KEY (serial,layer)\
);" },
{ "items", "CREATE TABLE items (\
serial unsigned int(10) NOT NULL default '0',\
id unsigned smallint(5) NOT NULL default '0',\
color unsigned smallint(5) NOT NULL default '0',\
cont unsigned int(10) NOT NULL default '0',\
layer unsigned tinyint(3) NOT NULL default '0',\
amount smallint(5)  NOT NULL default '0',\
hp smallint(6) NOT NULL default '0',\
maxhp smallint(6) NOT NULL default '0',\
movable tinyint(3)  NOT NULL default '0',\
owner unsigned int(10) NOT NULL default '0',\
visible tinyint(3)  NOT NULL default '0',\
priv tinyint(3)  NOT NULL default '0',\
baseid varchar(64) NOT NULL default '',\
PRIMARY KEY (serial)\
);" },
{ "npcs", "CREATE TABLE npcs (\
serial unsigned int(10) NOT NULL default '0',\
summontime int(11)  NOT NULL default '0',\
additionalflags int(11)  NOT NULL default '0',\
owner unsigned int(10) NOT NULL default '0',\
stablemaster unsigned int(10) NOT NULL default '0',\
ai varchar(255) default NULL,\
wandertype smallint(3) NOT NULL default '0',\
wanderx1 smallint(6) NOT NULL default '0',\
wanderx2 smallint(6) NOT NULL default '0',\
wandery1 smallint(6) NOT NULL default '0',\
wandery2 smallint(6) NOT NULL default '0',\
wanderradius smallint(6) NOT NULL default '0',\
PRIMARY KEY (serial)\
);" },
{ "players", "CREATE TABLE players (\
serial unsigned int(10) NOT NULL default '0',\
account varchar(16) default NULL,\
additionalflags int(10) NOT NULL default '0',\
visualrange unsigned tinyint(3) NOT NULL default '0',\
profile longtext,\
fixedlight unsigned tinyint(3) NOT NULL default '0',\
strlock tinyint(4) NOT NULL default '0',\
dexlock tinyint(4) NOT NULL default '0',\
intlock tinyint(4) NOT NULL default '0',\
maxcontrolslots tinyint(4) NOT NULL default '5',\
PRIMARY KEY (serial)\
);" },
{ "skills", "CREATE TABLE skills (\
serial unsigned int(10) NOT NULL default '0',\
skill unsigned tinyint(2) NOT NULL default '0',\
value smallint(6) NOT NULL default '0',\
locktype tinyint(4) default '0',\
cap smallint(6) default '0',\
PRIMARY KEY (serial,skill)\
);" },
{ "tags", "CREATE TABLE tags (\
serial unsigned int(10) NOT NULL default '0',\
name varchar(64) NOT NULL default '',\
type varchar(6) NOT NULL default '',\
value longtext NOT NULL,\
PRIMARY KEY (serial,name)\
);" },
{ "uobject", "CREATE TABLE uobject (\
name varchar(255) default NULL,\
serial unsigned int(10) NOT NULL default '0',\
multis unsigned int(10) NOT NULL default '0',\
pos_x unsigned smallint(5)  NOT NULL default '0',\
pos_y unsigned smallint(5)  NOT NULL default '0',\
pos_z tinyint(4) NOT NULL default '0',\
pos_map unsigned tinyint(1) NOT NULL default '0',  \
events varchar(255) default NULL,\
havetags unsigned tinyint(1) NOT NULL default '0',\
PRIMARY KEY (serial)\
);" },
{ "uobjectmap", "CREATE TABLE uobjectmap (\
serial unsigned int(10) NOT NULL default '0',\
type varchar(80)  NOT NULL default '',\
PRIMARY KEY (serial)\
);" },
{ "effects", "CREATE TABLE effects (\
id unsigned int(10) NOT NULL default '0',\
objectid varchar(64) NOT NULL,\
expiretime unsigned int(10) NOT NULL,\
dispellable tinyint(4) NOT NULL default '0',\
source unsigned int(10) NOT NULL default '0',\
destination unsigned int(10) NOT NULL default '0',\
PRIMARY KEY (id)\
);" },
{ "effects_properties", "CREATE TABLE effects_properties (\
id unsigned int(10) NOT NULL default '0',\
keyname varchar(64) NOT NULL,\
type varchar(64) NOT NULL,\
value text NOT NULL,\
PRIMARY KEY (id,keyname)\
);" },
{ "spawnregions", "CREATE TABLE spawnregions (\
spawnregion varchar(64) NOT NULL,\
serial unsigned int(10) NOT NULL default '0',\
PRIMARY KEY (spawnregion, serial)\
);" },
{ NULL, NULL }
};

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
	std::list<cUObject*> pendingObjects;

	void purgePendingObjects()
	{
		std::list<cUObject*>::const_iterator it;
		for ( it = pendingObjects.begin(); it != pendingObjects.end(); ++it )
		{
			delete * it;
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

	_npcCount = 0;
	_playerCount = 0;
	_charCount = 0;
	_itemCount = 0;
	lastTooltip = 0;
	_lastCharSerial = 0;
	_lastItemSerial = ITEM_SPACE;
	backupThreads.setAutoDelete(true);
}

/*!
	Destructs the world container and claims back the memory of it's contained objects
*/
cWorld::~cWorld()
{
	cBackupThread *thread;
	for (thread = backupThreads.first(); thread; thread = backupThreads.next()) {
		thread->wait();
	}

	// Free pending objects
	p->purgePendingObjects();

	// Destroy our private implementation
	delete p;
}


static void quickdelete( P_ITEM pi ) throw()
{
	// Minimal way of deleting an item
	pi->SetOwnSerial( -1 );

	PersistentBroker::instance()->addToDeleteQueue( "items", QString( "serial = '%1'" ).arg( pi->serial() ) );

	// Also delete all items inside if it's a container.
	cItem::ContainerContent container( pi->content() );
	cItem::ContainerContent::const_iterator it( container.begin() );
	cItem::ContainerContent::const_iterator end( container.end() );
	for ( ; it != end; ++it )
		quickdelete( *it );

	// if it is within a multi, delete it from the multis vector
	if ( pi->multi() )
	{
		pi->multi()->removeObject( pi );
	}

	MapObjects::instance()->remove( pi );
	World::instance()->unregisterObject( pi );
}

void cWorld::unload()
{
	cComponent::unload();
}

/*
	Load a tag from the worldsave
	The type byte has already been read.
	What remains is:
	8-bit value type
	32-bit serial
	32-bit value1
	32-bit value2 (only used for doubles)
*/
void cWorld::loadTag( cBufferedReader& reader, unsigned int version )
{
	cUObject *object = findObject( reader.readInt() );
	QString name = reader.readUtf8();
	cVariant variant;
	variant.serialize( reader, version );

	if ( object )
	{
		object->setTag( name, variant );
	}
}

void cWorld::loadBinary( QPtrList<PersistentObject> &objects )
{
		QString filename = Config::instance()->binarySavepath();

		if ( QFile::exists( filename ) )
		{
			cBufferedReader reader( "WOLFPACK", DATABASE_VERSION );
			reader.open( filename );

			Console::instance()->log( LOG_MESSAGE, QString( "Loading %1 objects from %2.\n" ).arg( reader.objectCount() ).arg( filename ) );
			Console::instance()->send( "0%" );

			unsigned char type;
			const QMap<unsigned char, QCString> &typemap = reader.typemap();
			unsigned int loaded = 0;
			unsigned int count = reader.objectCount();
			unsigned int lastpercent = 0;
			unsigned int percent = 0;
			do
			{
				type = reader.readByte();
				if ( typemap.contains( type ) )
				{
					PersistentObject *object = PersistentFactory::instance()->createObject( typemap[type] );

					if (object) {
						try {
							object->load( reader );
							objects.append(object);
						} catch (wpException e) {
							Console::instance()->log( LOG_WARNING, e.error() + "\n" );
						}
					} else {
						// Skip an unknown object type.
					}

					loaded += 100;
					percent = loaded / count;
					if ( percent != lastpercent )
					{
						unsigned int revert = QString::number( lastpercent ).length() + 1;
						for ( unsigned int i = 0; i < revert; ++i )
						{
							Console::instance()->send( "\b" );
						}

						lastpercent = percent;
						Console::instance()->send( QString::number( percent ) + "%" );
					}
					// Special Type for Tags
				}
				else if ( type == 0xFA )
				{
					QString spawnregion = reader.readUtf8();
					SERIAL serial = reader.readInt();

					cSpawnRegion *region = SpawnRegions::instance()->region( spawnregion );
					cUObject *object = findObject( serial );
					if (object && region) {
						object->setSpawnregion(region);
					} else if (object) {
						object->remove();
					}
				}
				else if ( type == 0xFB )
				{
					QString name = reader.readUtf8();
					QString value = reader.readUtf8();
					setOption(name, value);
				}
				else if ( type == 0xFC )
				{
					Timers::instance()->load(reader);
				}
				else if ( type == 0xFD )
				{
					cGuild *guild = 0;

					try {
						guild = new cGuild();
						guild->load(reader, reader.version());
						Guilds::instance()->registerGuild(guild);
					} catch (wpException e) {
						delete guild;
					}
				}
				else if ( type == 0xFE )
				{
					loadTag( reader, reader.version() );
				}
				else if ( type != 0xFF )
				{
					Console::instance()->log(LOG_ERROR, QString( "Invalid worldfile, unknown and unskippable type %1." ).arg( type ) );
					return;
				}
			}
			while ( type != 0xFF );
			reader.close();

			// post process all loaded objects
			QPtrList<PersistentObject>::const_iterator cit(objects.begin());
			
			while (cit != objects.end()) {
				(*cit)->postload(reader.version());
				++cit;
			}
		}
}

void cWorld::loadSQL( QPtrList<PersistentObject> &objects )
{
	if ( !PersistentBroker::instance()->openDriver( Config::instance()->databaseDriver() ) )
		{
			Console::instance()->log( LOG_ERROR, QString( "Unknown Worldsave Database Driver '%1', check your wolfpack.xml" ).arg( Config::instance()->databaseDriver() ) );
			return;
		}

		if ( !PersistentBroker::instance()->connect( Config::instance()->databaseHost(), Config::instance()->databaseName(), Config::instance()->databaseUsername(), Config::instance()->databasePassword() ) )
		{
			throw QString( "Unable to open the world database." );
		}

		QString objectID;
		register unsigned int i = 0;

		while ( tableInfo[i].name )
		{
			if ( !PersistentBroker::instance()->tableExists( tableInfo[i].name ) )
			{
				PersistentBroker::instance()->executeQuery( tableInfo[i].create );

				// create default settings
				if ( !strcmp( tableInfo[i].name, "settings" ) )
				{
					setOption("db_version", WP_DATABASE_VERSION);
				}
			}

			++i;
		}

		// Load Options
		QString settingsSql = "SELECT option,value FROM settings;";
		if (Config::instance()->databaseDriver() == "mysql") {
			settingsSql = "SELECT `option`,`value` FROM `settings`;";
		}
		cDBResult oresult = PersistentBroker::instance()->query(settingsSql);
		while (oresult.fetchrow()) {
			setOption(oresult.getString(0), oresult.getString(1));
		}
		oresult.free();

		// Get Database Version (Since Version 7 SQL has it)
		QString db_version;
		getOption("db_version", db_version, "7");
		
		if (db_version.toInt() != DATABASE_VERSION) {
			cPythonScript *script = ScriptManager::instance()->getGlobalHook(EVENT_UPDATEDATABASE);
			if (!script || !script->canHandleEvent(EVENT_UPDATEDATABASE)) {
				throw wpException(QString("Unable to load world database. Version mismatch: %1 != %2.").arg(db_version.toInt()).arg(DATABASE_VERSION));
			}

			PyObject *args = Py_BuildValue("(ii)", DATABASE_VERSION, db_version.toInt());
			bool result = script->callEventHandler(EVENT_UPDATEDATABASE, args);
			Py_DECREF(args);
			
			if (!result) {
				throw wpException(QString("Unable to load world database. Version mismatch: %1 != %2.").arg(db_version.toInt()).arg(DATABASE_VERSION));
			}
		}

		QStringList types = PersistentFactory::instance()->objectTypes();

		for ( uint j = 0; j < types.count(); ++j )
		{
			QString type = types[j];

			QString countQuery = PersistentFactory::instance()->findSqlCountQuery(type);
			cDBResult res = PersistentBroker::instance()->query( countQuery );

			// Find out how many objects of this type are available
			if ( !res.isValid() )
				throw PersistentBroker::instance()->lastError();

			res.fetchrow();
			Q_UINT32 count = res.getInt( 0 );
			res.free();

			if ( count == 0 )
				continue; // Move on...

			Console::instance()->send( "\n" + tr( "Loading " ) + QString::number( count ) + tr( " objects of type " ) + type );

			res = PersistentBroker::instance()->query( PersistentFactory::instance()->findSqlQuery( type ) );

			// Error Checking
			if ( !res.isValid() )
				throw PersistentBroker::instance()->lastError();

			//Q_UINT32 sTime = getNormalizedTime();
			PersistentObject* object;
			progress_display progress( count );

			// Fetch row-by-row
			PersistentBroker::instance()->driver()->setActiveConnection( CONN_SECOND );
			while ( res.fetchrow() ) {
				unsigned short offset = 0;
				char** row = res.data();

				// do something with data
				object = PersistentFactory::instance()->createObject( type );
				object->load( row, offset );
				objects.append(object);

				++progress;
			}

			while ( progress.count() < progress.expected_count() )
				++progress;

			res.free();
			PersistentBroker::instance()->driver()->setActiveConnection();
		}

		// Load Temporary Effects
		Timers::instance()->load();

		// It's not possible to use cItemIterator during postprocessing because it skips lingering items
		ItemMap::iterator iter;
		QPtrList<cItem> deleteItems;

		for ( iter = p->items.begin(); iter != p->items.end(); ++iter )
		{
			P_ITEM pi = iter->second;
			SERIAL contserial = reinterpret_cast<SERIAL>( pi->container() );

			SERIAL multiserial = ( SERIAL ) ( pi->multi() );
			cMulti* multi = dynamic_cast<cMulti*>( findItem( multiserial ) );
			pi->setMulti( multi );
			if ( multi )
			{
				multi->addObject( pi );
			}

			if ( !contserial )
			{
				pi->setUnprocessed( false ); // This is for safety reasons
				int max_x = Maps::instance()->mapTileWidth( pi->pos().map ) * 8;
				int max_y = Maps::instance()->mapTileHeight( pi->pos().map ) * 8;
				if ( pi->pos().x > max_x || pi->pos().y > max_y )
				{
					Console::instance()->log( LOG_ERROR, QString( "Item with invalid position %1,%2,%3,%4.\n" ).arg( pi->pos().x ).arg( pi->pos().y ).arg( pi->pos().z ).arg( pi->pos().map ) );
					deleteItems.append( pi );
					continue;
				}
				else
				{
					MapObjects::instance()->add( pi );
				}
			}
			else
			{
				// 1. Handle the Container Value
				if ( isItemSerial( contserial ) )
				{
					P_ITEM pCont = FindItemBySerial( contserial );

					if ( pCont )
					{
						pCont->addItem( pi, false, true, true );
					}
					else
					{
						Console::instance()->log( LOG_ERROR, QString( "Item with invalid container [0x%1].\n" ).arg( contserial, 0, 16 ) );
						deleteItems.append( pi ); // Queue this item up for deletion
						continue; // Skip further processing
					}
				}
				else if ( isCharSerial( contserial ) )
				{
					P_CHAR pCont = FindCharBySerial( contserial );

					if ( pCont )
					{
						pCont->addItem( ( cBaseChar::enLayer ) pi->layer(), pi, true, true );
					}

					if ( !pCont || pi->container() != pCont )
					{
						Console::instance()->log( LOG_ERROR, QString( "Item with invalid wearer [%1].\n" ).arg( contserial ) );
						deleteItems.append( pi );
						continue;
					}
				}

				pi->setUnprocessed( false );
			}

			pi->flagUnchanged(); // We've just loaded, nothing changes.
		}

		// Post Process Characters
		cCharIterator charIter;
		P_CHAR pChar;
		for ( pChar = charIter.first(); pChar; pChar = charIter.next() )
		{
			P_NPC pNPC = dynamic_cast<P_NPC>( pChar );

			// Find Guarding
			// this needs to move to postprocessing
			if ( pChar->guarding() )
			{
				SERIAL guarding = ( SERIAL ) pChar->guarding();

				P_CHAR pGuarding = FindCharBySerial( guarding );
				if ( pGuarding )
				{
					pChar->setGuarding( pGuarding );
					pGuarding->addGuard( pChar, true );
				}
				else
				{
					Console::instance()->send( tr( "The guard target of Serial 0x%1 is invalid: %2" ).arg( pChar->serial(), 16 ).arg( guarding, 16 ) );
					pChar->setGuarding( 0 );
				}
			}

			cTerritory* region = Territories::instance()->region( pChar->pos().x, pChar->pos().y, pChar->pos().map );
			pChar->setRegion( region );

			SERIAL multiserial = ( SERIAL ) ( pChar->multi() );
			cMulti* multi = dynamic_cast<cMulti*>( findItem( multiserial ) );
			pChar->setMulti( multi );
			if ( multi )
			{
				multi->addObject( pChar );
			}

			pChar->flagUnchanged(); // We've just loaded, nothing changes
		}
	
		// post process all loaded objects
		// Note from DarkStorm: I THINK it's important to do this before
		// the deletion of objects, otherwise you might have items in this
		// list that are already deleted.
		QPtrList<PersistentObject>::const_iterator cit(objects.begin());
		
		while (cit != objects.end()) {
			(*cit)->postload(0);
			++cit;
		}

		if ( deleteItems.count() > 0 )
		{
			// Do we have to delete items?
			for ( P_ITEM pItem = deleteItems.first(); pItem; pItem = deleteItems.next() )
				quickdelete( pItem );

			Console::instance()->send( QString::number( deleteItems.count() ) + " deleted due to invalid container or position.\n" );
			deleteItems.clear();
		}
	
		// Load SpawnRegion information
		cDBResult result = PersistentBroker::instance()->query( "SELECT spawnregion,serial FROM spawnregions;" );

		while ( result.fetchrow() )
		{
			QString spawnregion = result.getString( 0 );
			SERIAL serial = result.getInt( 1 );

			cSpawnRegion *region = SpawnRegions::instance()->region( spawnregion );
			cUObject *object = findObject( serial );
			if (object && region) {
				object->setSpawnregion(region);
			} else if (object) {
				object->remove();
			}
		}

		result.free();

		Guilds::instance()->load();

		// load server time from db
		QString db_time;
		QString default_time = Config::instance()->getString( "General", "UO Time", "", true );
		getOption( "worldtime", db_time, default_time );
		UoTime::instance()->setMinutes( db_time.toInt() );

		PersistentBroker::instance()->disconnect();
}

void cWorld::load()
{
	unsigned int loadStart = getNormalizedTime();
	QPtrList<PersistentObject> objects;

	if ( Config::instance()->databaseDriver() == "binary" )
	{
		loadBinary( objects );
	}
	else
	{
		loadSQL( objects );
	}
	

	
	unsigned int duration = getNormalizedTime() - loadStart;

	Console::instance()->send( "\n\b\b\b\b" ); // 100%
	Console::instance()->log( LOG_MESSAGE, QString( "\nThe world loaded in %1 ms.\n" ).arg( duration ) );

	cComponent::load();
}

void cWorld::save() {
	// Broadcast a message to all connected clients
	Network::instance()->broadcast(tr("Worldsave Initialized"));

	Console::instance()->send( "Saving World..." );
	setOption("db_version", WP_DATABASE_VERSION); // Make SURE it's saved

	// Send a nice status gump to all sockets if enabled
	bool fancy = Config::instance()->getBool( "General", "Fancy Worldsave Status", true, true );
	if ( fancy )
	{
		// Create a fancy gump as promised
		cGump gump;
		gump.setNoClose( true );
		gump.setNoDispose( true );
		gump.setNoMove( true );
		gump.setX( -10 );
		gump.setY( -10 );
		gump.setType( 0x98FA2C10 );

		gump.addResizeGump( 0, 0, 9200, 291, 90 );
		gump.addCheckertrans( 0, 0, 291, 90 );
		gump.addText( 47, 19, tr( "WORLDSAVE IN PROGRESS" ), 2122 );
		gump.addText( 47, 37, tr( "Saving %1 items." ).arg( itemCount() ), 2100 );
		gump.addText( 47, 55, tr( "Saving %1 characters." ).arg( charCount() ), 2100 );
		gump.addTilePic( 3, 25, 4167 );

		// Send it to all connected ingame sockets
		for ( cUOSocket*socket = Network::instance()->first(); socket; socket = Network::instance()->next() )
		{
			socket->send( new cGump( gump ) );
		}
	}

	unsigned int startTime = getNormalizedTime();

	try
	{
		// Save the Current Time
		setOption("worldtime", QString::number( UoTime::instance()->getMinutes() ));

		if (Config::instance()->databaseDriver() == "binary") {
			// Make a backup of the old world.
			backupWorld(Config::instance()->binarySavepath(), Config::instance()->binaryBackups(), Config::instance()->binaryCompressBackups());

			// Save in binary format
			cBufferedWriter writer( "WOLFPACK", DATABASE_VERSION );
			writer.open( Config::instance()->binarySavepath() );

			cCharIterator charIterator;
			P_CHAR character;
			for (character = charIterator.first(); character; character = charIterator.next()) {
				if (!character->multi()) {
					character->save(writer);
				}
			}

			cItemIterator itemIterator;
			P_ITEM item;
			for (item = itemIterator.first(); item; item = itemIterator.next()) {
				if (!item->container() && !item->multi()) {
					item->save( writer );
				}
			}

			// Write Guilds
			cGuilds::iterator git;
			for (git = Guilds::instance()->begin(); git != Guilds::instance()->end(); ++git) {
				(*git)->save(writer, writer.version());
			}

			// Write Temporary Effects
			Timers::instance()->save(writer);

			// Write Options
			QMap<QString, QString>::iterator oit;
			for (oit = options.begin(); oit != options.end(); ++oit) {
				writer.writeByte(0xFB);
				writer.writeUtf8(oit.key());
				writer.writeUtf8(oit.data());
			}

			writer.writeByte( 0xFF ); // Terminator Type
			writer.close();

			Config::instance()->flush();
			p->purgePendingObjects();
		} else {
			if (!PersistentBroker::instance()->openDriver( Config::instance()->databaseDriver())) {
				Console::instance()->log( LOG_ERROR, QString( "Unknown Worldsave Database Driver '%1', check your wolfpack.xml" ).arg( Config::instance()->databaseDriver() ) );
				return;
			}

			try {
				PersistentBroker::instance()->connect( Config::instance()->databaseHost(), Config::instance()->databaseName(), Config::instance()->databaseUsername(), Config::instance()->databasePassword() );
			} catch (QString& e) {
				Console::instance()->log( LOG_ERROR, QString( "Couldn't open the database: %1\n" ).arg( e ) );
				return;
			}

			unsigned int i = 0;

			while ( tableInfo[i].name )
			{
				if ( !PersistentBroker::instance()->tableExists( tableInfo[i].name ) )
				{
					PersistentBroker::instance()->executeQuery( tableInfo[i].create );
				}

				++i;
			}

			// Try to Benchmark
			Config::instance()->flush();

			// Flush old items
			PersistentBroker::instance()->flushDeleteQueue();

			p->purgePendingObjects();

			PersistentBroker::instance()->startTransaction();

			PersistentBroker::instance()->executeQuery( "DELETE FROM spawnregions;" );

			cItemIterator iItems;
			for ( P_ITEM pItem = iItems.first(); pItem; pItem = iItems.next() )
			{
				PersistentBroker::instance()->saveObject( pItem );

				if ( pItem->spawnregion() )
				{
					QString name = PersistentBroker::instance()->quoteString( pItem->spawnregion()->name() );
					QString query = QString( "INSERT INTO spawnregions VALUES('%1',%2);" ).arg( name ).arg( pItem->serial() );
					PersistentBroker::instance()->executeQuery( query );
				}
			}

			cCharIterator iChars;
			for ( P_CHAR pChar = iChars.first(); pChar; pChar = iChars.next() )
			{
				PersistentBroker::instance()->saveObject( pChar );

				if ( pChar->spawnregion() )
				{
					QString name = PersistentBroker::instance()->quoteString( pChar->spawnregion()->name() );
					QString query = QString( "INSERT INTO spawnregions VALUES('%1',%2);" ).arg( name ).arg( pChar->serial() );
					PersistentBroker::instance()->executeQuery( query );
				}
			}

			Timers::instance()->save();

			Guilds::instance()->save();

            PersistentBroker::instance()->commitTransaction();

			// Write Options
			PersistentBroker::instance()->executeQuery("DELETE FROM settings;");

			QMap<QString, QString>::iterator oit;
			for (oit = options.begin(); oit != options.end(); ++oit) {
				QString sql = QString("INSERT INTO settings VALUES('%1','%2');")
					.arg(PersistentBroker::instance()->quoteString(oit.key()))
					.arg(PersistentBroker::instance()->quoteString(oit.data()));
				PersistentBroker::instance()->executeQuery(sql);
			}

			PersistentBroker::instance()->disconnect();
		}

		// Save the accounts
		Accounts::instance()->save();

		Server::instance()->refreshTime();

		Console::instance()->changeColor( WPC_GREEN );
		Console::instance()->send( " Done" );
		Console::instance()->changeColor( WPC_NORMAL );

		Console::instance()->send( QString( " [%1ms]\n" ).arg( Server::instance()->time() - startTime ) );
	}
	catch ( QString& e )
	{
		PersistentBroker::instance()->rollbackTransaction();

		Console::instance()->changeColor( WPC_RED );
		Console::instance()->send( " Failed\n" );
		Console::instance()->changeColor( WPC_NORMAL );

		Console::instance()->log( LOG_ERROR, "Saving failed: " + e );
	}

	// Broadcast a message to all connected clients
	Network::instance()->broadcast(tr("Worldsave Completed In %1ms").arg(Server::instance()->time() - startTime));

	if ( fancy )
	{
		cUOTxCloseGump close;
		close.setType( 0x98FA2C10 );

		for ( cUOSocket*socket = Network::instance()->first(); socket; socket = Network::instance()->next() )
		{
			socket->send( &close );
		}
	}

	Timing::instance()->setLastWorldsave( getNormalizedTime() );
}

/*
 * Gets a value from the settings table and returns the value
 */
void cWorld::getOption( const QString& name, QString& value, const QString fallback )
{
	QMap<QString, QString>::iterator it = options.find(name);	
	if (it == options.end()) {
		value = fallback;
	} else {
		value = it.data();
	}
}

/*
 * Sets a value in the settings table.
 */
void cWorld::setOption( const QString& name, const QString& value )
{
	options.insert(name, value, true);
}

void cWorld::registerObject( cUObject* object )
{
	if ( !object )
	{
		Console::instance()->log( LOG_ERROR, "Couldn't register a NULL object in the world." );
		return;
	}

	registerObject( object->serial(), object );
}

void cWorld::registerObject( SERIAL serial, cUObject* object )
{
	if ( !object )
	{
		Console::instance()->log( LOG_ERROR, "Trying to register a null object in the World." );
		return;
	}

	// Check if the Serial really is correct
	if ( isItemSerial( serial ) )
	{
		ItemMap::iterator it = p->items.find( serial - ITEM_SPACE );

		if ( it != p->items.end() )
		{
			Console::instance()->log( LOG_ERROR, QString( "Trying to register an item with the Serial 0x%1 which is already in use." ).arg( serial, 0, 16 ) );
			return;
		}

		// Insert the Item into our Registry
		P_ITEM pItem = dynamic_cast<P_ITEM>( object );

		if ( !pItem )
		{
			Console::instance()->log( LOG_ERROR, QString( "Trying to register an object with an item serial (0x%1) which is no item." ).arg( serial, 0, 16 ) );
			return;
		}

		p->items.insert( std::make_pair( serial - ITEM_SPACE, pItem ) );
		_itemCount++;

		if ( serial > _lastItemSerial )
			_lastItemSerial = serial;
	}
	else if ( isCharSerial( serial ) )
	{
		CharMap::iterator it = p->chars.find( serial );

		if ( it != p->chars.end() )
		{
			Console::instance()->log( LOG_ERROR, QString( "Trying to register a character with the Serial 0x%1 which is already in use." ).arg( QString::number( serial, 0, 16 ) ) );
			return;
		}

		// Insert the Character into our Registry
		P_CHAR pChar = dynamic_cast<P_CHAR>( object );

		if ( !pChar )
		{
			Console::instance()->log( LOG_ERROR, QString( "Trying to register an object with a character serial (0x%1) which is no character." ).arg( QString::number( serial, 0, 16 ) ) );
			return;
		}

		p->chars.insert( std::make_pair( serial, pChar ) );
		_charCount++;

		if ( serial > _lastCharSerial )
			_lastCharSerial = serial;

		if ( pChar->objectType() == enPlayer )
		{
			++_playerCount;
		}
		else if ( pChar->objectType() == enNPC )
		{
			++_npcCount;
		}
	}
	else
	{
		Console::instance()->log( LOG_ERROR, QString( "Tried to register an object with an invalid Serial (0x%1) in the World." ).arg( QString::number( serial, 0, 16 ) ) );
		return;
	}
}

void cWorld::unregisterObject( cUObject* object )
{
	if ( !object )
	{
		Console::instance()->log( LOG_ERROR, "Trying to unregister a null object from the world." );
		return;
	}

	unregisterObject( object->serial() );
}

void cWorld::unregisterObject( SERIAL serial )
{
	if ( isItemSerial( serial ) )
	{
		ItemMap::iterator it = p->items.find( serial - ITEM_SPACE );

		if ( it == p->items.end() )
		{
			Console::instance()->log( LOG_ERROR, QString( "Trying to unregister a non-existing item with the serial 0x%1." ).arg( serial, 0, 16 ) );
			return;
		}

		p->items.erase( it );
		_itemCount--;

		if ( _lastItemSerial == serial )
			_lastItemSerial--;
	}
	else if ( isCharSerial( serial ) )
	{
		CharMap::iterator it = p->chars.find( serial );

		if ( it == p->chars.end() )
		{
			Console::instance()->log( LOG_ERROR, QString( "Trying to unregister a non-existing character with the serial 0x%1." ).arg( serial, 0, 16 ) );
			return;
		}

		P_CHAR pChar = it->second;
		if ( pChar->objectType() == enPlayer )
		{
			--_playerCount;
		}
		else if ( pChar->objectType() == enNPC )
		{
			--_npcCount;
		}

		p->chars.erase( it );
		_charCount--;

		if ( _lastCharSerial == serial )
			_lastCharSerial--;
	}
	else
	{
		Console::instance()->log( LOG_ERROR, QString( "Trying to unregister an object with an invalid serial (0x%1)." ).arg( serial, 0, 16 ) );
		return;
	}
}

P_ITEM cWorld::findItem( SERIAL serial ) const
{
	if ( !isItemSerial( serial ) )
		return 0;

	if ( serial > _lastItemSerial )
		return 0;

	ItemMap::const_iterator it = p->items.find( serial - ITEM_SPACE );

	if ( it == p->items.end() )
		return 0;

	return it->second;
}

P_CHAR cWorld::findChar( SERIAL serial ) const
{
	if ( !isCharSerial( serial ) )
		return 0;

	if ( serial > _lastCharSerial )
		return 0;

	CharMap::const_iterator it = p->chars.find( serial );

	if ( it == p->chars.end() )
		return 0;

	return it->second;
}

P_OBJECT cWorld::findObject( SERIAL serial ) const
{
	if ( isItemSerial( serial ) )
		return findItem( serial );
	else if ( isCharSerial( serial ) )
		return findChar( serial );
	else
		return 0;
}

void cWorld::deleteObject( cUObject* object )
{
	if ( !object )
	{
		Console::instance()->log( LOG_ERROR, "Tried to delete a null object from the worldsave." );
		return;
	}

	// Delete from Database
	object->del();

	// Mark it as Free
	object->free = true;

	p->pendingObjects.push_back( object );
	unregisterObject( object );
}

// "Really" delete objects that are pending to be deleted.
void cWorld::purge()
{
	p->purgePendingObjects();
}

QMap<QDateTime, QString> listBackups(const QString &filename) {
	// Get the path the file is in
	QString name = QFileInfo(filename).baseName(false);

	QDir dir = QFileInfo(filename).dir();
	QStringList entries = dir.entryList(name + "*", QDir::Files);
	QMap<QDateTime, QString> backups;

	QStringList::iterator sit;
	for (sit = entries.begin(); sit != entries.end(); ++sit) {
		QString backup = QFileInfo(*sit).baseName(false);
		QString timestamp = backup.right(backup.length() - name.length());
		QDate date;
		QTime time;
		
		// Length has to be -YYYYMMDD-HHMM (14 Characters)
		if (timestamp.length() != 14) {
			continue;
		}

		bool ok[5];
		int year = timestamp.mid(1, 4).toInt(&ok[0]);
		int month = timestamp.mid(5, 2).toInt(&ok[1]);
		int day = timestamp.mid(7, 2).toInt(&ok[2]);
		int hour = timestamp.mid(10, 2).toInt(&ok[3]);
		int minute = timestamp.mid(12, 2).toInt(&ok[4]);

		if (!ok[0] || !ok[1] || !ok[2] || !ok[3] || !ok[4]) {
			continue;
		}

		date.setYMD(year, month, day);
		time.setHMS(hour, minute, 0);
		
		backups.insert(QDateTime(date, time), *sit);
	}

	return backups;
}

/*
	Backup old worldfile
*/
void cWorld::backupWorld(const QString &filename, unsigned int count, bool compress) {
	// Looks like there is nothing to backup
	if (count == 0 || !QFile::exists(filename)) {
		return;
	}

	// Check if we need to remove a previous backup
	QMap<QDateTime, QString> backups = listBackups(filename);
	
	if (backups.count() >= count) {
		// Remove the oldest backup
		QDateTime current;
		QString backup = QString::null;

		QMap<QDateTime, QString>::iterator it;
		for (it = backups.begin(); it != backups.end(); ++it) {
			if (current.isNull() || it.key() < current) {
				current = it.key();
				backup = it.data();
			}
		}

		if (!backup.isNull() && !QFile::remove(backup)) {
			Console::instance()->log(LOG_ERROR, QString("Unable to remove backup %1. No new backup has been created.\n").arg(backup));
			return;
		}
	}

	// Rename the old worldfile to the new backup name
	QString backupName = QFileInfo(filename).dirPath(true) + QDir::separator() + QFileInfo(filename).baseName(false);
	QDateTime current = QDateTime::currentDateTime();
	backupName.append(current.toString("-yyyyMMdd-hhmm")); // Append Timestamp
	backupName.append(".");
	backupName.append(QFileInfo(filename).extension(true)); // Append Extension

	// Rename the old worldfile
	QDir dir = QDir::current();
	dir.rename(filename, backupName, true);

	// Start the compression thread if requested by the user
	cBackupThread *backupThread = new cBackupThread();
	backupThread->setFilename(backupName);
	backupThread->start(QThread::LowPriority);
}

extern "C" {
	extern void *gzopen(const char *path, const char *mode);
	extern int gzwrite(void *file, void *buf, unsigned len);
	extern int gzclose(void *file);
};

/*
	Pipe a backup trough
*/
void cBackupThread::run() {
	// Open the backup input file and the backup output file and compress
	QFile input(filename);
	QString outputName = filename + ".gz";

	if (!input.open(IO_ReadOnly)) {
		return;
	}

	void *output = gzopen(outputName.latin1(), "wb");
	if (!output) {
		input.close();
		return;
	}

	int readSize;
	char buffer[4096];
	while ((readSize = input.readBlock(buffer, 4096)) > 0) {
		gzwrite(output, buffer, readSize);
	}
	
	input.close();
	gzclose(output);

	input.remove();
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
	if ( p->it == World::instance()->p->items.end() )
		return 0;

	if ( p->it->second->free )
	{
		p->it++;
		return next();
	}

	return ( p->it++ )->second;
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
	if ( p->it == World::instance()->p->chars.end() )
		return 0;

	if ( p->it->second->free )
	{
		p->it++;
		return next();
	}

	return ( p->it++ )->second;
}