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

//////////////////////////////////////////////////////////////////////
// TmpEff.cpp: implementation of temporary effects
//				cut from Wolfpack.cpp by Duke, 25.10.2000

#define RANK_ARRAY_SIZE 65535

#include "platform.h"

// Wolfpack Includes
#include "TmpEff.h"
#include "chars.h"
#include "items.h"
#include "globals.h"
#include "debug.h"
#include "srvparams.h"
#include "wolfpack.h"
#include "iserialization.h"
#include "classes.h"
#include "network.h"
#include "wpdefmanager.h"
#include "network/uosocket.h"
#include "skills.h"

#include <algorithm>
#include <typeinfo>


#undef  DBGFILE
#define DBGFILE "tmpeff.cpp"


/////////////
// Name:	reverseIncognito
// Purpose:	undo the effects of the incognito spell
// History: by AntiChrist
//			isolated from two functions by Duke, 10.6.2001
//
static void reverseIncognito(P_CHAR pc)
{

	if(pc->incognito())//let's ensure it's under incognito effect!
	{
		pc->setId(pc->xid());
		
		pc->setSkin( pc->xskin() );	// SKIN COLOR
		
		pc->name = pc->orgname().latin1();	// NAME
		
		if(pc->hairserial() != INVALID_SERIAL)//if hairs exist, restore hair style/color
		{
			P_ITEM pHair = FindItemBySerial(pc->hairserial());
			if(pHair)
			{
				if(pHair->incognito) //let's ensure it was marked as under incognito effect
				{
					//stores old hair values
					pHair->setColor( static_cast<unsigned short>(pHair->moreb1() << 8) + pHair->moreb2() );
					pHair->setId( static_cast<UI16>( pHair->moreb3() << 8 ) + pHair->moreb4() );
					pHair->incognito=false;
				}
			}
		}
		
		if(pc->beardserial()>-1)//if beard exists, restore beard style/color
		{
			P_ITEM pBeard = FindItemBySerial(pc->beardserial());
			if(pBeard)
			{
				if(pBeard->incognito) //let's ensure it was marked as under incognito effect
				{
					//restores old beard values
					pBeard->setColor( static_cast<unsigned short>(pBeard->moreb1()<<8) + pBeard->moreb2() );
					pBeard->setId( static_cast<UI16>( pBeard->moreb3() << 8 ) + pBeard->moreb4() );
					pBeard->incognito=false;
				}
			}
		}
		
		//only refresh once, when poly finished
		teleport(pc);
		int socket=calcSocketFromChar(pc);//calculate only once
		if (socket!=-1)
		{
			wornitems(socket, pc);//send update to current socket
			int j;
			for (j=0;j<now;j++)
			{//and to all inrange sockets (without re-sending to current socket)//AntiChrist
				if (perm[j] && inrange1p(pc, currchar[j]) && (j!=socket))
					wornitems(j, pc);
			}
		}
		pc->setIncognito(false);//AntiChrist
	}
}

int cTempEffect::getDest()
{
	return destSer;
}

void cTempEffect::setDest(int ser)
{
	destSer=ser;
}

int cTempEffect::getSour()
{
	return sourSer;
}

void cTempEffect::setSour(int ser)
{
	sourSer=ser;
}

void cTempEffect::setExpiretime_s(int seconds)
{
	expiretime=uiCurrentTime+(seconds*MY_CLOCKS_PER_SEC);
}

void cTempEffect::setExpiretime_ms(float milliseconds)
{
	expiretime=uiCurrentTime+(int)floor(( milliseconds / 1000 )*MY_CLOCKS_PER_SEC);
}

void cTempEffect::Serialize(ISerialization &archive)
{
	if (archive.isReading())
	{
		archive.read( "expiretime",		this->expiretime ); // exptime must be subtracted from current server clock time, so it can be recalculated on next server startup
		archive.read( "dispellable",	this->dispellable );
		archive.read( "srcserial",		this->sourSer );
		archive.read( "destserial",		this->destSer );
		this->expiretime += uiCurrentTime;
	}
	else if( archive.isWritting() )
	{
		archive.write( "expiretime",	( this->expiretime - uiCurrentTime ) ); // exptime must be subtracted from current server clock time, so it can be recalculated on next server startup
		archive.write( "dispellable",	this->dispellable );
		archive.write( "srcserial",		this->getSour() );
		archive.write( "destserial",	this->getDest() );
	}
	cSerializable::Serialize( archive );
}

void cTmpEff::Init()
{
	sourSer = INVALID_SERIAL;
	destSer = INVALID_SERIAL;
	expiretime=0;
	num=0;
	more1=255;
	more2=255;
	more3=255;
	dispellable=0;
}


void cTmpEff::On(P_CHAR pc)
{
	if (!pc)
		return;
	
	signed short tempsignedshort;

	switch(this->num)
	{
//	case 1:	pc->priv2 |= 0x02;		break;
	case 1: pc->setPriv2(pc->priv2() | 0x02); break;
//	case 2:	pc->fixedlight = SrvParams->worldBrightLevel();break;
	case 2: pc->setFixedLight( SrvParams->worldBrightLevel() );break;
	case 3:	pc->chgDex(-1 * this->more1);	break;
//	case 4:	pc->in-=this->more1;		break;
	case 4: pc->setIn( (tempsignedshort = pc->in() ) - this->more1 ); break;
//	case 5:	pc->st-=this->more1;		break;
	case 5: pc->setSt( (tempsignedshort = pc->st() ) - this->more1 );  break;
	case 6:	pc->chgDex(this->more1);	break;
//	case 7:	pc->in+=this->more1;		break;
	case 7: pc->setIn( (tempsignedshort = pc->in() ) + this->more1 );  break;
//	case 8:	pc->st+=this->more1;		break;
	case 8: pc->setSt( (tempsignedshort = pc->st() ) + this->more1 );	break;
	case 11:
//		pc->st+=this->more1;
		pc->setSt( ( tempsignedshort = pc->st() ) + this->more1 );		
		pc->chgDex(this->more2);
//		pc->in+=this->more3;
		pc->setIn( pc->in() + this->more3 );
		break;
	case 12:
//		pc->st-=this->more1;
		pc->setSt( ( tempsignedshort = pc->st() ) - this->more1 );		
		pc->chgDex(-1 * this->more2);
//		pc->in-=this->more3;
		pc->setIn( pc->in() - this->more3 );
		break;
	}
}

