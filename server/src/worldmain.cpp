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
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================

// WorldMain.cpp: implementation of the CWorldMain class.
//
//////////////////////////////////////////////////////////////////////

#include "worldmain.h"
#include "progress.h"
#include "charsmgr.h"
#include "corpse.h"
#include "itemsmgr.h"
#include "TmpEff.h"
#include "guildstones.h"
#include "regions.h"
#include "srvparams.h"
#include "chars.h"
#include "pfactory.h"
#include "iserialization.h"
#include "wolfpack.h"
#include "utilsys.h"
#include "mapstuff.h"
#include "territories.h"
#include "accounts.h"
#include "books.h"

#undef  DBGFILE
#define DBGFILE "worldmain.cpp"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


// Items Saver thread
void CWorldMain::cItemsSaver::run() throw()
{
	try
	{
		waitMutex.acquire();
		ISerialization* archive = cPluginFactory::serializationArchiver( module );
		archive->prepareWritting( "items" );
		AllItemsIterator iterItems;
		for (iterItems.Begin(); !iterItems.atEnd(); ++iterItems)
		{
			P_ITEM pi = iterItems.GetData();
			if( pi && pi->objectID() != "RESOURCEITEM" )
				archive->writeObject( pi );
		}
		archive->close();
		delete archive;
		waitMutex.release();
	}
	catch( ... )
	{
		waitMutex.release();
	}
}

void CWorldMain::cItemsSaver::wait()
{
	try
	{
		waitMutex.acquire();
		waitMutex.release();
	}
	catch ( ... )
	{
		waitMutex.release();
	}
}

CWorldMain::CWorldMain()
{
	announce(false);

	iWsc = cWsc = NULL;
	Cur = 0;
	PerLoop = -1;
	isSaving = false;
}

CWorldMain::~CWorldMain()
{
	if (iWsc)
		fclose(iWsc);
	if (cWsc)
		fclose(cWsc);
}


