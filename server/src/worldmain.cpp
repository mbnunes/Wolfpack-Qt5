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
#include <qdatetime.h>
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
#include "dbdriver.h"
#include "pagesystem.h"

#include "python/utilities.h"
#include "python/tempeffect.h"

// Library Includes
#include <qcstring.h>

#include <zthread/Thread.h>
#include <zthread/FastMutex.h>


#undef  DBGFILE
#define DBGFILE "worldmain.cpp"

class cItemsSaver : public ZThread::Thread
{
private:
	ZThread::FastMutex waitMutex;
public:
	virtual ~cItemsSaver() throw() {}
	virtual void run() throw();
	void wait();
};


// Items Saver thread
void cItemsSaver::run() throw()
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

void cItemsSaver::wait()
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
}

void CWorldMain::loadnewworld( QString module ) // Load world
{
	ISerialization* archive = cPluginFactory::serializationArchiver(module);

	QString objectID;
	unsigned int i = 0;

	clConsole.send( tr("Loading World...\n") );

	QStringList types = UObjectFactory::instance()->objectTypes();

	for( uint j = 0; j < types.count(); ++j )
	{
		QString type = types[j];
		
		cDBResult res = persistentBroker->query( QString( "SELECT COUNT(*) FROM uobjectmap WHERE type = '%1'" ).arg( type ) );

		// Find out how many objects of this type are available		
		if( !res.isValid() )
			throw persistentBroker->lastError();			

		res.fetchrow();
		UINT32 count = res.getInt( 0 );
		res.free();

		clConsole.send( tr("Loading ") + QString::number( count ) + tr(" objects of type ") + type );

		res = persistentBroker->query( UObjectFactory::instance()->findSqlQuery( type ) );

		// Error Checking		
		if( !res.isValid() )
			throw persistentBroker->lastError();

		UINT32 sTime = getNormalizedTime();
		UINT16 offset;
		cUObject *object;

		progress_display progress( count );

		// Fetch row-by-row
		persistentBroker->driver()->setActiveConnection( CONN_SECOND );
		while( res.fetchrow() )
		{
			char **row = res.data();
			// do something with data
			object = UObjectFactory::instance()->createObject( type );
			offset = 2; // Skip the first two fields
			object->load( row, offset );
			++progress;
		}
		res.free();
		persistentBroker->driver()->setActiveConnection();

		clConsole.send( tr("Loaded %1 objects in %2 msecs\n\n").arg(progress.count()).arg(getNormalizedTime() - sTime) );
	}

	// Load Pages
	cPagesManager::getInstance()->load();

	// Load Temporary Effects
	archive = cPluginFactory::serializationArchiver(module);

	archive->prepareReading( "effects" );
	clConsole.send(tr("Loading Temp. Effects %1...\n").arg(archive->size()));
	progress_display progress( archive->size() );
	for ( i = 0; i < archive->size(); ++progress, ++i)
	{
		archive->readObjectID(objectID);

		cTempEffect* pTE = NULL;

		if( objectID == "TmpEff" )
			pTE = new cTmpEff;

		else if( objectID == "HIDECHAR" )
			pTE = new cDelayedHideChar( INVALID_SERIAL );

		else if( objectID == "cPythonEffect" )
			pTE = new cPythonEffect;

		else
		{
			clConsole.log( LOG_FATAL, tr( "An unknown temporary Effect class was found: %1" ).arg( objectID ) );
			continue; // an error occured..
		}

		archive->readObject( pTE );

		TempEffects::instance()->insert( pTE );
	}
	clConsole.send(tr(" Done.\n"));
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
	UI32 savestarttime = getNormalizedTime();

	if ( announce() )
	{
		cNetwork::instance()->broadcast( tr( "World data saving..." ) );
		clConsole.send(tr("Worldsave Started!\n") );
		clConsole.send(tr("items  : %1\n").arg( ItemsManager::instance()->size() ));
		clConsole.send(tr("chars  : %1\n").arg( CharsManager::instance()->size() ));
		clConsole.send(tr("effects: %1\n").arg( TempEffects::instance()->countSerializables() ));
	}

	SrvParams->flush();
	if (SrvParams->serverLog()) savelog("Server data save\n","server.log");

	// Flush old items
	persistentBroker->flushDeleteQueue();

	try
	{
		cItemsSaver* itemsSaver = new cItemsSaver;
		itemsSaver->start();
//		AllItemsIterator iterItems;
//		for( iterItems.Begin(); !iterItems.atEnd(); ++iterItems )
//			persistentBroker->saveObject( iterItems.GetData() );

		AllCharsIterator iterChars;
		for( iterChars.Begin(); !iterChars.atEnd(); ++iterChars )
			persistentBroker->saveObject( iterChars.GetData() );
		itemsSaver->wait();
		itemsSaver->join();
		delete itemsSaver;
	}
	catch( QString error )
	{
		clConsole.ChangeColor( WPC_RED );
		clConsole.send( tr("\nERROR") );
		clConsole.ChangeColor( WPC_NORMAL );
		clConsole.send( ": " + error + "\n" );
	}
	catch( ... )
	{
		clConsole.ChangeColor( WPC_RED );
		clConsole.send( tr("\nERROR") );
		clConsole.ChangeColor( WPC_NORMAL );
		clConsole.send( tr(": Unhandled Exception")+"\n" );
	}

	ISerialization *archive = cPluginFactory::serializationArchiver( module );
	archive->prepareWritting( "effects" );
	TempEffects::instance()->serialize( *archive );
	archive->close();
	delete archive;

	// Save The pages
	cPagesManager::getInstance()->save();
	
	// Save the accounts
	clConsole.PrepareProgress( tr( "Saving %1 accounts" ).arg( Accounts::instance()->count() ).latin1() );
	Accounts::instance()->save();
	clConsole.ProgressDone();
	
	if ( announce() )
	{
		cNetwork::instance()->broadcast( tr( "Worldsave Done!\n" ) );
		clConsole.PrepareProgress( tr("World saved in %1 sec").arg( (float)(((float)getNormalizedTime() - (float)savestarttime) / CLOCKS_PER_SEC ) ) );
		clConsole.ProgressDone();
	}

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
		if ( pi->multis() == INVALID_SERIAL )
		{
			cMulti* pi_multi = cMulti::findMulti( pi->pos() );
			if( !pi_multi )
			{
				Items->DeleItem(pItem);
			}
		}
	}
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
	P_CHAR pc = FindCharBySerial(charserial);
	if (pc == NULL) return false;
 	bool foundMatch = false;
	cChar::ContainerContent container(pc->content());
	cChar::ContainerContent::const_iterator it (container.begin());
	cChar::ContainerContent::const_iterator end(container.end());
	for (; it != end; ++it )
	{
		P_ITEM pci = *it;

 		if (pci != NULL)
 		{
			if( ( pci->layer() == 0x01 || pci->layer() == 0x02 ) && ( pci->container() == pc ) )
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
