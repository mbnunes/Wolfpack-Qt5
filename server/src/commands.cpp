//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
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

#include "globals.h"
#include "accounts.h"
#include "skills.h"
#include "commands.h"
#include "gumps.h"
#include "maps.h"
#include "wpscriptmanager.h"
#include "network/uosocket.h"
#include "spawnregions.h"
#include "srvparams.h"
#include "targetrequests.h"
#include "territories.h"
#include "tilecache.h"
#include "chars.h"
#include "wpconsole.h"
#include "wpdefmanager.h"
#include "wpscriptmanager.h"
#include "pagesystem.h"
#include "makemenus.h"
#include "mapobjects.h"
#include "resources.h"
#include "contextmenu.h"
#include "newmagic.h"
#include "spellbook.h"

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
	if( !pChar->account()->authorized("command", pCommand ))
	{
		socket->sysMessage( tr( "Access to command '%1' was denied" ).arg( pCommand.lower() ) );
		return;
	}

	// Dispatch the command
	dispatch( socket, pCommand, pArgs );
}

// Selects the right command Stub
void cCommands::dispatch( cUOSocket *socket, const QString &command, QStringList &arguments )
{
	// Just in case we have been called directly
	if( !socket || !socket->player() )
		return;

	// Check for custom commands
	WPDefaultScript *script = ScriptManager->getCommandHook( command );

	if( script )
	{
		QString argString = arguments.join( " " );
		if( argString.isNull() )
			argString = "";

		script->onCommand( socket, command, argString );
		return;
	}	

	for( UINT32 index = 0; commands[index].command; ++index )
		if( command == commands[index].name )
		{
			(commands[index].command)( socket, command, arguments );
			return;
		}

	socket->sysMessage( tr("Unknown Command") );
}

void cCommands::loadACLs( void )
{
	// make sure it's clean
	QMap< QString, cAcl* >::iterator itA (_acls.begin());
	for ( ; itA != _acls.end(); ++itA )
		delete itA.data();
	_acls.clear();

	QStringList ScriptSections = DefManager->getSections( WPDT_PRIVLEVEL );
	
	if( ScriptSections.isEmpty() )
	{
		clConsole.ChangeColor( WPC_RED );
		clConsole.send( tr("WARNING: No ACLs for players, counselors, gms and admins defined!\n") );
		clConsole.ChangeColor( WPC_NORMAL );
		return;
	}

	// We are iterating trough a list of ACLs
	// In each loop we create one acl
	for( QStringList::iterator it = ScriptSections.begin(); it != ScriptSections.end(); ++it )
	{
		const QDomElement *Tag = DefManager->getSection( WPDT_PRIVLEVEL, *it );

		if( Tag->isNull() )
			continue;

		QString ACLname = Tag->attribute("id");

		if ( ACLname == QString::null )
		{
			clConsole.ChangeColor( WPC_RED );
			clConsole.send( tr("WARNING: Tag %1 lacks \"id\" attribute").arg(Tag->tagName()) );
			clConsole.ChangeColor( WPC_NORMAL );
			continue;
		}
		
		// While we are in this loop we are building an ACL
		cAcl *acl = new cAcl;
		acl->name = ACLname;
		QMap< QString, bool > group;
		QString groupName;

		QDomNode childNode = Tag->firstChild();
		while( !childNode.isNull() )
		{
			if( childNode.isElement() )
			{
				QDomElement childTag = childNode.toElement();
				if( childTag.nodeName() == "group" )
				{
					groupName = childTag.attribute("name", QString::null);
					QDomNode chchildNode = childTag.firstChild();
					while( !chchildNode.isNull() )
					{
						if( chchildNode.isElement() )
						{
							QDomElement chchildTag = chchildNode.toElement();
							if( chchildTag.nodeName() == "action" )
							{
								QString name = chchildTag.attribute( "name", "any" );
								bool permit = chchildTag.attribute( "permit", "false" ) == "true" ? true : false;
								group.insert( name, permit );
							}
						}
						chchildNode = chchildNode.nextSibling();
					}

					if( !group.isEmpty() )
					{
						acl->groups.insert( groupName, group );
						group.clear();
					}
				}
			}
			childNode = childNode.nextSibling();			
		}

		_acls.insert( ACLname, acl );	
	}
	DefManager->unload( WPDT_PRIVLEVEL );
}

