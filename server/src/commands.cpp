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
#include "config.h"
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
#include "network.h"

// System Includes
#include <functional>

// Main Command processing function
void cCommands::process( cUOSocket *socket, const QString &command )
{
	if( !socket->player() )
		return;

	P_PLAYER pChar = socket->player();
	QStringList pArgs = QStringList::split( " ", command, true );

	// No Command? No Processing
	if( pArgs.isEmpty() )
		return;

	QString pCommand = pArgs[0].upper(); // First element should be the command

	// Remove it from the argument list
	pArgs.erase( pArgs.begin() );

	// Check if the priviledges are ok
	if( !pChar->account()->authorized("command", pCommand.latin1() ))
	{
		socket->sysMessage( tr( "Access to command '%1' was denied" ).arg( pCommand.lower() ) );
		socket->log( QString("Access to command '%1' was denied\n").arg(pCommand.lower()) );
		return;
	}

	// Dispatch the command
	if ( dispatch( socket, pCommand, pArgs ) )
		socket->log( QString( "Used command '%1'.\n" ).arg( command ) );
}

// Selects the right command Stub
bool cCommands::dispatch( cUOSocket *socket, const QString &command, const QStringList &arguments )
{
	// Just in case we have been called directly
	if( !socket || !socket->player() )
		return false;

	// Check for custom commands
	PyObject *function = ScriptManager::instance()->getCommandHook( command.latin1() );

	if( function )
	{
		QString argString = arguments.join( " " );

		if( argString.isNull() )
			argString = "";

		PyObject *args = Py_BuildValue( "O&uu", PyGetSocketObject, socket, command.ucs2(), argString.ucs2() );

		PyObject *result = PyObject_CallObject( function, args );
		Py_XDECREF( result );
		reportPythonError();

		Py_DECREF( args );

		return true;
	}

	for( uint index = 0; commands[index].command; ++index )
		if( command == commands[index].name )
		{
			(commands[index].command)( socket, command, arguments );
			return true;
		}

	socket->sysMessage( tr("Unknown Command") );
	return false;
}

