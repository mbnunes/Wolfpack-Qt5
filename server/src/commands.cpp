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
#include "worldmain.h"
#include "wpconsole.h"
#include "wpdefmanager.h"
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
	clConsole.PrepareProgress( "Loading Access Control Lists" );

	QStringList ScriptSections = DefManager->getSections( WPDT_PRIVLEVEL );
	
	if( ScriptSections.isEmpty() )
	{
		clConsole.ProgressFail();
		clConsole.ChangeColor( WPC_RED );
		clConsole.send( "WARNING: No ACLs for players, counselors, gms and admins defined!\n" );
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
			clConsole.send( QString("WARNING: Tag %1 lacks \"id\" attribute").arg(Tag->tagName()) );
			clConsole.ChangeColor( WPC_NORMAL );
			continue;
		}
		
		// While we are in this loop we are building an ACL
		stAcl *acl = new stAcl;
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
		QString argument = args.join(" ");
		if( parseCoordinates( argument, newPos ) )
		{
			// This is a bandwith saving method
			// Before we're moving the character anywhere we remove it
			// only from the sockets in range and then resend it to only the new sockets in range
			if( newPos.map != pChar->pos.map )
			{
				cUOTxChangeMap changemap;
				changemap.setMap( newPos.map );
				socket->send( &changemap );
			}
			
			pChar->removeFromView( false );
			pChar->moveTo( newPos );
			pChar->resend( false );
			socket->resendWorld();
			return;
		}

		// When we reached this point it's clear that we didn't find any valid coordinates in our arguments
		const QDomElement *node = DefManager->getSection( WPDT_LOCATION, argument );

		if( !node->isNull() && parseCoordinates( node->text(), newPos ) )
		{
			if( newPos.map != pChar->pos.map )
			{
				cUOTxChangeMap changemap;
				changemap.setMap( newPos.map );
				socket->send( &changemap );
			}

			pChar->removeFromView( false );
			pChar->moveTo( newPos );
			pChar->resend( false );
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

	const QDomElement *node = DefManager->getSection( WPDT_ITEM, param );

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

	const QDomElement *node = DefManager->getSection( WPDT_NPC, param );

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
        if( key == "name" && pItem )
			pItem->setName( value );
		else if( key == "name" && pChar )
			pChar->name = value;

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
			QStringList::const_iterator it = events.begin();
			while( it != events.end() )
			{
				WPDefaultScript *script = ScriptManager->find( *(it++) );
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
				pChar->setDir( hex2dec( value ).toULong() );
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
			pObject->removeFromView();
			pObject->moveTo( newCoords );

			if( pChar )
				pChar->resend( false );
			else if( pItem );
				pItem->update();
		}
		
		// Char title
		else if( ( key == "title" ) && pChar )
			pChar->setTitle( value );

		// Item Amount
		else if( ( key == "amount" ) && pItem && ( hex2dec( value ).toULong() > 0 ) )
			pItem->setAmount( hex2dec( value ).toULong() );			

		// LoDamage + HiDamage
		else if( key == "lodamage" )
		{
			if( pItem )
				pItem->setLodamage( hex2dec( value ).toInt() );
			else if( pChar )
				pChar->setLoDamage( hex2dec( value ).toInt() );
		}

		else if( key == "hidamage" )
		{
			if( pItem )
				pItem->setHidamage( hex2dec( value ).toInt() );
			else if( pChar )
				pChar->setHiDamage( hex2dec( value ).toInt() );
		}

		// Str Dex Int
		else if( key == "str" )
			if( pChar )
			{
				pChar->setSt( hex2dec( value ).toInt() );
				for( UINT8 i = 0; i < ALLSKILLS; ++i )
					Skills->updateSkillLevel( pChar, i );
			}
			else
				pItem->setSt( hex2dec( value ).toInt() );

		else if( key == "dex" )
			if( pChar )
			{
				pChar->setDex( hex2dec( value ).toInt() );
				for( UINT8 i = 0; i < ALLSKILLS; ++i )
					Skills->updateSkillLevel( pChar, i );
			}
			else
				pItem->setDx( hex2dec( value ).toInt() );

		else if( key == "int" )
			if( pChar )
			{
				pChar->setIn( hex2dec( value ).toInt() );
				for( UINT8 i = 0; i < ALLSKILLS; ++i )
					Skills->updateSkillLevel( pChar, i );
			}
			else
				pItem->setIn( hex2dec( value ).toInt() );

		// Type
		else if( key == "type" && pItem )
			pItem->setType( hex2dec( value ).toInt() );

		// NPC Wander
		else if( key == "npcwander" && pChar )
			pChar->setNpcWander( hex2dec( value ).toInt() );

		// NPC AI Type
		else if( key == "npcaitype" && pChar )
			pChar->setNpcAIType( hex2dec( value ).toInt() );

		// Health + Stamina + Mana
		else if( key == "hp" || key == "health" || key == "hitpoints" )
		{
			if( pChar )
				pChar->setHp( hex2dec( value ).toInt() );
			else 
				pItem->setHp( hex2dec( value ).toInt() );
		}

		else if( key == "stamina" && pChar )
			pChar->setStm( hex2dec( value ).toInt() );

		else if( key == "mana" && pChar )
			pChar->setMn( hex2dec( value ).toInt() );

		else if( key == "movable" && pItem )
			pItem->setMagic( hex2dec( value ).toInt() );

		else if( key == "more1" && pItem )
			pItem->setMore1( hex2dec( value ).toInt() );

		else if( key == "more2" && pItem )
			pItem->setMore2( hex2dec( value ).toInt() );

		else if( key == "more3" && pItem )
			pItem->setMore3( hex2dec( value ).toInt() );

		else if( key == "more4" && pItem )
			pItem->setMore4( hex2dec( value ).toInt() );

		else if( key == "morex" && pItem )
			pItem->setMoreX( hex2dec( value ).toInt() );

		else if( key == "morey" && pItem )
			pItem->setMoreY( hex2dec( value ).toInt() );

		else if( key == "morez" && pItem )
			pItem->setMoreZ( hex2dec( value ).toInt() );

		else if( key == "map" && pChar )
		{
			pChar->removeFromView();
			pChar->moveTo( Coord_cl( pChar->pos.x, pChar->pos.y, pChar->pos.z, value.toInt() ) );
			pChar->resend( false );

			if ( pChar->socket() )
				pChar->socket()->resendWorld();
		}

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

void commandResend( cUOSocket *socket, const QString &command, QStringList &args )
{
	socket->resendPlayer( false );
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
			cCharStuff::DeleteChar( pChar );
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
			QValueVector<cChar*> characters = account->caracterList();
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
		cwmWorldState->savenewworld( SrvParams->getString( "General", "SaveModule", "xml" ) );
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
			map_st mapTile = Map->seekMap( pos );
			land_st lTile = TileCache::instance()->getLand( mapTile.id );
			
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
			tile_st sTile = TileCache::instance()->getTile( target->model() );

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
				result = pChar->name;
		
		else if( ( key == "title" ) )
			result = pChar->title();

		else if( ( key == "str" ) )
			if( pChar )
				result = QString( "%1" ).arg( pChar->st() );
			else 
				result = QString( "%1" ).arg( pItem->st() );

		else if( ( key == "dex" ) )
			if( pChar )
				result = QString( "%1" ).arg( pChar->effDex() );
			else 
				result = QString( "%1" ).arg( pItem->dx() );

		else if( ( key == "int" ) )
			if( pChar )
				result = QString( "%1" ).arg( pChar->in() );
			else 
				result = QString( "%1" ).arg( pItem->in() );

		else if( key == "more1" && pItem )
			result = QString( "%1" ).arg( pItem->more1() );

		else if( key == "more2" && pItem )
			result = QString( "%1" ).arg( pItem->more2() );

		else if( key == "more3" && pItem )
			result = QString( "%1" ).arg( pItem->more3() );

		else if( key == "more4" && pItem )
			result = QString( "%1" ).arg( pItem->more4() );

		else if( key == "morex" && pItem )
			result = QString( "%1" ).arg( pItem->morex() );

		else if( key == "morey" && pItem )
			result = QString( "%1" ).arg( pItem->morey() );

		else if( key == "morez" && pItem )
			result = QString( "%1" ).arg( pItem->morez() );

		else if( key == "visible" && pItem )
			result = QString( "%1" ).arg( pItem->visible );

		else if( key == "movable" && pItem )
			result = QString( "%1" ).arg( pItem->magic() );

		else if( key == "type" && pItem )
			result = QString( "%1" ).arg( pItem->type() );

		else if( key == "lodamage" )
		{
			if( pChar )
				result = QString( "%1" ).arg( pChar->lodamage() );
			else if( pItem )
				result = QString( "%1" ).arg( pItem->lodamage() );
		}

		else if( key == "hidamage" )
		{
			if( pChar )
				result = QString( "%1" ).arg( pChar->hidamage() );
			else if( pItem )
				result = QString( "%1" ).arg( pItem->hidamage() );
		}

		else if( key == "pos" || key == "p" )
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

		else if( key == "id" || key == "model" || key == "body" )
		{
			if( pItem )
				result = QString( "0x%1" ).arg( pItem->id(), 0, 16 );
			else if( pChar )
				result = QString( "0x%1" ).arg( pChar->id(), 0, 16 );
		}

		else if( key == "dir")
		{
			if( pItem )
				result = QString( "%1" ).arg( pItem->dir );
			else if( pChar )
				result = QString( "%1" ).arg( pChar->dir() );
		}

		else if( key == "events" ) 
		{
			QStringList events = QStringList::split( ",", pObject->eventList() );
			QStringList::const_iterator it = events.begin();
			while( it != events.end() )
			{
				result = QString( "%1\n" ).arg( *it );
				++it;
			}
		}

		else if( key == "amount" && pItem )
		{
			result = QString( "%1" ).arg( pItem->amount() );
		}

		else if( key == "npcwander" && pChar )
			result = QString( "%1" ).arg( pChar->npcWander() );

		else if( key == "npcaitype" && pChar )
			result = QString( "%1" ).arg( pChar->npcaitype() );

		else if( key == "hp" || key == "health" || key == "hitpoints" )
		{
			if( pChar )
				result = QString( "%1" ).arg( pChar->hp() );
			else 
				result = QString( "%1" ).arg( pItem->hp() );
		}

		else if( key == "stamina" && pChar )
			result = QString( "%1" ).arg( pChar->stm() );

		else if( key == "mana" && pChar )
			result = QString( "%1" ).arg( pChar->mn() );

		else if( key == "restock" && pItem )
			result = QString::number( pItem->restock );

		else if( key == "value" && pItem )
			result = QString::number( pItem->value );

		else if( key.left( 4 ) == "tag." )
		{
			QString tagName = key.right( key.length() - 4 );

			cVariant value = pObject->tags.get( tagName );

			if( !value.isValid() )
			{
				socket->sysMessage( tr( "Unknown tag '%1'" ).arg( tagName ) );
				return true;
			}

			result = value.toString();
		}

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

void commandShutDown( cUOSocket *socket, const QString &command, QStringList &args )
{
	// Shutdown
	// Shutdown x (x in seconds)
	if( args.count() == 0 )
		keeprun = 0;
}

void commandReload( cUOSocket *socket, const QString &command, QStringList &args )
{
	// Reload accounts
	// Reload scripts
	// Reload all

	if( args.count() == 0 )
	{
		socket->sysMessage( tr( "Usage: reload <accounts|scripts|all>" ) );
		return;
	}

	QString subCommand = args[0].lower();
	AllCharsIterator iter;

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
		SpawnRegions::instance()->reload();
		cAllTerritories::getInstance()->reload();
		Resources::instance()->reload();
		MakeMenus::instance()->reload();

		cCommands::instance()->loadACLs();

		ScriptManager->reload(); // Reload Scripts
		NewMagic->load();
		ContextMenus::instance()->reload();

		// Update the Regions
		for( iter.Begin(); !iter.atEnd(); iter++ )
		{
			P_CHAR pChar = iter.GetData();

			if( pChar )
			{
				cTerritory *region = cAllTerritories::getInstance()->region( pChar->pos.x, pChar->pos.y );
				pChar->setRegion( region );
			}
		}

		cNetwork::instance()->reload(); // This will be integrated into the normal definition system soon
		socket->sysMessage( tr("Definitions, scripts and wolfpack.xml reloaded") );
	}
	if( subCommand == "all" )
	{
		Accounts::instance()->reload();
		clConsole.send( "Reloading definitions, scripts and wolfpack.xml\n" );

		SrvParams->reload(); // Reload wolfpack.xml
		DefManager->reload(); //Reload Definitions
		SpawnRegions::instance()->reload();
		cAllTerritories::getInstance()->reload();
		Resources::instance()->reload();
		MakeMenus::instance()->reload();
		cCommands::instance()->loadACLs();

		ScriptManager->reload(); // Reload Scripts
		ContextMenus::instance()->reload();
		NewMagic->load();

		// Update the Regions
		for( iter.Begin(); !iter.atEnd(); iter++ )
		{
			P_CHAR pChar = iter.GetData();

			if( pChar )
			{
				cTerritory *region = cAllTerritories::getInstance()->region( pChar->pos.x, pChar->pos.y );
				pChar->setRegion( region );
			}
		}

		cNetwork::instance()->reload(); // This will be integrated into the normal definition system soon
		socket->sysMessage( tr("Accounts, definitions, scripts and wolfpack.xml reloaded") );
	}
}

void commandMakeMenu( cUOSocket *socket, const QString &command, QStringList &args )
{
	// Makemenu <menusection>

	if( args.count() == 0 )
	{
		socket->sysMessage( tr( "Usage: makemenu <menusection>" ) );
		return;
	}

	MakeMenus::instance()->callMakeMenu( socket, args[0] );
}

class cModifySpellbook: public cTargetRequest
{
private:
	UINT8 spell;
	bool deleteMode;
public:
	cModifySpellbook( UINT8 _spell, bool _deleteMode = false )
	{
		deleteMode = _deleteMode;
		spell = _spell;
	}

	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		if( !socket->player() )
			return true;

		// Check if we really targetted a spellbook
		if( isItemSerial( target->serial() ) )
		{
			P_ITEM pItem = FindItemBySerial( target->serial() );

			if( pItem )
			{
				cSpellBook *pBook = dynamic_cast< cSpellBook* >( pItem );

				if( pBook )
				{
					if( spell >= 64 )
					{
						socket->sysMessage( tr( "The spell id you specified is invalid." ) );
					}
					else
					{
						if( deleteMode )
						{
							pBook->removeSpell( spell );
							socket->sysMessage( tr( "You removed spell %1 from this spellbook." ).arg( spell ) );
						}
						else
						{
							pBook->addSpell( spell );
							socket->sysMessage( tr( "You added spell %1 to this spellbook." ).arg( spell ) );
						}
					}
				}
				else
				socket->sysMessage( tr( "This is not a valid spellbook." ) );
			}
			else
				socket->sysMessage( tr( "This is not a valid spellbook." ) );
		}
		else
			socket->sysMessage( tr( "This is not a valid spellbook." ) );

		return true;
	}

	virtual void canceled( cUOSocket *socket ) { Q_UNUSED(socket); }
};