void CWorldMain::loadnewworld(QString module) // Load world
{
	ISerialization* archive = cPluginFactory::serializationArchiver(module);

	QString objectID;
	register unsigned int i;

	// Load Chars
	archive->prepareReading( "chars" );
	clConsole.send("Loading Characters %i...\n", archive->size());
	progress_display progress(archive->size());
	for ( i = 0; i < archive->size(); ++progress, ++i)
	{
		archive->readObjectID(objectID);
		P_CHAR pc = NULL;
		if ( objectID == "CHARACTER" )
		{
			pc = new cChar;
			pc->Init(false);
		}
		else
			continue; // Something went wrong and we have an NULL pointer
		archive->readObject( pc );
		cCharsManager::getInstance()->registerChar( pc );
		int zeta;
		for (zeta = 0;zeta<ALLSKILLS;zeta++) if (pc->lockSkill[zeta]!=0 && pc->lockSkill[zeta]!=1 && pc->lockSkill[zeta]!=2) pc->lockSkill[zeta]=0;

		//AntiChrist bugfix for hiding
		pc->priv2 &= 0xf7; // unhide - AntiChrist
		pc->setHidden( 0 );
		pc->setStealth( -1 );

		//AntiChrist bugfix for magic reflect
		pc->priv2 &= 0xBF;
		pc->SetSpawnSerial( pc->spawnSerial() );

		cTerritory* Region = cAllTerritories::getInstance()->region( pc->pos.x, pc->pos.y );
		if( Region != NULL )
			pc->region = Region->name();

		pc->setAntispamtimer( 0 );   //LB - AntiSpam -
		pc->setAntiguardstimer( 0 ); //AntiChrist - AntiSpam for "GUARDS" call - to avoid (laggy) guards multi spawn

		if (pc->id() <= 0x3e1)
		{
			unsigned short k = pc->id();
			unsigned short c1 = pc->skin();
			unsigned short b = c1&0x4000;
			if ((b == 16384 && (k >=0x0190 && k<=0x03e1)) || c1==0x8000)
			{
				if (c1!=0xf000)
				{
					pc->setSkin( 0xF000 );
					pc->setXSkin( 0xF000 );
					clConsole.send("char/player: %s : %i correted problematic skin hue\n", pc->name.c_str(),pc->serial);
				}
			}
		} 
		else	// client crashing body --> delete if non player esle put onl”x a warning on server screen
			// we dont want to delete that char, dont we ?
		{
			if (pc->account() == 0)
			{
				Npcs->DeleteChar(pc);
			} 
			else
			{
				pc->setId(0x0190);
				clConsole.send("player: %s with bugged body-value detected, restored to male shape\n",pc->name.c_str());
			}
		}

		if(pc->stablemaster_serial() == INVALID_SERIAL)
		{ 
			mapRegions->Add(pc); 
		} 
		else
			stablesp.insert(pc->stablemaster_serial(), pc->serial);

		if (pc->isPlayer() && pc->account() == 0) pc->setMenupriv(-1);


		int max_x = cMapStuff::mapTileWidth(pc->pos) * 8;
		int max_y = cMapStuff::mapTileHeight(pc->pos) * 8;
		if ((pc->pos.x < 100 && pc->pos.y < 100 && pc->account() ==0) || ((pc->pos.x>max_x || pc->pos.y>max_y) && pc->account() == 0))
		// if ((pc->pos.x < 100 && pc->pos.y < 100 && pc->account ==-1) || ((pc->pos.x>max_x || pc->pos.y>max_y || pc->pos.x<0 || pc->pos.y<0) && pc->account==-1))
		{
			Npcs->DeleteChar(pc); //character in an invalid location
		}
		if ((pc->pos.x < 100 && pc->pos.y < 100 && pc->account() != 0) || (( pc->pos.x>max_x || pc->pos.y>max_y ) && pc->account() !=0))
		// if ((pc->pos.x < 100 && pc->pos.y < 100 && pc->account !=-1) || ((pc->pos.x>max_x || pc->pos.y>max_y || pc->pos.x<0 || pc->pos.y<0) && pc->account!=-1))
		{
			Coord_cl pos(pc->pos);
			pos.x = 900;
			pos.y = 300;
			pos.z = 30;
			pc->moveTo(pos); //player in an invalid location
		}
		setcharflag(pc);//AntiChrist
	}
	clConsole.send(" Done.\n");
	archive->close();
	delete archive;

	SERIAL newCont;

	// Load Items
	archive = cPluginFactory::serializationArchiver(module);

	archive->prepareReading( "items" ); // Load Items
	clConsole.send( "Loading Items %i...\n", archive->size() );
	progress.restart(archive->size());
	for ( i = 0; i < archive->size(); ++progress, ++i )
	{
		archive->readObjectID(objectID);
		P_ITEM pi = NULL;
		if ( objectID == "ITEM" )
		{
			pi = new cItem;
		} 
		else if ( objectID == "HOUSE" )
		{
			pi = dynamic_cast<P_ITEM>(new cHouse);
		}
		else if ( objectID == "BOAT" )
		{
			pi = dynamic_cast<P_ITEM>(new cBoat);
		}
		else if ( objectID == "BOOK" )
		{
			pi = dynamic_cast<P_ITEM>(new cBook);
		}
		else if ( objectID == "GUILDSTONE" )
		{
			pi = dynamic_cast<P_ITEM>(new cGuildStone);
		}
		else if( objectID == "CORPSE" )
		{
			pi = dynamic_cast<P_ITEM>(new cCorpse);
		}
		else // somethine went wrong and we have a NULL pointer.
			continue; 

		pi->Init( false );
		archive->readObject( pi );
		cItemsManager::getInstance()->registerItem( pi );
		if( objectID == "GUILDSTONE" ) // register as guild as well
			guilds.push_back(pi->serial);
		pi->timeused_last = getNormalizedTime();

		// Set the outside indices
		pi->SetSpawnSerial( pi->spawnserial );
		pi->setContSerial( pi->contserial );
		pi->SetOwnSerial( pi->ownserial );

		if( pi->maxhp() == 0) 
			pi->setMaxhp( pi->hp() );

		// Tauriel adding region pointers
		if (pi->isInWorld())
		{
			int max_x = Map->mapTileWidth(pi->pos) * 8;
			int max_y = Map->mapTileHeight(pi->pos) * 8;
			if (pi->pos.x>max_x || pi->pos.y>max_y) 
			{
				Items->DeleItem(pi);	//these are invalid locations, delete them!
			}
			else
				mapRegions->Add(pi);
		}
	}
	clConsole.send(" Done.\n");
	archive->close();
	delete archive;

	// Load Temporary Effects
	archive = cPluginFactory::serializationArchiver(module);

	archive->prepareReading( "effects" );
	clConsole.send("Loading Temp. Effects %i...\n", archive->size());
	progress.restart(archive->size());
	for ( i = 0; i < archive->size(); ++progress, ++i)
	{
		archive->readObjectID(objectID);

		cTempEffect* pTE = NULL;

		if( objectID == "TmpEff" )
			pTE = new cTmpEff;

		else if( objectID == "PythonEffect" )
			pTE = new cPythonEffect;

		else		
			continue; // an error occured..

		archive->readObject( pTE );

		cTempEffects::getInstance()->insert( pTE );
	}
	clConsole.send(" Done.\n");
	archive->close();

	delete archive;
}

