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

#include "worldmain.h"
#include "progress.h"
#include "charsmgr.h"
#include "corpse.h"
#include "itemsmgr.h"
#include "TmpEff.h"
#include "guildstones.h"
#include "mapobjects.h"
#include "srvparams.h"
#include "chars.h"
#include "pfactory.h"
#include "iserialization.h"
#include "wolfpack.h"
#include "utilsys.h"
#include "maps.h"
#include "territories.h"
#include "accounts.h"
#include "books.h"
#include "multis.h"
#include "spellbook.h"
#include "persistentbroker.h"
#include "network/uopacket.h"

// Library Includes
#include <qsqlcursor.h>
#include <qcstring.h>

#undef  DBGFILE
#define DBGFILE "worldmain.cpp"


// Items Saver thread
void CWorldMain::cItemsSaver::run() throw()
{
	try
	{
		waitMutex.acquire();
		AllItemsIterator iterItems;
		for (iterItems.Begin(); !iterItems.atEnd(); ++iterItems)
		{
			P_ITEM pi = iterItems.GetData();
			if( pi && pi->objectID() != "RESOURCEITEM" )
				persistentBroker->saveObject(pi);
		}
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


void CWorldMain::loadnewworld( QString module ) // Load world
{
	ISerialization* archive = cPluginFactory::serializationArchiver(module);

	QString objectID;
	register unsigned int i = 0;

	clConsole.send( "Loading World...\n" );

	mysql_init( mysql );

	// This should be in SrvParams
	//mysql_options(mysql,MYSQL_OPT_COMPRESS,0);
	
	if( !mysql_real_connect( mysql, SrvParams->databaseHost().latin1(), SrvParams->databaseUsername().latin1(),SrvParams->databasePassword().latin1(), SrvParams->databaseName().latin1(), 0, NULL, 0 ) )
	{
		fprintf( stderr, "Failed to connect to database: Error: %s\n", mysql_error(mysql) );
		return;
	}

	QStringList types = UObjectFactory::instance()->objectTypes();

	for( INT32 j = 0; j < types.count(); ++j )
	{
		QString type = types[j];

		// Find out how many objects of this type are available		
		if( mysql_query( mysql, QString( "SELECT COUNT(*) FROM uobjectmap WHERE type = '" + type + "'" ).latin1() ) )
			throw mysql_error( mysql );

		MYSQL_RES *result = mysql_use_result( mysql );
		MYSQL_ROW row = mysql_fetch_row( result );

		UINT32 count = atoi( row[0] );

		mysql_free_result( result );

		clConsole.send( "Loading " + QString::number( count ) + " objects of type " + type + "\n" );

		// Error Checking		
		if( mysql_query( mysql, UObjectFactory::instance()->findSqlQuery( type ).latin1() ) )
			throw mysql_error( mysql );

		result = mysql_use_result( mysql );

		UINT32 sTime = getNormalizedTime();
		UINT16 offset;
		cUObject *object;

		progress_display progress( count );

		// Fetch row-by-row
		while( ( row = mysql_fetch_row( result ) ) )
		{
			// do something with data
			object = UObjectFactory::instance()->createObject( type );
			offset = 2; // Skip the first two fields
			object->load( row, offset );

			++progress;
		}

		mysql_free_result( result );

		clConsole.send( "Loaded %i objects in %i msecs\n", progress.count(), getNormalizedTime() - sTime );
	}

	mysql_close( mysql );

//	clConsole.send( "Loaded %i in %i msecs\n", i, getNormalizedTime() - sTime );

	// Load Temporary Effects
	archive = cPluginFactory::serializationArchiver(module);

	archive->prepareReading( "effects" );
	clConsole.send("Loading Temp. Effects %i...\n", archive->size());
	progress_display progress( archive->size() );
	for ( i = 0; i < archive->size(); ++progress, ++i)
	{
		archive->readObjectID(objectID);

		cTempEffect* pTE = NULL;

		if( objectID == "TmpEff" )
			pTE = new cTmpEff;

		else if( objectID == "PythonEffect" )
			pTE = new cPythonEffect;

		else if( objectID == "HIDECHAR" )
			pTE = new cDelayedHideChar( INVALID_SERIAL );

		else		
			continue; // an error occured..

		archive->readObject( pTE );

		TempEffects::instance()->insert( pTE );
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
			clConsole.send("items  : %i\n", ItemsManager::instance()->size());
			clConsole.send("chars  : %i\n", CharsManager::instance()->size());
			clConsole.send("effects: %i\n", TempEffects::instance()->size());
		}
		isSaving = true;
	}


	cItemsSaver ItemsThread(module);
	ItemsThread.run(); // thread disabled for now.

	SrvParams->flush();
	if (SrvParams->serverLog()) savelog("Server data save\n","server.log");

	ISerialization* archive = cPluginFactory::serializationArchiver( module );
	archive->prepareWritting( "chars" );
	AllCharsIterator iterChars;
	for (iterChars.Begin(); !iterChars.atEnd(); ++iterChars)
	{
//		archive->writeObject( iterChars.GetData() );
		persistentBroker->saveObject(iterChars.GetData());
	}
	archive->close();
	delete archive;

	archive = cPluginFactory::serializationArchiver( module );
	archive->prepareWritting( "effects" );
	TempEffects::instance()->serialize( *archive );
	archive->close();
	delete archive;

	// Save the accounts
	clConsole.PrepareProgress( tr( "Saving %1 accounts" ).arg( Accounts::instance()->count() ).latin1() );
	Accounts::instance()->save();
	clConsole.ProgressDone();
	
//	ItemsThread.join();

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
	if( pi->corpse() )
	{
/*		serial=pi->serial;
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
*/	}
	else
	{
		if ( pi->multis == INVALID_SERIAL )
		{
			cMulti* pi_multi = cMulti::findMulti( pi->pos );
			if( !pi_multi )
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
	cChar::ContainerContent container(pc->content());
	cChar::ContainerContent::const_iterator it (container.begin());
	cChar::ContainerContent::const_iterator end(container.end());
	for (; it != end; ++it )
	{
		P_ITEM pci = *it;

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