void cTmpEff::Off(P_CHAR pc)
{
	if (!pc)
		return;
	
	signed short tempsignedshort;

	switch(this->num)
	{
//	case 1:	pc->priv2 &= 0xFD;			break;
	case 1: pc->setPriv2( pc->priv2() & 0xFD); break;
	case 2:	pc->setFixedLight('\xFF');		break;
	case 3:	pc->chgDex(this->more1);	break;
//	case 4:	pc->in+=this->more1;		break;
	case 4: pc->setIn( ( tempsignedshort = pc->in() ) + this->more1 );		break;
//	case 5:	pc->st+=this->more1;		break;
	case 5: pc->setSt( ( tempsignedshort = pc->st() ) + this->more1 );		break;
	case 6:	pc->chgDex(-1 * this->more1);break;
//	case 7:	pc->in-=this->more1;		break;
	case 7: pc->setIn( ( tempsignedshort = pc->in() ) - this->more1 );		break;
//	case 8:	pc->st-=this->more1;		break;
	case 8: pc->setSt( ( tempsignedshort = pc->st() ) - this->more1 );		break;
	case 11:
//		pc->st-=this->more1;
		pc->setSt( ( tempsignedshort = pc->st() ) - this->more1 );		
		pc->chgDex(-1 * this->more2);
//		pc->in-=this->more3;
		pc->setIn( (tempsignedshort = pc->in() ) - this->more3 );
		break;
	case 12:
//		pc->st+=this->more1;
		pc->setSt( ( tempsignedshort = pc->st() ) + this->more1 );		
		pc->chgDex(this->more2);
//		pc->in+=this->more3;
		pc->setIn( ( tempsignedshort = pc->in() ) + this->more3 );
		break;
	}
}


void cTmpEff::Reverse()
{
	P_CHAR pc_s = FindCharBySerial(getDest());
	signed short tempsignedshort;

	switch(num)
	{
//	case 1:	pc_s->priv2 &= 0xFD;	break;
	case 1: pc_s->setPriv2(pc_s->priv2() & 0xFD);	break;
	case 2:	pc_s->setFixedLight('\xFF');break;
	case 3:	pc_s->chgDex(more1);	break;
//	case 4:	pc_s->in+=more1;		break;
	case 4: pc_s->setIn( ( tempsignedshort = pc_s->in() ) + more1 );		break;
//	case 5:	pc_s->st+=more1;		break;
	case 5: pc_s->setSt( ( tempsignedshort = pc_s->st() ) + more1 );		break;
	case 6:	pc_s->chgDex(-1 * more1);break;
//	case 7:	pc_s->in-=more1;		break;
	case 7: pc_s->setIn( ( tempsignedshort = pc_s->in() ) - more1 );		break;
//	case 8:	pc_s->st-=more1;		break;
	case 8: pc_s->setSt( ( tempsignedshort = pc_s->st() ) - more1 );		break;
	case 11:
//		pc_s->st-=more1;
		pc_s->setSt( ( tempsignedshort = pc_s->st() ) - more1 );		
		pc_s->chgDex(-1 * more2);
//		pc_s->in-=more3;
		pc_s->setIn( ( tempsignedshort = pc_s->in() ) - more3 );
		break;
	case 12:
//		pc_s->st+=more1;
		pc_s->setSt( ( tempsignedshort = pc_s->st() ) + more1 );		
		pc_s->chgDex(more2);
//		pc_s->in+=more3;
		pc_s->setIn( ( tempsignedshort = pc_s->in() ) + more3 );
		break;
	case 18: //Polymorph spell by AntiChrist
		if(pc_s->polymorph())
		{
			pc_s->setId(pc_s->xid());
			pc_s->setPolymorph(false);
			teleport(pc_s);
		}
		break;
	case 19: //Incognito spell by AntiChrist
		reverseIncognito(pc_s);
		break;

	case 21:
		int toDrop;
		toDrop = more1; //Effect->more1;
		if( ( pc_s->baseSkill(PARRYING) - toDrop ) < 0 )
			pc_s->setBaseSkill(PARRYING, 0);
		else
			pc_s->setBaseSkill(PARRYING, pc_s->baseSkill(PARRYING) - toDrop);
		
	default:
		clConsole.send("ERROR: Fallout of switch\n"); //Morrolan
		return;
	}//switch
	
	Items->CheckEquipment(pc_s); //AntiChrist - checks equipments for stats requirements
}