//o---------------------------------------------------------------------------o
//|	Class		:	CWorldMain::savenewworld(char x)
//|	Date		:	Unknown
//|	Programmer	:	Unknown		
//o---------------------------------------------------------------------------o
//| Purpose		:	Save current world state. Stores all values in an easily
//|					readable script file "*.wsc". This stores all world items
//|					and NPC/PC character information for a given shard
//o---------------------------------------------------------------------------o
void CWorldMain::savenewworld(QString module)
{
	static unsigned long ocCount, oiCount;
	UI32 savestarttime = getNormalizedTime();

//	AllTmpEff->Off();

	if ( !Saving() )
	{
		//	gcollect();
		if ( announce() )
		{
			sysbroadcast("World data saving....");
			clConsole.send("Worldsave Started!\n" );
			clConsole.send("items  : %i\n", cItemsManager::getInstance()->size());
			clConsole.send("chars  : %i\n", cCharsManager::getInstance()->size());
			clConsole.send("effects: %i\n", cTempEffects::getInstance()->size());
		}
		isSaving = true;
	}


	cItemsSaver ItemsThread(module);
	ItemsThread.start();

	SrvParams->flush();
	if (SrvParams->serverLog()) savelog("Server data save\n","server.log");

	ISerialization* archive = cPluginFactory::serializationArchiver( module );
	archive->prepareWritting( "chars" );
	AllCharsIterator iterChars;
	for (iterChars.Begin(); !iterChars.atEnd(); ++iterChars)
	{
		archive->writeObject( iterChars.GetData() );
	}
	archive->close();
	delete archive;

	archive = cPluginFactory::serializationArchiver( module );
	archive->prepareWritting( "effects" );
	cTempEffects::getInstance()->serialize( *archive );
	archive->close();
	delete archive;

	// Save the accounts
	clConsole.PrepareProgress( tr( "Saving %1 accounts" ).arg( Accounts->count() ).latin1() );
	Accounts->save();
	clConsole.ProgressDone();
	
	ItemsThread.join();

	if ( announce() )
	{
		sysbroadcast("Worldsave Done!\n");
		char temp[128];
		sprintf( temp, "World saved in %.03f sec", (float)(((float)getNormalizedTime() - (float)savestarttime) / CLOCKS_PER_SEC ) );
		clConsole.PrepareProgress( temp );
		clConsole.ProgressDone();
	}

	isSaving = false;

	uiCurrentTime = getNormalizedTime();
}

int CWorldMain::announce()
{
	return DisplayWorldSaves;
}

void CWorldMain::announce(int choice)
{
	if(choice<1)
		DisplayWorldSaves=0;
	else
		DisplayWorldSaves=1;
}

bool CWorldMain::Saving( void )
{
	return isSaving;
}

void CWorldMain::SaveChar( P_CHAR pc )
{
}

static void decay1(P_ITEM pi, P_ITEM pItem)
{
	long serial;
	if( pi->corpse() == 1 )
	{
		serial=pi->serial;
		unsigned int ci;
		vector<SERIAL> vecContainer( contsp.getData(pi->serial) );
		for (ci=0;ci<vecContainer.size();ci++)
		{
			P_ITEM pi_j = FindItemBySerial(vecContainer[ci]);
			if( pi_j != NULL )
			{
				if ((pi_j->contserial==pi->serial) &&
					(pi_j->layer()!=0x0B)&&(pi_j->layer()!=0x10))
				{
					pi_j->setContSerial(-1);
					pi_j->moveTo(pi->pos);
					Items->DeleItem(pi_j);
				}
			}
		}
	}
	else
	{
		if ( pi->multis == INVALID_SERIAL )
		{
			P_ITEM pi_multi = findmulti( pi->pos );
			if( pi_multi == NULL )
			{
				Items->DeleItem(pItem);
			}
		}
	}
}

void CWorldMain::SaveItem( P_ITEM pi, P_ITEM pDefault)
{
}

//o--------------------------------------------------------------------------
//|	Function		-	int CWorldMain::RemoveItemFromCharBody(int charserial, int type1, int type2);
//|	Date				-	Unknown
//|	Programmer	-	Abaddon
//o--------------------------------------------------------------------------
//|	Purpose			-	Remove the specified item from a characters paper doll
//o--------------------------------------------------------------------------
//|	Returns			-	true if removed, false otherwise
//o--------------------------------------------------------------------------
bool CWorldMain::RemoveItemsFromCharBody( int charserial, int type1, int type2 )
{ 
	int serial;
	P_CHAR pc = FindCharBySerial(charserial);
	if (pc == NULL) return false;
 	serial= pc->serial;
 	bool foundMatch = false;
	unsigned int ci;
	vector<SERIAL> vecContainer = contsp.getData(serial);
	for (ci=0;ci<vecContainer.size();ci++)
 	{
		P_ITEM pci = FindItemBySerial(vecContainer[ci]);

 		if (pci != NULL)
 		{
			if( ( pci->layer() == 0x01 || pci->layer() == 0x02 ) && ( pci->contserial == serial ) )
 			{
 				// in first hand, or second hand
 				if( ( pci->id() >> 8 ) == type1 && ( pci->id()&0x00FF ) == type2 )
				{
 					Items->DeleItem( pci );
 					foundMatch = true;
 				}
 			}
 		}
	}
	return foundMatch;
}