// COMMAND IMPLEMENTATION
// Purpose:
// .go >> Gump with possible targets
// .go x,y,z,[map] >> Go to those coordinates
// .go placename >> Go to that specific place
void commandGo( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(command);
	P_CHAR pChar = socket->player();

	if( !pChar )
		return;

	UI08 map = pChar->pos().map;

	if( args.isEmpty() )
	{
		socket->sysMessage( "Bringin up travel gump" );
		return;
	}
	else
	{

		Coord_cl newPos = pChar->pos();
		QString argument = args.join(" ");
		if( parseCoordinates( argument, newPos ) )
		{
			// This is a bandwith saving method
			// Before we're moving the character anywhere we remove it
			// only from the sockets in range and then resend it to only the new sockets in range
			
			pChar->removeFromView( false );
			pChar->moveTo( newPos );
			pChar->resend( false, true );
			socket->resendPlayer( map == pChar->pos().map );
			socket->resendWorld();
			return;
		}

		// When we reached this point it's clear that we didn't find any valid coordinates in our arguments
		const QDomElement *node = DefManager->getSection( WPDT_LOCATION, argument );

		if( !node->isNull() && parseCoordinates( node->text(), newPos ) )
		{
			pChar->removeFromView( false );
			pChar->moveTo( newPos );
			pChar->resend( false, true );
			socket->resendPlayer( map == pChar->pos().map );
			socket->resendWorld();
			return;
		}			
	}

	// If we reached this end it's definetly an invalid command
	socket->sysMessage( tr( "Usage: go [location|x,y,z,[map]]" ) );
}

void commandResurrect( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(args);
	Q_UNUSED(command);
	socket->sysMessage( tr( "Select the being you want to resurrect" ) );
	socket->attachTarget( new cResurectTarget );
}

void commandWhere( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(args);
	Q_UNUSED(command);
	P_PLAYER pChar = socket->player();

	if( !pChar )
		return;

	cTerritory *mRegion = AllTerritories::instance()->region( pChar->pos().x, pChar->pos().y, pChar->pos().map );

	QString message = tr( "You are" );

	if( mRegion )
		message.append( " " + tr( "in %1" ).arg( mRegion->name() ) );

	message.append( " " + tr( "at %1,%2,%3 on map %4" ).arg( pChar->pos().x ).arg( pChar->pos().y ).arg( pChar->pos().z ).arg( pChar->pos().map ) );
	pChar->message( message );
}

void commandKill( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(args);
	Q_UNUSED(command);
	socket->sysMessage( tr( "Please select a target to kill" ) );
	socket->attachTarget( new cKillTarget );
}

void commandFix( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(args);
	Q_UNUSED(command);
	// TODO: Eventually check if the character is stuck etc.
	socket->resendPlayer();
}

void commandAddItem( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(command);
	QString param = args.join( " " ).stripWhiteSpace();

	const QDomElement *node = DefManager->getSection( WPDT_ITEM, param );

	if( node && !node->isNull() )
	{
		socket->sysMessage( tr( "Where do you want to add the item '%1'" ).arg( param ) );
		socket->attachTarget( new cAddItemTarget( param ) );
	}

	return;
}

void commandStatic( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(command);
	QString param = args.join( " " ).stripWhiteSpace();

	const QDomElement *node = DefManager->getSection( WPDT_ITEM, param );

	if( node && !node->isNull() )
	{
		socket->sysMessage( tr( "Where do you want to add the item '%1'" ).arg( param ) );
		socket->attachTarget( new cAddItemTarget( param ) );
	}

	return;
}

void commandAddNpc( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(command);
	QString param = args.join( " " ).stripWhiteSpace();

	const QDomElement *node = DefManager->getSection( WPDT_NPC, param );

	if( node && !node->isNull() )
	{
		socket->sysMessage( tr( "Where do you want to add the npc '%1'" ).arg( param ) );
		socket->attachTarget( new cAddNpcTarget( param ) );
	}

	return;
}