void commandAddSpell( cUOSocket *socket, const QString &command, QStringList &args )
{
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

void commandRemoveSpell( cUOSocket *socket, const QString &command, QStringList &args )
{
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

class cAddEventTarget: public cTargetRequest
{
private:
	QString _event;
public:
	cAddEventTarget( const QString &event ): _event( event ) {}
	
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		cUObject *pObject = 0;
		
		if( isCharSerial( target->serial() ) )
			pObject = FindCharBySerial( target->serial() );
		else if( isItemSerial( target->serial() ) )
			pObject = FindItemBySerial( target->serial() );

		// We have to have a valid target
		if( !pObject )
		{
			socket->sysMessage( tr( "You have to target a character or an item." ) );
			return true;
		}

		// Check if we already have the event
		if( pObject->hasEvent( _event ) )
		{
			socket->sysMessage( tr( "This object already has the event '%1'" ).arg( _event ) );
			return true;
		}

		WPDefaultScript *script = ScriptManager->find( _event );

		if( !script )
		{
			socket->sysMessage( tr( "Invalid event: '%1'" ).arg( _event ) );
			return true;
		}

		pObject->addEvent( script );
		return true;
	}
};

void commandAddEvent( cUOSocket *socket, const QString &command, QStringList &args )
{
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

class cRemoveEventTarget: public cTargetRequest
{
private:
	QString _event;
public:
	cRemoveEventTarget( const QString &event ): _event( event ) {}
	
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		cUObject *pObject = 0;
		
		if( isCharSerial( target->serial() ) )
			pObject = FindCharBySerial( target->serial() );
		else if( isItemSerial( target->serial() ) )
			pObject = FindItemBySerial( target->serial() );

		// We have to have a valid target
		if( !pObject )
		{
			socket->sysMessage( tr( "You have to target a character or an item." ) );
			return true;
		}

		// Check if we already have the event
		if( !pObject->hasEvent( _event ) )
		{
			socket->sysMessage( tr( "This object doesn't have the event '%1'" ).arg( _event ) );
			return true;
		}

		pObject->removeEvent( _event );
		return true;
	}
};

void commandRemoveEvent( cUOSocket *socket, const QString &command, QStringList &args )
{
	if( args.size() < 1 )
	{
		socket->sysMessage( "Usage: removeevent <identifier>" );
		return;
	}	

    QString event = args.join( " " );

	socket->sysMessage( tr( "Please select a target to remove event '%1' from." ).arg( event ) );
	socket->attachTarget( new cRemoveEventTarget( event ) );
}

class cMoveTarget: public cTargetRequest
{
private:
	INT16 x,y,z;
public:
	cMoveTarget( INT16 _x, INT16 _y, INT8 _z ): x( _x ), y( _y ), z( _z ) {}
	
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		cUObject *pObject = 0;
		
		if( isCharSerial( target->serial() ) )
			pObject = FindCharBySerial( target->serial() );
		else if( isItemSerial( target->serial() ) )
			pObject = FindItemBySerial( target->serial() );

		// We have to have a valid target
		if( !pObject )
		{
			socket->sysMessage( tr( "You have to target a character or an item." ) );
			return true;
		}

		// Move the object relatively
		pObject->removeFromView();
		Coord_cl newPos = pObject->pos + Coord_cl( x, y, z );
		pObject->moveTo( newPos );
		
		if( pObject->isChar() )
		{
			P_CHAR pChar = dynamic_cast< P_CHAR >( pObject );
			if( pChar )
				pChar->resend();
		}
		else if( pObject->isItem() )
		{
			P_ITEM pItem = dynamic_cast< P_ITEM >( pObject );
			if( pItem )
				pItem->update();
		}

		return true;
	}
};

