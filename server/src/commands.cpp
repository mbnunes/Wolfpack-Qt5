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

#include "globals.h"
#include "accounts.h"
#include "classes.h"
#include "commands.h"
#include "gumps.h"
#include "mapstuff.h"
#include "wpscriptmanager.h"
#include "network/uosocket.h"
#include "spawnregions.h"
#include "srvparams.h"
#include "targetrequests.h"
#include "territories.h"
#include "tilecache.h"
#include "worldmain.h"
#include "wpconsole.h"
#include "wpdefmanager.h"
#include "pagesystem.h"
#include "makemenus.h"

// System Includes
#include <functional>

// Main Command processing function
void cCommands::process( cUOSocket *socket, const QString &command )
{
	// TODO: Insert processing instructions for script-defined commands here

	if( !socket->player() )
		return;

	P_CHAR pChar = socket->player();
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

	for( UINT32 index = 0; commands[index].command; ++index )
		if( command == commands[index].name )
		{
			(commands[index].command)( socket, command, arguments );
			return;
		}

	socket->sysMessage( "Unknown Command" );
}

void cCommands::loadACLs( void )
{
	clConsole.PrepareProgress( "Loading Access Control Lists." );

	QStringList ScriptSections = DefManager->getSections( WPDT_PRIVLEVEL );
	
	if( ScriptSections.isEmpty() )
	{
		clConsole.ProgressFail();
		clConsole.ChangeColor( WPC_RED );
		clConsole.send( "WARNING: No ACLs for players, counselors, gms and admins defined!\n" );
		clConsole.ChangeColor( WPC_NORMAL );
		return;
	}

	QString groupName;

	// We are iterating trough a list of ACLs
	// In each loop we create one acl
	for( QStringList::iterator it = ScriptSections.begin(); it != ScriptSections.end(); ++it )
	{
		QDomElement *Tag = DefManager->getSection( WPDT_PRIVLEVEL, *it );

		if( Tag->isNull() )
			continue;

		QString ACLname = Tag->attribute("id");

		if ( ACLname == QString::null )
		{
			clConsole.ChangeColor( WPC_RED );
			clConsole.send( QString("WARNING: Tag %1 lacks \"id\" attribute").arg(Tag->tagName()) );
			clConsole.ChangeColor( WPC_NORMAL );
			continue;
		}
		
		// While we are in this loop we are building an ACL
		stAcl *acl = new stAcl;
		acl->name = ACLname;
		QMap< QString, bool > group;

		QDomElement n = Tag->firstChild().toElement();
		while( !n.isNull() )
		{
			if ( n.nodeName() == "group" )
			{
				groupName = n.attribute("name", QString::null);
				n = n.firstChild().toElement();
			} 
			else if ( n.nodeName() == "action" )
			{
				QString name = n.attribute( "name", "any" );
				bool permit = n.attribute( "permit", "false" ) == "true" ? true : false;
				group.insert( name, permit );
				n = n.nextSibling().toElement(); // Process next action
			}

			if ( n.isNull() && n.parentNode() != *Tag )
			{
				n = n.parentNode().nextSibling().toElement();
				acl->groups.insert( groupName, group );
				group.clear();
			}
		}

		_acls.insert( ACLname, acl );	
	}
	clConsole.ProgressDone();
}

// COMMAND IMPLEMENTATION
// Purpose:
// .go >> Gump with possible targets
// .go x,y,z,[map] >> Go to those coordinates
// .go placename >> Go to that specific place
void commandGo( cUOSocket *socket, const QString &command, QStringList &args )
{
	P_CHAR pChar = socket->player();

	if( !pChar )
		return;

	if( args.isEmpty() )
	{
		socket->sysMessage( "Bringin up travel gump" );
		return;
	}
	else
	{
		Coord_cl newPos = pChar->pos;
		for( QStringList::const_iterator i = args.begin(); i != args.end(); ++i )
			if( parseCoordinates( (*i), newPos ) )
			{
				// This is a bandwith saving method
				// Before we're moving the character anywhere we remove it
				// only from the sockets in range and then resend it to only the new sockets in range
				pChar->removeFromView( false );
				pChar->moveTo( newPos );
				pChar->resend( false );
				socket->resendPlayer();
				socket->resendWorld();
				return;
			}

		// When we reached this point it's clear that we didn't find any valid coordinates in our arguments
		QString location = args.join( " " );
		QDomElement *node = DefManager->getSection( WPDT_LOCATION, location );

		if( !node->isNull() && parseCoordinates( node->text(), newPos ) )
		{
			pChar->removeFromView( false );
			pChar->moveTo( newPos );
			pChar->resend( false );
			socket->resendPlayer();
			socket->resendWorld();
			return;
		}			
	}

	// If we reached this end it's definetly an invalid command
	socket->sysMessage( tr( "Usage: go [location|x,y,z,[map]]" ) );
}

