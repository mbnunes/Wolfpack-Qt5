/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2017 by holders identified in AUTHORS.txt
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
 * Wolfpack Homepage: https://github.com/Mutilador/Wolfpack
 */

// Wolfpack Includes
#include "console.h"
#include "pythonscript.h"
#include "log.h"
#include "profile.h"

#include "world.h"
#include "network/network.h"
#include "network/uosocket.h"
#include "serverconfig.h"
#include "player.h"
#include "accounts.h"
#include "inlines.h"
#include "mapobjects.h"
#include "items.h"
#include "basechar.h"
#include "npc.h"
#include "muls/tilecache.h"
#include "definitions.h"

// Library Includes
#include <QString>
#include <QtGlobal>
#include <QThread>
#include <QMutex>

using namespace std;

// Constructor
cConsole::cConsole()
{
	progress = QString();
}

// Destuctor
cConsole::~cConsole()
{
	// Clean up any terminal settings
}

void cConsole::log( uchar logLevel, const QString& message, bool timestamp )
{
	// Legacy Code
	QString msg = message;

	if ( msg.endsWith( "\n" ) )
		msg = msg.left( msg.length() - 1 );

	Log::instance()->print( ( eLogLevel ) logLevel, msg + "\n", timestamp );
}

// Prepare a "progess" line
void cConsole::sendProgress( const QString& sMessage )
{
	send( sMessage + "... " );
	changeColor( WPC_NORMAL );
	progress = sMessage + "... ";
}

// Print Progress Done
void cConsole::sendDone()
{
	progress = QString();
	changeColor( WPC_GREEN );
	send( tr( "Done\n" ) );
	changeColor( WPC_NORMAL );
}

// Print "Fail"
void cConsole::sendFail()
{
	progress = QString();
	changeColor( WPC_RED );
	send( tr( "Failed\n" ) );
	changeColor( WPC_NORMAL );
}

// Print "Skip" (maps etc.)
void cConsole::sendSkip()
{
	progress = QString();
	changeColor( WPC_YELLOW );
	send( tr( "Skipped\n" ) );
	changeColor( WPC_NORMAL );
}