void commandAdd( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(command);
	// Bring up the Add-menu
	if( args.count() < 1 )
	{
		MakeMenus::instance()->callMakeMenu( socket, "ADD_MENU" );
		return;
	}

	QString param = args.join( " " ).stripWhiteSpace();
    
	const QDomElement *node = DefManager->getSection( WPDT_ITEM, param );

	// An item definition with that name exists
	if( node && !node->isNull() )
	{
		socket->sysMessage( tr( "Where do you want to add the item '%1'" ).arg( param ) );
		socket->attachTarget( new cAddItemTarget( param ) );
		return;
	}

	node = DefManager->getSection( WPDT_NPC, param );
	
	// Same for NPCs
	if( node && !node->isNull() )
	{
		socket->sysMessage( tr( "Where do you want to add the npc '%1'" ).arg( param ) );
		socket->attachTarget( new cAddNpcTarget( param ) );
		return;
	}
	
	socket->sysMessage( tr( "Item or NPC Definition '%1' not found" ).arg( param ) );
	return;
}

void commandSet( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(command);
	if( args.size() < 1 )
	{
		socket->sysMessage( "Usage: set <key> <value>" );
		return;
	}	

    QString key = args[0];
	args.erase( args.begin() );
    QString value = args.join( " " );

	socket->sysMessage( tr( "Please select a target to 'set %1 %2' " ).arg( key ).arg( value ) );
	socket->attachTarget( new cSetTarget( key, value ) );
}

void commandResend( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(args);
	Q_UNUSED(command);
	socket->resendPlayer( false );
	socket->resendWorld();
}


void commandRemove( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(socket);
	Q_UNUSED(command);
	Q_UNUSED(args);
	socket->attachTarget( new cRemoveTarget );
}

void commandAccount( cUOSocket *socket, const QString &command, QStringList &args ) throw()
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
			AccountRecord *account = Accounts::instance()->getRecord( args[1].left( 30 ) );
			QValueVector<P_PLAYER> characters = account->caracterList();
			Accounts::instance()->remove( account );
			UINT32 i = 0;
			for(; i < characters.size(); ++i )
				if( characters[i] )
					cCharStuff::DeleteChar( characters[i] );
			
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
			AccountRecord *account = Accounts::instance()->getRecord( args[1].left( 30 ) );
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
				if( !cCommands::instance()->getACL( value ) )
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
			AccountRecord *account = Accounts::instance()->getRecord( args[1].left( 30 ) );
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


void commandTele( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(socket);
	Q_UNUSED(command);
	Q_UNUSED(args);
	socket->attachTarget( new cTeleTarget );
}

void commandSave( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(socket);
	Q_UNUSED(command);
	World::instance()->save();
}

