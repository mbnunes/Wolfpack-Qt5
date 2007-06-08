/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2007 by holders identified in AUTHORS.txt
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
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#include "basics.h"
#include "coord.h"
#include "console.h"
#include "definitions.h"
#include "exceptions.h"
#include "exportdefinitions.h"
#include "network/uosocket.h"
#include "network/network.h"
#include "muls/tilecache.h"

#include <QSet>
#include <QFile>
#include <QSqlQuery>
#include <QVariant>

cDefinitionExporter::cDefinitionExporter() {
	socket = 0;
}

cDefinitionExporter::~cDefinitionExporter() {
}

void cDefinitionExporter::setSocket(cUOSocket *socket) {
	this->socket = socket;
}

/*
	Report a change of status.
*/
void cDefinitionExporter::reportStatus(const QString &message) {
	if (socket) {
		socket->sysMessage(message);
		socket->log(LOG_MESSAGE, message + "\n");
	} else {
		Console::instance()->log(LOG_MESSAGE, message + "\n");
	}
}

/*
	Report an error.
*/
void cDefinitionExporter::reportError(const QString &message) {
	if (socket) {
		socket->sysMessage(message);
		socket->log(LOG_ERROR, message + "\n");
	} else {
		Console::instance()->log(LOG_ERROR, message + "\n");
	}
}

/*
	Generate the category database.
*/
void cDefinitionExporter::generate(const QString &filename) {
	if ( QFile::exists( filename ) && !QFile::remove( filename ) ) {
		reportError(tr("Unable to remove existing database %1.").arg(filename));
		return;
	}
	
	driver = QSqlDatabase::addDatabase( "QSQLITE", "definitionExporter" );
	driver.setDatabaseName(filename);

	if (!driver.open()) {
		reportError(tr("Unable to open %1 for writing.").arg(filename));
		return;
	}

	// Notify the players because it WILL take some time
	Network::instance()->broadcast(tr("Exporting gm tool database. Please wait."));

	// Report that we start exporting definitions
	reportStatus(tr("Exporting definitions to %1.").arg(filename));

	try {
		driver.exec("BEGIN;"); // Begin the SQL transaction

		createTables(); // Initialize the database
		exportItems(); // Export the item definitions
		exportLocations(); // Export the location definitions
		exportNpcs(); // Export the npc definitions
		exportMultis(); // Export the multi definitions

		driver.exec("COMMIT;"); // End the SQL transaction

		reportStatus(tr("Finished exporting definitions.")); // Report that the export has finished
	} catch ( const wpException& e ) {
		reportError(e.error()); // Report errors to the client or console
	}

	driver.close();

	Network::instance()->broadcast(tr("Export finished."));
}

// This class is used to represent an item entry in the database
class cItemInfo {
public:
	QString name;
	QString categoryname;
	unsigned short color;
	unsigned short dispid;
	unsigned int category;
	cItemInfo() : color(0), dispid(0), category(0) {}
};

/*
	Process an item definition.
*/
static void processItem(cItemInfo &item, const cElement *node, bool multi = false) {
	// If there is an inherit tag, inherit a parent item definition.
	QString inherit = node->getAttribute("inherit");
	if (inherit != QString::null) {
		const cElement *parent = Definitions::instance()->getDefinition(multi ? WPDT_MULTI : WPDT_ITEM, inherit);
		if (parent) {
			processItem(item, parent);
		}
	}

	int count = node->childCount();
	for (int i = 0; i < count; ++i) {
		const cElement *child = node->getChild(i);

		// Inherit properties from another item definition
		if (child->name() == "inherit") {
			const cElement *parent = 0;
			if (child->hasAttribute("id")) {
				parent = Definitions::instance()->getDefinition( multi ? WPDT_MULTI : WPDT_ITEM, child->getAttribute( "id" ) );
			} else {
				parent = Definitions::instance()->getDefinition( multi ? WPDT_MULTI : WPDT_ITEM, child->text() );
			}

			if (parent) {
				processItem(item, parent);
			}
		} else if ( multi && child->name() == "name" ) {
			item.name = child->text();
		} else if ( child->name() == "id" ) {
			item.dispid = child->value().toInt();
		} else if ( !multi && child->name() == "color" ) {
			item.color = child->value().toInt();
		} else if ( !multi && child->name() == "category" ) {
			// Split into category and description
			QString category = child->text();

			int rearIndex = category.lastIndexOf('\\');
			if (rearIndex != -1) {
				item.categoryname = category.left(rearIndex);
				item.name = category.mid(rearIndex + 1);
			}
		}
	}
}

