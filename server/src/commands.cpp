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

#include "accounts.h"
#include "skills.h"
#include "commands.h"
#include "gumps.h"
#include "muls/maps.h"
#include "scriptmanager.h"
#include "network/uosocket.h"
#include "spawnregions.h"
#include "serverconfig.h"
#include "targetrequests.h"
#include "territories.h"
#include "muls/tilecache.h"
#include "console.h"
#include "definitions.h"
#include "scriptmanager.h"
#include "pagesystem.h"
#include "sectors.h"
#include "contextmenu.h"
#include "pythonscript.h"
#include "network/network.h"
#include "dbdriver.h"

// System Includes
#include <functional>

// Main Command processing function
void cCommands::process( cUOSocket* socket, const QString& command )
{
	if ( !socket->player() )
		return;

	P_PLAYER pChar = socket->player();
	QStringList pArgs = QStringList::split( " ", command, true );

	// No Command? No Processing
	if ( pArgs.isEmpty() )
		return;

	QString pCommand = pArgs[0].upper(); // First element should be the command

	// Remove it from the argument list
	pArgs.erase( pArgs.begin() );

	// Check if the priviledges are ok
	if ( !pChar->account()->authorized( "command", pCommand.latin1() ) )
	{
		socket->sysMessage( tr( "Access to command '%1' was denied" ).arg( pCommand.lower() ) );
		socket->log( tr( "Access to command '%1' was denied\n" ).arg( pCommand.lower() ) );
		return;
	}

	// Dispatch the command
	if ( dispatch( socket, pCommand, pArgs ) )
		socket->log( tr( "Used command '%1'.\n" ).arg( command ) );
}

// Selects the right command Stub
bool cCommands::dispatch( cUOSocket* socket, const QString& command, const QStringList& arguments )
{
	// Just in case we have been called directly
	if ( !socket || !socket->player() )
		return false;

	// Check for custom commands
	PyObject* function = ScriptManager::instance()->getCommandHook( command.latin1() );

	if ( function )
	{
		QString argString = arguments.join( " " );

		if ( argString.isNull() )
			argString = "";

		PyObject* args = Py_BuildValue( "O&uu", PyGetSocketObject, socket, command.ucs2(), argString.ucs2() );

		PyObject* result = PyObject_CallObject( function, args );
		Py_XDECREF( result );
		reportPythonError();

		Py_DECREF( args );

		return true;
	}

	for ( uint index = 0; commands[index].command; ++index )
		if ( command == commands[index].name )
		{
			( commands[index].command ) ( socket, command, arguments );
			return true;
		}

	socket->sysMessage( tr( "Unknown Command" ) );
	return false;
}

void cCommands::loadACLs( void )
{
	// make sure it's clean
	QMap<QString, cAcl*>::iterator itA( _acls.begin() );
	for ( ; itA != _acls.end(); ++itA )
		delete itA.data();
	_acls.clear();

	QStringList ScriptSections = Definitions::instance()->getSections( WPDT_PRIVLEVEL );

	if ( ScriptSections.isEmpty() )
	{
		Console::instance()->log( LOG_WARNING, "No ACLs for players, counselors, gms and admins defined!\n" );
		return;
	}

	// We are iterating trough a list of ACLs
	// In each loop we create one acl
	for ( QStringList::iterator it = ScriptSections.begin(); it != ScriptSections.end(); ++it )
	{
		const cElement* Tag = Definitions::instance()->getDefinition( WPDT_PRIVLEVEL, *it );

		if ( !Tag )
			continue;

		QString ACLname = *it;

		// While we are in this loop we are building an ACL
		cAcl* acl = new cAcl;
		acl->name = ACLname;
		acl->rank = Tag->getAttribute( "rank", "1" ).toUShort();
		if ( acl->rank == 0 || acl->rank == 255 )
		{
			acl->rank = 1;
		}

		QMap<QString, bool> group;
		QCString groupName;


		for ( unsigned int i = 0; i < Tag->childCount(); ++i )
		{
			const cElement* childTag = Tag->getChild( i );
			if ( childTag->name() == "group" )
			{
				groupName = childTag->getAttribute( "name" );

				for ( unsigned int j = 0; j < childTag->childCount(); ++j )
				{
					const cElement* groupTag = childTag->getChild( j );

					if ( groupTag->name() == "action" )
					{
						QString name = groupTag->getAttribute( "name", "any" );
						bool permit = groupTag->getAttribute( "permit", "false" ) == "true" ? true : false;
						group.insert( name, permit );
					}
				}

				if ( !group.isEmpty() )
				{
					acl->groups.insert( groupName, group );
					group.clear();
				}
			}
		}

		_acls.insert( ACLname, acl );
	}

	// Renew the ACL pointer for all loaded accounts
	Accounts::instance()->clearAcls();
}

/*
	\command set
	\description Change properties of characters and items.
	\usage - <code>set key value</code>
	Key is the name of the property you want to set.
	Value is the new property value.
	\notes See the object reference for <object id="char">characters</object>
	and <object id="item">items</object> for valid property keys. All integer, float, string,
	character and item properties can be set using this command as well. In addition to the
	properties you find there, you can also set skills by using skill.skillname as the key and
	the skill value multiplied by ten as the value (i.e. 100.0% = 1000).
*/
void commandSet( cUOSocket* socket, const QString& command, const QStringList& args ) throw()
{
	Q_UNUSED( command );
	if ( args.size() < 1 )
	{
		socket->sysMessage( tr( "Usage: set <key> <value>" ) );
		return;
	}

	QString key = args[0];
	QStringList realargs( args );
	realargs.remove( realargs.begin() );
	QString value;
	if ( realargs.size() == 0 )
	{
		value = "";
	}
	else
	{
		value = realargs.join( " " );
	}

	// Alias for speed sake on setting stats.
	if ( key == "str" )
	{
		socket->sysMessage( tr( "Please select a target to 'set %1 %2' " ).arg( "strength" ).arg( value ) );
		socket->attachTarget( new cSetTarget( "strength", value ) );
	}
	else if ( key == "dex" )
	{
		socket->sysMessage( tr( "Please select a target to 'set %1 %2' " ).arg( "dexterity" ).arg( value ) );
		socket->attachTarget( new cSetTarget( "dexterity", value ) );
	}
	else if ( key == "int" )
	{
		socket->sysMessage( tr( "Please select a target to 'set %1 %2' " ).arg( "intelligence" ).arg( value ) );
		socket->attachTarget( new cSetTarget( "intelligence", value ) );
	}
	else
	{
		socket->sysMessage( tr( "Please select a target to 'set %1 %2' " ).arg( key ).arg( value ) );
		socket->attachTarget( new cSetTarget( key, value ) );
	}
}

