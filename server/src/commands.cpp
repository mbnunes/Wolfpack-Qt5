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

#include "commands.h"
#include "globals.h"
#include "wpconsole.h"
#include "wpdefmanager.h"
#include "territories.h"
#include "targetrequests.h"
#include "network/uosocket.h"

// Main Command processing function
void cCommands::process( cUOSocket *socket, const QString &command )
{
	// TODO: Insert processing instructions for script-defined commands here

	if( !socket->player() )
		return;

	P_CHAR pChar = socket->player();
	QString pLevel = pChar->privlvl();
	QStringList pArgs = QStringList::split( " ", command, true );
	
	// No Command? No Processing
	if( pArgs.isEmpty() )
		return;

	QString pCommand = pArgs[0].upper(); // First element should be the command

	// Remove it from the argument list
	pArgs.erase( pArgs.begin() );

	// Check if the priviledges are ok
	if( !containsCmd( pLevel, pCommand ) )
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

// adds a command (string) into the stringlist of the privlvl if it's not in yet
void cCommands::addCmdToPrivLvl( QString privlvl, QString command )
{
	if( !privlvl_commands[ privlvl ].commands.contains( command ) )
		privlvl_commands[ privlvl ].commands.push_back( command );
}

// rmvs a command (sting) from the stringlist of the privlvl
void cCommands::rmvCmdFromPrivLvl( QString privlvl, QString command )
{
	if( privlvl_commands[ privlvl ].commands.contains( command ) )
		privlvl_commands[ privlvl ].commands.remove( command );
}

// explains itself :)
bool cCommands::containsCmd( QString privlvl, QString command )
{
	return ( ( privlvl_commands[ privlvl ].commands.contains( command ) && privlvl_commands[ privlvl ].implicit ) ||
		   ( !privlvl_commands[ privlvl ].commands.contains( command ) && !privlvl_commands[ privlvl ].implicit ) );
}

void cCommands::loadPrivLvlCmds( void )
{
	clConsole.PrepareProgress( "Loading PrivLvl Command Lists." );

	QStringList ScriptSections = DefManager->getSections( WPDT_PRIVLEVEL );
	
	if( ScriptSections.isEmpty() )
	{
		clConsole.ProgressFail();
		clConsole.ChangeColor( WPC_RED );
		clConsole.send( "WARNING: Privlvls for admins, gms, counselors and players undefined!\n" );
		clConsole.ChangeColor( WPC_NORMAL );
		return;
	}
	
	for(QStringList::iterator it = ScriptSections.begin(); it != ScriptSections.end(); ++it )
	{
		QDomElement *Tag = DefManager->getSection( WPDT_PRIVLEVEL, *it );

		if( Tag->isNull() || !Tag->attributes().contains( "id" ) )
			continue;
		
		QString privlvl = Tag->attribute( "id" );
		privlvl_commands[privlvl].implicit = !( Tag->attributes().contains( "type" ) && Tag->attribute( "type" ) == "explicit" );

		QDomNode childNode = Tag->firstChild();
		while( !childNode.isNull() )
		{
			this->addCmdToPrivLvl( privlvl, childNode.nodeName().upper() );
			childNode = childNode.nextSibling();
		}
	}
	clConsole.ProgressDone();
}

// COMMAND IMPLEMENTATION
// Purpose:
// .go >> Target Cursor and select a teleport target
// .go x,y,z,[plane] >> Go to those coordinates
// .go placename >> Go to that specific place
void commandGo( cUOSocket *socket, const QString &command, QStringList &args )
{
	P_CHAR pChar = socket->player();

	if( !pChar )
		return;

	if( args.isEmpty() )
	{
		socket->sysMessage( "Bringin up target cursor" );
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
			return;
		}			
	}

	// If we reached this end it's definetly an invalid command
	socket->sysMessage( tr( "Usage: go [location|x,y,z,[plane]]" ) );
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

	message.append( tr( "at %1,%2,%3 on plane %4" ).arg( pChar->pos.x ).arg( pChar->pos.y ).arg( pChar->pos.z ).arg( pChar->pos.plane ) );
	pChar->message( message );
}