class cResurectTarget: public cTargetRequest
{
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		P_CHAR pChar = FindCharBySerial( target->serial() );

		if( !pChar )
		{
			socket->sysMessage( tr( "This is not a living being." ) );
			return true;
		}

		Targ->NpcResurrectTarget( pChar );
		return true;
	}
};

void commandResurrect( cUOSocket *socket, const QString &command, QStringList &args )
{
	socket->sysMessage( tr( "Select the being you want to resurrect" ) );
	socket->attachTarget( new cResurectTarget );
}

void commandWhere( cUOSocket *socket, const QString &command, QStringList &args )
{
	P_CHAR pChar = socket->player();

	if( !pChar )
		return;

	cTerritory *mRegion = cAllTerritories::getInstance()->region( pChar->pos.x, pChar->pos.y );

	QString message = tr( "You are " );

	if( mRegion )
		message.append( tr( "in %1 " ).arg( mRegion->name() ) );

	message.append( tr( "at %1,%2,%3 on map %4" ).arg( pChar->pos.x ).arg( pChar->pos.y ).arg( pChar->pos.z ).arg( pChar->pos.map ) );
	pChar->message( message );
}



class cKillTarget: public cTargetRequest
{
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		if( !socket->player() )
			return true;

		P_CHAR pChar = FindCharBySerial( target->serial() );

		if( !pChar )
		{
			socket->sysMessage( tr( "You need to target a living being" ) );
			return true;
		}

		pChar->kill();
		return true;
	}
};

void commandKill( cUOSocket *socket, const QString &command, QStringList &args )
{
	socket->sysMessage( tr( "Please select a target to kill" ) );
	socket->attachTarget( new cKillTarget );
}

void commandFix( cUOSocket *socket, const QString &command, QStringList &args )
{
	// TODO: Eventually check if the character is stuck etc.
	socket->resendPlayer();
}

void commandAddItem( cUOSocket *socket, const QString &command, QStringList &args )
{
	QString param = args.join( " " ).stripWhiteSpace();

	QDomElement *node = DefManager->getSection( WPDT_ITEM, param );

	if( node && !node->isNull() )
	{
		socket->sysMessage( tr( "Where do you want to add the item '%1'" ).arg( param ) );
		socket->attachTarget( new cAddItemTarget( param ) );
	}

	return;
}

void commandAddNpc( cUOSocket *socket, const QString &command, QStringList &args )
{
	QString param = args.join( " " ).stripWhiteSpace();

	QDomElement *node = DefManager->getSection( WPDT_NPC, param );

	if( node && !node->isNull() )
	{
		socket->sysMessage( tr( "Where do you want to add the npc '%1'" ).arg( param ) );
		socket->attachTarget( new cAddNpcTarget( param ) );
	}

	return;
}