/*
	\command save
	\description Forces the world to be saved.
*/
void commandSave( cUOSocket* socket, const QString& command, const QStringList& args ) throw()
{
	Q_UNUSED( args );
	Q_UNUSED( socket );
	Q_UNUSED( command );

	World::instance()->save();
}

/*
	\command servertime
	\description Shows the current server uptime in miliseconds.
*/
void commandServerTime( cUOSocket* socket, const QString& command, const QStringList& args ) throw()
{
	Q_UNUSED( args );
	Q_UNUSED( command );
	socket->sysMessage( tr( "Server time: %1" ).arg( Server::instance()->time() ) );
}

/*
	\command show
	\description Show properties of characters and items.
	\usage - <code>show key</code>
	Key is the name of the property you want to see.
	\notes See the <command id="SET">SET</command> command for more information.
*/
void commandShow( cUOSocket* socket, const QString& command, const QStringList& args ) throw()
{
	Q_UNUSED( command );
	socket->sysMessage( tr( "Please select a target" ) );
	socket->attachTarget( new cShowTarget( args.join( " " ) ) );
}

/*
	\command spawnregion
	\description Control a spawnregion.
	\usage - <code>spawnregion respawn id</code>
	- <code>spawnregion clear id</code>
	- <code>spawnregion fill id</code>
	- <code>spawnregion info id</code>
	The respawn subcommand will initiate a respawn of the given spawnregion.
	The clear subcommand will delete all objects spawned by the given region.
	The fill subcommand will initiate a maximal respawn of the given spawnregion.
	The info subcommand will show a dialog with information about the given spawnregion.
	\notes The region id can be <i>all</i> in which case all spawnregions will be affected.
*/
void commandSpawnRegion( cUOSocket* socket, const QString& command, const QStringList& args ) throw()
{
	Q_UNUSED( command );
	// Spawnregion respawn region_name
	// Spawnregion clear   region_name
	// Spawnregion fill    region_name
	// Spawnregion info    region_name

	// region_name can be "all"

	if ( args.count() == 0 )
	{
		socket->sysMessage( tr( "Usage: spawnregion <respawn|clear|fill>" ) );
		return;
	}

	QString subCommand = args[0].lower();

	// respawn spawnregion
	if ( subCommand == "respawn" )
	{
		if ( args.count() < 2 )
		{
			socket->sysMessage( tr( "Usage: spawnregion respawn <region_name>" ) );
		}
		else if ( args[1].lower() != "all" )
		{
			cSpawnRegion* spawnRegion = SpawnRegions::instance()->region( args[1] );
			if ( !spawnRegion )
			{
				socket->sysMessage( tr( "Spawnregion %1 does not exist" ).arg( args[1] ) );
			}
			else
			{
				spawnRegion->reSpawn();
				socket->sysMessage( tr( "Spawnregion '%1' has respawned" ).arg( args[1] ) );
			}
		}
		else if ( args[1].lower() == "all" )
		{
			SpawnRegions::instance()->reSpawn();
			socket->sysMessage( tr( "All spawnregions have respawned" ) );
		}
	}

	// clear spawnregions (despawn)
	else if ( subCommand == "clear" )
	{
		if ( args.count() < 2 )
		{
			socket->sysMessage( tr( "Usage: spawnregion clear <region_name>" ) );
		}
		else if ( args[1].lower() != "all" )
		{
			cSpawnRegion* spawnRegion = SpawnRegions::instance()->region( args[1] );
			if ( !spawnRegion )
			{
				socket->sysMessage( tr( "Spawnregion %1 does not exist" ).arg( args[1] ) );
			}
			else
			{
				spawnRegion->deSpawn();
				socket->sysMessage( tr( "Spawnregion '%1' has been cleared" ).arg( args[1] ) );
			}
		}
		else if ( args[1].lower() == "all" )
		{
			SpawnRegions::instance()->deSpawn();
			socket->sysMessage( tr( "All spawnregions have been cleared" ) );
		}
	}

	// fill spawnregions up (respawnmax)
	else if ( subCommand == "fill" )
	{
		if ( args.count() < 2 )
		{
			socket->sysMessage( tr( "Usage: spawnregion fill <region_name>" ) );
		}
		else if ( args[1].lower() != "all" )
		{
			cSpawnRegion* spawnRegion = SpawnRegions::instance()->region( args[1] );
			if ( !spawnRegion )
			{
				socket->sysMessage( tr( "Spawnregion %1 does not exist" ).arg( args[1] ) );
			}
			else
			{
				spawnRegion->reSpawnToMax();
				socket->sysMessage( tr( "Spawnregion '%1' has respawned to maximum" ).arg( args[1] ) );
			}
		}
		else if ( args[1].lower() == "all" )
		{
			SpawnRegions::instance()->reSpawnToMax();
			socket->sysMessage( tr( "All spawnregions have respawned to maximum" ) );
		}
	}

	// show spawnregion info
	else if ( subCommand == "info" )
	{
		if ( args.count() < 2 )
		{
			socket->sysMessage( tr( "Usage: spawnregion info <region_name>" ) );
		}
		else if ( args[1].lower() != "all" )
		{
			cSpawnRegion* spawnRegion = SpawnRegions::instance()->region( args[1] );
			if ( !spawnRegion )
			{
				socket->sysMessage( tr( "Spawnregion %1 does not exist" ).arg( args[1] ) );
			}
			else
			{
				cSpawnRegionInfoGump* pGump = new cSpawnRegionInfoGump( spawnRegion );
				socket->send( pGump );
			}
		}
		else if ( args[1].lower() == "all" )
		{
			// Display a gump with this information
			cGump* pGump = new cGump();

			// Basic .INFO Header
			pGump->addResizeGump( 0, 40, 0xA28, 450, 210 ); //Background
			pGump->addGump( 105, 18, 0x58B ); // Fancy top-bar
			pGump->addGump( 182, 0, 0x589 ); // "Button" like gump
			pGump->addTilePic( 202, 23, 0x14eb ); // Type of info menu

			pGump->addText( 160, 90, tr( "Spawnregion Global Info" ), 0x530 );

			// Give information about the spawnregions
			pGump->addText( 50, 120, tr( "Spawnregions: %1" ).arg( SpawnRegions::instance()->size() ), 0x834 );
			pGump->addText( 50, 140, tr( "NPCs: %1 of %2" ).arg( SpawnRegions::instance()->npcs() ).arg( SpawnRegions::instance()->maxNpcs() ), 0x834 );
			pGump->addText( 50, 160, tr( "Items: %1 of %2" ).arg( SpawnRegions::instance()->items() ).arg( SpawnRegions::instance()->maxItems() ), 0x834 );

			// OK button
			pGump->addButton( 50, 200, 0xF9, 0xF8, 0 ); // Only Exit possible

			socket->send( pGump );
		}
	}
}