// This structure represents a category in the category tree
struct stCategory {
	unsigned int id;
	unsigned int parent;
};

/*
	This is a general purpose helper function for
	getting ids for categories from a dictionary and
	at the same time ensuring that the entire category
	is in the dictionary.
*/
static int getCategoryId(QHash<QString, stCategory*> &categories, QString &name, QSqlDatabase &driver, const char *table) {
	QHash<QString, stCategory*>::iterator it = categories.find(name);
	stCategory *category = 0;
	if ( it != categories.end() )
		category = it.value();

	// name is an unknown category
	if (!category) {
		category = new stCategory; // Create a new category
		category->id = categories.count() + 1; // Give this category a unique id
		categories.insert(name, category); // Insert it into the category map

		// Now link the category with its parent
		int parentOffset = name.lastIndexOf('\\');

		// This is not a toplevel category
		if (parentOffset != -1) {
			QString parentName = name.left(parentOffset);
			category->parent = getCategoryId(categories, parentName, driver, table);
		} else {
			category->parent = 0; // This actually is a toplevel category
		}

		// Insert the category into the database
		QString sql = QString("INSERT INTO %4 VALUES(%1,'%2',%3,0);")
			.arg( category->id )
			.arg( name.mid(parentOffset + 1).replace( "'", "''" ) )
			.arg( category->parent )
			.arg( table );
		driver.exec( sql.toUtf8() );
	}

	return category->id;
}

/*
	Export the item definitions.
*/
void cDefinitionExporter::exportItems() {
	cDefinitions::Iterator it = Definitions::instance()->begin(WPDT_ITEM);
	cDefinitions::Iterator end = Definitions::instance()->end(WPDT_ITEM);

	// This QMap maps category names to their respective ids
	QHash<QString, stCategory*> categories;

	// Iterate over the item definitions
	for (; it != end; ++it) {
		const cElement *element = *it;

		cItemInfo item;
		processItem(item, element); // Process the item

		// No category information for this item
		if (item.categoryname.isEmpty() || item.name.isEmpty()) {
			continue; // Skip this item
		}

		// Retrieve the category id for the final item category
		item.category = getCategoryId(categories, item.categoryname, driver, "categories");

        // Insert the item into the table.
		QString id = (*it)->getAttribute("id");
		QString sql = QString( "INSERT INTO items VALUES(NULL,'%1',%2,%3,%4,'%5');" )
		.arg( item.name.replace( "'", "''" ) )
		.arg( item.category )
		.arg( item.dispid )
		.arg( item.color )
		.arg( id.replace( "'", "''" ) );
		driver.exec(sql.toUtf8());
	}
	foreach( stCategory* s, categories )
	{
		delete s;
	}
}

/*
	Export known locations to the database.
*/
void cDefinitionExporter::exportLocations() {
	// iterate over all locations
	cDefinitions::Iterator it = Definitions::instance()->begin(WPDT_LOCATION);
	cDefinitions::Iterator end = Definitions::instance()->end(WPDT_LOCATION);

	// This QMap maps category names to their respective ids
	QHash<QString, stCategory*> categories;

	// Iterate over the location definitions
	for (; it != end; ++it) {
		const cElement *element = *it;

		QString category = element->getAttribute("category");

		if (!category.isEmpty()) {
			// Split into name and category
			int offset = category.lastIndexOf('\\');
			if (offset != -1) {
				QString name = category.mid(offset + 1);
				category = category.left(offset);

				// Resolve the category into an id
				unsigned int catid = getCategoryId(categories, category, driver, "locationcategories");

				// Parse the coordinate of the location
				Coord coord;
				parseCoordinates(element->text(), coord);
				QString id = element->getAttribute("id");

				// Build the SQL for this location
				QString sql = QString( "INSERT INTO locations VALUES(NULL,'%1',%2,%3,%4,%5,%6,'%7');" )
				.arg( name.replace( "'", "''" ) )
				.arg( catid )
				.arg( coord.x )
				.arg( coord.y )
				.arg( coord.z )
				.arg( coord.map )
				.arg( id.replace( "'", "''" ) );

				driver.exec( sql.toUtf8() );
			}
		}
	}
}