void commandAdd( cUOSocket *socket, const QString &command, QStringList &args )
{
	// Bring up the Add-menu
	if( args.count() < 1 )
	{
		cMakeMenu* pMakeMenu = cAllMakeMenus::getInstance()->getMenu( "ADD_MENU" );
		if( !pMakeMenu )
			socket->sysMessage( "Addmenu undefined!" );
		else
			socket->send( new cMakeMenuGump( pMakeMenu ) );
		return;
	}

	QString param = args.join( " " ).stripWhiteSpace();
    
	QDomElement *node = DefManager->getSection( WPDT_ITEM, param );

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

class cSetTarget: public cTargetRequest
{
private:
	QString key,value;
public:
	cSetTarget( const QString nKey, const QString nValue ) {
		key = nKey;
		value = nValue;
	}

	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		P_CHAR pChar = FindCharBySerial( target->serial() );
		P_ITEM pItem = FindItemBySerial( target->serial() );

		cUObject *pObject = NULL;

		if( pItem )
			pObject = pItem;
		else if( pChar )
			pObject = pChar;

		// Only characters and items
		if( !pObject )
		{
			socket->sysMessage( tr( "Please select a valid character or item" ) );
			return true;
		}

		// Object name
        if( key == "name" )
			pObject->name = value.latin1();

		// Object color
		else if( ( key == "color" ) || ( key == "skin" ) )
		{
			if( pItem )
				pItem->setColor( hex2dec( value ).toULong() );
			else if( pChar )
				pChar->setSkin( hex2dec( value ).toULong() );
		}

		// Events
		else if( key == "events" ) 
		{
			pObject->clearEvents();
			QStringList events = QStringList::split( ",", value );
			for( INT32 i = 0; i < events.count(); ++i )
			{
				WPDefaultScript *script = ScriptManager->find( events[i] );
				if( script )
					pObject->addEvent( script );
			}
		}

		// Object id
		else if( ( key == "id" ) || ( key == "body" ) || ( key == "model" ) )
		{
			if( pItem )
				pItem->setId( hex2dec( value ).toULong() );
			else if( pChar )
				pChar->setId( hex2dec( value ).toULong() );
		}

		// Object direction
		else if( key == "dir" )
		{
			if( pItem )
				pItem->dir = hex2dec( value ).toULong();
			else if( pChar )
				pChar->dir = hex2dec( value ).toULong();
		}

		// Object position
		else if( ( key == "pos" ) || ( key == "p" ) )
		{
			Coord_cl newCoords = pObject->pos;
			if( !parseCoordinates( value, newCoords ) )
			{
				socket->sysMessage( tr( "Invalid coordinates '%1'" ).arg( value ) );
				return true;
			}
			pObject->moveTo( newCoords );
		}
		
		// Char title
		else if( ( key == "title" ) && pChar )
			pChar->setTitle( value );

		// Item Amount
		else if( ( key == "amount" ) && pItem && ( hex2dec( value ).toULong() > 0 ) )
			pItem->setAmount( hex2dec( value ).toULong() );			

		// Str Dex Int
		else if( key == "str" )
			if( pChar )
			{
				pChar->st = hex2dec( value ).toInt();
				for( UINT8 i = 0; i < ALLSKILLS; ++i )
					Skills->updateSkillLevel( pChar, i );
			}
			else
				pItem->st = hex2dec( value ).toInt();

		else if( key == "dex" )
			if( pChar )
			{
				pChar->setDex( hex2dec( value ).toInt() );
				for( UINT8 i = 0; i < ALLSKILLS; ++i )
					Skills->updateSkillLevel( pChar, i );
			}
			else
				pItem->dx = hex2dec( value ).toInt();

		else if( key == "int" )
			if( pChar )
			{
				pChar->in = hex2dec( value ).toInt();
				for( UINT8 i = 0; i < ALLSKILLS; ++i )
					Skills->updateSkillLevel( pChar, i );
			}
			else
				pItem->in = hex2dec( value ).toInt();

		// NPC Wander
		else if( key == "npcwander" && pChar )
			pChar->npcWander = hex2dec( value ).toInt();

		// NPC AI Type
		else if( key == "npcaitype" && pChar )
			pChar->setNpcAIType( hex2dec( value ).toInt() );

		// Health + Stamina + Mana
		else if( key == "hp" || key == "health" || key == "hitpoints" )
		{
			if( pChar )
				pChar->hp = hex2dec( value ).toInt();
			else 
				pItem->setHp( hex2dec( value ).toInt() );
		}

		else if( key == "stamina" && pChar )
			pChar->stm = hex2dec( value ).toInt();

		else if( key == "mana" && pChar )
			pChar->mn = hex2dec( value ).toInt();

		else if( key == "morex" && pItem )
			pItem->morex = hex2dec( value ).toInt();

		else if( key == "morey" && pItem )
			pItem->morey = hex2dec( value ).toInt();

		else if( key == "morez" && pItem )
			pItem->morez = hex2dec( value ).toInt();

		// Object tags
		else if( key.left( 4 ) == "tag." )
		{
			QString tagName = key.right( key.length() - 4 );
			bool ok = false;
			hex2dec( value ).toInt( &ok );

			if( !ok )
				pObject->tags.set( tagName, cVariant( value ) );
			else
				pObject->tags.set( tagName, cVariant( hex2dec( value ).toInt() ) );
		}
		// Unknown Tag
		else
		{
			bool found = false;

			if( pChar )
			for( UINT8 i = 0; i < ALLSKILLS; ++i )
				if( key.upper() == skillname[i] )
				{
					pChar->setBaseSkill( i, hex2dec( value ).toInt() );
					Skills->updateSkillLevel( pChar, i );
					socket->sendSkill( i );
					found = true;
					break;
				}


			if( !found )
			{
				socket->sysMessage( tr( "Unknown key '%1'" ).arg( key ) );
				return true;
			}
		}

		if( pChar )
			pChar->resend();
		else if( pItem )
			pItem->update();
		return true;
	}
};

