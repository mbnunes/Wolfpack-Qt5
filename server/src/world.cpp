//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 2001-2004 by holders identified in authors.txt
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
#include "world.h"
#include "console.h"
#include "globals.h"
#include "srvparams.h"
#include "dbdriver.h"
#include "progress.h"
#include "persistentbroker.h"
#include "accounts.h"
#include "inlines.h"
#include "guilds.h"
#include "basechar.h"
#include "network.h"
#include "player.h"
#include "npc.h"
#include "log.h"
#include "Timing.h"
#include "basics.h"
#include <sqlite.h>

// Postprocessing stuff, can be deleted later on
#include "maps.h"
#include "sectors.h"
#include "territories.h"
#include "spawnregions.h"

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

// Don't forget to change the version number before changing tableInfo!
#define WP_DATABASE_VERSION "1"

// This is used for autocreating the tables
struct {
	const char *name;
	const char *create;
} tableInfo[] =
{
	{ "guilds", "CREATE TABLE guilds ( \
	serial int(11) NOT NULL default '0', \
	name varchar(255) NOT NULL default '', \
	abbreviation varchar(255) NOT NULL default '', \
	charta LONGTEXT NOT NULL, \
	website varchar(255) NOT NULL default '', \
	alignment int(2) NOT NULL default '0', \
	leader int(11) NOT NULL default '-1', \
	founded int(11) NOT NULL default '0', \
	guildstone int(11) NOT NULL default '-1', \
	PRIMARY KEY(serial) \
	);" },

	{ "guilds_members",	"CREATE TABLE guilds_members ( \
	guild int(11) NOT NULL default '0', \
	player int(11) NOT NULL default '0', \
	showsign int(1) NOT NULL default '0', \
	guildtitle varchar(255) NOT NULL default '', \
	joined int(11) NOT NULL default '0', \
	PRIMARY KEY(guild,player) \
	);"},

	{ "guilds_canidates", "CREATE TABLE guilds_canidates ( \
	guild int(11) NOT NULL default '0', \
	player int(11) NOT NULL default '0', \
	PRIMARY KEY(guild,player) \
	);"},

	{ "settings", "CREATE TABLE settings ( \
	option varchar(255) NOT NULL default '', \
	value varchar(255) NOT NULL default '', \
	PRIMARY KEY (option) \
	);" },

	{ "boats", "CREATE TABLE boats ( \
	serial int(11) NOT NULL default '0', \
	autosail tinyint(1) NOT NULL default '0', \
	boatdir tinyint(1) NOT NULL default '0', \
	itemserial1 int(11) NOT NULL default '-1', \
	itemserial2 int(11) NOT NULL default '-1', \
	itemserial3 int(11) NOT NULL default '-1', \
	itemserial4 int(11) NOT NULL default '-1', \
	multi1 smallint(6) default '0', \
	multi2 smallint(6) default '0', \
	multi3 smallint(6) default '0', \
	multi4 smallint(6) default '0', \
	PRIMARY KEY (serial) \
	);" },

	{ "boats_itemids", "CREATE TABLE boats_itemids ( \
	serial int(11) NOT NULL default '0', \
	a tinyint(1)  NOT NULL default '0', \
	b tinyint(1)  NOT NULL default '0', \
	id smallint(6)  default '0', \
	PRIMARY KEY (serial) \
	);" },

	{ "boats_itemoffsets", "CREATE TABLE boats_itemoffsets (\
	serial int(11) NOT NULL default '0',\
	a tinyint(1)  NOT NULL default '0',\
	b tinyint(1)  NOT NULL default '0',\
	c tinyint(1)  NOT NULL default '0',\
	offset smallint(6) default '0',\
	PRIMARY KEY (serial)\
	);" },

	{ "characters", "CREATE TABLE characters (\
	serial int(11) NOT NULL default '0',\
	name varchar(255) default NULL,\
	title varchar(255) default NULL,\
	creationdate varchar(255) default NULL,\
	body smallint(5)  NOT NULL default '0',\
	orgbody smallint(5)  NOT NULL default '0',\
	skin smallint(5)  NOT NULL default '0',\
	orgskin smallint(5)  NOT NULL default '0',\
	saycolor smallint(5)  NOT NULL default '0',\
	emotecolor smallint(5)  NOT NULL default '0',\
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
	kills int(10)  NOT NULL default '0',\
	deaths int(10)  NOT NULL default '0',\
	def int(10)  NOT NULL default '0',\
	hunger int(11) NOT NULL default '0',\
	poison int(11) NOT NULL default '0',\
	poisoned int(10)  NOT NULL default '0',\
	murderertime int(11)  NOT NULL default '0',\
	criminaltime int(11)  NOT NULL default '0',\
	gender tinyint(1)  NOT NULL default '0',\
	propertyflags int(11)  NOT NULL default '0',\
	murderer int(11) NOT NULL default '-1',\
	guarding int(11) NOT NULL default '-1',\
	PRIMARY KEY (serial)\
	);" },

	{ "corpses", "CREATE TABLE corpses (\
	serial int(11) NOT NULL default '0',\
	bodyid smallint(6)  NOT NULL default '0',\
	hairstyle smallint(6)  NOT NULL default '0',\
	haircolor smallint(6)  NOT NULL default '0',\
	beardstyle smallint(6)  NOT NULL default '0',\
	beardcolor smallint(6)  NOT NULL default '0',\
	PRIMARY KEY (serial)\
	);" },

	{ "corpses_equipment", "CREATE TABLE corpses_equipment (\
	serial int(11) NOT NULL default '0',\
	layer tinyint(3)  NOT NULL default '0',\
	item int(11) NOT NULL default '-1',  \
	PRIMARY KEY (serial,layer)\
	);" },

	{ "houses", "CREATE TABLE houses (\
	serial int(11) NOT NULL default '0',\
	nokey tinyint(1) NOT NULL default '0',\
	charpos_x smallint(6) NOT NULL default '0',\
	charpos_y smallint(6) NOT NULL default '0',\
	charpos_z smallint(6) NOT NULL default '0',\
	PRIMARY KEY (serial)\
	);" },

	{ "items", "CREATE TABLE items (\
	serial int(11) NOT NULL default '0',\
	id smallint(5)  NOT NULL default '0',\
	color smallint(5)  NOT NULL default '0',\
	cont int(11) NOT NULL default '-1',\
	layer tinyint(3)  NOT NULL default '0',\
	type smallint(5)  NOT NULL default '0',\
	amount smallint(5)  NOT NULL default '0',\
	decaytime int(10)  NOT NULL default '0',\
	weight float NOT NULL default '0',\
	hp smallint(6) NOT NULL default '0',\
	maxhp smallint(6) NOT NULL default '0',\
	magic tinyint(3)  NOT NULL default '0',\
	owner int(11) NOT NULL default '-1',\
	visible tinyint(3)  NOT NULL default '0',\
	spawnregion varchar(255) default NULL,\
	priv tinyint(3)  NOT NULL default '0',\
	sellprice int(11) NOT NULL default '0',\
	buyprice int(11) NOT NULL default '0',\
	restock smallint(5)  NOT NULL default '0',\
	baseid varchar(32) NOT NULL default '',\
	PRIMARY KEY (serial)\
	);" },

	{ "multis", "CREATE TABLE multis (\
	serial int(11) NOT NULL default '0',\
	coowner int(11) NOT NULL default '-1',\
	deedsection varchar(255) NOT NULL default '',\
	PRIMARY KEY (serial)\
	);" },

	{ "multis_bans", "CREATE TABLE multis_bans (\
	serial int(11) NOT NULL default '0',\
	ban int(11) NOT NULL default '-1',\
	PRIMARY KEY (serial,ban)\
	);" },

	{ "multis_friends", "CREATE TABLE multis_friends (\
	serial int(11) NOT NULL default '0',\
	friend int(11) NOT NULL default '-1',\
	PRIMARY KEY (serial,friend)\
	);" },

	{ "npcs", "CREATE TABLE npcs (\
	serial int(11) NOT NULL default '0',\
	mindamage smallint(6)  NOT NULL default '0',\
	maxdamage smallint(6)  NOT NULL default '0',\
	tamingminskill smallint(6)  NOT NULL default '0',\
	summontime int(11)  NOT NULL default '0',\
	additionalflags int(11)  NOT NULL default '0',\
	owner int(11) NOT NULL default '-1',\
	carve varchar(255) default NULL,\
	spawnregion varchar(255) default NULL,\
	stablemaster int(11) NOT NULL default '-1',\
	lootlist varchar(255) default NULL,\
	ai varchar(255) default NULL,\
	wandertype smallint(3) NOT NULL default '0',\
	wanderx1 smallint(6) NOT NULL default '0',\
	wanderx2 smallint(6) NOT NULL default '0',\
	wandery1 smallint(6) NOT NULL default '0',\
	wandery2 smallint(6) NOT NULL default '0',\
	wanderradius smallint(6) NOT NULL default '0',\
	fleeat smallint(3)  NOT NULL default '10',\
	spellslow int(11)  NOT NULL default '0',\
	spellshigh int(11)  NOT NULL default '0',\
	PRIMARY KEY (serial)\
	);" },

	{ "players", "CREATE TABLE players (\
	serial int(11) NOT NULL default '0',\
	account varchar(255) default NULL,\
	additionalflags int(10)  NOT NULL default '0',\
	visualrange tinyint(3)  NOT NULL default '0',\
	profile longtext,\
	fixedlight tinyint(3)  NOT NULL default '0',\
	PRIMARY KEY (serial)\
	);" },

	{ "skills", "CREATE TABLE skills (\
	serial int(11) NOT NULL default '0',\
	skill tinyint(3)  NOT NULL default '0',\
	value smallint(6) NOT NULL default '0',\
	locktype tinyint(4) default '0',\
	cap smallint(6) default '0',\
	PRIMARY KEY (serial,skill)\
	);" },

	{ "tags", "CREATE TABLE tags (\
	serial int(11) NOT NULL default '0',\
	name varchar(64) NOT NULL default '',\
	type varchar(6) NOT NULL default '',\
	value longtext NOT NULL,\
	PRIMARY KEY (serial,name)\
	);" },

	{ "uobject", "CREATE TABLE uobject (\
	name varchar(255) default NULL,\
	serial int(11) NOT NULL default '0',\
	multis int(11) NOT NULL default '-1',\
	direction char(1) NOT NULL default '0',\
	pos_x smallint(6)  NOT NULL default '0',\
	pos_y smallint(6)  NOT NULL default '0',\
	pos_z smallint(6) NOT NULL default '0',\
	pos_map tinyint(4) NOT NULL default '0',  \
	events varchar(255) default NULL,\
	bindmenu varchar(255) default NULL,\
	havetags tinyint(1) NOT NULL default '0',\
	PRIMARY KEY (serial)\
	);" },

	{ "uobjectmap", "CREATE TABLE uobjectmap (\
	serial int(11) NOT NULL default '0',\
	type varchar(80)  NOT NULL default '',\
	PRIMARY KEY (serial)\
	);" },

	{ "effects", "CREATE TABLE effects (\
	id int NOT NULL,\
	objectid varchar(64) NOT NULL,\
	expiretime int NOT NULL,\
	dispellable tinyint NOT NULL default '0',\
	source int NOT NULL default '-1',\
	destination int NOT NULL default '-1',\
	PRIMARY KEY (id)\
	);" },

	{ "effects_properties", "CREATE TABLE effects_properties (\
	id int NOT NULL,\
	keyname varchar(64) NOT NULL,\
	type varchar(64) NOT NULL,\
	value text NOT NULL,\
	PRIMARY KEY (id,keyname)\
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
	Console::instance()->send( "Loading World...\n" );

	if( !persistentBroker->openDriver( SrvParams->databaseDriver() ) )
	{
		Console::instance()->log( LOG_ERROR, QString( "Unknown Worldsave Database Driver '%1', check your wolfpack.xml").arg( SrvParams->databaseDriver() ) );
		return;
	}

	if (!persistentBroker->connect( SrvParams->databaseHost(), SrvParams->databaseName(), SrvParams->databaseUsername(), SrvParams->databasePassword())) {
		throw QString("Unable to open the world database.");
	}

	QString objectID;
	register unsigned int i = 0;

	while( tableInfo[i].name )
	{
		if( !persistentBroker->tableExists( tableInfo[i].name ) )
		{
			persistentBroker->executeQuery( tableInfo[i].create );

			// create default settings
			if( !strcmp( tableInfo[i].name, "settings" ) )
			{
				setOption("db_version", WP_DATABASE_VERSION, false);
			}
		}

		++i;
	}

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

		Console::instance()->send( "\n"+tr("Loading ") + QString::number( count ) + tr(" objects of type ") + type );

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

		while( progress.count() < progress.expected_count() )
			++progress;

		res.free();
		persistentBroker->driver()->setActiveConnection();

		//Console::instance()->send( "Loaded %i objects in %i msecs\n", progress.count(), getNormalizedTime() - sTime );
	}

	// Load Pages
	// cPagesManager::getInstance()->load();

	// Load Temporary Effects
	TempEffects::instance()->load();

	Console::instance()->PrepareProgress( "Postprocessing" );

	// It's not possible to use cItemIterator during postprocessing because it skips lingering items
	ItemMap::iterator iter;
	QPtrList< cItem > deleteItems;

	for( iter = p->items.begin(); iter != p->items.end(); ++iter )
	{
		P_ITEM pi = iter->second;
		SERIAL contserial = reinterpret_cast<SERIAL>(pi->container());

		// We used free for uncontained items
		if (!pi->free) {
			int max_x = Map->mapTileWidth(pi->pos().map) * 8;
			int max_y = Map->mapTileHeight(pi->pos().map) * 8;
			if (pi->pos().x > max_x || pi->pos().y > max_y) {
				deleteItems.append( pi );
				continue;
			} else {
				MapObjects::instance()->add(pi);
			}
		} else {
			// Flag the container value as valid
			pi->free = false;

			// 1. Handle the Container Value
			if (isItemSerial(contserial)) {
				P_ITEM pCont = FindItemBySerial(contserial);

				if (pCont) {
					pCont->addItem(pi, false, true, true);
				} else {
					deleteItems.append(pi); // Queue this item up for deletion
					continue; // Skip further processing
				}
			}
			else if (isCharSerial(contserial)) {
				P_CHAR pCont = FindCharBySerial( contserial );

				if (pCont) {
					pCont->addItem((cBaseChar::enLayer) pi->layer(), pi, true, true);
				} else {
					deleteItems.append(pi);
					continue;
				}
			}
		}

		// If this item has a multiserial then add it to the multi
		if (isItemSerial(pi->multis())) {
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
				Console::instance()->send( tr( "The owner of Serial 0x%1 is invalid: %2" ).arg( pNPC->serial(), 16 ).arg( owner, 16 ) );
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
				Console::instance()->send( tr( "The guard target of Serial 0x%1 is invalid: %2" ).arg( pChar->serial(), 16 ).arg( guarding, 16 ) );
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

		pChar->flagUnchanged(); // We've just loaded, nothing changes
	}

	// Post process spawnregions
	SpawnRegions::instance()->postWorldLoading();

	Console::instance()->ProgressDone();

	Console::instance()->PrepareProgress( "Deleting lost items" );

	// Do we have to delete items?
	for( P_ITEM pItem = deleteItems.first(); pItem; pItem = deleteItems.next() )
		quickdelete( pItem );

	Console::instance()->ProgressDone();

	if( deleteItems.count() > 0 )
	{
		Console::instance()->send( QString::number( deleteItems.count() ) + " deleted due to invalid container or position.\n" );
		deleteItems.clear();
	}

	// Load Guilds
	Guilds::instance()->load();

	// load server time from db
	Console::instance()->PrepareProgress( "Setting Worldtime" );
	QString db_time;
	QString default_time = SrvParams->getString( "General", "UO Time", "0", true );
	getOption( "worldtime", db_time, default_time );
	uoTime.setTime_t( db_time.toInt() );
	Console::instance()->ProgressDone();

	Console::instance()->send(QString("Worldtime is %1 on %3. %4 in year %5").arg(uoTime.time().toString()).arg(uoTime.date().day()).arg(QDate::monthName(uoTime.date().month())).arg(uoTime.date().year() - 1970) + ".\n" );

	persistentBroker->disconnect();

	Console::instance()->send("World Loading ");
	Console::instance()->ChangeColor( WPC_GREEN );
	Console::instance()->send( "Completed\n" );
	Console::instance()->ChangeColor( WPC_NORMAL );
}

void cWorld::save()
{
	Console::instance()->send( "Saving World..." );
	Timing::instance()->setLastWorldsave(getNormalizedTime());

	// Send a nice status gump to all sockets if enabled
	bool fancy = SrvParams->getBool("General", "Fancy Worldsave Status", true, true);
	if (fancy) {
		// Create a fancy gump as promised
        cGump gump;
		gump.setNoClose(true);
		gump.setNoDispose(true);
		gump.setNoMove(true);
		gump.setX(-10);
		gump.setY(-10);
		gump.setType(0x98FA2C10);
		
		gump.addResizeGump(0, 0, 9200, 291, 90);
		gump.addCheckertrans(0, 0, 291, 90);
		gump.addText(47, 19, tr("WORLDSAVE IN PROGRESS"), 2122);
		gump.addText(47, 37, tr("Saving %1 items.").arg(itemCount()), 2100);
		gump.addText(47, 55, tr("Saving %1 characters.").arg(charCount()), 2100);
		gump.addTilePic(3, 25, 4167);

		// Send it to all connected ingame sockets
		for (cUOSocket *socket = cNetwork::instance()->first(); socket; socket = cNetwork::instance()->next()) {
			socket->send(new cGump(gump));
		}
	}

	if( !persistentBroker->openDriver( SrvParams->databaseDriver() ) )
	{
		Console::instance()->log( LOG_ERROR, QString( "Unknown Worldsave Database Driver '%1', check your wolfpack.xml").arg( SrvParams->databaseDriver() ) );
		return;
	}

	try
	{
		persistentBroker->connect( SrvParams->databaseHost(), SrvParams->databaseName(), SrvParams->databaseUsername(), SrvParams->databasePassword() );
	}
	catch( QString &e )
	{
		Console::instance()->log( LOG_ERROR, QString( "Couldn't open the database: %1\n" ).arg( e ) );
		return;
	}

	unsigned int i = 0;

	while( tableInfo[i].name )
	{
		if( !persistentBroker->tableExists( tableInfo[i].name ) )
		{
			persistentBroker->executeQuery( tableInfo[i].create );
		}

		++i;
	}

	unsigned int startTime = getNormalizedTime();

	// Try to Benchmark
	SrvParams->flush();

	// Flush old items
	persistentBroker->flushDeleteQueue();

	p->purgePendingObjects();

	persistentBroker->startTransaction();

	try
	{
		cItemIterator iItems;
		for( P_ITEM pItem = iItems.first(); pItem; pItem = iItems.next() )
			persistentBroker->saveObject( pItem );

		cCharIterator iChars;
		for( P_CHAR pChar = iChars.first(); pChar; pChar = iChars.next() )
			persistentBroker->saveObject( pChar );

		TempEffects::instance()->save();

		Guilds::instance()->save();

		persistentBroker->commitTransaction();

		// Save the Current Time
		setOption( "worldtime", QString::number( uoTime.toTime_t() ), false );

		// Save the accounts
		Accounts::instance()->save();

		uiCurrentTime = getNormalizedTime();

		Console::instance()->ChangeColor( WPC_GREEN );
		Console::instance()->send( " Done" );
		Console::instance()->ChangeColor( WPC_NORMAL );

		Console::instance()->send( QString( " [%1ms]\n" ).arg( uiCurrentTime - startTime ) );
	} catch(QString &e) {
		persistentBroker->rollbackTransaction();

		Console::instance()->ChangeColor( WPC_RED );
		Console::instance()->send( " Failed\n" );
		Console::instance()->ChangeColor( WPC_NORMAL );

		Console::instance()->log( LOG_ERROR, "Saving failed: " + e );
	}

	if (fancy) {
		cUOTxCloseGump close;
		close.setType(0x98FA2C10);

		for (cUOSocket *socket = cNetwork::instance()->first(); socket; socket = cNetwork::instance()->next()) {
			socket->send(&close);
		}
	}

	persistentBroker->disconnect();
}

/*
 * Gets a value from the settings table and returns the value
 */
void cWorld::getOption( const QString name, QString &value, const QString fallback )
{
	if( !persistentBroker->openDriver( SrvParams->databaseDriver() ) )
	{
		Console::instance()->log( LOG_ERROR, QString( "Unknown Worldsave Database Driver '%1', check your wolfpack.xml").arg( SrvParams->databaseDriver() ) );
		return;
	}

	try
	{
		persistentBroker->connect( SrvParams->databaseHost(), SrvParams->databaseName(), SrvParams->databaseUsername(), SrvParams->databasePassword() );
	}
	catch( QString &e )
	{
		Console::instance()->log( LOG_ERROR, QString( "Couldn't open the database: %1\n" ).arg( e ) );
		return;
	}

	cDBResult res = persistentBroker->query( QString( "SELECT option,value FROM settings WHERE option = '%1'" ).arg( persistentBroker->quoteString( name ) ) );

	if( !res.isValid() || !res.fetchrow() )
	{
		res.free();
		value = fallback;
		return;
	}

	value = res.getString( 0 );

	res.free();

	persistentBroker->disconnect();
}

/*
 * Sets a value in the settings table.
 */
void cWorld::setOption( const QString name, const QString value, bool newconnection )
{
	if (newconnection) 
	{
		if( !persistentBroker->openDriver( SrvParams->databaseDriver() ) )
		{
			Console::instance()->log( LOG_ERROR, QString( "Unknown Worldsave Database Driver '%1', check your wolfpack.xml").arg( SrvParams->databaseDriver() ) );
			return;
		}
	
		try
		{
			if (!persistentBroker->driver()) {
				persistentBroker->connect( SrvParams->databaseHost(), SrvParams->databaseName(), SrvParams->databaseUsername(), SrvParams->databasePassword() );
			}
		}
		catch( QString &e )
		{
			Console::instance()->log( LOG_ERROR, QString( "Couldn't open the database: %1\n" ).arg( e ) );
			return;
		}
	}

	// check if the option already exists
	persistentBroker->executeQuery( QString( "DELETE FROM settings WHERE option = '%1'" ).arg( persistentBroker->quoteString( name ) ) );

	QString sql;
	sql = "INSERT INTO settings VALUES('%1','%2')";
	sql = sql.arg( persistentBroker->quoteString( name ), persistentBroker->quoteString( value ) );

	persistentBroker->executeQuery( sql );

	if (newconnection) {
		persistentBroker->disconnect();
	}
}

void cWorld::registerObject( cUObject *object )
{
	if( !object )
	{
		Console::instance()->log( LOG_ERROR, "Couldn't register a NULL object in the world." );
		return;
	}

	registerObject( object->serial(), object );
}

void cWorld::registerObject( SERIAL serial, cUObject *object )
{
	if( !object )
	{
		Console::instance()->log( LOG_ERROR, "Trying to register a null object in the World." );
		return;
	}

	// Check if the Serial really is correct
	if( isItemSerial( serial ) )
	{
		ItemMap::iterator it = p->items.find( serial - ITEM_SPACE );

		if( it != p->items.end() )
		{
			Console::instance()->log( LOG_ERROR, QString( "Trying to register an item with the Serial 0x%1 which is already in use." ).arg( serial, 16 ) );
			return;
		}

		// Insert the Item into our Registry
		P_ITEM pItem = dynamic_cast< P_ITEM >( object );

		if( !pItem )
		{
			Console::instance()->log( LOG_ERROR, QString( "Trying to register an object with an item serial (0x%1) which is no item." ).arg( serial, 16 ) );
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
			Console::instance()->log( LOG_ERROR, QString( "Trying to register a character with the Serial 0x%1 which is already in use." ).arg( QString::number( serial, 16 ) ) );
			return;
		}

		// Insert the Character into our Registry
		P_CHAR pChar = dynamic_cast< P_CHAR >( object );

		if( !pChar )
		{
			Console::instance()->log( LOG_ERROR, QString( "Trying to register an object with a character serial (0x%1) which is no character." ).arg( QString::number( serial, 16 ) ) );
			return;
		}

		p->chars.insert( std::make_pair( serial, pChar ) );
		_charCount++;

		if( serial > _lastCharSerial )
			_lastCharSerial = serial;
	}
	else
	{
		Console::instance()->log( LOG_ERROR, QString( "Tried to register an object with an invalid Serial (0x%1) in the World." ).arg( QString::number( serial, 16 ) ) );
		return;
	}
}

void cWorld::unregisterObject( cUObject *object )
{
	if( !object )
	{
		Console::instance()->log( LOG_ERROR, "Trying to unregister a null object from the world." );
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
			Console::instance()->log( LOG_ERROR, QString( "Trying to unregister a non-existing item with the serial 0x%1." ).arg( serial, 0, 16 ) );
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
			Console::instance()->log( LOG_ERROR, QString( "Trying to unregister a non-existing character with the serial 0x%1." ).arg( serial, 0, 16 ) );
			return;
		}

		p->chars.erase( it );
		_charCount--;

		if( _lastCharSerial == serial )
			_lastCharSerial--;
	}
	else
	{
		Console::instance()->log( LOG_ERROR, QString( "Trying to unregister an object with an invalid serial (0x%08x)." ).arg( serial ) );
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
		Console::instance()->log( LOG_ERROR, "Tried to delete a null object from the worldsave." );
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