void cTmpEff::Expire()
{
	int k;
	signed short tempsignedshort;

	P_CHAR pc_s = FindCharBySerial(getDest());
	if (   num != 9		// grinding
		&& num != 10	// create potion
		&& num != 13	// door close
		&& num != 14	// training dummy
		&& num != 17)	// explosion
	{
		//Added by TANiS to fix errors, memory corruption and door auto-close 10-6-98
		// Check to see if it's a dead char and delete the wrong effect, or if it's just
		// a door auto-close effect and process it the right way.
		if ( !pc_s )
		{
			return;		// just remove this effect
		} //End of TANiS' change
	}

	cUOSocket *socket = NULL;
	if( pc_s ) 
		socket = pc_s->socket();

	switch(num)
	{
	case 1:
		if( pc_s->isFrozen() )
		{
//			pc_s->priv2 &= 0xFD;
			pc_s->setPriv2(pc_s->priv2() & 0xFD);
			UOXSOCKET sk=calcSocketFromChar((pc_s));
			if (sk!=-1) 
				sysmessage(sk, tr("You are no longer frozen.").latin1());
			Magic->afterParticles(38, pc_s); 			
		}
		break;
	case 2:
		pc_s->setFixedLight('\xFF');
		dolight(calcSocketFromChar(pc_s), SrvParams->worldBrightLevel());
		break;
	case 3:
		pc_s->chgDex( more1 );
		if( socket )
			socket->updateStamina();
		break;
	case 4:
//		pc_s->in += more1;
		pc_s->setIn( (tempsignedshort = pc_s->in() ) + more1 );
		if( socket )
			socket->updateMana();
		break;
	case 5:
//		pc_s->st += more1;
		pc_s->setSt( ( tempsignedshort = pc_s->st() ) + more1 );		
		// TODO: Update the health-bars for all people in range (It *HAS* changed here)
        if( socket )
			socket->updateHealth();
		break;
	case 6:
		pc_s->chgDex( -1 * more1 );
		pc_s->setStm( QMIN(pc_s->stm(), pc_s->effDex()) );
		if( socket )
			socket->updateStamina();
		break;
	case 7:
//		pc_s->in-=more1;
		pc_s->setIn( (tempsignedshort = pc_s->in() ) - more1 );
		pc_s->setMn( QMIN(pc_s->mn(), pc_s->in()) );
		if( socket )
			socket->updateMana();
		break;
	case 8:
//		pc_s->st-=more1;
		pc_s->setSt( ( tempsignedshort = pc_s->st() ) + more1 );	
		pc_s->setHp( QMIN(pc_s->hp(), pc_s->st()) );
		if( socket )
			socket->updateHealth();
		break;
	case 9:
		if( more1 == 0 )
		{
			if( more2 != 0 )
				pc_s->emote( tr( "*%1 continues grinding.*" ).arg( pc_s->name.c_str() ) );
			
			pc_s->soundEffect( 0x242 );
		}
		break;
	case 10:
		{
		pc_s = FindCharBySerial( getSour() );
		P_ITEM pMortar = FindItemBySerial( getDest() );
		//AntiChrist - to prevent crashes
		if( pMortar )
			Skills->CreatePotion( pc_s, more1, more2, pMortar );
		}
		break;
	case 11:
//		pc_s->st-=more1;
		pc_s->setSt( ( tempsignedshort = pc_s->st() ) - more1 );
		pc_s->setHp( QMIN(pc_s->hp(), pc_s->st()) );
		pc_s->chgDex(-1 * more2);
		pc_s->setStm( QMIN(pc_s->stm(), pc_s->effDex()) );
//		pc_s->in-=more3;
		pc_s->setIn( ( tempsignedshort = pc_s->in() ) - more3 );
		pc_s->setMn( QMIN(pc_s->mn(), pc_s->in()) );
		if( socket )
			socket->sendStatWindow();
		break;
	case 12:
//		pc_s->st+=more1;
		pc_s->setSt( ( tempsignedshort = pc_s->st() ) + more1 );
		pc_s->chgDex(more2);
//		pc_s->in+=more3;
		pc_s->setIn( ( tempsignedshort = pc_s->in() ) + more3 );
		if( socket )
			socket->sendStatWindow();
		break;
	case 13:
		{
			P_ITEM pDoor = FindItemBySerial( getDest() );
			if( pDoor )
			{
				if( !pDoor->dooropen )
					break;
				pDoor->dooropen = 0;
				dooruse( NULL, pDoor );
			}
			break;
		}
	case 15: //reactive armor
		pc_s->setRa(0);
		break;
	case 16: // Explosion potions
		// TODO: Let the item itself talk here...
		if( socket )
			socket->sysMessage( QString( "%1" ).arg( more3 ) );
		break;
	case 17: //Explosion potion explosion
		pc_s = FindCharBySerial( getSour() );
		explodeitem( calcSocketFromChar( pc_s ), FindItemBySerial( getDest() ) ); //explode this item
		break;
	case 18: //Polymorph spell wearoff
		if( pc_s->polymorph() )//let's ensure it's under polymorph effect!
		{
			pc_s->setId( pc_s->xid() );
		
			pc_s->setPolymorph( false );
			pc_s->resend( false );
		}
		break;
	case 19: //Incognito spell wearoff
		reverseIncognito( pc_s );
		break;

	case 21:
		int toDrop;
		toDrop = more1; //Effect->more1;
		if( ( pc_s->baseSkill(PARRYING) - toDrop ) < 0 )
			pc_s->setBaseSkill(PARRYING, 0);
		else
			pc_s->setBaseSkill(PARRYING, pc_s->baseSkill(PARRYING) - toDrop);
		break;
		
	case 33: // delayed hiding for gms after flamestrike effect
		if( socket )
			socket->sysMessage( tr( "You have hidden yourself well." ) );
		pc_s->setHidden( 1 );
		pc_s->resend();
		break;
		
	case 34: // delayed unhide for gms
		// Changed to be uniform with delayed hideing  (Aldur)
		if( socket )
			socket->sysMessage( tr( "You are now visible." ) );
		pc_s->setHidden( 0 ); 
		pc_s->resend();
		break;
		
	case 35: // heals some hp (??)
//		pc_s->hp += more1;
		pc_s->setHp( (tempsignedshort = pc_s->hp() ) + more1 );
		
		// TODO: Need to update the inrange sockets
		if( socket )
			socket->updateHealth();

		if( !more2 )
			tempeffect( pc_s, pc_s, 35, more1+1, 1, more3, 0 );
		
		break;
		
	default:
		LogErrorVar( "Fallout of switch (num = %i).", num);
		break;
	}
	Items->CheckEquipment(pc_s); //AntiChrist - checks equipments for stats requirements
}

void cTmpEff::Serialize(ISerialization &archive)
{
	if( archive.isReading() )
	{
		archive.read( "more1",			this->more1 );
		archive.read( "more2",			this->more2 );
		archive.read( "more3",			this->more3 );
		archive.read( "num",			this->num );
	}
	else if( archive.isWritting() )
	{
		archive.write( "more1",			this->more1 );
		archive.write( "more2",			this->more2 );
		archive.write( "more3",			this->more3 );
		archive.write( "num",			this->num );
	}
	cTempEffect::Serialize(archive);
}

void cPythonEffect::Expire()
{
	// Get everything before the last dot
	if( functionName.contains( "." ) )
	{
		// Find the last dot
		INT32 position = functionName.findRev( "." );
		QString sModule = functionName.left( position );
		QString sFunction = functionName.right( functionName.length() - (position+1) );

		PyObject *pModule = PyImport_ImportModule( const_cast< char* >( sModule.latin1() ) );

		if( pModule )
		{
			PyObject *pFunc = PyObject_GetAttrString( pModule, const_cast< char* >( sFunction.latin1() ) );
			if( pFunc && PyCallable_Check( pFunc ) )
			{
				PyEval_CallObject( pFunc, args );
				
				if( PyErr_Occurred() )
					PyErr_Print();
			}
		}
	}

	Py_DECREF( args );
}