void commandSet( cUOSocket *socket, const QString &command, QStringList &args )
{
	if( args.size() < 2 )
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

void commandResend( cUOSocket *socket, const QString &command, QStringList &args )
{
	socket->resendPlayer();
	socket->resendWorld();
}

class cRemoveTarget: public cTargetRequest
{
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		P_CHAR pChar = FindCharBySerial( target->serial() );
		P_ITEM pItem = FindItemBySerial( target->serial() );

		if( pChar )
		{
			if( pChar->socket() )
			{
				socket->sysMessage( "You cannot delete logged in characters" );
				return true;
			}

			if( pChar->account() )
				pChar->account()->removeCharacter( pChar );
			Npcs->DeleteChar( pChar );
		}
		else if( pItem )
		{
			Items->DeleItem( pItem );
		}
		else
			socket->sysMessage( "You need to select either an item or a character" );
		return true;
	}
};

void commandRemove( cUOSocket *socket, const QString &command, QStringList &args )
{
	socket->attachTarget( new cRemoveTarget );
}

void commandAccount( cUOSocket *socket, const QString &command, QStringList &args )
{
	// Account Create User Pass
	// Account Remove User
	// Account Password User Pass
	// Account Show User Pass
	if( args.count() == 0 )
	{
		socket->sysMessage( tr( "Usage: account <create|remove|password|show>" ) );
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
		else if( Accounts->getRecord( args[1].left( 30 ) ) )
		{
			socket->sysMessage( tr( "Account '%1' already exists" ).arg( args[1].left( 30 ) ) );
		}
		else
		{
			Accounts->createAccount( args[1].left( 30 ), args[2].left( 30 ) );
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
		else if( !Accounts->getRecord( args[1].left( 30 ) ) )
		{
			socket->sysMessage( tr( "Account '%1' does not exist" ).arg( args[1].left( 30 ) ) );
		}
		else
		{
			AccountRecord *account = Accounts->getRecord( args[1].left( 30 ) );
			QValueVector<cChar*> characters = account->caracterList();
			Accounts->remove( account );
			UINT32 i = 0;
			for(; i < characters.size(); ++i )
				if( characters[i] )
					Npcs->DeleteChar( characters[i] );
			
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
		else if( !Accounts->getRecord( args[1].left( 30 ) ) )
		{
				socket->sysMessage( tr( "Account '%1' does not exist" ).arg( args[1].left( 30 ) ) ); 
		}
		else
		{
			AccountRecord *account = Accounts->getRecord( args[1].left( 30 ) );
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
					account->block();
					socket->sysMessage( tr( "Account '%1' has been blocked" ).arg( account->login() ) );
				}
				else if( value.lower() == "off" )
				{
					account->unBlock();
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
			socket->sysMessage( tr( "Usage: account set <username> <key>" ) );
		}
		else if( !Accounts->getRecord( args[1].left( 30 ) ) )
		{
			socket->sysMessage( tr( "Account '%1' does not exist" ).arg( args[1].left( 30 ) ) );
		}
		else
		{
			AccountRecord *account = Accounts->getRecord( args[1].left( 30 ) );
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
				QValueVector< P_CHAR > pCharList = account->caracterList();

				for( UINT32 i = 0; i < pCharList.size(); ++i )
					if( pCharList[i] )
						sCharList.push_back( QString( "0x%1" ).arg( pCharList[i]->serial, 8, 16 ) );

				socket->sysMessage( tr( "Account '%1' has the following characters: %2" ).arg( account->login() ).arg( sCharList.join( ", " ) ) );
			}
			else
			{
				socket->sysMessage( tr( "Unknown field '%1' for account '%2'" ).arg( args[2] ).arg( account->login() ) );
			}
		}
	}
}

class cTeleTarget: public cTargetRequest
{
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		// This is a GM command so we do not check anything but send the 
		// char where he wants to move
		if( !socket->player() )
			return true;
	
		socket->player()->removeFromView( false );

		Coord_cl newPos = socket->player()->pos;
		newPos.x = target->x();
		newPos.y = target->y();
		newPos.z = target->z();
		socket->player()->moveTo( newPos );

		socket->player()->resend( false );
		socket->resendWorld();
        socket->resendPlayer();
		return true;
	}
};

void commandTele( cUOSocket *socket, const QString &command, QStringList &args )
{
	socket->attachTarget( new cTeleTarget );
}

void commandSave( cUOSocket *socket, const QString &command, QStringList &args )
{
	if( args.count() > 0 )
		cwmWorldState->savenewworld( args[0] );
	else
		cwmWorldState->savenewworld( SrvParams->worldSaveModule() );
}

#define FLAG_STUB( a, b, c ) if( tile.a & b ) flags.push_back( tr( c ) )