bool cConsole::handleCommand( const QString& command )
{
	cUOSocket* mSock;
	QList<cUOSocket*> socketList;
	int i;
	char c = command.toLatin1()[0];
	c = toupper( c );

	if ( c == 'S' )
	{
		Server::instance()->setSecure( !Server::instance()->getSecure() );

		if ( !Server::instance()->getSecure() )
			Console::instance()->send( tr( "WOLFPACK: Secure mode disabled. Press ? for a commands list.\n" ) );
		else
			Console::instance()->send( tr( "WOLFPACK: Secure mode re-enabled.\n" ) );

		return true;
	}

	// Allow Help in Secure Mode
	if ( Server::instance()->getSecure() && c != '?' )
	{
		Console::instance()->send( tr( "WOLFPACK: Secure mode prevents keyboard commands! Press 'S' to disable.\n" ) );
		return false;
	}

	switch ( c )
	{
	case 'Q':
		Console::instance()->send( tr( "WOLFPACK: Immediate Shutdown initialized!\n" ) );
		Server::instance()->cancel();
		break;

	case '#':
		World::instance()->save();
		Config::instance()->flush();
		break;

	case 'P':
		dumpProfilingInfo();
		break;

	case 'W':
		Console::instance()->send( tr( "Current Users in the World:\n" ) );

		socketList = Network::instance()->sockets();
		i = 0;

		foreach ( mSock, socketList )
		{
			if ( mSock->player() )
				Console::instance()->send( QString( "%1) %2 [%3]\n" ).arg( ++i ).arg( mSock->player()->name() ).arg( QString::number( mSock->player()->serial(), 16 ) ) );
		}

		Console::instance()->send( tr( "Total Users Online: %1\n" ).arg( Network::instance()->count() ) );
		break;
	case 'A':
		//reload the accounts file
		Server::instance()->queueAction( RELOAD_ACCOUNTS );
		break;
	case 'R':
		Server::instance()->queueAction( RELOAD_SCRIPTS );
		break;
	case 'D':
	{
		Console::instance()->send( tr( "World Diagnostics:\n" ) );
		Console::instance()->send( tr( "Total Chars: %1, Total Items: %2\n" )
			.arg( World::instance()->charCount() ).arg( World::instance()->itemCount() ) );

		cCharIterator it;
		for ( P_CHAR p = it.first(); p; p = it.next() )
		{
			if ( p->name() == "Mutilador" || dynamic_cast<P_PLAYER>( p ) )
			{
				Console::instance()->send( tr( "Player '%1' [0x%2] at (%3, %4, %5, map %6)\n" )
					.arg( p->name() ).arg( QString::number( p->serial(), 16 ) )
					.arg( p->pos().x ).arg( p->pos().y ).arg( (int)p->pos().z ).arg( (int)p->pos().map ) );

				// List items in circle
				MapItemsIterator iIt = MapObjects::instance()->listItemsInCircle( p->pos(), 18 );
				int itemCount = 0;
				for ( P_ITEM item = iIt.first(); item; item = iIt.next() )
				{
					itemCount++;
					if ( itemCount <= 15 )
					{
						Console::instance()->send( tr( "  Item 0x%1: id=0x%2 (%3) at (%4, %5, %6)\n" )
							.arg( QString::number( item->serial(), 16 ) ).arg( QString::number( item->id(), 16 ) ).arg( item->baseid() )
							.arg( item->pos().x ).arg( item->pos().y ).arg( (int)item->pos().z ) );
					}
				}
				Console::instance()->send( tr( "Total items around player: %1\n" ).arg( itemCount ) );

				// List chars in circle
				MapCharsIterator cIt = MapObjects::instance()->listCharsInCircle( p->pos(), 18 );
				int charCount = 0;
				for ( P_CHAR chr = cIt.first(); chr; chr = cIt.next() )
				{
					if ( chr == p ) continue;
					charCount++;
					Console::instance()->send( tr( "  NPC '%1' [0x%2] (baseid '%3'): body=0x%4 at (%5, %6, %7), equipped items=%8\n" )
						.arg( chr->name() ).arg( QString::number( chr->serial(), 16 ) ).arg( chr->baseid() ).arg( QString::number( chr->body(), 16 ) )
						.arg( chr->pos().x ).arg( chr->pos().y ).arg( (int)chr->pos().z )
						.arg( chr->content().size() ) );
					cBaseChar::ItemContainer cont = chr->content();
					for ( auto eIt = cont.begin(); eIt != cont.end(); ++eIt )
					{
						P_ITEM eq = eIt.value();
						Console::instance()->send( tr( "    Layer 0x%1: id=0x%2 (serial 0x%3) color=0x%4 baseid '%5'\n" )
							.arg( QString::number( eq->layer(), 16 ) ).arg( QString::number( eq->id(), 16 ) )
							.arg( QString::number( eq->serial(), 16 ) ).arg( QString::number( eq->color(), 16 ) ).arg( eq->baseid() ) );
					}
				}
				Console::instance()->send( tr( "Total NPCs around player: %1\n" ).arg( charCount ) );
			}
		}

		int npcsWithClothes = 0;
		int npcsTotal = 0;
		int humanTotal = 0;
		int humanWithClothes = 0;
		int humanNaked = 0;
		QMap<QString, int> nakedHumanBaseids;
		cCharIterator allChars;
		for ( P_CHAR c = allChars.first(); c; c = allChars.next() )
		{
			if ( !dynamic_cast<P_PLAYER>( c ) )
			{
				npcsTotal++;
				bool hasClothes = false;
				cBaseChar::ItemContainer cont = c->content();
				for ( auto eIt = cont.begin(); eIt != cont.end(); ++eIt )
				{
					if ( eIt.value()->layer() <= 0x19 )
					{
						hasClothes = true;
						break;
					}
				}
				if ( hasClothes )
					npcsWithClothes++;

				if ( c->isHuman() )
				{
					humanTotal++;
					if ( hasClothes )
						humanWithClothes++;
					else
					{
						humanNaked++;
						nakedHumanBaseids[c->baseid()]++;
					}
				}
			}
		}
		Console::instance()->send( tr( "World NPC equipment summary: %1 of %2 NPCs have clothes/equipment (<= 0x19)\n" )
			.arg( npcsWithClothes ).arg( npcsTotal ) );
		Console::instance()->send( tr( "Human NPCs: Total=%1, Clothed=%2, NAKED=%3\n" )
			.arg( humanTotal ).arg( humanWithClothes ).arg( humanNaked ) );
		Console::instance()->send( tr( "Sample naked human baseids:\n" ) );
		int sampleCount = 0;
		for ( auto nbIt = nakedHumanBaseids.begin(); nbIt != nakedHumanBaseids.end(); ++nbIt )
		{
			if ( sampleCount++ < 15 )
			{
				Console::instance()->send( tr( "  '%1': %2 naked NPCs\n" ).arg( nbIt.key() ).arg( nbIt.value() ) );
			}
		}

		// Count items per map
		int mapItems[6] = { 0, 0, 0, 0, 0, 0 };
		int containerItems = 0;
		cItemIterator allItems;
		for ( P_ITEM it = allItems.first(); it; it = allItems.next() )
		{
			if ( it->container() )
			{
				containerItems++;
			}
			else
			{
				unsigned char m = it->pos().map;
				if ( m < 5 )
					mapItems[m]++;
				else
					mapItems[5]++;
			}
		}
		Console::instance()->send( tr( "Items on ground: Map0=%1, Map1=%2, Map2=%3, Map3=%4, Map4=%5, Other=%6, InContainers/Equipped=%7\n" )
			.arg( mapItems[0] ).arg( mapItems[1] ).arg( mapItems[2] ).arg( mapItems[3] ).arg( mapItems[4] ).arg( mapItems[5] ).arg( containerItems ) );

		break;
	}
	case 'E':
	{
		Console::instance()->send( tr( "Re-equipping naked human NPCs...\n" ) );
		int reequipped = 0;
		cCharIterator allChars;
		for ( P_CHAR c = allChars.first(); c; c = allChars.next() )
		{
			if ( !dynamic_cast<P_PLAYER>( c ) && c->isHuman() )
			{
				bool hasClothes = false;
				cBaseChar::ItemContainer cont = c->content();
				for ( auto eIt = cont.begin(); eIt != cont.end(); ++eIt )
				{
					if ( eIt.value()->layer() <= 0x19 )
					{
						hasClothes = true;
						break;
					}
				}
				if ( !hasClothes && !c->baseid().isEmpty() )
				{
					const cElement* def = Definitions::instance()->getDefinition( WPDT_NPC, c->baseid() );
					if ( def )
					{
						c->applyDefinition( def );
						c->resend( true );
						reequipped++;
					}
				}
			}
		}
		Console::instance()->send( tr( "Successfully re-equipped %1 naked human NPCs! Remember to press '#' to save world.\n" ).arg( reequipped ) );
		break;
	}
	case 'L':
	{
		Console::instance()->send( tr( "Loading world decorations (doors, signs, fixtures)...\n" ) );
		{
			PythonGILLocker locker;
			int res = PyRun_SimpleString(
				"import commands.decoration\n"
				"commands.decoration.load_decoration(1)\n"
				"commands.decoration.load_decoration(0)\n"
			);
			if ( res != 0 )
			{
				PyErr_Print();
				Console::instance()->send( tr( "Python decoration loading returned an error!\n" ) );
			}
		}
		Console::instance()->send( tr( "Finished loading decorations! Remember to press '#' to save world to world.bin.\n" ) );
		break;
	}
	case '?':
		Console::instance()->send( tr( "Console commands:\n" ) );
		Console::instance()->send( tr( "	Q: Shutdown the server.\n" ) );
		Console::instance()->send( tr( "	# - Save world\n" ) );
		Console::instance()->send( tr( "	W - Display logged in characters\n" ) );
		Console::instance()->send( tr( "	A - Reload accounts\n" ) );
		Console::instance()->send( tr( "	R - Reload scripts\n" ) );
		Console::instance()->send( tr( "	D - World diagnostics (player, items, NPCs)\n" ) );
		Console::instance()->send( tr( "	E - Re-equip all naked NPCs in world\n" ) );
		Console::instance()->send( tr( "	L - Load decorations for Trammel and Felucca\n" ) );
		Console::instance()->send( tr( "	S - Toggle Secure mode " ) );
		if ( Server::instance()->getSecure() )
			Console::instance()->send( tr( "[enabled]\n" ) );
		else
			Console::instance()->send( tr( "[disabled]\n" ) );
		Console::instance()->send( tr( "	? - Commands list (this)\n" ) );
		Console::instance()->send( tr( "End of commands list.\n" ) );
		break;
	default:
		break;
	}

	return true;
}

void cConsole::queueCommand( const QString& command )
{
	QMutexLocker lock( &commandMutex );
	commandQueue.push_back( command );
}