void cPythonEffect::Serialize( ISerialization &archive )
{
	if( archive.isReading() )
	{
		archive.read( "functionname",	functionName );

		UINT32 pCount;
		QString type;
		archive.read( "pcount", pCount );
		args = PyTuple_New( pCount );

		for( UINT32 i = 0; i < pCount; ++i )
		{
			archive.read( QString( "pt%1" ).arg( i ), type );

			// Read an integer
			if( type == "i" )
			{
				INT32 data;
				archive.read( QString( "pv%1" ).arg( i ), data );
				PyTuple_SetItem( args, i, PyInt_FromLong( data ) );
			}
			// Read a string
			else if( type == "s" )
			{
				QString data;
				archive.read( QString( "pv%1" ).arg( i ), data );
				PyTuple_SetItem( args, i, PyString_FromString( data.latin1() ) );
			}
			else
				PyTuple_SetItem( args, i, Py_None );
		}		
	}
	else if( archive.isWritting() )
	{
		archive.write( "functionname",	functionName );

		archive.write( "pcount", PyTuple_Size( args ) );

		// Serialize the py object
		for( UINT32 i = 0; i < PyTuple_Size( args ); ++i )
		{
			if( PyInt_Check( PyTuple_GetItem( args, i ) ) )
			{
				archive.write( QString( "pt%1" ).arg( i ), QString( "i" ) );
				archive.write( QString( "pv%1" ).arg( i ), (int)PyInt_AsLong( PyTuple_GetItem( args, i ) ) );
			}
			else if( PyString_Check( PyTuple_GetItem( args, i ) ) )
			{
				archive.write( QString( "pt%1" ).arg( i ), QString( "s" ) );
				archive.write( QString( "pv%1" ).arg( i ), PyString_AsString( PyTuple_GetItem( args, i ) ) );
			}
			// Something we can't save -> Py_None on load
			else
				archive.write( QString( "pt%1" ).arg( i ), QString( "n" ) );
		}
	}

	cTempEffect::Serialize( archive );
}

/*
void cTempEffects::Off()
{
	register unsigned int i;
	for ( i = 0; i < teffects.size(); ++i)
	{
		cTempEffects *pTEs = teffects[i];
		if( typeid(*pTEs) == typeid(cTmpEff) ) // is it a "normal" temp effect
		{
			cTmpEff *pTE = dynamic_cast<cTmpEff *>(pTEs);
			
			P_CHAR pc = FindCharBySerial(pTE->getDest());
			if (pc)
				pTE->Off(pc);
		}
	}
}

void cTempEffects::On()
{
	register unsigned int i;
	for ( i = 0; i < teffects.size(); ++i)
	{
		cTempEffects *pTEs = teffects[i];
		if( typeid(*pTEs) == typeid(cTmpEff) ) // is it a "normal" temp effect
		{
			cTmpEff *pTE = dynamic_cast<cTmpEff *>(pTEs);
	
			P_CHAR pc = FindCharBySerial(pTE->getDest());
			if (pc)
				pTE->On(pc);
		}
	}
}
*/

void cTempEffects::check()
{
	while( teffects.accessMin() && teffects.accessMin()->expiretime <= uiCurrentTime )
	{
		teffects.accessMin()->Expire();
		teffects.deleteMin();
	}

/*	for debugging
	if( RandomNum(0,100) > 95 )
	{
		cScriptEffect* pTE = new cScriptEffect();
		pTE->expiretime = uiCurrentTime + 10000;
		teffects.insert( pTE );

		pTE = new cScriptEffect();
		pTE->expiretime = uiCurrentTime + 50000;
		teffects.insert( pTE );

		pTE = new cScriptEffect();
		pTE->expiretime = uiCurrentTime + 100000;
		teffects.insert( pTE );
	} */
}

/*
bool cTempEffects::Exists( P_CHAR pc_source, P_CHAR pc_dest, int num )
{
	cTmpEff *pTE;
	for( register int i = 0; i < teffects.size(); i++ )
	{
		if( teffects[i] != NULL && typeid( teffects[i] ) == typeid( cTmpEff ) )
		{
			pTE = dynamic_cast<cTmpEff *>(teffects[i]);
			if( pTE->getSour() == pc_source->serial &&
				pTE->getDest() == pc_dest->serial &&
				pTE->num == num )
				return true;
		}
	}
	return false;
}
*/