QStringList getFlagNames( const tile_st &tile )
{
	QStringList flags;

	// Flag 1
	FLAG_STUB( flag1, 0x01, "background" );
	FLAG_STUB( flag1, 0x02, "weapon" );
	FLAG_STUB( flag1, 0x04, "transparent" );
	FLAG_STUB( flag1, 0x08, "translucent" );
	FLAG_STUB( flag1, 0x10, "wall" );
	FLAG_STUB( flag1, 0x20, "damaging" );
	FLAG_STUB( flag1, 0x40, "impassable" );
	FLAG_STUB( flag1, 0x80, "wet" );

	// Flag 2
	//FLAG_STUB( flag2, 0x01, "unknown1" ); 
	FLAG_STUB( flag2, 0x02, "surface" );
	FLAG_STUB( flag2, 0x04, "stairs" );
	FLAG_STUB( flag2, 0x08, "stackable" );
	FLAG_STUB( flag2, 0x10, "window" );
	FLAG_STUB( flag2, 0x20, "no shoot" );
	FLAG_STUB( flag2, 0x40, "a" );
	FLAG_STUB( flag2, 0x80, "an" );

	// Flag 3
	FLAG_STUB( flag3, 0x01, "internal" ); 
	FLAG_STUB( flag3, 0x02, "foliage" );
	FLAG_STUB( flag3, 0x04, "partial hue" );
	//FLAG_STUB( flag3, 0x08, "unknown2" );
	FLAG_STUB( flag3, 0x10, "map" );
	FLAG_STUB( flag3, 0x20, "container" );
	FLAG_STUB( flag3, 0x40, "wearable" );
	FLAG_STUB( flag3, 0x80, "lightsource" );

	// Flag 4
	FLAG_STUB( flag4, 0x01, "animation" ); 
	FLAG_STUB( flag4, 0x02, "no diagonal" );
	//FLAG_STUB( flag4, 0x04, "unknown3" );
	FLAG_STUB( flag4, 0x08, "armor" );
	FLAG_STUB( flag4, 0x10, "roof" );
	FLAG_STUB( flag4, 0x20, "door" );
	FLAG_STUB( flag4, 0x40, "stair back" );
	FLAG_STUB( flag4, 0x80, "stair right" );

	return flags;
}

class cInfoTarget: public cTargetRequest
{
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		if( !socket->player() )
			return true;

		Coord_cl pos = socket->player()->pos;
		pos.x = target->x();
		pos.y = target->y();
		pos.z = target->z();

		// Map Target
		if( !target->model() && !target->serial() )
		{
			map_st mapTile = Map->SeekMap( pos );
			land_st lTile = cTileCache::instance()->getLand( mapTile.id );
			
			// Display a gump with this information
			cGump* pGump = new cGump();

			// Basic .INFO Header
			pGump->addResizeGump( 0, 40, 0xA28, 450, 250 ); //Background
			pGump->addGump( 105, 18, 0x58B ); // Fancy top-bar
			pGump->addGump( 182, 0, 0x589 ); // "Button" like gump
			pGump->addTilePic( 202, 23, 0x14eb ); // Type of info menu

            pGump->addText( 175, 90, tr( "Landscape Info" ), 0x530 );

			// Give information about the tile
			pGump->addText( 50, 120, tr( "Name: %1" ).arg( lTile.name ), 0x834 );
			pGump->addText( 50, 145, tr( "ID: 0x%1" ).arg( mapTile.id, 0, 16 ), 0x834 );
			pGump->addText( 50, 170, tr( "Z Height: %1" ).arg( mapTile.z ), 0x834 );

			// Wet ? Impassable ? At least these are the most interesting
			QStringList flags;

			if( lTile.flag1&0x80 )
				flags.push_back( tr( "wet" ) );

			if( lTile.flag1&0x40 )
				flags.push_back( tr( "impassable" ) );

			if( lTile.flag2&0x02 )
				flags.push_back( tr( "surface" ) );

			if( lTile.flag2&0x04 )
				flags.push_back( tr( "stairs" ) );

			pGump->addText( 50, 195, tr( "Properties: %1" ).arg( flags.join( ", " ) ), 0x834 );

			// OK button
			pGump->addButton( 50, 240, 0xF9, 0xF8, 0 ); // Only Exit possible

			socket->send( pGump );
		}
		// Static Tiles
		else if( target->model() && !target->serial() )
		{
			tile_st sTile = cTileCache::instance()->getTile( target->model() );

			// Display a gump with this information
			cGump* pGump = new cGump();

			// Basic .INFO Header
			pGump->addResizeGump( 0, 40, 0xA28, 450, 300 ); //Background
			pGump->addGump( 105, 18, 0x58B ); // Fancy top-bar
			pGump->addGump( 182, 0, 0x589 ); // "Button" like gump
			pGump->addTilePic( 202, 23, 0x14EF ); // Display our tile

            pGump->addText( 175, 90, tr( "Static Info" ), 0x530 );
			
			// Give information about the tile
			pGump->addText( 50, 120, tr( "Name: %1" ).arg( sTile.name ), 0x834 );
			pGump->addText( 50, 140, tr( "ID: 0x%1" ).arg( target->model(), 0, 16 ), 0x834 );
			pGump->addText( 50, 160, tr( "Position: %1,%2,%3" ).arg( target->x() ).arg( target->y() ).arg( target->z() ), 0x834 );
			pGump->addText( 50, 180, tr( "Weight: %1" ).arg( (UINT8)sTile.weight ), 0x834 );
			pGump->addText( 50, 200, tr( "Height: %1" ).arg( (UINT8)sTile.height ), 0x834 );

			// Wet ? Impassable ? At least these are the most interesting
			QStringList flags = getFlagNames( sTile );

			pGump->addText( 50, 220, tr( "Properties: %1" ).arg( flags.join( ", " ) ), 0x834 );

			// OK button
			pGump->addButton( 50, 275, 0xF9, 0xF8, 0 ); // Only Exit possible

			// Item Preview
			pGump->addResizeGump( 300, 120, 0xBB8, 110, 150 );
			pGump->addTilePic( 340, 160 - ( sTile.height / 2 ), target->model() );

			socket->send( pGump );
		}
		