void commandMove( cUOSocket *socket, const QString &command, QStringList &args )
{
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

class cNukeTarget: public cTargetRequest
{
private:
	INT16 x1, y1;
public:
	cNukeTarget( INT16 _x1 = -1, INT16 _y1 = -1 ): x1( _x1 ), y1( _y1 ) {}

	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		// Set the first corner and readd ourself
		if( x1 == -1 || y1 == -1 )
		{
			x1 = target->x();
			y1 = target->y();
			socket->sysMessage( tr( "Please select the second corner of the box you want to nuke." ) );
			return false;
		}
		else
		{
			INT16 x2;
			if( target->x() < x1 )
			{
				x2 = x1;
				x1 = target->x();
			}
			else
				x2 = target->x();

			INT16 y2;
			if( target->y() < y1 )
			{
				y2 = y1;
				y1 = target->y();
			}
			else
				y2 = target->y();

			socket->sysMessage( tr( "Nuking from %1,%2 to %3,%4" ).arg( x1 ).arg( y1 ).arg( x2 ).arg( y2 ) );
			UINT16 dCount = 0;
			vector< P_ITEM > toDelete;

			// This could eventually be optimized
			AllItemsIterator iter;
			for( iter.Begin(); !iter.atEnd(); ++iter )
			{
				P_ITEM pItem = iter.GetData();

				if( pItem && pItem->isInWorld() && pItem->pos.x >= x1 && pItem->pos.x <= x2 && pItem->pos.y >= y1 && pItem->pos.y <= y2 )
				{
					// Delete the item
					toDelete.push_back( pItem );
					dCount++;
				}
			}

			for( vector< P_ITEM >::iterator sIter = toDelete.begin(); sIter != toDelete.end(); ++sIter )
				Items->DeleItem( (*sIter) );

			socket->sysMessage( tr( "Deleted %1 items." ).arg( dCount ) );
			return true;
		}		
	}
};