void commandFix( cUOSocket *socket, const QString &command, QStringList &args )
{
	// TODO: Eventually check if the character is stuck etc.
	socket->updatePlayer();
}

void commandAddItem( cUOSocket *socket, const QString &command, QStringList &args )
{
}

void commandAddNpc( cUOSocket *socket, const QString &command, QStringList &args )
{
}

void commandAdd( cUOSocket *socket, const QString &command, QStringList &args )
{
	// Bring up the Add-menu
	if( args.count() < 1 )
	{
		// TODO: Show Add menu here
		socket->sysMessage( "Showing addmenu [unimplemented]" );
		return;
	}

	QString param = args.join( " " ).stripWhiteSpace();
    
	QDomElement *node = DefManager->getSection( WPDT_ITEM, param );

	// An item definition with that name exists
	if( node && !node->isNull() )
	{
		socket->sysMessage( tr( "Where do you want to add the '%1'" ).arg( param ) );
		socket->attachTarget( new cAddNpcTarget( param ) );
		return;
	}

	node = DefManager->getSection( WPDT_NPC, param );
	
	// Same for NPCs
	if( node && !node->isNull() )
	{
		socket->sysMessage( tr( "Where do you want to add the '%1'" ).arg( param ) );
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

	virtual void responsed( cUOSocket *socket, cUORxTarget *target )
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
			return;
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
				return;
			}
			pObject->moveTo( newCoords );
		}
		
		// Char title
		else if( ( key == "title" ) && pChar )
			pChar->setTitle( value );

		// Item Amount
		else if( ( key == "amount" ) && pItem && ( hex2dec( value ).toULong() > 0 ) )
			pItem->setAmount( hex2dec( value ).toULong() );			

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
			socket->sysMessage( tr( "Unknown key '%1'" ).arg( key ) );
			return;
		}

		if( pChar )
			pChar->update();
		else if( pItem )
			pItem->update();
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
	socket->resendWorld();
}

class cRemoveTarget: public cTargetRequest
{
public:
	virtual void responsed( cUOSocket *socket, cUORxTarget *target )
	{
		P_CHAR pChar = FindCharBySerial( target->serial() );
		P_ITEM pItem = FindItemBySerial( target->serial() );

		if( pChar )
		{
			if( pChar->socket() )
			{
				socket->sysMessage( "You cannot delete logged in characters" );
				return;
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
		socket->sysMessage( tr( "Usage: account <Create|Remove|Password|Show>" ) );
		return;
	}

	QString subCommand = args[0].lower();

	// Create Accounts
	if( subCommand == "create" )
	{
		// Create a new account
		if( args.count() < 3 )
		{
			socket->sysMessage( tr( "Usage: account create <Username> <Password>" ) );
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
			socket->sysMessage( tr( "Usage: account remove <Username>" ) );
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

			for( UINT32 i = 0; i < characters.size(); ++i )
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
			socket->sysMessage( tr( "Usage: account set <Username> <Key> <Value>" ) );
		}
		else if( !Accounts->getRecord( args[1] ) )
		{
				socket->sysMessage( tr( "Account '%1' does not exist" ).arg( args[1] ) );
		}
	}

	// Show properties of accounts
	else if( subCommand == "show" )
	{
		if( args.count() < 3 )
		{
			socket->sysMessage( tr( "Usage: account set <Username> <Key>" ) );
		}
	}
}

// Command Table (Keep this at the end)
stCommand cCommands::commands[] =
{
	{ "ADD", commandAdd },
	{ "ACCOUNT", commandAccount },
	{ "REMOVE", commandRemove },
	{ "RESEND", commandResend },
	{ "ADDITEM", commandAddItem },
	{ "ADDNPC", commandAddNpc },
	{ "GO", commandGo },
	{ "WHERE", commandWhere },
	{ "FIX", commandFix },
	{ "SET", commandSet },
	{ NULL, NULL }
};