		// dynamic objects
		else if( target->serial() )
		{
			if( isCharSerial( target->serial() ) )
			{
				P_CHAR pChar = FindCharBySerial( target->serial() );
				if( pChar )
				{
					cCharInfoGump* pGump = new cCharInfoGump( pChar );
					socket->send( pGump );
				}
			}
			else if( isItemSerial( target->serial() ) )
			{
				P_ITEM pItem = FindItemBySerial( target->serial() );
				if( pItem )
				{
					cItemInfoGump* pGump = new cItemInfoGump( pItem );
					socket->send( pGump );
				}
			}
		}
		return true;
	}
};

void commandInfo( cUOSocket *socket, const QString &command, QStringList &args )
{
	socket->sysMessage( tr( "Please select a target" ) );
	socket->attachTarget( new cInfoTarget );
}

class cShowTarget: public cTargetRequest
{
private:
	QString key;
public:
	cShowTarget( const QString _key ) { key = _key; }
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		// Check for a valid target
		cUObject *pObject;
		P_ITEM pItem = FindItemBySerial( target->serial() );
		P_CHAR pChar = FindCharBySerial( target->serial() );
		if( !pChar && !pItem )
			return true;

		if( pChar )
			pObject = pChar;
		else
			pObject = pItem;

		QString result;

		if( key == "name" )
			if( pItem )
				result = pItem->name();
			else
				result = pChar->name.c_str();
		
		else if( ( key == "title" ) )
			result = pChar->title();

		else if( ( key == "str" ) )
			if( pChar )
				result = QString( "%1" ).arg( pChar->st );
			else 
				result = QString( "%1" ).arg( pItem->st );

		else if( ( key == "dex" ) )
			if( pChar )
				result = QString( "%1" ).arg( pChar->effDex() );
			else 
				result = QString( "%1" ).arg( pItem->dx );

		else if( ( key == "int" ) )
			if( pChar )
				result = QString( "%1" ).arg( pChar->in );
			else 
				result = QString( "%1" ).arg( pItem->in );

		else if( key == "morex" && pItem )
			result = QString( "%1" ).arg( pItem->morex );

		else if( key == "morey" && pItem )
			result = QString( "%1" ).arg( pItem->morey );

		else if( key == "morez" && pItem )
			result = QString( "%1" ).arg( pItem->morez );

		else if( key == "pos" )
			result = QString( "%1,%2,%3,%4" ).arg( pObject->pos.x ).arg( pObject->pos.y ).arg( pObject->pos.z ).arg( pObject->pos.map );

		else if( key == "color" )
		{
			if( pItem )
				result = QString( "0x%1" ).arg( pItem->color(), 0, 16 );
			else if( pChar )
				result = QString( "0x%1" ).arg( pChar->skin(), 0, 16 );
		}

		else if( key == "skin" )
		{
			if( pChar )
				result = QString( "0x%1" ).arg( pChar->skin(), 0, 16 );
		}

		else if( key == "npcwander" && pChar )
			result = QString( "%1" ).arg( pChar->npcWander );

		else if( key == "npcaitype" && pChar )
			result = QString( "%1" ).arg( pChar->npcaitype() );

		else if( key == "hp" || key == "health" || key == "hitpoints" )
		{
			if( pChar )
				result = QString( "%1" ).arg( pChar->hp );
			else 
				result = QString( "%1" ).arg( pItem->hp() );
		}