void commandNuke( cUOSocket *socket, const QString &command, QStringList &args )
{
	// Nuke does not take any parameters (At least for now it does not.)
	socket->sysMessage( tr( "Please select first corner of the box you want to nuke." ) );
	socket->attachTarget( new cNukeTarget );
}

class cTileTarget: public cTargetRequest
{
private:
	INT16 x1, y1;
	INT8 z;
	QStringList ids;
public:
	cTileTarget( INT8 _z, const QStringList &_ids ): x1( -1 ), y1( -1 ), z( _z ), ids( _ids ) {}

	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		if( !socket->player() )
			return true;

		// Set the first corner and readd ourself
		if( x1 == -1 || y1 == -1 )
		{
			x1 = target->x();
			y1 = target->y();
			socket->sysMessage( tr( "Please select the second corner." ) );
			return false;
		}
		else
		{
			INT16 x2;
			if( target->x() < x1 )
			{
				x2 = x1;
				x1 = target->x();
			}
			else
				x2 = target->x();

			INT16 y2;
			if( target->y() < y1 )
			{
				y2 = y1;
				y1 = target->y();
			}
			else
				y2 = target->y();

			socket->sysMessage( tr( "Tiling from %1,%2 to %3,%4" ).arg( x1 ).arg( y1 ).arg( x2 ).arg( y2 ) );
			UINT16 dCount = 0;

			for( UINT16 x = x1; x <= x2; ++x )
				for( UINT16 y = y1; y <= y2; ++y )
				{
					// Select a Random Tile from the list
					QString id = ids[ RandomNum( 0, ids.count()-1 ) ];
					P_ITEM pItem = Items->createScriptItem( id );
					
					if( pItem )
					{
						pItem->pos.x = x;
						pItem->pos.y = y;
						pItem->pos.z = z;
						pItem->pos.map = socket->player()->pos.map;
						cMapObjects::getInstance()->add( pItem );
						pItem->update();
						dCount++;
					}
				}

			socket->sysMessage( tr( "Created %1 items." ).arg( dCount ) );
			return true;
		}		
	}
};