void cCommands::loadACLs( void )
{
	// make sure it's clean
	QMap< QString, cAcl* >::iterator itA (_acls.begin());
	for ( ; itA != _acls.end(); ++itA )
		delete itA.data();
	_acls.clear();

	QStringList ScriptSections = Definitions::instance()->getSections( WPDT_PRIVLEVEL );

	if (ScriptSections.isEmpty()) {
		Console::instance()->log(LOG_WARNING, "No ACLs for players, counselors, gms and admins defined!\n");
		return;
	}

	// We are iterating trough a list of ACLs
	// In each loop we create one acl
	for( QStringList::iterator it = ScriptSections.begin(); it != ScriptSections.end(); ++it )
	{
		const cElement *Tag = Definitions::instance()->getDefinition( WPDT_PRIVLEVEL, *it );

		if( !Tag )
			continue;

		QString ACLname = *it;

		// While we are in this loop we are building an ACL
		cAcl *acl = new cAcl;
		acl->name = ACLname;
		acl->rank = Tag->getAttribute("rank", "1").toUShort();
		if (acl->rank == 0 || acl->rank == 255) {
			acl->rank = 1;
		}

		QMap< QString, bool > group;
		QCString groupName;


		for( unsigned int i = 0; i < Tag->childCount(); ++i )
		{
			const cElement *childTag = Tag->getChild( i );
			if( childTag->name() == "group" )
			{
				groupName = childTag->getAttribute( "name" );

				for( unsigned int j = 0; j < childTag->childCount(); ++j )
				{
					const cElement *groupTag = childTag->getChild( j );

					if( groupTag->name() == "action" )
					{
						QString name = groupTag->getAttribute( "name", "any" );
						bool permit = groupTag->getAttribute( "permit", "false" ) == "true" ? true : false;
						group.insert( name, permit );
					}
				}

				if( !group.isEmpty() )
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
	\command resurrect
	\description Resurrects a character.
	\notes If the character is standing on his corpse he will regain
	all his posessions and equipment.
*/
void commandResurrect( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(args);
	Q_UNUSED(command);
	socket->sysMessage( tr( "Select the being you want to resurrect" ) );
	socket->attachTarget( new cResurectTarget );
}

/*
	\command kill
	\description Kills the selected character.
	\notes You cannot kill invulnerable characters this way.
*/
void commandKill( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(args);
	Q_UNUSED(command);
	socket->sysMessage( tr( "Please select a target to kill" ) );
	socket->attachTarget( new cKillTarget );
}

/*
	\command fix
	\description Resend the player information.
*/
void commandFix( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(args);
	Q_UNUSED(command);
	// TODO: Eventually check if the character is stuck etc.
	socket->resendPlayer();
	socket->player()->resendTooltip();
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
void commandSet( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(command);
	if( args.size() < 1 )
	{
		socket->sysMessage( "Usage: set <key> <value>" );
		return;
	}

	QString key = args[0];
	QStringList realargs(args);
	realargs.remove(realargs.begin());
	QString value;
	if (realargs.size() == 0)
	{
		value = "";
	}
	else
	{
		value = realargs.join(" ");
	}

	// Alias for speed sake on setting stats.
	if( key == "str" )
	{
		socket->sysMessage( tr( "Please select a target to 'set %1 %2' " ).arg( "strength" ).arg( value ) );
		socket->attachTarget( new cSetTarget( "strength", value ) );
	}
	else if( key == "dex" )
	{
		socket->sysMessage( tr( "Please select a target to 'set %1 %2' " ).arg( "dexterity" ).arg( value ) );
		socket->attachTarget( new cSetTarget( "dexterity", value ) );
	}
	else if( key == "int" )
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
	\command resend
	\description Resend the player and the surrounding objects.
*/
void commandResend( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(args);
	Q_UNUSED(command);
	socket->resendPlayer(false);
}

/*
	\command remove
	\description Delete an item or character.
*/
void commandRemove( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(socket);
	Q_UNUSED(command);
	Q_UNUSED(args);
	socket->attachTarget( new cRemoveTarget );
}

/*
	\command account
	\usage - <code>account create username password</code>
	- <code>account remove username</code>
	- <code>account set username key value</code>
	- <code>account show username key</code>
	Use the create subcommand to create a new account with the given username and password.
	To remove an account along with all player characters on that account, use the remove
	subcommand and pass the username to it.
	To change properties of a given account, use the set subcommand and pass the username,
	the property key and the new property value to it. See the notes for a list of valid property keys.
	To view properties of an account, use the show subcommand and pass the property key to it.

	\notes The following properties can be set for accounts:
    <i>password</i>
	The account password.

	<i>acl</i>
	The name of the access control list.

	<i>block</i>
	Block status of the account.

	In addition to the writeable properties, the following properties can be shown:

	<i>loginattempts</i>
	How many failed login attempts have been made since the last successful login.

	<i>lastlogin</i>
	When was the last successful login made.

	<i>chars</i>
	Prints a list of player characters on this account.

	Valid values for the block property are either on, off or for how long the account should be blocked.

	If you have enabled MD5 passwords, you can only view the hashed password when showing the password property.
*/
void commandAccount( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(command);
	// Account Create User Pass
	// Account Remove User
	// Account Set User Pass
	// Account Show User Pass
	if( args.count() == 0 )
	{
		socket->sysMessage( tr( "Usage: account <create|remove|set|show>" ) );
		return;
	}

	QString subCommand = args[0].lower();

	// Create Accounts
	if( subCommand == "create" )
	{
		// Create a new account
		if( args.count() < 3 )
		{
			socket->sysMessage( tr( "Usage: account create <username> <password>" ) );
		}
		else if( Accounts::instance()->getRecord( args[1].left( 30 ) ) )
		{
			socket->sysMessage( tr( "Account '%1' already exists" ).arg( args[1].left( 30 ) ) );
		}
		else
		{
			Accounts::instance()->createAccount( args[1].left( 30 ), args[2].left( 30 ) );
			socket->sysMessage( tr( "Account '%1' with password '%2' has been created" ).arg( args[1].left( 30 ) ).arg( args[2].left( 30 ) ) );
		}
	}

	// Remove an Account and all associated characters
	else if( subCommand == "remove" )
	{
		if( args.count() < 2 )
		{
			socket->sysMessage( tr( "Usage: account remove <username>" ) );
		}
		else if( !Accounts::instance()->getRecord( args[1].left( 30 ) ) )
		{
			socket->sysMessage( tr( "Account '%1' does not exist" ).arg( args[1].left( 30 ) ) );
		}
		else
		{
			cAccount *account = Accounts::instance()->getRecord( args[1].left( 30 ) );
			QValueVector<P_PLAYER> characters = account->caracterList();
			Accounts::instance()->remove( account );
			UINT32 i = 0;
			for(; i < characters.size(); ++i )
				if( characters[i] )
					characters[i]->remove();

			socket->sysMessage( tr( "Account '%1' and %2 characters have been removed" ).arg( args[1].left( 30 ) ).arg( i+1 ) );
		}
	}

	// Set properties of accounts
	else if( subCommand == "set" )
	{
		if( args.count() < 4 )
		{
			socket->sysMessage( tr( "Usage: account set <username> <key> <value>" ) );
		}
		else if( !Accounts::instance()->getRecord( args[1].left( 30 ) ) )
		{
				socket->sysMessage( tr( "Account '%1' does not exist" ).arg( args[1].left( 30 ) ) );
		}
		else
		{
			cAccount *account = Accounts::instance()->getRecord( args[1].left( 30 ) );
			QString key = args[2];
			QString value = args[3];

			if( key == "password" )
			{
				account->setPassword( value.left( 30 ) ); // Maximum of 30 Chars allowed
				socket->sysMessage( tr( "The password of account '%1' has been set to '%2'" ).arg( account->login() ).arg( value.left( 30 ) ) );
			}
			else if( key == "block" )
			{
				if( value.lower() == "on" )
				{
					account->setBlocked( true );
					socket->sysMessage( tr( "Account '%1' has been blocked" ).arg( account->login() ) );
				}
				else if( value.lower() == "off" )
				{
					account->setBlocked( false );
					socket->sysMessage( tr( "Account '%1' has been unblocked" ).arg( account->login() ) );
				}
				else
				{
					bool ok = false;
					UINT32 blockTime = hex2dec( value ).toUInt( &ok );

					if( ok )
					{
						account->block( blockTime );
						socket->sysMessage( tr( "Account '%1' will be blocked for %2 seconds" ).arg( account->login() ).arg( blockTime ) );
					}
					else
					{
						socket->sysMessage( tr( "Usage: account set <username> block <on|off>" ) );
					}
				}
			}
			else if( key == "acl" )
			{
				if( !Commands::instance()->getACL( value.latin1() ) )
				{
					socket->sysMessage( tr( "You tried to specify an unknown acl '%1'" ).arg( value ) );
				}
				else
				{
					account->setAcl( value );
					account->refreshAcl();
				}
			}
			else
			{
				socket->sysMessage( tr( "Unknown field '%1' for account '%2'" ).arg( args[2] ).arg( account->login() ) );
			}
		}
	}
	// Show properties of accounts
	else if( subCommand == "show" )
	{
		if( args.count() < 3 )
		{
			socket->sysMessage( tr( "Usage: account show <username> <key>" ) );
		}
		else if( !Accounts::instance()->getRecord( args[1].left( 30 ) ) )
		{
			socket->sysMessage( tr( "Account '%1' does not exist" ).arg( args[1].left( 30 ) ) );
		}
		else
		{
			cAccount *account = Accounts::instance()->getRecord( args[1].left( 30 ) );
			QString key = args[2];

			if( key == "password" )
			{
				socket->sysMessage( tr( "The password of account '%1' is '%2'" ).arg( account->login() ).arg( account->password() ) );
			}
			else if( key == "block" )
			{
				if( account->isBlocked() )
					socket->sysMessage( tr( "Account '%1' is currently blocked" ).arg( account->login() ) );
				else if( account->secsToUnblock() )
					socket->sysMessage( tr( "Account '%1' will be unblocked in %2 seconds" ).arg( account->login() ).arg( account->secsToUnblock() ) );
				else
					socket->sysMessage( tr( "Account '%1' is currently not blocked" ).arg( account->login() ) );
			}
			else if( key == "loginattempts" )
			{
				socket->sysMessage( tr( "There were %1 unsuccesul login attempts for account '%2'" ).arg( account->loginAttempts() ).arg( account->login() ) );
			}
			else if( key == "lastlogin" )
			{
				socket->sysMessage( tr( "The last login of account '%1' was on %2" ).arg( account->login() ).arg( account->lastLogin().toString( Qt::ISODate ) ) );
			}
			else if( key == "acl" )
			{
				socket->sysMessage( tr( "The acl of account '%1' is %2" ).arg( account->login() ).arg( account->acl() ) );
			}
			else if( key == "chars" )
			{
				QStringList sCharList;
				QValueVector< P_PLAYER > pCharList = account->caracterList();

				for( UINT32 i = 0; i < pCharList.size(); ++i )
					if( pCharList[i] )
						sCharList.push_back( QString( "0x%1" ).arg( pCharList[i]->serial(), 8, 16 ) );

				socket->sysMessage( tr( "Account '%1' has the following characters: %2" ).arg( account->login() ).arg( sCharList.join( ", " ) ) );
			}
			else
			{
				socket->sysMessage( tr( "Unknown field '%1' for account '%2'" ).arg( args[2] ).arg( account->login() ) );
			}
		}
	}
}

/*
	\command tele
	\description Transports you directly to the targetted location.
*/
void commandTele( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(socket);
	Q_UNUSED(command);
	Q_UNUSED(args);
	socket->attachTarget( new cTeleTarget );
}

/*
	\command save
	\description Forces the world to be saved.
*/
void commandSave( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(args);
	Q_UNUSED(socket);
	Q_UNUSED(command);

	World::instance()->save();
}

/*
	\command servertime
	\description Shows the current server uptime in miliseconds.
*/
void commandServerTime( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(args);
	Q_UNUSED(command);
	socket->sysMessage( tr( "Server time: %1" ).arg( Server::instance()->time() ) );
}

/*
	\command show
	\description Show properties of characters and items.
	\usage - <code>show key</code>
	Key is the name of the property you want to see.
	\notes See the <command id="SET">SET</command> command for more information.
*/
void commandShow( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(command);
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
void commandSpawnRegion( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(command);
	// Spawnregion respawn region_name
	// Spawnregion clear   region_name
	// Spawnregion fill    region_name
	// Spawnregion info    region_name

	// region_name can be "all"

	if( args.count() == 0 )
	{
		socket->sysMessage( tr( "Usage: spawnregion <respawn|clear|fill>" ) );
		return;
	}

	QString subCommand = args[0].lower();

	// respawn spawnregion
	if( subCommand == "respawn" )
	{
		if( args.count() < 2 )
		{
			socket->sysMessage( tr( "Usage: spawnregion respawn <region_name>" ) );
		}
		else if( args[1].lower() != "all" )
		{
			cSpawnRegion* spawnRegion = SpawnRegions::instance()->region( args[1] );
			if( !spawnRegion )
			{
				socket->sysMessage( tr( "Spawnregion %1 does not exist" ).arg( args[1] ) );
			}
			else
			{
				spawnRegion->reSpawn();
				socket->sysMessage( tr( "Spawnregion '%1' has respawned" ).arg( args[1] ) );
			}
		}
		else if( args[1].lower() == "all" )
		{
			SpawnRegions::instance()->reSpawn();
			socket->sysMessage( tr( "All spawnregions have respawned" ) );
		}
	}

	// clear spawnregions (despawn)
	else if( subCommand == "clear" )
	{
		if( args.count() < 2 )
		{
			socket->sysMessage( tr( "Usage: spawnregion clear <region_name>" ) );
		}
		else if( args[1].lower() != "all" )
		{
			cSpawnRegion* spawnRegion = SpawnRegions::instance()->region( args[1] );
			if( !spawnRegion )
			{
				socket->sysMessage( tr( "Spawnregion %1 does not exist" ).arg( args[1] ) );
			}
			else
			{
				spawnRegion->deSpawn();
				socket->sysMessage( tr( "Spawnregion '%1' has been cleared" ).arg( args[1] ) );
			}
		}
		else if( args[1].lower() == "all" )
		{
			SpawnRegions::instance()->deSpawn();
			socket->sysMessage( tr( "All spawnregions have been cleared" ) );
		}
	}

	// fill spawnregions up (respawnmax)
	else if( subCommand == "fill" )
	{
		if( args.count() < 2 )
		{
			socket->sysMessage( tr( "Usage: spawnregion fill <region_name>" ) );
		}
		else if( args[1].lower() != "all" )
		{
			cSpawnRegion* spawnRegion = SpawnRegions::instance()->region( args[1] );
			if( !spawnRegion )
			{
				socket->sysMessage( tr( "Spawnregion %1 does not exist" ).arg( args[1] ) );
			}
			else
			{
				spawnRegion->reSpawnToMax();
				socket->sysMessage( tr( "Spawnregion '%1' has respawned to maximum" ).arg( args[1] ) );
			}
		}
		else if( args[1].lower() == "all" )
		{
			SpawnRegions::instance()->reSpawnToMax();
			socket->sysMessage( tr( "All spawnregions have respawned to maximum" ) );
		}
	}

	// show spawnregion info
	else if( subCommand == "info" )
	{
		if( args.count() < 2 )
		{
			socket->sysMessage( tr( "Usage: spawnregion info <region_name>" ) );
		}
		else if( args[1].lower() != "all" )
		{
			cSpawnRegion* spawnRegion = SpawnRegions::instance()->region( args[1] );
			if( !spawnRegion )
			{
				socket->sysMessage( tr( "Spawnregion %1 does not exist" ).arg( args[1] ) );
			}
			else
			{
				cSpawnRegionInfoGump* pGump = new cSpawnRegionInfoGump( spawnRegion );
				socket->send( pGump );
			}
		}
		else if( args[1].lower() == "all" )
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
	\command tags
	\description Manage the tags for an object.
	\usage - <code>tags set [key] [value]</code>
	- <code>tags set [key] [value] string</code>
	- <code>tags set [key] [value] value</code>
	Key denotes the name of the tag you want to set, remove or view.
	Value is the value of the new tag. If you specify 'value' as the last
	parameter, the tag will be evaluated and set as an integer.
	The set subcommand will attach a new tag or change an existing tag.
	The get subcommand will show the value of a tag attached to the object.
	The remove subcommand will remove a tag from the object.
	The info subcommand will show a dialog with all attached tags.

	\notes The remove subcommand also accepts <i>all</i> as the key which will
	remove all attached tags from the object.
*/
void commandTags( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(command);
	// Tags set		<key> <value>	(as string/value)
	// Tags get		<key>
	// Tags remove	<key> or all
	// Tags info

	if( args.count() == 0 )
	{
		socket->sysMessage( tr( "Usage: tags <set|get|remove|info>" ) );
		return;
	}

	QString subCommand = args[0].lower();

	// set
	if( subCommand == "set" )
	{
		if( args.count() < 3 )
		{
			socket->sysMessage( tr( "Usage: tags set <key> <value> (as value/string)" ) );
		}
		else
		{
			UINT8 type = 0; // 0 - string, 1 - value
			if( args.count() == 5 )
			{
				if( args[4].lower() == "value" )
					type = 1;
			}
			socket->sysMessage( tr( "Please select a target" ) );
			socket->attachTarget( new cSetTagTarget( args[1], args[2], type ) );
		}
	}
	if( subCommand == "get" )
	{
		if( args.count() < 2 )
		{
			socket->sysMessage( tr( "Usage tags get <key>" ) );
		}
		else
		{
			socket->sysMessage( tr( "Please select a target" ) );
			socket->attachTarget( new cGetTagTarget( args[1] ) );
		}
	}
	if( subCommand == "remove" )
	{
		if( args.count() < 2 )
		{
			socket->sysMessage( tr( "Usage tags remove <key>" ) );
		}
		else
		{
			socket->sysMessage( tr( "Please select a target" ) );
			socket->attachTarget( new cRemoveTagTarget( args[1] ) );
		}
	}
	if( subCommand == "info" )
	{
		socket->sysMessage( tr( "Please select a target" ) );
		socket->attachTarget( new cTagsInfoTarget() );
	}
}

/*
	\command who
	\description Manage connected clients.
	\notes The gump shown will allow you to travel to the client, send messages or bring them directly to you.
*/
void commandWho( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(args);
	Q_UNUSED(command);
	// Who
	cWhoMenuGump* pGump = new cWhoMenuGump( 1 );
	socket->send( pGump );
}

/*
	\command pages
	\description Manage support tickets.
*/
void commandPages( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(args);
	Q_UNUSED(command);
	// Pages
	if( socket->player() )
	{
		cPagesGump* pGump = NULL;

		if( socket->player()->isGM() )
			pGump = new cPagesGump( 1, PT_GM );
		else if( socket->player()->isCounselor() )
			pGump = new cPagesGump( 1, PT_COUNSELOR );

		if( pGump )
			socket->send( pGump );
	}
}

/*
	\command shutdown
	\description Shutdown the Wolfpack server.
*/
void commandShutDown( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(socket);
	Q_UNUSED(command);
	// Shutdown
	if( args.count() == 0 )
		Server::instance()->cancel();
}

/*
	\command staff
	\description Toggle the staff flag for your account.
	\notes The staff flag controls whether you are treated as a priviledged user or not.
*/
void commandStaff( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(command);
	if( socket->account()->isStaff() || ( args.count() > 0 && args[0].toInt() == 0 ) )
	{
		socket->account()->setStaff( false );
		socket->sysMessage( tr( "Staff is now '0'." ) );
	}
	else if( !socket->account()->isStaff() || ( args.count() > 0 && args[0].toInt() == 1 ) )
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
void commandReload( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(command);
	// Reload accounts
	// Reload scripts
	// Reload all

	if( args.count() == 0 )
	{
		socket->sysMessage( tr( "Usage: reload <accounts|scripts|python|all>" ) );
		return;
	}

	QString subCommand = args[0].lower();

	// accounts
	if( subCommand == "accounts" )
	{
		Server::instance()->reload("accounts");
	}
	if( subCommand == "python" )
	{
		Server::instance()->reload("scripts");
	}

	if( subCommand == "scripts" )
	{
		Server::instance()->reload("definitions");
	}
	
	if (subCommand == "all") {
		Server::instance()->reload("configuration"); // This will reload nearly everything
	}
}

/*
	\command addevent
	\description Attach a script to an object.
	\usage - <code>addevent [script]</code>
	Script is the id of the script you want to attach.
*/
void commandAddEvent( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(command);
	if (args.size() < 1) {
		socket->sysMessage( "Usage: addevent <identifier>" );
		return;
	}

    QString event = args.join( " " );

	// No such event
	if( !ScriptManager::instance()->find( event.latin1() ) )
	{
		socket->sysMessage( tr( "Invalid event: '%1'" ).arg( event ) );
		return;
	}

	socket->sysMessage( tr( "Please select a target to add event '%1' to." ).arg( event ) );
	socket->attachTarget( new cAddEventTarget( event ) );
}

/*
	\command removeevent
	\description Remove a script from an object.
	\usage - <code>removeevent [script]</code>
	Script is the id of the script you want to remove.
*/
void commandRemoveEvent( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(command);
	if( args.size() < 1 )
	{
		socket->sysMessage( "Usage: removeevent <identifier>" );
		return;
	}

    QString event = args.join( " " );

	socket->sysMessage( tr( "Please select a target to remove event '%1' from." ).arg( event ) );
	socket->attachTarget( new cRemoveEventTarget( event ) );
}

/*
	\command move
	\description Move an object relatively to its current position.
	\usage - <code>move [x]</code>
	- <code>move [x],[y]</code>
	- <code>move [x],[y],[z]</code>
	X, y and z are the offsets the object should be moved by.
*/
void commandMove( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(command);
	if( args.size() < 1 )
	{
		socket->sysMessage( "Usage: move <x,y,z>" );
		return;
	}

    // Our first argument should be the relative position
	QStringList relPos = QStringList::split( ",", args[0] );

	if( relPos.count() < 1 )
	{
		socket->sysMessage( "Usage: move <x,y,z>" );
		return;
	}

	if( relPos.count() < 2 )
		relPos.push_back( "0" );

	if( relPos.count() < 3 )
		relPos.push_back( "0" );

	INT16 x = relPos[0].toInt();
	INT16 y = relPos[1].toInt();
	INT16 z = relPos[2].toInt();

	socket->sysMessage( tr( "Please select a target to 'move %1,%2,%3'." ).arg( x ).arg( y ).arg( z ) );
	socket->attachTarget( new cMoveTarget( x, y, z ) );
}

/*
	\command allshow
	\description Toggles the allshow flag of your account.
	\notes The allshow flag determines whether you can see logged out characters.
*/
void commandAllShow( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(command);
	if( !socket->player() || !socket->player()->account() )
		return;

	// Switch
	if( !args.count() )
		socket->player()->account()->setAllShow( !socket->player()->account()->isAllShow() );
	// Set
	else
		socket->player()->account()->setAllShow( args[0].toInt() != 0 );

	if( socket->player()->account()->isAllShow() )
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
void commandAllMove( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(command);
	if( !socket->player() || !socket->player()->account() )
		return;

	// Switch
	if( !args.count() )
		socket->player()->account()->setAllMove( !socket->player()->account()->isAllMove() );
	// Set
	else
		socket->player()->account()->setAllMove( args[0].toInt() != 0 );

	if( socket->player()->account()->isAllMove() )
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
void commandRestock( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(args);
	Q_UNUSED(command);
	socket->sysMessage( tr( "Please select the vendor you want to restock." ) );
	socket->attachTarget( new cRestockTarget );
}

/*
	\command allskills
	\description Sets all skills of your character.
	\usage - <code>allskills [value]</code>
	Value is the value all skills should be set to. It's multiplied by 10 (100.0% = 1000).
*/
void commandAllSkills( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(command);
	if( args.count() < 1 )
	{
		socket->sysMessage( tr( "Usage: allskills <value>" ) );
		return;
	}

	P_PLAYER pChar = socket->player();
	UINT32 value = args[0].toInt();

	if( pChar )
	{
		for( int i = 0; i < ALLSKILLS; ++i )
		{
			pChar->setSkillValue( i, value );


			if( pChar->socket() )
				pChar->socket()->sendSkill( i );
		}
	}
}

/*
	\command broadcast
	\description Broadcast a message to all connected clients.
	\usage - <code>broadcast [message]</code>
	Message is the message you want to broadcast to everyone.
*/
void commandBroadcast( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(socket);
	Q_UNUSED(command);
	Network::instance()->broadcast(args.join( " " ));
}

/*
	\command invis
	\description Toggle invisibility.
*/
void commandInvis( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(command);
	socket->player()->removeFromView();

	if( socket->player()->isInvisible() || ( args.count() > 0 && args[0].toInt() == 0 ) )
	{
		socket->player()->setInvisible( false );
		socket->sysMessage( tr( "Invisible is now '0'." ) );
	}
	else if( !socket->player()->isInvisible() || ( args.count() > 0 && args[0].toInt() == 1 ) )
	{
		socket->player()->setInvisible( true );
		socket->sysMessage( tr( "Invisible is now '1'." ) );
	}

	socket->player()->resend(false);
}
/*
	\command pagenotify
	\description Toggle notification about new support tickets.
	\notes If you opt to turn this flag on, you will be notified about incoming pages.
*/
void commandPageNotify( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(command);
	if( socket->account()->isPageNotify() || ( args.count() > 0 && args[0].toInt() == 0 ) )
	{
		socket->account()->setPageNotify( false );
		socket->sysMessage( tr( "PageNotify is now '0'." ) );
	}
	else if( !socket->account()->isPageNotify() || ( args.count() > 0 && args[0].toInt() == 1 ) )
	{
		socket->account()->setPageNotify( true );
		socket->sysMessage( tr( "PageNotify is now '1'." ) );
	}
}

/*
	\command password
	\description Change your current password.
	\usage - <code>password [value]</code>
	Value is the new password for your account. Its length is
	limited to 30 characters.
*/
void commandPassword( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(command);
	if( args.count() < 1 )
	{
		socket->sysMessage( tr( "Usage: password <newpassword>" ) );
		return;
	}

	QString password = args.join( " " );
	if( password.length() > 30 )
	{
		socket->sysMessage( tr( "Your password can have a maximum of 30 characters." ) );
		return;
	}

	socket->account()->setPassword( password );
	socket->sysMessage( tr( "Your password has been changed." ) );
}

/*
	\command gmtalk
	\description Broadcast a message to connected gamemasters.
	\usage - <code>gmtalk [message]</code>
	Send a message to all other connected gamemasters.
*/
void commandGmtalk( cUOSocket *socket, const QString &command, const QStringList &args ) throw()
{
	Q_UNUSED(command);
	if( args.count() < 1 )
	{
		socket->sysMessage( tr( "Usage: gmtalk <message>" ) );
		return;
	}

	QString message = "<" + socket->player()->name() + ">: " + args.join( " " );

	cUOSocket *mSock = 0;
	for( mSock = Network::instance()->first(); mSock; mSock = Network::instance()->next() )
	{
		if( mSock->player() && mSock->player()->isGM() )
			mSock->sysMessage( message, 0x539 );
	}
}

/*
	\command doorgen
	\description Generate doors in passage ways.
	\notes This command is not guranteed to work correctly. Please see if
	you find any broken doors after you use this command. Don't use this command
	on custom maps.
*/
void commandDoorGenerator( cUOSocket* socket, const QString &command, const QStringList &args ) throw()
{
	class DoorGenerator
	{
		enum DoorFacing {
			WestCW  = 0, EastCCW, WestCCW, EastCW, SouthCW, NorthCCW, SouthCCW, NorthCW
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
			static int SouthFrames[] = {
				0x0006,0x0008,0x000B,0x001A,0x001B,0x001F,
				0x0038,0x0057,0x0059,0x005B,0x005D,0x0080,
			    0x0081,0x0082,0x0084,0x0090,0x0091,0x0094,
			    0x0096,0x0099,0x00A6,0x00A7,0x00AA,0x00AE,
			    0x00B0,0x00B3,0x00C7,0x00C9,0x00F8,0x00FA,
				0x00FD,0x00FE,0x0100,0x0103,0x0104,0x0106,
			    0x0109,0x0127,0x0129,0x012B,0x012D,0x012F,
			    0x0131,0x0132,0x0134,0x0135,0x0137,0x0139,
			    0x013B,0x014C,0x014E,0x014F,0x0151,0x0153,
				0x0155,0x0157,0x0158,0x015A,0x015D,0x015E,
			    0x015F,0x0162,0x01CF,0x01D1,0x01D4,0x01FF,
				0x0204,0x0206,0x0208,0x020A };
			return isFrame( id, SouthFrames, sizeof(SouthFrames) );
		}

		bool isNorthFrame( int id )
		{
			static int NorthFrames[] = {
				0x0006,0x0008,0x000D,0x001A,0x001B,0x0020,
				0x003A,0x0057,0x0059,0x005B,0x005D,0x0080,
				0x0081,0x0082,0x0084,0x0090,0x0091,0x0094,
                0x0096,0x0099,0x00A6,0x00A7,0x00AC,0x00AE,
				0x00B0,0x00C7,0x00C9,0x00F8,0x00FA,0x00FD,
                0x00FE,0x0100,0x0103,0x0104,0x0106,0x0109,
				0x0127,0x0129,0x012B,0x012D,0x012F,0x0131,
				0x0132,0x0134,0x0135,0x0137,0x0139,0x013B,
				0x014C,0x014E,0x014F,0x0151,0x0153,0x0155,
                0x0157,0x0158,0x015A,0x015D,0x015E,0x015F,
				0x0162,0x01CF,0x01D1,0x01D4,0x01FF,0x0201,
				0x0204,0x0208,0x020A };
			return isFrame( id, NorthFrames, sizeof( NorthFrames ) );
		}

		bool isEastFrame( int id )
		{
			static int EastFrames[] = {
				0x0007,0x000A,0x001A,0x001C,0x001E,0x0037,0x0058,
                0x0059,0x005C,0x005E,0x0080,0x0081,0x0082,0x0084,
				0x0090,0x0092,0x0095,0x0097,0x0098,0x00A6,0x00A8,
                0x00AB,0x00AE,0x00AF,0x00B2,0x00C7,0x00C8,0x00EA,
				0x00F8,0x00F9,0x00FC,0x00FE,0x00FF,0x0102,0x0104,
                0x0105,0x0108,0x0127,0x0128,0x012B,0x012C,0x012E,
                0x0130,0x0132,0x0133,0x0135,0x0136,0x0138,0x013A,
                0x014C,0x014D,0x014F,0x0150,0x0152,0x0154,0x0156,
                0x0158,0x0159,0x015C,0x015E,0x0160,0x0163,0x01CF,
                0x01D0,0x01D3,0x01FF,0x0203,0x0205,0x0207,0x0209 };
			return isFrame( id, EastFrames, sizeof( EastFrames ) );
		}

		bool isWestFrame( int id )
		{
			static int WestFrames[] = {
				0x0007,0x000C,0x001A,0x001C,0x0021,0x0039,0x0058,0x0059,
				0x005C,0x005E,0x0080,0x0081,0x0082,0x0084,0x0090,0x0092,
				0x0095,0x0097,0x0098,0x00A6,0x00A8,0x00AD,0x00AE,0x00AF,
				0x00B5,0x00C7,0x00C8,0x00EA,0x00F8,0x00F9,0x00FC,0x00FE,
				0x00FF,0x0102,0x0104,0x0105,0x0108,0x0127,0x0128,0x012C,
				0x012E,0x0130,0x0132,0x0133,0x0135,0x0136,0x0138,0x013A,
				0x014C,0x014D,0x014F,0x0150,0x0152,0x0154,0x0156,0x0158,
				0x0159,0x015C,0x015E,0x0160,0x0163,0x01CF,0x01D0,0x01D3,
				0x01FF,0x0200,0x0203,0x0207,0x0209 };
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
		    int doorTop = z + 20;

			if ( y == 1743 && x >= 1343 && x <= 1344 )
				return 0;
		    if ( y == 1679 && x >= 1392 && x <= 1393 )
				return 0;
		    if ( x == 1320 && y >= 1618 && y <= 1640 )
				return 0;
		    if ( x == 1383 && y >= 1642 && y <= 1643 )
				return 0;
			if ( !Maps::instance()->canFit( x, y, z, map, 16) )
				return 0;
			cItem* door = cItem::createFromScript( QString::number( 0x6A5 + 2*int(facing), 16 ) );
			door->moveTo( Coord_cl( x, y, z, map ), true );
			return door;
		}
		public:

		int generate ( int region[], int map, cUOSocket* socket )
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
						int z  = tiles.data().zoff;
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

    int BritRegions[][4] = {
		{  250,  750,  775, 1330 },
        {  525, 2095,  925, 2430 },
        { 1025, 2155, 1265, 2310 },
		{ 1635, 2430, 1705, 2508 },
		{ 1775, 2605, 2165, 2975 },
		{ 1055, 3520, 1570, 4075 },
		{ 2860, 3310, 3120, 3630 },
		{ 2470, 1855, 3950, 3045 },
		{ 3425,  990, 3900, 1455 },
		{ 4175,  735, 4840, 1600 },
		{ 2375,  330, 3100, 1045 },
		{ 2100, 1090, 2310, 1450 },
		{ 1495, 1400, 1550, 1475 },
		{ 1085, 1520, 1415, 1910 },
		{ 1410, 1500, 1745, 1795 },
		{ 5120, 2300, 6143, 4095 } };
	int IlshRegions[][4] = { { 0, 0, 288*8, 200*8 } };
	int MalasRegions[][4] = { { 0, 0, 320*8, 256*8 } };

	socket->sysMessage("Generating doors, please wait ( Slow )");
    int count = 0;
	if ( Maps::instance()->hasMap( 0 ) )
	{
		for ( int i = 0; i < 16; ++i )
		{
			socket->sysMessage(QString("doing [%1, %2, %3, %4]").arg(BritRegions[i][0]).arg(BritRegions[i][1]).arg(BritRegions[i][2]).arg(BritRegions[i][3]) );
			count += generator.generate( BritRegions[i], 0, socket );
			socket->sysMessage( tr("Doors so far: %1").arg(count) );
		}
	}
}

// Command Table (Keep this at the end)
stCommand cCommands::commands[] =
{
	{ "ACCOUNT", commandAccount },
	{ "ADDEVENT", commandAddEvent },
	{ "ALLMOVE", commandAllMove },
	{ "ALLSHOW", commandAllShow },
	{ "ALLSKILLS", commandAllSkills },
	{ "BROADCAST", commandBroadcast },
	{ "DOORGEN", commandDoorGenerator },
	{ "FIX", commandFix },
	{ "GMTALK", commandGmtalk },
	{ "INVIS", commandInvis },
	{ "KILL", commandKill },
	{ "MOVE", commandMove },
	{ "PAGES", commandPages },
	{ "PAGENOTIFY", commandPageNotify },
	{ "PASSWORD", commandPassword },
	{ "RELOAD", commandReload },
	{ "REMOVE", commandRemove },
	{ "REMOVEEVENT", commandRemoveEvent },
	{ "RESEND", commandResend },
	{ "RESTOCK", commandRestock },
	{ "RESURRECT", commandResurrect },
	{ "SAVE", commandSave },
	{ "SERVERTIME", commandServerTime },
	{ "SET", commandSet },
	{ "SHOW", commandShow },
	{ "SHUTDOWN", commandShutDown },
	{ "STAFF", commandStaff },
	{ "SPAWNREGION", commandSpawnRegion },
	{ "TAGS", commandTags },
	{ "TELE", commandTele },
	{ "WHO", commandWho },
	{ NULL, NULL }
};