		else if( key == "stamina" && pChar )
			result = QString( "%1" ).arg( pChar->stm );

		else if( key == "mana" && pChar )
			result = QString( "%1" ).arg( pChar->mn );

		// Check if key was a skillname
		else 
		{
			bool found = false;

			if( pChar )
			for( UINT8 i = 0; i < ALLSKILLS; ++i )
				if( key.upper() == skillname[i] )
				{
					result = QString( "%1" ).arg( pChar->baseSkill( i ) );
					found = true;
					break;
				}

			if( !found )
			{
				socket->sysMessage( tr( "Unknown key '%1'" ).arg( key ) );
				return true;
			}
		}

		socket->sysMessage( tr( "'%1' is '%2'" ).arg( key ).arg( result ) );
		return true;
	}
};

void commandShow( cUOSocket *socket, const QString &command, QStringList &args )
{
	socket->sysMessage( tr( "Please select a target" ) );
	socket->attachTarget( new cShowTarget( args.join( " " ) ) );
}

class cBankTarget: public cTargetRequest
{
private:
	UINT8 layer;
public:
	cBankTarget( UINT8 data ) { layer = data; }

	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		P_CHAR pChar = FindCharBySerial( target->serial() );
		if( !pChar )
		{
			socket->sysMessage( tr( "This does not appear to be a living being." ) );
			return true;
		}

		P_ITEM pItem = pChar->GetItemOnLayer( layer );

		if( !pItem )
		{
			socket->sysMessage( tr( "This being does not have a container on layer 0x%1" ).arg( layer, 2, 16 ) );
			return true;
		}

		socket->sendContainer( pItem );
		return true;
	}
};

void commandBank( cUOSocket *socket, const QString &command, QStringList &args )
{
	socket->sysMessage( tr( "Please chose the owner of the container you want to open" ) );

	bool ok = false;
	UINT8 layer = hex2dec( args.join( " " ) ).toUShort( &ok );

	if( !ok || !layer )
		layer = 0x1D; // Bank layer	

	socket->attachTarget( new cBankTarget( layer ) );
}

void commandAction( cUOSocket *socket, const QString &command, QStringList &args )
{
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

void commandSpawnRegion( cUOSocket *socket, const QString &command, QStringList &args )
{
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
			cSpawnRegion* spawnRegion = cAllSpawnRegions::getInstance()->region( args[1] );
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
			cAllSpawnRegions::getInstance()->reSpawn();
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
			cSpawnRegion* spawnRegion = cAllSpawnRegions::getInstance()->region( args[1] );
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
			cAllSpawnRegions::getInstance()->deSpawn();
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
			cSpawnRegion* spawnRegion = cAllSpawnRegions::getInstance()->region( args[1] );
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
			cAllSpawnRegions::getInstance()->reSpawnToMax();
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
			cSpawnRegion* spawnRegion = cAllSpawnRegions::getInstance()->region( args[1] );
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
			pGump->addText( 50, 120, tr( "Spawnregions: %1" ).arg( cAllSpawnRegions::getInstance()->size() ), 0x834 );
			pGump->addText( 50, 140, tr( "NPCs: %1 of %2" ).arg( cAllSpawnRegions::getInstance()->npcs() ).arg( cAllSpawnRegions::getInstance()->maxNpcs() ), 0x834 );
			pGump->addText( 50, 160, tr( "Items: %1 of %2" ).arg( cAllSpawnRegions::getInstance()->items() ).arg( cAllSpawnRegions::getInstance()->maxItems() ), 0x834 );
			
			// OK button
			pGump->addButton( 50, 200, 0xF9, 0xF8, 0 ); // Only Exit possible

			socket->send( pGump );
		}
	}
}

class cSetTagTarget: public cTargetRequest
{
private:
	UINT8 type_;
	QString key_;
	QString value_;
public:
	cSetTagTarget( QString key, QString value, UINT8 type ) 
	{ 
		type_	= type; 
		key_	= key;
		value_	= value;
	}

	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		if( isCharSerial( target->serial() ) )
		{
			P_CHAR pChar = FindCharBySerial( target->serial() );
			if( pChar )
			{
				if( type_ )
					pChar->tags.set( key_, cVariant( value_.toInt() ) );
				else
					pChar->tags.set( key_, cVariant( value_ ) );
			}
			return true;
		}
		else if( isItemSerial( target->serial() ) )
		{
			P_ITEM pItem = FindItemBySerial( target->serial() );
			if( pItem )
			{
				if( type_ )
					pItem->tags.set( key_, cVariant( value_.toInt() ) );
				else
					pItem->tags.set( key_, cVariant( value_ ) );
			}
			return true;
		}
		return false;
	}
};