void commandTile( cUOSocket *socket, const QString &command, QStringList &args )
{
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

void commandAllShow( cUOSocket *socket, const QString &command, QStringList &args )
{
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

void commandAllMove( cUOSocket *socket, const QString &command, QStringList &args )
{
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

void commandShowSerials( cUOSocket *socket, const QString &command, QStringList &args )
{
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

class cRestockTarget: public cTargetRequest
{
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		P_CHAR pChar = FindCharBySerial( target->serial() );

		if( pChar )
		{
			pChar->restock();
			socket->sysMessage( tr( "This vendor's inventar has been restocked." ) );
		}
		else
		{
			socket->sysMessage( tr( "Please target a valid vendor." ) );
		}

		return true;
	}
};

void commandRestock( cUOSocket *socket, const QString &command, QStringList &args )
{
	socket->sysMessage( tr( "Please select the vendor you want to restock." ) );
	socket->attachTarget( new cRestockTarget );
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
	{ "BANK",			commandBank },
	{ "FIX",			commandFix },
	{ "GO",				commandGo },
	{ "INFO",			commandInfo },
	{ "KILL",			commandKill },
	{ "MAKEMENU",		commandMakeMenu },
	{ "MOVE",			commandMove },
	{ "NUKE",			commandNuke },
	{ "PAGES",			commandPages },
	{ "RELOAD",			commandReload },
	{ "REMOVE",			commandRemove },
	{ "REMOVEEVENT",	commandRemoveEvent },
	{ "REMOVESPELL",	commandRemoveSpell },
	{ "RESEND",			commandResend },
	{ "RESTOCK",		commandRestock },
	{ "RESURRECT",		commandResurrect },
	{ "SAVE",			commandSave },
	{ "SET",			commandSet },
	{ "SHOW",			commandShow },
	{ "SHOWSERIALS",	commandShowSerials },
	{ "SHUTDOWN",		commandShutDown },
	{ "SPAWNREGION",	commandSpawnRegion },
	{ "TAGS",			commandTags },
	{ "TELE",			commandTele },
	{ "TILE",			commandTile },	
	{ "WHERE",			commandWhere },
	{ "WHO",			commandWho },
	{ NULL, NULL }
};