bool cTempEffects::add(P_CHAR pc_source, P_CHAR pc_dest, int num, unsigned char more1, unsigned char more2, unsigned char more3, short dur)
{
	
	int color, color1, color2, socket; //used for incognito spell
	signed short tempsignedshort;

	if ( pc_source == NULL || pc_dest == NULL )
		return false;

	cTempEffect *pTEs;
	cTmpEff *pTE;

	std::vector< cTempEffect* > teffects_ = teffects.asVector();
	std::vector< cTempEffect* >::iterator it = teffects_.begin();

	while( it != teffects_.end() )
	{
		pTEs = (*it);
		if( typeid(*pTEs) == typeid(cTmpEff) )
			pTE = dynamic_cast<cTmpEff *>(pTEs);
		else
		{
			it++;
			continue;			
		}

		if (pTE->getDest() == pc_dest->serial)
		{
			if ((pTE->num==3 && num==3)||
				(pTE->num==4 && num==4)||
				(pTE->num==5 && num==5)||
				(pTE->num==6 && num==6)||
				(pTE->num==7 && num==7)||
				(pTE->num==8 && num==8)||
				(pTE->num==11&& num==11)||
				(pTE->num==12&& num==12)||
				(pTE->num==18&& num==18)|| //added Poly reverse - AntiChrist (9/99)
				(pTE->num==21&& num==21)||
				(pTE->num==19&& num==19)|| //added Incognito reverse - AntiChrist (12/99)
				(pTE->num==18&& num==19)|| //reverse poly effect if we have to use incognito - AntiChrist (12/99)
				(pTE->num==19&& num==18) )  //reverse incognito effect if we have to use poly - AntiChrist (12/99)
			{
				pTE->Reverse();
				teffects.erase( pTEs ); // Should we continue searching?
			}
		}
		it++;
	}

	pTE = new cTmpEff;
	pTE->Init();
	pTE->setSour(pc_source->serial);
	pTE->setDest(pc_dest->serial);
	pTE->num = num;

	cUOSocket *mSock = pc_dest->socket();

	switch (num)
	{
	case 1:
//		pc_dest->priv2 |= 0x02;
		pc_dest->setPriv2(pc_dest->priv2() | 0x02);
		pTE->setExpiretime_s(pc_source->skill(MAGERY)/100);
		pTE->more1=0;
		pTE->more2=0;
		pTE->dispellable=1;
		break;
	case 2:	// night sight
		pc_dest->setFixedLight( SrvParams->worldBrightLevel() );
		dolight(calcSocketFromChar((pc_dest)), SrvParams->worldBrightLevel());
		Magic->afterParticles(6, pc_dest); // shows particles for UO:3D clients, like On OSI servers

		if(dur > 0)		// if a duration is given (potions), use that (Duke, 30.12.2000)
			pTE->setExpiretime_s(dur);
		else
			pTE->setExpiretime_s(pc_source->skill(MAGERY)*10);
		pTE->more1=0;
		pTE->more2=0;
		pTE->dispellable=1;
		break;
	case 3:
		if (pc_dest->effDex()<more1)
			more1=pc_dest->effDex();
		pc_dest->chgDex(-1 * more1);
		pc_dest->setStm( QMIN(pc_dest->stm(), pc_dest->effDex()) );
		if( mSock )
			mSock->updateStamina();
		pTE->setExpiretime_s(pc_source->skill(MAGERY)/10);
		pTE->more1=more1;
		pTE->more2=0;
		pTE->dispellable=1;
		break;
	case 4:
		if (pc_dest->in()<more1)
			more1=pc_dest->in();
//		pc_dest->in-=more1;
		pc_dest->setIn( (tempsignedshort = pc_dest->in() ) - more1 );
		pc_dest->setMn( QMIN(pc_dest->mn(), pc_dest->in()) );
		if( mSock )
			mSock->updateMana();
		pTE->setExpiretime_s(pc_source->skill(MAGERY)/10);
		pTE->more1=more1;
		pTE->more2=0;
		pTE->dispellable=1;
		break;
	case 5:
		if (pc_dest->st()<more1)
			more1=pc_dest->st();
//		pc_dest->st-=more1;
		pc_dest->setSt( ( tempsignedshort = pc_dest->st() ) - more1 );
		pc_dest->setHp( QMIN(pc_dest->hp(), pc_dest->st()) );
		if( mSock )
			mSock->updateHealth();
		pTE->setExpiretime_s(pc_source->skill(MAGERY)/10);
		pTE->more1=more1;
		pTE->more2=0;
		pTE->dispellable=1;
		break;
	case 6:
		if (pc_dest->effDex()+more1>250)
			more1=250-pc_dest->effDex();
		pc_dest->chgDex(more1);

		if( mSock )
			mSock->updateStamina();
		
		// if a duration is given (potions), use that
		if( dur )		
			pTE->setExpiretime_s(dur);
		else
			pTE->setExpiretime_s(pc_source->skill(MAGERY)/10);

		pTE->more1=more1;
		pTE->more2=0;
		pTE->dispellable=1;
		break;
	case 7:
		if (pc_dest->in()+more1>255)
			more1=pc_dest->in()-255;
//		pc_dest->in+=more1;
		pc_dest->setIn( (tempsignedshort = pc_dest->in() ) + more1 );	
		if( mSock )
			mSock->updateMana();
		pTE->setExpiretime_s(pc_source->skill(MAGERY)/10);
		pTE->more1=more1;
		pTE->more2=0;
		pTE->dispellable=1;
		break;
	case 8:
		if (pc_dest->st()+more1>255)
			more1=pc_dest->st()-255;
//		pc_dest->st+=more1;
		pc_dest->setSt( ( tempsignedshort = pc_dest->st() ) + more1 ); 
		if( mSock )
			mSock->updateHealth();

		// if a duration is given (potions), use that
		if( dur )		
			pTE->setExpiretime_s(dur);
		else			// else use caster's skill
			pTE->setExpiretime_s(pc_source->skill(MAGERY)/10);
		pTE->more1=more1;
		pTE->more2=0;
		pTE->dispellable=1;
		break;
	case 9:
		pTE->setExpiretime_s(more2);
		pTE->more1=more1;
		pTE->more2=more2;
		break;
	case 10:
		pTE->setExpiretime_s(12);
		pTE->more1=more1;
		pTE->more2=more2;
		break;
	case 11: // Bless
		if (pc_dest->st()+more1>255)
//			more1=pc_dest->st-255;
			pc_dest->setSt( ( tempsignedshort = pc_dest->st() ) - 255 ); 
		if (pc_dest->effDex()+more2>250)
			more2=250-pc_dest->effDex();
		if (pc_dest->in()+more3>255)
			more3=pc_dest->in()-255;
//		pc_dest->st+=more1;
		pc_dest->setSt( ( tempsignedshort = pc_dest->st() ) + more1 ); 
		pc_dest->chgDex(more2);
//		pc_dest->in+=more3;
		pc_dest->setIn( ( tempsignedshort = pc_dest->in() ) + more3 );

		// TODO: Send to other players here.
		if( mSock )
			mSock->sendStatWindow();

		pTE->setExpiretime_s(pc_source->skill(MAGERY)/10);
		pTE->more1=more1;
		pTE->more2=more2;
		pTE->more3=more3;
		pTE->dispellable=1;
		break;
	case 12: // Curse
		if (pc_dest->st()<more1)
			more1=pc_dest->st();			
		if (pc_dest->effDex()<more2)
			more2=pc_dest->effDex();
		if (pc_dest->in()<more3)
			more3=pc_dest->in();
//		pc_dest->st-=more1;
		pc_dest->setSt( ( tempsignedshort = pc_dest->st() ) - more1 ); 
		pc_dest->chgDex(-1 * more2);
//		pc_dest->in-=more3;
		pc_dest->setIn( ( tempsignedshort = pc_dest->in() ) - more3 );

		// TODO: Send to other players here.
		if( mSock )
			mSock->sendStatWindow();

		pTE->setExpiretime_s(pc_source->skill(MAGERY)/10);
		pTE->more1=more1;
		pTE->more2=more2;
		pTE->more3=more3;
		pTE->dispellable=1;
		break;
	case 15: // Reactive armor
		pTE->setExpiretime_s(pc_source->skill(MAGERY)/10);
		pTE->dispellable=1;
		break;
	case 16: //Explosion potions	Tauriel
		pTE->setExpiretime_s(more2);
		pTE->more1=more1; //item/potion
		pTE->more2=more2; //seconds
		pTE->more3=more3; //countdown#
		break;
	case 18: //Polymorph - AntiChrist 09/99
		pTE->setExpiretime_s(polyduration);

		int c1,b,k;
		//Grey flag when polymorphed - AntiChrist (9/99)
		pc_dest->setCrimflag((polyduration*MY_CLOCKS_PER_SEC)+uiCurrentTime);
//		if(pc_dest->onHorse()) 
//			k = unmounthorse(calcSocketFromChar(pc_dest));
		if( pc_dest->onHorse() )
			pc_dest->unmount();
		k=(more1<<8)+more2;

		pc_dest->setXid(pc_dest->id());//let's backup previous id

		if (k>=0x000 && k<=0x3e1) // lord binary, body-values >0x3e crash the client
		{
			pc_dest->setId(k); // allow only non crashing ones

			c1 = pc_dest->skin(); // transparency for monsters allowed, not for players,
														 // if polymorphing from monster to player we have to switch from transparent to semi-transparent
			b=c1&0x4000;
			if (b==16384 && (k >= 0x0190 && k <= 0x03e1))
			{
				if (c1!=0x8000)
				{
					pc_dest->setSkin(0xF000);
					pc_dest->setXSkin(0xF000);
				}
			}
		}
		pc_dest->setPolymorph(true);
		break;
	case 19://incognito spell - AntiChrist (10/99)//revised by AntiChrist - 9/12/99
		{
			pTE->setExpiretime_s(90);

			//AntiChrist 11/11/99
			//If char is already under polymorph effect, let's reverse the
			//polymorph effect to avoid problems
			if(pc_dest->polymorph())
			{
				pc_dest->setId(pc_dest->xid());
				pc_dest->setPolymorph(false);
				teleport(pc_dest);
			}
			int j;

			//first: let's search for beard and hair serial
			//(we could use alredy saved serials...but it's better
			//to recalculate them)
			pc_dest->setHairSerial(INVALID_SERIAL);
			pc_dest->setBeardSerial(INVALID_SERIAL);

			P_ITEM pi;
			unsigned int ci;
			vector<SERIAL> vecContainer = contsp.getData(pc_dest->serial);
			for ( ci = 0; ci < vecContainer.size(); ci++)
			{
				pi = FindItemBySerial(vecContainer[ci]);
				if(pi->layer()==0x10)//beard
					pc_dest->setBeardSerial(pi->serial);
				if(pi->layer()==0x0B)//hairs
					pc_dest->setHairSerial(pi->serial);
			}
			// ------ SEX ------
			pc_dest->setXid(pc_dest->id());

			//if we already have a beard..can't turn to female
			if(pc_dest->beardserial() != INVALID_SERIAL)
			{//if character has a beard...only male
				pc_dest->setId(0x0190);//male
			}
			else
			{//if no beard let's randomly change
				if((rand()%2)==0) pc_dest->setId(0x0190);//male
				else pc_dest->setId(0x0191);//or female
			}

			// --- SKINCOLOR ---
			pc_dest->setXSkin(pc_dest->skin());
			color=rand()%6;
			switch(color)
			{
				case 0:				pc_dest->setSkin(0x83EA);				break;
				case 1:				pc_dest->setSkin(0x8405);				break;
				case 2:				pc_dest->setSkin(0x83EF);				break;
				case 3:				pc_dest->setSkin(0x83F5);				break;
				case 4:				pc_dest->setSkin(0x841C);				break;
				case 5:				pc_dest->setSkin(0x83FB);				break;
				default:												break;
			}

			// ------ NAME -----
			pc_dest->setOrgname( pc_dest->name.c_str() );

			if(pc_dest->id()==0x0190) 
				pc_dest->name = DefManager->getRandomListEntry( "1" );
			else 
				pc_dest->name = DefManager->getRandomListEntry( "2" );//get a name from female list

			//
			//damn..this formula seems to include also some bad color...
			//i'll test this later
			//AntiChrist
			//
			//use unique color for hair&beard
			//color=0x044E+(rand()%(0x04AD-0x044E));

			//i had to track down some valid value
			//for hair/beard colors, cause that
			//formula contained some bad value =(
			//but now it works perfectly :)
			//AntiChrist-11/11/99
			color=rand()%8;
			switch(color)
			{
				case 0:	color=0x044e;	break;
				case 1:	color=0x0455;	break;
				case 2:	color=0x045e;	break;
				case 3:	color=0x0466;	break;
				case 4:	color=0x046a;	break;
				case 5:	color=0x0476;	break;
				case 6:	color=0x0473;	break;
				case 7:	color=0x047c;	break;
				default://it should not go here...but..who nows =P
					color=0x044e;
			}
			color1=color>>8;
			color2=color%256;

			// ------ HAIR -----
			if(pc_dest->hairserial() != INVALID_SERIAL)//if hairs exist
			{//change hair style/color
				P_ITEM pHair = FindItemBySerial(pc_dest->hairserial());
				if(pHair)
				{
					//stores old hair values...
					pHair->setMoreb1( static_cast<unsigned char>(pHair->color()>>8) );
					pHair->setMoreb2( static_cast<unsigned char>(pHair->color()%256) );
					pHair->setMoreb3( ((pHair->id()&0xFF00) >> 8) );
					pHair->setMoreb4( pHair->id()&0x00FF );

					//and change them with random ones
					switch( rand() % 10 )
					{
						case 0: pHair->setId( 0x203B ); break;
						case 1: pHair->setId( 0x203C ); break;
						case 2:	pHair->setId( 0x203D ); break;
						case 3:	pHair->setId( 0x2044 ); break;
						case 4:	pHair->setId( 0x2045 ); break;
						case 5:	pHair->setId( 0x2046 ); break;
						case 6:	pHair->setId( 0x2047 ); break;
						case 7:	pHair->setId( 0x2048 ); break;
						case 8:	pHair->setId( 0x2049 ); break;
						case 9:	pHair->setId( 0x204A ); break;
						default: pHair->setId( 0x204A ); break;
					}

					//random color
					pHair->setColor( color );
					//let's check for invalid values
					if ( pHair->color() < 0x044E || pHair->color() > 0x04AD )
					{
						pHair->setColor( 0x044E );
					}

					pHair->incognito = true;//AntiChrist
				}//if j!=-1
			}//if hairserial!=-1


			// -------- BEARD --------
			if(pc_dest->id()==0x0190)// only if a man
			if(pc_dest->beardserial() != INVALID_SERIAL)//if beard exist
			{//change beard style/color
				P_ITEM pBeard = FindItemBySerial(pc_dest->beardserial());
				if(pBeard)
				{
					//clConsole.send("BEARD FOUND!!\n");
					//stores old beard values
					pBeard->setMoreb1( static_cast<unsigned char>(pBeard->color()>>8) );
					pBeard->setMoreb2( static_cast<unsigned char>(pBeard->color()%256) );
					pBeard->setMoreb3( static_cast<unsigned char>(pBeard->id() >> 8) );
					pBeard->setMoreb4( static_cast<unsigned char>(pBeard->id()%256) );

					//changes them with random ones
					switch(rand()%7)
					{
						case 0: pBeard->setId( 0x203E ); break;
						case 1: pBeard->setId( 0x203F ); break;
						case 2:	pBeard->setId( 0x2040 ); break;
						case 3:	pBeard->setId( 0x2041 ); break;
						case 4:	pBeard->setId( 0x204B ); break;
						case 5:	pBeard->setId( 0x204C ); break;
						case 6:	pBeard->setId( 0x204D ); break;
						default:pBeard->setId( 0x204D ); break;
					}

					//random color
					pBeard->setColor( color );

					if ( pBeard->color() < 0x044E || pBeard->color() > 0x04AD )
					{
						pBeard->setColor( 0x044E );
					}

					pBeard->incognito=true;//AntiChrist
				}//if j!=-1
			}//if beardserial!=-1

			//only refresh once
			teleport(pc_dest);

			socket=calcSocketFromChar(pc_dest);

			wornitems(socket, pc_dest);//send update to current socket

			for (j=0;j<now;j++)
			{//and to all inrange sockets (without re-sending to current socket)//AntiChrist
				if (perm[j] && inrange1p(pc_dest, currchar[j]) && (j!=socket))
					wornitems(j, pc_dest);
			}

			pc_dest->setIncognito(true);//AntiChrist
		}
		break;

	case 20: // LSD potions, LB 5'th nov 1999
		k=calcSocketFromChar(pc_source);
		if (k==-1) return 0;
		sysmessage(k,"Hmmm, tasty, LSD");
		LSD[k]=1;
		pc_source->setHp( pc_source->st() );
		pc_source->setMn( pc_source->in() );
		impowncreate(k, pc_source, 0);
		pTE->setExpiretime_s(90);
		break;

	case 21:		// protection
		pTE->setExpiretime_s(12);
		pTE->dispellable=1;
		pTE->more1=more1;
		pc_dest->setBaseSkill(PARRYING, pc_dest->baseSkill(PARRYING) + more1);
		break;

	case 33: // gm hiding
		pTE->setExpiretime_s(more1);
		pTE->more1=more1;
		break;

	case 34: // gm un hiding
		pTE->setExpiretime_s(more1);
		pTE->more1=more1;
		break;

	case 35:
	    //heals some hp in 9 seconds - Solarin
		k=calcSocketFromChar(pc_source);
		pTE->setExpiretime_s(more3);
		if (!more2)
			sysmessage(k,"You start healing yourself...");
		else
			sysmessage(k,"You continue to heal...");

		pTE->more1=more1; //hp to give back
		pTE->more2=more2; //# of call to decide which string to display on checktempeffect
		pTE->more3=more3; //delay#
		break;

	case 44: // special fencing paralyzation -Frazurbluu-
//		pc_dest->priv2 |= 0x02;
		pc_dest->setPriv2(pc_dest->priv2() | 0x02);
		pTE->setExpiretime_s(5);
		pTE->num=1;
		pTE->more1=0;
		pTE->more2=0;
		break;

	case 45: // special sword use concussion intelligence loss -Frazurbluu-
		if (pc_dest->in()<more1)
			more1=pc_dest->in();
//		pc_dest->in-=more1;
		pc_dest->setIn( ( tempsignedshort = pc_dest->in() ) - more1 );
		pc_dest->setMn( QMIN( pc_dest->mn(), pc_dest->in() ) );
		if( mSock )
			mSock->updateMana();
		pTE->setExpiretime_s(30);
		pTE->num=4;
		pTE->more1=more1;
		pTE->more2=0;
		break;

	default:
		LogErrorVar("Fallout of switch! Value (%d)",num);
		return 0;
	}

	cTempEffects::getInstance()->teffects.insert( pTE );
	return 1;
}