/*
	\command pages
	\description Manage support tickets.
*/
void commandPages( cUOSocket* socket, const QString& command, const QStringList& args ) throw()
{
	Q_UNUSED( args );
	Q_UNUSED( command );
	// Pages
	if ( socket->player() )
	{
		cPagesGump* pGump = NULL;

		if ( socket->player()->isGM() )
			pGump = new cPagesGump( 1, PT_GM );
		else if ( socket->player()->isCounselor() )
			pGump = new cPagesGump( 1, PT_COUNSELOR );

		if ( pGump )
			socket->send( pGump );
	}
}

/*
	\command shutdown
	\description Shutdown the Wolfpack server.
*/
void commandShutDown( cUOSocket* socket, const QString& command, const QStringList& args ) throw()
{
	Q_UNUSED( socket );
	Q_UNUSED( command );
	// Shutdown
	if ( args.count() == 0 )
		Server::instance()->cancel();
}

/*
	\command staff
	\description Toggle the staff flag for your account.
	\notes The staff flag controls whether you are treated as a priviledged user or not.
*/
void commandStaff( cUOSocket* socket, const QString& command, const QStringList& args ) throw()
{
	Q_UNUSED( command );
	if ( socket->account()->isStaff() || ( args.count() > 0 && args[0].toInt() == 0 ) )
	{
		socket->account()->setStaff( false );
		socket->sysMessage( tr( "Staff is now '0'." ) );
	}
	else if ( !socket->account()->isStaff() || ( args.count() > 0 && args[0].toInt() == 1 ) )
	{
		socket->account()->setStaff( true );
		socket->sysMessage( tr( "Staff is now '1'." ) );
	}
}

/*
	\command reload
	\description Reload certain aspects of the server.
	\usage - <code>reload accounts</code>
	- <code>reload scripts</code>
	- <code>reload python</code>
	- <code>reload all</code>
	Reload the given server component.
	\notes The <i>accounts</i> parameter will reload all accounts from the account database.
	The <i>python</i> parameter will reload all Python scripts.
	The <i>scripts</i> parameter will reload all XML definitions and Python scripts.
	The <i>all</i> parameter will reload all three.
*/
void commandReload( cUOSocket* socket, const QString& command, const QStringList& args ) throw()
{
	Q_UNUSED( command );
	// Reload accounts
	// Reload scripts
	// Reload all

	if ( args.count() == 0 )
	{
		socket->sysMessage( tr( "Usage: reload <accounts|scripts|python|all>" ) );
		return;
	}

	QString subCommand = args[0].lower();

	// accounts
	if ( subCommand == "accounts" )
	{
		Server::instance()->reload( "accounts" );
	}
	else if ( subCommand == "python" )
	{
		Server::instance()->reload( "scripts" );
	}
	else if ( subCommand == "scripts" )
	{
		Server::instance()->reload( "definitions" );
	}

	if ( subCommand == "all" )
	{
		Server::instance()->reload( "configuration" ); // This will reload nearly everything
	}
}

/*
	\command allshow
	\description Toggles the allshow flag of your account.
	\notes The allshow flag determines whether you can see logged out characters.
*/
void commandAllShow( cUOSocket* socket, const QString& command, const QStringList& args ) throw()
{
	Q_UNUSED( command );
	if ( !socket->player() || !socket->player()->account() )
		return;

	// Switch
	if ( !args.count() )
		socket->player()->account()->setAllShow( !socket->player()->account()->isAllShow() );
	// Set
	else
		socket->player()->account()->setAllShow( args[0].toInt() != 0 );

	if ( socket->player()->account()->isAllShow() )
		socket->sysMessage( tr( "AllShow = '1'" ) );
	else
		socket->sysMessage( tr( "AllShow = '0'" ) );

	socket->resendWorld( true );
}

/*
	\command allmove
	\description Toggles the allmove flag of your account.
	\notes The allmove flag determines whether you can move immovable objects.
*/
void commandAllMove( cUOSocket* socket, const QString& command, const QStringList& args ) throw()
{
	Q_UNUSED( command );
	if ( !socket->player() || !socket->player()->account() )
		return;

	// Switch
	if ( !args.count() )
		socket->player()->account()->setAllMove( !socket->player()->account()->isAllMove() );
	// Set
	else
		socket->player()->account()->setAllMove( args[0].toInt() != 0 );

	if ( socket->player()->account()->isAllMove() )
		socket->sysMessage( tr( "AllMove is [enabled]" ) );
	else
		socket->sysMessage( tr( "AllMove is [disabled]" ) );

	// Resend the world to us
	socket->resendWorld( true );
}