class cGetTagTarget: public cTargetRequest
{
private:
	QString key_;
public:
	cGetTagTarget( QString key ) 
	{ 
		key_	= key;
	}

	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		if( isCharSerial( target->serial() ) )
		{
			P_CHAR pChar = FindCharBySerial( target->serial() );
			if( pChar )
			{
				socket->sysMessage( tr("Tag \"%1\" has value \"%2\".").arg( key_ ).arg( pChar->tags.get( key_ ).asString() ) );
			}
			return true;
		}
		else if( isItemSerial( target->serial() ) )
		{
			P_ITEM pItem = FindItemBySerial( target->serial() );
			if( pItem )
			{
				socket->sysMessage( tr("Tag \"%1\" has value \"%2\".").arg( key_ ).arg( pItem->tags.get( key_ ).asString() ) );
			}
			return true;
		}
		return false;
	}
};

class cRemoveTagTarget: public cTargetRequest
{
private:
	QString key_;
public:
	cRemoveTagTarget( QString key ) 
	{ 
		key_	= key;
	}

	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		if( isCharSerial( target->serial() ) )
		{
			P_CHAR pChar = FindCharBySerial( target->serial() );
			if( pChar )
			{
				if( key_.lower() == "all" )
				{
					QStringList keys = pChar->tags.getKeys();
					QStringList::const_iterator it = keys.begin();
					while( it != keys.end() )
					{
						pChar->tags.remove( (*it) );
						it++;
					}
					socket->sysMessage( tr("All tags removed.") );
				}
				else
				{
					pChar->tags.remove( key_ );
					socket->sysMessage( tr("Tag \"%1\" removed.").arg( key_ ) );
				}
			}
			return true;
		}
		else if( isItemSerial( target->serial() ) )
		{
			P_ITEM pItem = FindItemBySerial( target->serial() );
			if( pItem )
			{
				if( key_.lower() == "all" )
				{
					QStringList keys = pItem->tags.getKeys();
					QStringList::const_iterator it = keys.begin();
					while( it != keys.end() )
					{
						pItem->tags.remove( (*it) );
						it++;
					}
					socket->sysMessage( tr("All tags removed.") );
				}
				else
				{
					pItem->tags.remove( key_ );
					socket->sysMessage( tr("Tag \"%1\" removed.").arg( key_ ) );
				}
			}
			return true;
		}
		return false;
	}
};

class cTagsInfoTarget: public cTargetRequest
{
public:
	cTagsInfoTarget() {}

	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		if( isCharSerial( target->serial() ) )
		{
			P_CHAR pChar = FindCharBySerial( target->serial() );
			if( pChar )
			{
				cTagsInfoGump* pGump = new cTagsInfoGump( pChar );
				socket->send( pGump );
			}
			return true;
		}
		else if( isItemSerial( target->serial() ) )
		{
			P_ITEM pItem = FindItemBySerial( target->serial() );
			if( pItem )
			{
				cTagsInfoGump* pGump = new cTagsInfoGump( pItem );
				socket->send( pGump );
			}
			return true;
		}
		return false;
	}
};

void commandTags( cUOSocket *socket, const QString &command, QStringList &args )
{
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

void commandWho( cUOSocket *socket, const QString &command, QStringList &args )
{
	// Who
	cWhoMenuGump* pGump = new cWhoMenuGump( 1 );
	socket->send( pGump );
}

void commandPages( cUOSocket *socket, const QString &command, QStringList &args )
{
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

// Command Table (Keep this at the end)
stCommand cCommands::commands[] =
{
	{ "ACCOUNT",		commandAccount },
	{ "ACTION",			commandAction },
	{ "ADD",			commandAdd },
	{ "ADDITEM",		commandAddItem },
	{ "ADDNPC",			commandAddNpc },
	{ "BANK",			commandBank },
	{ "FIX",			commandFix },
	{ "GO",				commandGo },
	{ "INFO",			commandInfo },
	{ "KILL",			commandKill },
	{ "PAGES",			commandPages },
	{ "REMOVE",			commandRemove },
	{ "RESEND",			commandResend },
	{ "RESURRECT",		commandResurrect },
	{ "SAVE",			commandSave },
	{ "SET",			commandSet },
	{ "SHOW",			commandShow },
	{ "SPAWNREGION",	commandSpawnRegion },
	{ "TAGS",			commandTags },
	{ "TELE",			commandTele },
	{ "WHERE",			commandWhere },
	{ "WHO",			commandWho },
	{ NULL, NULL }
};