void commandInfo( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{ 
	Q_UNUSED(command);
	SERIAL serial = args.size() > 0 ? args[0].toUInt() : INVALID_SERIAL;
	if( serial != INVALID_SERIAL )
	{
		if( isCharSerial( serial ) )
		{
			P_CHAR pc = FindCharBySerial( serial );
			if( pc )
			{
				cCharInfoGump* pGump = new cCharInfoGump( pc );
				socket->send( pGump );
			}
			else
				socket->sysMessage( tr("The given serial is invalid!") );
		}
		else
		{
			P_ITEM pi = FindItemBySerial( serial );
			if( pi )
			{
				cItemInfoGump* pGump = new cItemInfoGump( pi );
				socket->send( pGump );
			}
			else
				socket->sysMessage( tr("The given serial is invalid!") );
		}
	}
	else
	{
		socket->sysMessage( tr( "Please select a target" ) );
		socket->attachTarget( new cInfoTarget );
	}
}

void commandServerTime( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(command);
	socket->sysMessage( tr( "Server time: %1" ).arg( uiCurrentTime ) );
}

void commandShow( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(command);
	socket->sysMessage( tr( "Please select a target" ) );
	socket->attachTarget( new cShowTarget( args.join( " " ) ) );
}

void commandBank( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(command);
	socket->sysMessage( tr( "Please chose the owner of the container you want to open" ) );

	bool ok = false;
	UINT8 layer = hex2dec( args.join( " " ) ).toUShort( &ok );

	if( !ok || !layer )
		layer = 0x1D; // Bank layer	

	socket->attachTarget( new cBankTarget( layer ) );
}

void commandAction( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(command);
	bool ok = false;
	UINT32 action = hex2dec( args.join( " " ) ).toInt( &ok );

	if( ok )
	{
		if( socket->player() )
		{
			socket->player()->action( action );
		}
	}
	else
	{
		socket->sysMessage( tr( "Invalid parameter: '%1'" ).arg( args.join( " " ) ) );
	}
}

void commandSpawnRegion( cUOSocket *socket, const QString &command, QStringList &args ) throw()
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

void commandTags( cUOSocket *socket, const QString &command, QStringList &args ) throw()
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

void commandWho( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(args);
	Q_UNUSED(command);
	// Who
	cWhoMenuGump* pGump = new cWhoMenuGump( 1 );
	socket->send( pGump );
}

void commandPages( cUOSocket *socket, const QString &command, QStringList &args ) throw()
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

void commandShutDown( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(socket);
	Q_UNUSED(command);
	// Shutdown
	// Shutdown x (x in seconds)
	if( args.count() == 0 )
		keeprun = 0;
}

void commandReload( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(command);
	// Reload accounts
	// Reload scripts
	// Reload all

	if( args.count() == 0 )
	{
		socket->sysMessage( tr( "Usage: reload <accounts|scripts|all>" ) );
		return;
	}

	QString subCommand = args[0].lower();

	// accounts
	if( subCommand == "accounts" )
	{
		Accounts::instance()->reload();
		socket->sysMessage( tr("Accounts reloaded") );
	}
	if( subCommand == "python" )
	{
		clConsole.send( "Reloading python scripts\n" );
		ScriptManager->reload();
		NewMagic->load();
		ContextMenus::instance()->reload();
	}
	if( subCommand == "scripts" )
	{
		clConsole.send( "Reloading definitions, scripts and wolfpack.xml\n" );

		SrvParams->reload(); // Reload wolfpack.xml
		DefManager->reload(); //Reload Definitions
		Accounts::instance()->reload();
		SpawnRegions::instance()->reload();
		AllTerritories::instance()->reload();
		Resources::instance()->reload();
		MakeMenus::instance()->reload();
		ScriptManager->reload(); // Reload Scripts
		NewMagic->load();
		Skills->reload();
		ContextMenus::instance()->reload();

		// Update the Regions
		cCharIterator iter;
		P_CHAR pChar;
		for( pChar = iter.first(); pChar; pChar = iter.next() )
		{
			cTerritory *region = AllTerritories::instance()->region( pChar->pos().x, pChar->pos().y, pChar->pos().map );
			pChar->setRegion( region );
		}

		cNetwork::instance()->reload(); // This will be integrated into the normal definition system soon
		socket->sysMessage( tr("Definitions, scripts and wolfpack.xml reloaded") );
	}
	if( subCommand == "all" )
	{		
		clConsole.send( "Reloading definitions, scripts and wolfpack.xml\n" );

		SrvParams->reload(); // Reload wolfpack.xml
		DefManager->reload(); //Reload Definitions
		Accounts::instance()->reload();
		SpawnRegions::instance()->reload();
		AllTerritories::instance()->reload();
		Resources::instance()->reload();
		MakeMenus::instance()->reload();
		ScriptManager->reload(); // Reload Scripts
		ContextMenus::instance()->reload();
		NewMagic->load();

		// Update the Regions
		cCharIterator iter;
		P_CHAR pChar;
		for( pChar = iter.first(); pChar; pChar = iter.next() )
		{
			cTerritory *region = AllTerritories::instance()->region( pChar->pos().x, pChar->pos().y, pChar->pos().map );
			pChar->setRegion( region );
		}

		cNetwork::instance()->reload(); // This will be integrated into the normal definition system soon
		socket->sysMessage( tr("Accounts, definitions, scripts and wolfpack.xml reloaded") );
	}
}

void commandMakeMenu( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(command);
	// Makemenu <menusection>

	if( args.count() == 0 )
	{
		socket->sysMessage( tr( "Usage: makemenu <menusection>" ) );
		return;
	}

	MakeMenus::instance()->callMakeMenu( socket, args[0] );
}

void commandAddSpell( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(command);
	bool ok = false;
	UINT8 spell = 0;

	if( args.count() > 0 )
		spell = args[0].toUInt( &ok );

	// Get the spellid
	if( !ok )
	{
		socket->sysMessage( tr( "Usage: addspell <spell-id>" ) );
		return;
	}
	

	socket->sysMessage( tr( "Please select the spellbook you want to add this spell to." ) );
	socket->attachTarget( new cModifySpellbook( spell, false ) );
}

void commandRemoveSpell( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(command);
	bool ok = false;
	UINT8 spell = 0;

	if( args.count() > 0 )
		spell = args[0].toUInt( &ok );

	// Get the spellid
	if( !ok )
	{
		socket->sysMessage( tr( "Usage: removespell <spell-id>" ) );
		return;
	}
	

	socket->sysMessage( tr( "Please select the spellbook you want to remove this spell from." ) );
	socket->attachTarget( new cModifySpellbook( spell, true ) );
}


void commandAddEvent( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(command);
	if( args.size() < 1 )
	{
		socket->sysMessage( "Usage: addevent <identifier>" );
		return;
	}	

    QString event = args.join( " " );

	// No such event
	if( !ScriptManager->find( event ) )
	{
		socket->sysMessage( tr( "Invalid event: '%1'" ).arg( event ) );
		return;
	}

	socket->sysMessage( tr( "Please select a target to add event '%1' to." ).arg( event ) );
	socket->attachTarget( new cAddEventTarget( event ) );
}

void commandRemoveEvent( cUOSocket *socket, const QString &command, QStringList &args ) throw()
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

void commandMove( cUOSocket *socket, const QString &command, QStringList &args ) throw()
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

void commandNuke( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(args);
	Q_UNUSED(command);
	// Nuke does not take any parameters (At least for now it does not.)
	socket->sysMessage( tr( "Please select first corner of the box you want to nuke." ) );
	socket->attachTarget( new cNukeTarget );
}

void commandTile( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(command);
	if( args.count() < 2 )
	{
		socket->sysMessage( "Usage: tile <z> <id>[,<idn>]" );
		return;
	}

	INT8 z = args[0].toInt();
	QStringList ids = QStringList::split( ",", args[1] );

	// Check if the given ids are valid
	for( QStringList::iterator iter = ids.begin(); iter != ids.end(); ++iter )
	{
		if( !DefManager->getSection( WPDT_ITEM, (*iter) ) )
		{
			socket->sysMessage( tr( "Item definition '%1' is undefined." ).arg( *iter ) );
			return;
		}
	}
	
	socket->sysMessage( tr( "Please select the first corner." ) );
	socket->attachTarget( new cTileTarget( z, ids ) );
}

void commandAllShow( cUOSocket *socket, const QString &command, QStringList &args ) throw()
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

void commandAllMove( cUOSocket *socket, const QString &command, QStringList &args ) throw()
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
		socket->sysMessage( tr( "AllMove = '1'" ) );
	else
		socket->sysMessage( tr( "AllMove = '0'" ) );

	// Resend the world to us
	socket->resendWorld( true );
}