/*
	\command restock
	\description Restock the items sold by a vendor.
*/
void commandRestock( cUOSocket* socket, const QString& command, const QStringList& args ) throw()
{
	Q_UNUSED( args );
	Q_UNUSED( command );
	socket->sysMessage( tr( "Please select the vendor you want to restock." ) );
	socket->attachTarget( new cRestockTarget );
}

/*
	\command broadcast
	\description Broadcast a message to all connected clients.
	\usage - <code>broadcast [message]</code>
	Message is the message you want to broadcast to everyone.
*/
void commandBroadcast( cUOSocket* socket, const QString& command, const QStringList& args ) throw()
{
	Q_UNUSED( socket );
	Q_UNUSED( command );
	Network::instance()->broadcast( args.join( " " ) );
}

/*
	\command pagenotify
	\description Toggle notification about new support tickets.
	\notes If you opt to turn this flag on, you will be notified about incoming pages.
*/
void commandPageNotify( cUOSocket* socket, const QString& command, const QStringList& args ) throw()
{
	Q_UNUSED( command );
	if ( socket->account()->isPageNotify() || ( args.count() > 0 && args[0].toInt() == 0 ) )
	{
		socket->account()->setPageNotify( false );
		socket->sysMessage( tr( "PageNotify is now '0'." ) );
	}
	else if ( !socket->account()->isPageNotify() || ( args.count() > 0 && args[0].toInt() == 1 ) )
	{
		socket->account()->setPageNotify( true );
		socket->sysMessage( tr( "PageNotify is now '1'." ) );
	}
}

/*
	\command gmtalk
	\description Broadcast a message to connected gamemasters.
	\usage - <code>gmtalk [message]</code>
	Send a message to all other connected gamemasters.
*/
void commandGmtalk( cUOSocket* socket, const QString& command, const QStringList& args ) throw()
{
	Q_UNUSED( command );
	if ( args.count() < 1 )
	{
		socket->sysMessage( tr( "Usage: gmtalk <message>" ) );
		return;
	}

	QString message = "<" + socket->player()->name() + ">: " + args.join( " " );

	cUOSocket* mSock = 0;
	for ( mSock = Network::instance()->first(); mSock; mSock = Network::instance()->next() )
	{
		if ( mSock->player() && mSock->player()->isGM() )
			mSock->sysMessage( message, 0x539 );
	}
}

/*
	Recursive processing function to get neccesary information about multis.
*/
static void processMulti( QMap<QCString, QString> &item, const cElement *node )
{
	// If there is an inherit tag, inherit a parent item definition.
	QString inherit = node->getAttribute( "inherit" );
	if ( inherit != QString::null )
	{
		const cElement *parent = Definitions::instance()->getDefinition( WPDT_MULTI, inherit );
		if ( parent ) {
			processMulti( item, parent );
		}
	}

	int count = node->childCount();
	int i;
	for ( i = 0; i < count; ++i ) {
		const cElement *child = node->getChild( i );

		// Inherit properties from another item definition
		if ( child->name() == "inherit" )
		{
			const cElement *parent = 0;

			if ( child->hasAttribute("id") )
			{
				parent = Definitions::instance()->getDefinition( WPDT_MULTI, child->getAttribute( "id" ) );
			}
			else
			{
				parent = Definitions::instance()->getDefinition( WPDT_MULTI, child->text() );
			}

			if ( parent )
			{
				processMulti( item, parent );
			}
		}
		else if ( child->name() == "id" )
		{
			item[ "dispid" ] = child->value();
		}
		else if ( child->name() == "name" )
		{
			item[ "name" ] = child->text();
		}
	}
}

/*
	Recursive processing function to get neccesary information about items.
*/
static void processItem( QMap<QCString, QString> &item, const cElement *node )
{
	// If there is an inherit tag, inherit a parent item definition.
	QString inherit = node->getAttribute( "inherit" );
	if ( inherit != QString::null )
	{
		const cElement *parent = Definitions::instance()->getDefinition( WPDT_ITEM, inherit );
		if ( parent )
		{
			processItem( item, parent );
		}
	}

	int count = node->childCount();
	int i;
	for ( i = 0; i < count; ++i )
	{
		const cElement *child = node->getChild( i );

		// Inherit properties from another item definition
		if ( child->name() == "inherit" )
		{
			const cElement *parent = 0;

			if ( child->hasAttribute("id") )
			{
				parent = Definitions::instance()->getDefinition( WPDT_ITEM, child->getAttribute( "id" ) );
			}
			else
			{
				parent = Definitions::instance()->getDefinition( WPDT_ITEM, child->text() );
			}

			if ( parent )
			{
				processItem( item, parent );
			}
		}
		else if ( child->name() == "id" )
		{
			item[ "dispid" ] = child->value();
		}
		else if ( child->name() == "name" )
		{
			item[ "name" ] = child->text();
		}
		else if ( child->name() == "color" )
		{
			item[ "color" ] = child->value();
		}
		else if ( child->name() == "category" )
		{
			item[ "categoryname" ] = child->text();
		}
	}
}

typedef QMap< unsigned char, QMap<QCString, QString> > EquipmentContainer;