// This structure is used to represent equipment for NPCs
struct stEquipmentInfo {
	unsigned short color;
	unsigned short anim;
};

// This class is used to represent an npc entry in the database
class cNpcInfo {
public:
	QString name;
	QString categoryname;
	unsigned short skin;
	unsigned short bodyid;
	unsigned int category;
	stEquipmentInfo equipment[0x19];

	cNpcInfo() : skin(0), bodyid(0), category(0) {
		memset(equipment, 0, sizeof(equipment)); // Clear the entire equipment info
	}
};

/*
	Process an npc definition.
*/
static void processNpc(cNpcInfo &npc, const cElement *node) {
	// If there is an inherit tag, inherit a parent npc definition.
	QString inherit = node->getAttribute("inherit");
	if (inherit != QString::null) {
		const cElement *parent = Definitions::instance()->getDefinition(WPDT_NPC, inherit);
		if (parent) {
			processNpc(npc, parent);
		}
	}

	int count = node->childCount();
	for (int i = 0; i < count; ++i) {
		const cElement *child = node->getChild(i);

		// Inherit properties from another npc definition
		if (child->name() == "inherit") {
			const cElement *parent = 0;
			if (child->hasAttribute("id")) {
				parent = Definitions::instance()->getDefinition( WPDT_NPC, child->getAttribute( "id" ) );
			} else {
				parent = Definitions::instance()->getDefinition( WPDT_NPC, child->text() );
			}

			if (parent) {
				processNpc(npc, parent);
			}
		} else if ( child->name() == "id" ) {
			npc.bodyid = child->value().toInt();
		} else if ( child->name() == "skin" ) {
			npc.skin = child->value().toInt();
		} else if ( child->name() == "category" ) {
			// Split into category and description
			QString category = child->text();

			int rearIndex = category.lastIndexOf('\\');
			if (rearIndex != -1) {
				npc.categoryname = category.left(rearIndex);
				npc.name = category.mid(rearIndex + 1);
			}
		} else if ( child->name() == "equipped" ) {
			// Process equipment information for the npc
			for (unsigned int j = 0; j < child->childCount(); ++j) {
				const cElement *subchild = child->getChild(j);

				// Get the equipped items baseid and try to reproduce it
				QString id = subchild->getAttribute("id");
				if (id.isEmpty()) {
					// If there is no id, check if there is a random list entry
					if (subchild->hasAttribute("list")) {
						id = Definitions::instance()->getRandomListEntry( subchild->getAttribute("list") );
					} else {
						continue; // Skip this item
					}
				}

				// Retrieve and parse the item definition
				const cElement *itemNode = Definitions::instance()->getDefinition(WPDT_ITEM, id);

				if (itemNode) {
					cItemInfo item;
					processItem(item, itemNode);

					// Get the layer and animation for the item and save it
					tile_st tile = TileCache::instance()->getTile(item.dispid);
					if (tile.layer != 0 && tile.layer < 0x19 && tile.animation != 0) {
						npc.equipment[tile.layer].color = item.color;
						npc.equipment[tile.layer].anim = tile.animation;
					}
				}
			}
		}
	}
}