bool cTempEffects::add(P_CHAR pc_source, P_ITEM piDest, int num, unsigned char more1, unsigned char more2, unsigned char more3)
{
//	if (pc_source == NULL)
//		return 0;

	cTmpEff *pTE = new cTmpEff;
	pTE->Init();
	if ( pc_source != NULL )
		pTE->setSour(pc_source->serial);
	else
		pTE->setSour( INVALID_SERIAL );
	pTE->setDest(piDest->serial);
	pTE->num=num;
	switch (num)
	{
	case 10:
		pTE->setExpiretime_s(12);
		pTE->more1=more1;
		pTE->more2=more2;
		break;
	case 13:
		if (piDest->dooropen)
		{
			piDest->dooropen = 0;
			return 0;
		}
		pTE->setExpiretime_s(10);
		piDest->dooropen=1;
		break;
	case 14: //Tauriel training dummies swing for 5(?) seconds
		pTE->setExpiretime_s(5);
		pTE->more2=0;
		break;
	case 17: //Explosion potion (explosion)	Tauriel (explode in 4 seconds)
		pTE->setExpiretime_s(4);
		pTE->more1=more1;
		pTE->more2=more2;
		break;
	default:
		clConsole.send("ERROR: Fallout of switch statement without default. TmpEff.cpp, tempeffect2()\n"); //Morrolan
		delete pTE; // do not leak this resource.
		return false;
	}
	this->teffects.insert(pTE);
	return true;
}