/*
	Recursive processing function to get neccesary information about npcs.
*/
static void processNpc( QMap<QCString, QString> &item, const cElement *node, EquipmentContainer &equipment )
{
	// If there is an inherit tag, inherit a parent item definition.
	QString inherit = node->getAttribute( "inherit" );
	if ( inherit != QString::null )
	{
		const cElement *parent = Definitions::instance()->getDefinition( WPDT_NPC, inherit );
		if ( parent )
		{
			processNpc( item, parent, equipment );
		}
	}

	int count = node->childCount();
	int i;
	for ( i = 0; i < count; ++i )
	{
		const cElement *child = node->getChild( i );

		// Inherit properties from another item definition
		if ( child->name() == "inherit" )
		{
			const cElement *parent = 0;

			if ( child->hasAttribute( "id" ) )
			{
				parent = Definitions::instance()->getDefinition( WPDT_NPC, child->getAttribute( "id" ) );
			}
			else
			{
				parent = Definitions::instance()->getDefinition( WPDT_NPC, child->text() );
			}

			if ( parent )
			{
				processNpc( item, parent, equipment );
			}
		}
		else if ( child->name() == "id" )
		{
			item[ "bodyid" ] = child->value();
		}
		else if ( child->name() == "skin" )
		{
			item[ "skin" ] = child->value();
		}
		else if ( child->name() == "category" )
		{
			item[ "categoryname" ] = child->text();
		}
		else if ( child->name() == "equipped" )
		{
			uint j;
			for ( j = 0; j < child->childCount(); ++j )
			{
				const cElement *subchild = child->getChild(j);

				if ( subchild->name() == "item" )
				{
					QString id;
					if ( subchild->hasAttribute( "id" ) )
					{
						id = subchild->getAttribute( "id" );
					}
					else if ( subchild->hasAttribute( "list" ) )
					{
						id = Definitions::instance()->getRandomListEntry( subchild->getAttribute( "list" ) );
					}

					const cElement *itemNode = Definitions::instance()->getDefinition( WPDT_ITEM, id );

					if ( itemNode )
					{
						QMap<QCString, QString> item;
						processItem( item, itemNode );

						unsigned int id = item[ "dispid" ].toInt();
						tile_st tile = TileCache::instance()->getTile( id );
						if ( tile.layer != 0 && tile.layer <= 0x19 && tile.animation != 0 )
						{
							item[ "anim" ] = QString::number( tile.animation );
							equipment.insert( tile.layer, item );
						}
					}
				}
			}
		}
	}
}

static void ensureCategory( QMap<QString, unsigned int> &categories, unsigned int &lastcategory, QString category )
{
	int pos = category.findRev( '\\' );
	if ( pos != -1 )
	{
		QString parentCategory = category.left( pos );
		ensureCategory( categories, lastcategory, parentCategory );
	}

	if ( !categories.contains( category ) )
	{
		categories.insert( category, ++lastcategory );
	}
}