void commandShowSerials( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(command);
	if( !socket->player() || !socket->player()->account() )
		return;

	// Switch
	if( !args.count() )
		socket->player()->account()->setShowSerials( !socket->player()->account()->isShowSerials() );
	// Set
	else 
		socket->player()->account()->setShowSerials( args[0].toInt() != 0 );

	if( socket->player()->account()->isShowSerials() )
		socket->sysMessage( tr( "ShowSerials = '1'" ) );
	else
		socket->sysMessage( tr( "ShowSerials = '0'" ) );	
}

void commandRestock( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(args);
	Q_UNUSED(command);
	socket->sysMessage( tr( "Please select the vendor you want to restock." ) );
	socket->attachTarget( new cRestockTarget );
}

void commandAllSkills( cUOSocket *socket, const QString &command, QStringList &args ) throw()
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

void commandBroadcast( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(socket);
	Q_UNUSED(command);
	cNetwork::instance()->broadcast( args.join( " " ).latin1() );
}

void commandInvis( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(command);
	socket->player()->removeFromView();
	
	if( socket->player()->hidden() || ( args.count() > 0 && args[0].toInt() == 0 ) )
	{
		socket->player()->setHidden( 0 );
		socket->player()->setPriv2( socket->player()->priv2() & 0xF7 );
		socket->sysMessage( tr( "Invisible is now '0'." ) );
	}
	else if( !socket->player()->hidden() || ( args.count() > 0 && args[0].toInt() == 1 ) )
	{
		socket->player()->setHidden( 1 );
		socket->player()->setPriv2( socket->player()->priv2() | 0x08 );
		socket->sysMessage( tr( "Invisible is now '1'." ) );
	}

	socket->player()->resend( false, false );
}