void cTempEffects::serialize(ISerialization &archive)
{
	std::vector< cTempEffect* > teffects_ = teffects.asVector();
	std::vector< cTempEffect* >::iterator it = teffects_.begin();
	while( it != teffects_.end() )
	{
		if( (*it)->isSerializable() )
			archive.writeObject( (*it) );
		++it;
	}
}

void cTempEffects::dispel( P_CHAR pc_dest )
{
	std::vector< cTempEffect* > teffects_ = teffects.asVector();
	std::vector< cTempEffect* >::iterator i = teffects_.begin();
	for( i = teffects_.begin(); i != teffects_.end(); i++ )
		if( i != NULL && (*i) != NULL && (*i)->dispellable && (*i)->getDest() == pc_dest->serial )
		{
			(*i)->Off( pc_dest );
			teffects.erase( (*i) );
		}
}

unsigned char tempeffect(P_CHAR pc_source, P_CHAR pc_dest, int num, unsigned char more1, unsigned char more2, unsigned char more3, short dur)
{
	return cTempEffects::getInstance()->add(pc_source, pc_dest, num, more1, more2, more3, dur);
}

unsigned char tempeffect2(P_CHAR source, P_ITEM piDest, int num, unsigned char more1, unsigned char more2, unsigned char more3)
{
	return cTempEffects::getInstance()->add(source, piDest, num, more1, more2, more3);
}

// cTimedSpellAction
cTimedSpellAction::cTimedSpellAction( SERIAL serial, UI08 nAction )
{
	if( !isCharSerial( serial ) )
	{
		character = INVALID_SERIAL;
		return;
	}

	// Display the animation once
	P_CHAR pc = FindCharBySerial( serial );
	pc->action( nAction );

	// Save our data
	character = serial;
	action = nAction;
	serializable = false;
	expiretime = uiCurrentTime + 750;
}

// Insert a new action if there are more than 75 ticks left
void cTimedSpellAction::Expire()
{
	if( character != INVALID_SERIAL )
		cTempEffects::getInstance()->teffects.insert( new cTimedSpellAction( character, action ) );
}