/*
	\command exportdefinitions
	\description Export the definitions used by the WPGM utility.
	\notes This command will export the definitions used by the WPGM utility to
	a file called categories.db in your wolfpack directory.
*/
void commandExportDefinitions( cUOSocket* socket, const QString& /*command*/, const QStringList& /*args*/) throw() 
{
	if ( QFile::exists( "categories.db" ) && !QFile::remove( "categories.db" ) ) 
	{
		socket->sysMessage( "Unable to remove existing categories.db." );
		return;
	}

	cSQLiteDriver driver;
	driver.setDatabaseName( "categories.db" );

	if ( !driver.open() )
	{
		socket->sysMessage( "Unable to open categories.db in your wolfpack directory." );
		return;
	}

	Console::instance()->log( LOG_MESSAGE, QString( "Exporting definitions to %1.\n" ).arg( "categories.db" ) );

	try
	{
		// Create Tables
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

		unsigned int lastcategory = 0;
		QMap<QString, unsigned int> categories;
		QMap<QString, unsigned int>::iterator categoriesIt;

		QStringList sections = Definitions::instance()->getSections(WPDT_ITEM);
		QStringList::const_iterator sectionIt;
		QMap<QCString, QString> item;

		for (sectionIt = sections.begin(); sectionIt != sections.end(); ++sectionIt) {
			const cElement *element = Definitions::instance()->getDefinition(WPDT_ITEM, *sectionIt);

			item.clear();
			item.insert( "name", QString::null );
			item.insert( "color", "0" );
			item.insert( "dispid", "0" );
			item.insert( "category", "0" );
			item.insert( "categoryname", QString::null );

			processItem( item, element );

			QString category = item[ "categoryname" ];

			if ( category.isNull() )
			{
				continue;
			}

			// Strip out the portion after the last slash
			int pos = category.findRev( '\\' );
			if ( pos != -1 )
			{
				category = category.left( pos );
			}

			// Create an id for the category
			if ( !categories.contains( category ) )
			{
				ensureCategory( categories, lastcategory, category );
				item[ "category" ] = QString::number( lastcategory );
			}
			else
			{
				item[ "category" ] = QString::number( categories[ category ] );
			}

			// See if there has been a custom name definition
			QString categoryname = item[ "categoryname" ];
			if ( pos != -1 )
			{
				item[ "name" ] = categoryname.right( categoryname.length() - ( pos + 1 ) );
			}
			else
			{
				item[ "name" ] = categoryname;
			}

			// Insert the item into the table.
			QString section = *sectionIt;
			QString sql = QString( "INSERT INTO items VALUES(NULL,'%1',%2,%3,%4,'%5');" )
				.arg( item[ "name" ].replace( "'", "''" ) )
				.arg( item[ "category" ] )
				.arg( item[ "dispid" ] )
				.arg( item[ "color" ] )
				.arg( section.replace( "'", "''" ) );
			driver.exec( sql );
		}

		// Ensure that all categories are in the list
		for ( categoriesIt = categories.begin(); categoriesIt != categories.end(); ++categoriesIt )
		{
			unsigned int parent = 0;
			int pos = categoriesIt.key().findRev( '\\' );
			if ( pos != -1 )
			{
				QString parentName = categoriesIt.key().left( pos );
				if ( categories.contains( parentName ) )
				{
					parent = categories[ parentName ];
				}
			}

			QString name = categoriesIt.key();
			name = name.right(name.length() - (pos + 1));

			QString sql = QString( "INSERT INTO categories VALUES(%1,'%2',%3,0);" )
				.arg( categoriesIt.data() )
				.arg( name.replace( "'", "''" ) )
				.arg( parent );
			driver.exec( sql );
		}

		categories.clear();
		lastcategory = 0;

		sections = Definitions::instance()->getSections( WPDT_LOCATION );
		for ( sectionIt = sections.begin(); sectionIt != sections.end(); ++sectionIt )
		{
			const cElement *element = Definitions::instance()->getDefinition( WPDT_LOCATION, *sectionIt );

			QString category = element->getAttribute( "category" );
			if ( category.isNull() )
			{
				continue;
			}

			int pos = category.findRev( '\\' );

			if ( pos == -1 )
			{
				continue;
			}

			QString name = category.right( category.length() - ( pos + 1 ) );
			category = category.left( pos );

			// Create an id for the category
			unsigned int categoryId;
			if ( !categories.contains( category ) )
			{
				ensureCategory( categories, lastcategory, category );
				categoryId = lastcategory;
			}
			else
			{
				categoryId = categories[ category ];
			}

			Coord_cl coord;
			parseCoordinates( element->text(), coord );
			QString id = *sectionIt;

			QString sql = QString( "INSERT INTO locations VALUES(NULL,'%1',%2,%3,%4,%5,%6,'%7');" )
				.arg( name.replace( "'", "''" ) )
				.arg( categoryId )
				.arg( coord.x )
				.arg( coord.y )
				.arg( coord.z )
				.arg( coord.map )
				.arg( id.replace( "'", "''" ) );

			driver.exec( sql );
		}

		// Ensure that all categories are in the list
		for ( categoriesIt = categories.begin(); categoriesIt != categories.end(); ++categoriesIt )
		{
			unsigned int parent = 0;
			int pos = categoriesIt.key().findRev( '\\' );
			if ( pos != -1 )
			{
				QString parentName = categoriesIt.key().left( pos );
				if ( categories.contains( parentName ) )
				{
					parent = categories[ parentName ];
				}
			}

			QString name = categoriesIt.key();
			name = name.right( name.length() - ( pos + 1 ) );

			QString sql = QString( "INSERT INTO locationcategories VALUES(%1,'%2',%3,0);" )
				.arg( categoriesIt.data() )
				.arg( name.replace( "'", "''" ) )
				.arg( parent );
			driver.exec( sql );
		}

		// Process NPCS
		lastcategory = 0;
		categories.clear();
		sections = Definitions::instance()->getSections( WPDT_NPC );

		EquipmentContainer equipment;

		for ( sectionIt = sections.begin(); sectionIt != sections.end(); ++sectionIt )
		{
			const cElement *element = Definitions::instance()->getDefinition( WPDT_NPC, *sectionIt );

			equipment.clear();
			item.clear();
			item.insert( "name", QString::null );
			item.insert( "skin", "0" );
			item.insert( "bodyid", "0" );
			item.insert( "category", "0" );
			item.insert( "categoryname", QString::null );

			processNpc( item, element, equipment );

			QString category = item[ "categoryname" ];

			if ( category.isNull() )
			{
				continue;
			}

			// Strip out the portion after the last slash
			int pos = category.findRev( '\\' );
			if ( pos != -1 )
			{
				category = category.left( pos );
			}

			// Create an id for the category
			if ( !categories.contains( category ) )
			{
				ensureCategory( categories, lastcategory, category );
				item[ "category" ] = QString::number( lastcategory );
			}
			else
			{
				item[ "category" ] = QString::number( categories[ category ] );
			}

			QString categoryname = item[ "categoryname" ];
			if ( pos != -1 )
			{
				item[ "name" ] = categoryname.right( categoryname.length() - ( pos + 1 ) );
			}
			else
			{
				item[ "name" ] = categoryname;
			}

			// Insert the item into the table.
			QString section = *sectionIt;
			QString sql = QString( "INSERT INTO npcs VALUES(NULL,'%1',%2,%3,%4,'%5');" )
				.arg( item[ "name" ].replace( "'", "''" ) )
				.arg( item[ "category" ] )
				.arg( item[ "bodyid" ] )
				.arg( item[ "skin" ] )
				.arg( section.replace( "'", "''" ) );
			driver.exec( sql );

			int lastInsertId = driver.lastInsertId();

			EquipmentContainer::iterator eIt;
			for ( eIt = equipment.begin(); eIt != equipment.end(); ++eIt )
			{
				QString sql = QString( "INSERT INTO npcequipment VALUES(%1,%2,%3,%4);" )
					.arg( lastInsertId )
					.arg( eIt.data()[ "anim" ].toInt() )
					.arg( eIt.key() )
					.arg( eIt.data()[ "color" ].toInt() );
				driver.exec( sql );
			}
		}

		// Ensure that all categories are in the list
		for ( categoriesIt = categories.begin(); categoriesIt != categories.end(); ++categoriesIt )
		{
			unsigned int parent = 0;
			int pos = categoriesIt.key().findRev( '\\' );
			if ( pos != -1 )
			{
				QString parentName = categoriesIt.key().left( pos );
				if ( categories.contains( parentName ) )
				{
					parent = categories[ parentName ];
				}
			}

			QString name = categoriesIt.key();
			name = name.right( name.length() - ( pos + 1 ) );

			QString sql = QString( "INSERT INTO npccategories VALUES(%1,'%2',%3,0);" )
				.arg( categoriesIt.data() )
				.arg( name.replace( "'", "''" ) )
				.arg( parent );
			driver.exec( sql );
		}

		sections = Definitions::instance()->getSections( WPDT_MULTI );
		for ( sectionIt = sections.begin(); sectionIt != sections.end(); ++sectionIt )
		{
			const cElement *element = Definitions::instance()->getDefinition( WPDT_MULTI, *sectionIt );

			item.clear();
			item.insert( "name", QString::null );
			item.insert( "dispid", "0" );

			processMulti( item, element );

			if (item["name"].isNull() || item["dispid"].toInt() < 0x4000) {
				continue;
			}

			// Insert the item into the table.
			QString section = *sectionIt;
			QString sql = QString( "INSERT INTO multis VALUES(NULL,'%1','%2',%3);" )
				.arg( item[ "name" ].replace( "'", "''" ) )
				.arg( section.replace( "'", "''" ) )
				.arg( item[ "dispid" ].toInt() );
			driver.exec( sql );
		}

		socket->sysMessage( "Finished exporting definitions to categories.db." );
	}
	catch( const QString &e )
	{
		socket->sysMessage( e );
	}
	catch( const wpException &e )
	{
		socket->sysMessage( e.error() );
	}

	driver.close();
}