void commandPageNotify( cUOSocket *socket, const QString &command, QStringList &args ) throw()
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

// Change password for current account
void commandPassword( cUOSocket *socket, const QString &command, QStringList &args ) throw()
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

void commandGmtalk( cUOSocket *socket, const QString &command, QStringList &args ) throw()
{
	Q_UNUSED(command);
	if( args.count() < 1 )
	{
		socket->sysMessage( tr( "Usage: gmtalk <message>" ) );
		return;
	}

	QString message = "<" + socket->player()->name() + ">: " + args.join( " " );

	cUOSocket *mSock = 0;
	for( mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		if( mSock->player() && mSock->player()->isGM() )
			mSock->sysMessage( message, 0x539 );
	}
}

// Command Table (Keep this at the end)
stCommand cCommands::commands[] =
{
	{ "ACCOUNT",		commandAccount },
	{ "ACTION",			commandAction },
	{ "ADD",			commandAdd },
	{ "ADDEVENT",		commandAddEvent },
	{ "ADDITEM",		commandAddItem },
	{ "ADDNPC",			commandAddNpc },
	{ "ADDSPELL",		commandAddSpell },
	{ "ALLMOVE",		commandAllMove },
	{ "ALLSHOW",		commandAllShow },
	{ "ALLSKILLS",		commandAllSkills },
	{ "BANK",			commandBank },
	{ "BROADCAST",		commandBroadcast },
	{ "FIX",			commandFix },
	{ "GO",				commandGo },
	{ "GMTALK",			commandGmtalk },
	{ "INFO",			commandInfo },
	{ "INVIS",			commandInvis },
	{ "KILL",			commandKill },
	{ "MAKEMENU",		commandMakeMenu },
	{ "MOVE",			commandMove },
	{ "NUKE",			commandNuke },
	{ "PAGES",			commandPages },
	{ "PAGENOTIFY",		commandPageNotify },
	{ "PASSWORD",		commandPassword },
	{ "RELOAD",			commandReload },
	{ "REMOVE",			commandRemove },
	{ "REMOVEEVENT",	commandRemoveEvent },
	{ "REMOVESPELL",	commandRemoveSpell },
	{ "RESEND",			commandResend },
	{ "RESTOCK",		commandRestock },
	{ "RESURRECT",		commandResurrect },
	{ "SAVE",			commandSave },
	{ "SERVERTIME",		commandServerTime },
	{ "SET",			commandSet },
	{ "SHOW",			commandShow },
	{ "SHOWSERIALS",	commandShowSerials },
	{ "SHUTDOWN",		commandShutDown },
	{ "SPAWNREGION",	commandSpawnRegion },
	{ "STATIC",			commandStatic },
	{ "TAGS",			commandTags },
	{ "TELE",			commandTele },
	{ "TILE",			commandTile },	
	{ "WHERE",			commandWhere },
	{ "WHO",			commandWho },
	{ NULL, NULL }
};