// Singleton
cTempEffects cTempEffects::instance;


//  Fibonacci Heap implementation
//
//  18.07.2002, Joerg Stueckler (sereg)
//  email: joe.di@gmx.de
//
//  based on "Algorithms and Data Structures", T.Ottmann/P.Widmayer, 4th Edition
//  Ch. 6.1.5., page 410
cTempEffect*	cTmpEffFibHeap::accessMin() // O(1)!
{
	return head;
}

void			cTmpEffFibHeap::insert( cTempEffect* pT )	// O(1) too!
{
	cTmpEffFibHeap toMeld = cTmpEffFibHeap( pT );
	this->meld( toMeld );
}

cTempEffect*	cTmpEffFibHeap::meld( cTmpEffFibHeap &nFheap )	// O(1) !!!
{
	if( !this->head )
	{
		this->head = nFheap.head;
	}
	else if( this->head && nFheap.head )
	{
		cTempEffect* minHead = NULL;
		cTempEffect* maxHead = NULL;
		if( this->head->expiretime < nFheap.head->expiretime )
		{
			minHead = this->head;
			maxHead = nFheap.head;
		}
		else
		{
			maxHead = this->head;
			minHead = nFheap.head;
		}
		minHead->left->right = maxHead;
		cTempEffect* rightElement = maxHead->left;
		maxHead->left = minHead->left;
		minHead->left = rightElement;
		rightElement->right = minHead;

		this->head = minHead;
	}
	
	return this->head;
}

void			cTmpEffFibHeap::deleteMin()		// O( lg N )
{
	if( this->head )
	{
		if( this->head->left == this->head ) // only one element in the heap
		{
			delete this->head;
			this->head = NULL;
			return;
		}
		else
		{
			if( this->head->son ) // min element has a son
			{
				// replace the head with its sons in the list
				this->head->right->left = this->head->son->left;
				this->head->son->left->right = this->head->right;
				this->head->son->left = this->head->left;
				this->head->left->right = this->head->son;

				cTempEffect* newHead = this->head->son;
				delete this->head;
				this->head = newHead;
			}
			else
			{
				// unlink the head from the list
				this->head->right->left = this->head->left;
				this->head->left->right = this->head->right;

				cTempEffect* newHead = this->head->right;
				delete this->head;
				this->head = newHead;
			}

			// now we have to iterate through the root list to find two trees with equal rank
			// we use an array to find collisions.
			cTempEffect* ranks[RANK_ARRAY_SIZE];
			memset( &ranks, 0, sizeof( ranks ) );

			cTempEffect* it = this->head;
			cTempEffect* minNode = it;
			cTempEffect* coll = NULL;

			bool collision = false;
			do
			{
				collision = false;
				it->father = NULL; // reset the fathers of the inserted sons of head (see above)
				if( it->expiretime < minNode->expiretime )
					minNode = it; // find the min node by the way :)

				if( ranks[ it->rank ] && ranks[ it->rank ] != it ) // there is already another address stored in the array!
				{
					coll = ranks[ it->rank ];
					ranks[ it->rank ] = NULL;

					if( coll->expiretime < it->expiretime )
					{
						cTempEffect* templeft = NULL;
						cTempEffect* tempright = NULL;
						if( coll->left == it )
						{
							if( coll->right != it )
							{
								templeft = it->left;
								tempright = coll->right;

								templeft->right = coll;
								tempright->left = it;
								it->left = coll;
								it->right = tempright;
								coll->left = templeft;
								coll->right = it;
							}
						}
						else if( coll->right == it )
						{
							templeft = coll->left;
							tempright = it->right;

							templeft->right = it;
							tempright->left = coll;
							it->left = templeft;
							it->right = coll;
							coll->left = it;
							coll->right = tempright;
						}
						else
						{
							// swap the min element into it position
							templeft = coll->left;
							tempright = coll->right;

							it->left->right = coll;
							it->right->left = coll;
							coll->right = it->right;
							coll->left = it->left;
						
							templeft->right = it;
							tempright->left = it;
							it->right = tempright;
							it->left = templeft;
						}
						templeft = it;
						it = coll;
						coll = templeft;
					}

					// delete max element (coll) out of the list
					// if coll was head move the head one node right
					if( coll == this->head )
						this->head = coll->right;
					coll->left->right = coll->right;
					coll->right->left = coll->left;

					// insert max into the list of sons of min
					if( it->son )
					{
						cTempEffect* rightElement = it->son->left;
						coll->left = rightElement;
						it->son->left = coll;
						rightElement->right = coll;
						coll->right = it->son;
					}
					else
					{
						it->son = coll;
						coll->left = coll;
						coll->right = coll;
					}
					coll->father = it;

					it->rank = it->rank + 1;

					it->marker = false; // reset the marker cause the node got an additional son

					it = it->left;
					collision = true;
				}
				else
				{
					ranks[ it->rank ] = it;
				}
				it = it->right;
			} while( it != this->head || collision);

			this->head = minNode;
		}
	}
}

void			cTmpEffFibHeap::decrease( cTempEffect* pT, int diffTime ) // O( 1 )
{
	if( !pT )
		return;

	pT->expiretime -= diffTime;
	if( pT->father )
	{
		// first cut pT out and insert it into the root list
		// mark the father, if the father is already marked,
		// run decrease(...) recursively with diffTime = 0
		pT->left->right = pT->right;
		pT->right->left = pT->left;

		this->insert( pT );

		if( pT->father->marker )
			this->decrease( pT->father, 0 );
		else
			pT->father->marker = true;

		pT->father = NULL;
		pT->marker = false;
	}
}

void			cTmpEffFibHeap::erase( cTempEffect *pT ) // O( lg N )
{
	this->decrease( pT, (pT->expiretime - this->head->expiretime) + 1 );
	this->deleteMin();
}

std::vector< cTempEffect* > cTempEffect::asVector()
{
	std::vector< cTempEffect* > list_ = std::vector< cTempEffect* >();
	
	cTempEffect* iterNode = this;

	do
	{
		list_.push_back( iterNode );
		if( iterNode->son )
		{
			std::vector< cTempEffect* > sons_ = iterNode->son->asVector();
			list_.insert( list_.end(), sons_.begin(), sons_.end() );
		}
		iterNode = iterNode->right;
	} while( iterNode != this );

	return list_;
}

std::vector< cTempEffect* >	cTmpEffFibHeap::asVector()
{
	if( this->head )
		return this->head->asVector();
	else
		return std::vector< cTempEffect* >();
}