/*
	\command doorgen
	\description Generate doors in passage ways.
	\notes This command is not guranteed to work correctly. Please see if
	you find any broken doors after you use this command. Don't use this command
	on custom maps.
*/
void commandDoorGenerator( cUOSocket* socket, const QString& /*command*/, const QStringList& args ) throw()
{
	Q_UNUSED( args );
	class DoorGenerator
	{
		enum DoorFacing
		{
			WestCW			= 0,
			EastCCW,
			WestCCW,
			EastCW,
			SouthCW,
			NorthCCW,
			SouthCCW,
			NorthCW
		};

		bool isFrame( int id, int frames[], int size )
		{
			id &= 0x3FFF;
			if ( id > frames[size - 1] )
				return false;

			for ( int i = 0; i < size; ++i )
			{
				int delta = id - frames[i];

				if ( delta < 0 )
					return false;
				else if ( delta == 0 )
					return true;
			}
			return false;
		}

		bool isSouthFrame( int id )
		{
			static int SouthFrames[] =
			{
			0x0006, 0x0008, 0x000B, 0x001A, 0x001B, 0x001F, 0x0038, 0x0057, 0x0059, 0x005B, 0x005D, 0x0080, 0x0081, 0x0082, 0x0084, 0x0090, 0x0091, 0x0094, 0x0096, 0x0099, 0x00A6, 0x00A7, 0x00AA, 0x00AE, 0x00B0, 0x00B3, 0x00C7, 0x00C9, 0x00F8, 0x00FA, 0x00FD, 0x00FE, 0x0100, 0x0103, 0x0104, 0x0106, 0x0109, 0x0127, 0x0129, 0x012B, 0x012D, 0x012F, 0x0131, 0x0132, 0x0134, 0x0135, 0x0137, 0x0139, 0x013B, 0x014C, 0x014E, 0x014F, 0x0151, 0x0153, 0x0155, 0x0157, 0x0158, 0x015A, 0x015D, 0x015E, 0x015F, 0x0162, 0x01CF, 0x01D1, 0x01D4, 0x01FF, 0x0204, 0x0206, 0x0208, 0x020A
			};
			return isFrame( id, SouthFrames, sizeof( SouthFrames ) );
		}

		bool isNorthFrame( int id )
		{
			static int NorthFrames[] =
			{
			0x0006, 0x0008, 0x000D, 0x001A, 0x001B, 0x0020, 0x003A, 0x0057, 0x0059, 0x005B, 0x005D, 0x0080, 0x0081, 0x0082, 0x0084, 0x0090, 0x0091, 0x0094, 0x0096, 0x0099, 0x00A6, 0x00A7, 0x00AC, 0x00AE, 0x00B0, 0x00C7, 0x00C9, 0x00F8, 0x00FA, 0x00FD, 0x00FE, 0x0100, 0x0103, 0x0104, 0x0106, 0x0109, 0x0127, 0x0129, 0x012B, 0x012D, 0x012F, 0x0131, 0x0132, 0x0134, 0x0135, 0x0137, 0x0139, 0x013B, 0x014C, 0x014E, 0x014F, 0x0151, 0x0153, 0x0155, 0x0157, 0x0158, 0x015A, 0x015D, 0x015E, 0x015F, 0x0162, 0x01CF, 0x01D1, 0x01D4, 0x01FF, 0x0201, 0x0204, 0x0208, 0x020A
			};
			return isFrame( id, NorthFrames, sizeof( NorthFrames ) );
		}

		bool isEastFrame( int id )
		{
			static int EastFrames[] =
			{
			0x0007, 0x000A, 0x001A, 0x001C, 0x001E, 0x0037, 0x0058, 0x0059, 0x005C, 0x005E, 0x0080, 0x0081, 0x0082, 0x0084, 0x0090, 0x0092, 0x0095, 0x0097, 0x0098, 0x00A6, 0x00A8, 0x00AB, 0x00AE, 0x00AF, 0x00B2, 0x00C7, 0x00C8, 0x00EA, 0x00F8, 0x00F9, 0x00FC, 0x00FE, 0x00FF, 0x0102, 0x0104, 0x0105, 0x0108, 0x0127, 0x0128, 0x012B, 0x012C, 0x012E, 0x0130, 0x0132, 0x0133, 0x0135, 0x0136, 0x0138, 0x013A, 0x014C, 0x014D, 0x014F, 0x0150, 0x0152, 0x0154, 0x0156, 0x0158, 0x0159, 0x015C, 0x015E, 0x0160, 0x0163, 0x01CF, 0x01D0, 0x01D3, 0x01FF, 0x0203, 0x0205, 0x0207, 0x0209
			};
			return isFrame( id, EastFrames, sizeof( EastFrames ) );
		}

		bool isWestFrame( int id )
		{
			static int WestFrames[] =
			{
			0x0007, 0x000C, 0x001A, 0x001C, 0x0021, 0x0039, 0x0058, 0x0059, 0x005C, 0x005E, 0x0080, 0x0081, 0x0082, 0x0084, 0x0090, 0x0092, 0x0095, 0x0097, 0x0098, 0x00A6, 0x00A8, 0x00AD, 0x00AE, 0x00AF, 0x00B5, 0x00C7, 0x00C8, 0x00EA, 0x00F8, 0x00F9, 0x00FC, 0x00FE, 0x00FF, 0x0102, 0x0104, 0x0105, 0x0108, 0x0127, 0x0128, 0x012C, 0x012E, 0x0130, 0x0132, 0x0133, 0x0135, 0x0136, 0x0138, 0x013A, 0x014C, 0x014D, 0x014F, 0x0150, 0x0152, 0x0154, 0x0156, 0x0158, 0x0159, 0x015C, 0x015E, 0x0160, 0x0163, 0x01CF, 0x01D0, 0x01D3, 0x01FF, 0x0200, 0x0203, 0x0207, 0x0209
			};
			return isFrame( id, WestFrames, sizeof( WestFrames ) );
		}

		bool coordHasEastFrame( int x, int y, int z, int map )
		{
			StaticsIterator tiles = Maps::instance()->staticsIterator( Coord_cl( x, y, z, map ), true );
			for ( ; !tiles.atEnd(); ++tiles )
			{
				if ( tiles.data().zoff == z && isEastFrame( tiles.data().itemid ) )
				{
					return true;
				}
			}
			return false;
		}

		bool coordHasSouthFrame( int x, int y, int z, int map )
		{
			StaticsIterator tiles = Maps::instance()->staticsIterator( Coord_cl( x, y, z, map ), true );
			for ( ; !tiles.atEnd(); ++tiles )
			{
				if ( tiles.data().zoff == z && isSouthFrame( tiles.data().itemid ) )
				{
					return true;
				}
			}
			return false;
		}

		cItem* addDoor( int x, int y, int z, int map, DoorFacing facing )
		{
			//int doorTop = z + 20;

			if ( y == 1743 && x >= 1343 && x <= 1344 )
				return 0;
			if ( y == 1679 && x >= 1392 && x <= 1393 )
				return 0;
			if ( x == 1320 && y >= 1618 && y <= 1640 )
				return 0;
			if ( x == 1383 && y >= 1642 && y <= 1643 )
				return 0;
			if ( !Maps::instance()->canFit( x, y, z, map ) )
				return 0;
			cItem* door = cItem::createFromScript( QString::number( 0x6A5 + 2 * int( facing ), 16 ) );
			door->moveTo( Coord_cl( x, y, z, map ), true );
			return door;
		}
	public:

		int generate( int region[], int map, cUOSocket* )
		{
			int count = 0;
			for ( int rx = region[0]; rx < region[2]; ++rx )
			{
				for ( int ry = region[1]; ry < region[3]; ++ry )
				{
					StaticsIterator tiles = Maps::instance()->staticsIterator( map, rx, ry, true );
					for ( ; !tiles.atEnd(); ++tiles )
					{
						int id = tiles.data().itemid;
						int z = tiles.data().zoff;
						if ( isWestFrame( id ) )
						{
							if ( coordHasEastFrame( rx + 2, ry, z, map ) )
							{
								addDoor( rx + 1, ry, z, map, WestCW );
								++count;
							}
							else if ( coordHasEastFrame( rx + 3, ry, z, map ) )
							{
								cItem* first = addDoor( rx + 1, ry, z, map, WestCW );
								cItem* second = addDoor( rx + 2, ry, z, map, EastCCW );
								count += 2;
								if ( first && second )
								{
									first->setTag( "link", second->serial() );
									second->setTag( "link", first->serial() );
								}
								else
								{
									if ( !first && second )
									{
										second->remove();
										--count;
									}
									if ( !second && first )
									{
										first->remove();
										--count;
									}
								}
							}
						}
						else if ( isNorthFrame( id ) )
						{
							if ( coordHasSouthFrame( rx, ry + 2, z, map ) )
							{
								addDoor( rx, ry + 1, z, map, SouthCW );
								++count;
							}
							else if ( coordHasSouthFrame( rx, ry + 3, z, map ) )
							{
								cItem* first = addDoor( rx, ry + 1, z, map, NorthCCW );
								cItem* second = addDoor( rx, ry + 2, z, map, SouthCW );
								count += 2;
								if ( first && second )
								{
									first->setTag( "link", second->serial() );
									second->setTag( "link", first->serial() );
								}
								else
								{
									if ( !first && second )
									{
										second->remove();
										--count;
									}
									if ( !second && first )
									{
										first->remove();
										--count;
									}
								}
							}
						}
					}
				}
			}
			return count;
		}
	};

	DoorGenerator generator;

	int BritRegions[][4] =
	{
	{  250,  750,  775, 1330 }, {  525, 2095,  925, 2430 }, { 1025, 2155, 1265, 2310 }, { 1635, 2430, 1705, 2508 }, { 1775, 2605, 2165, 2975 }, { 1055, 3520, 1570, 4075 }, { 2860, 3310, 3120, 3630 }, { 2470, 1855, 3950, 3045 }, { 3425,  990, 3900, 1455 }, { 4175,  735, 4840, 1600 }, { 2375,  330, 3100, 1045 }, { 2100, 1090, 2310, 1450 }, { 1495, 1400, 1550, 1475 }, { 1085, 1520, 1415, 1910 }, { 1410, 1500, 1745, 1795 }, { 5120, 2300, 6143, 4095 }
	};
	/*
	int IlshRegions[][4] =
	{
	{ 0, 0, 288 * 8, 200 * 8 }
	};
	int MalasRegions[][4] =
	{
	{ 0, 0, 320 * 8, 256 * 8 }
	};
	*/

	socket->sysMessage( "Generating doors, please wait ( Slow )" );
	int count = 0;
	if ( Maps::instance()->hasMap( 0 ) )
	{
		for ( int i = 0; i < 16; ++i )
		{
			socket->sysMessage( QString( "doing [%1, %2, %3, %4]" ).arg( BritRegions[i][0] ).arg( BritRegions[i][1] ).arg( BritRegions[i][2] ).arg( BritRegions[i][3] ) );
			count += generator.generate( BritRegions[i], 0, socket );
			socket->sysMessage( tr( "Doors so far: %1" ).arg( count ) );
		}
	}
}

// Command Table (Keep this at the end)
stCommand cCommands::commands[] =
{
{ "ALLMOVE", commandAllMove },
{ "EXPORTDEFINITIONS", commandExportDefinitions },
{ "ALLSHOW", commandAllShow },
{ "BROADCAST", commandBroadcast },
{ "DOORGEN", commandDoorGenerator },
{ "GMTALK", commandGmtalk },
{ "PAGES", commandPages },
{ "PAGENOTIFY", commandPageNotify },
{ "RELOAD", commandReload },
{ "RESTOCK", commandRestock },
{ "SAVE", commandSave },
{ "SERVERTIME", commandServerTime },
{ "SET", commandSet },
{ "SHOW", commandShow },
{ "SHUTDOWN", commandShutDown },
{ "STAFF", commandStaff },
{ "SPAWNREGION", commandSpawnRegion },
{ NULL, NULL }
};