/*
	Export the npc definitions.
*/
void cDefinitionExporter::exportNpcs() {
	cDefinitions::Iterator it = Definitions::instance()->begin(WPDT_NPC);
	cDefinitions::Iterator end = Definitions::instance()->end(WPDT_NPC);

	// This QMap maps category names to their respective ids
	QHash<QString, stCategory*> categories;

	// Iterate over the npc definitions
	for (; it != end; ++it) {
		const cElement *element = *it;

		cNpcInfo npc;
		processNpc(npc, element); // Process the npc

		// No category information for this npc
		if (npc.categoryname.isEmpty() || npc.name.isEmpty()) {
			continue; // Skip this npc
		}

		// Retrieve the category id for the final npc category
		npc.category = getCategoryId(categories, npc.categoryname, driver, "npccategories");

        // Insert the npc into the table.
		QString id = (*it)->getAttribute("id");
		QString sql = QString( "INSERT INTO npcs VALUES(NULL,'%1',%2,%3,%4,'%5');" )
		.arg( npc.name.replace( "'", "''" ) )
		.arg( npc.category )
		.arg( npc.bodyid )
		.arg( npc.skin )
		.arg( id.replace( "'", "''" ) );
		QSqlQuery q = driver.exec( sql.toUtf8() );

		int lastInsertId = q.lastInsertId().toInt();

		// Save equipment data (by layer)
		for (int i = 0; i < 0x19; ++i) {
			// Skip unoccupied layers
			if (npc.equipment[i].anim != 0) {
				QString sql = QString( "INSERT INTO npcequipment VALUES(%1,%2,%3,%4);" )
				.arg( lastInsertId )
				.arg( npc.equipment[i].anim )
				.arg( i )
				.arg( npc.equipment[i].color );
				driver.exec( sql.toUtf8() );
			}
		}
	}
	foreach( stCategory* s, categories )
	{
		delete s;
	}
}

/*
	Export the multi definitions.
*/
void cDefinitionExporter::exportMultis() {
	cDefinitions::Iterator it = Definitions::instance()->begin(WPDT_MULTI);
	cDefinitions::Iterator end = Definitions::instance()->end(WPDT_MULTI);

	// Iterate over the multi definitions
	for (; it != end; ++it) {
		const cElement *element = *it;

		cItemInfo item;
		processItem(item, element, true); // Use the item parser here to save code

		if (item.dispid >= 0x4000) {
			// Insert the multi into the table.
			QString id = (*it)->getAttribute("id");
			QString sql = QString( "INSERT INTO multis VALUES(NULL,'%1','%2',%3);" )
			.arg( item.name.replace( "'", "''" ) )
			.arg( id.replace( "'", "''" ) )
			.arg( item.dispid );
			driver.exec( sql.toUtf8() );
		}
	}
}

/*
	Create the tables used by the category database.
*/
void cDefinitionExporter::createTables() {
	driver.exec( "CREATE TABLE items (\
		id INTEGER PRIMARY KEY,\
		name varchar(255) NULL,\
		parent int NOT NULL,\
		artid int,\
		color int,\
		addid varchar(255)\
	);" );

	driver.exec( "CREATE TABLE categories (\
		id INTEGER PRIMARY KEY,\
		name varchar(255) NULL,\
		parent int NOT NULL,\
		type int\
	);" );

	driver.exec( "CREATE TABLE locationcategories (\
		id INTEGER PRIMARY KEY,\
		name varchar(255) NULL,\
		parent int NOT NULL,\
		type int\
	);" );

	driver.exec( "CREATE TABLE locations (\
		id INTEGER PRIMARY KEY,\
		name varchar(255) NULL,\
		parent INT NOT NULL,\
		posx INT NOT NULL,\
		posy INT NOT NULL,\
		posz INT NOT NULL,\
		posmap INT NOT NULL,\
		location varchar(255)\
	);" );

	driver.exec( "CREATE TABLE npcs (\
		id INTEGER PRIMARY KEY,\
		name varchar(255) NULL,\
		parent int NOT NULL,\
		bodyid int,\
		skin int,\
		addid varchar(255)\
	);" );

	driver.exec( "CREATE TABLE npccategories (\
		id INTEGER PRIMARY KEY,\
		name varchar(255) NULL,\
		parent int NOT NULL,\
		type int\
	);" );

	driver.exec( "CREATE TABLE npcequipment (\
		id int NOT NULL,\
		artid int NOT NULL,\
		layer int NOT NULL,\
		color int NOT NULL\
	);" );

	driver.exec( "CREATE TABLE multis (\
		id INTEGER PRIMARY KEY,\
		name varchar(255) NULL,\
		addid varchar(255) NULL,\
		multiid int NOT NULL\
	);" );
}
