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

#include <algorithm>
#include <typeinfo>


#undef  DBGFILE
#define DBGFILE "TmpEff.cpp"


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
		pc->setId(pc->xid);
		
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
	
	switch(this->num)
	{
	case 1:	pc->priv2 |= 0x02;		break;
	case 2:	pc->fixedlight = SrvParams->worldBrightLevel();break;
	case 3:	pc->chgDex(-1 * this->more1);	break;
	case 4:	pc->in-=this->more1;		break;
	case 5:	pc->st-=this->more1;		break;
	case 6:	pc->chgDex(this->more1);	break;
	case 7:	pc->in+=this->more1;		break;
	case 8:	pc->st+=this->more1;		break;
	case 11:
		pc->st+=this->more1;
		pc->chgDex(this->more2);
		pc->in+=this->more3;
		break;
	case 12:
		pc->st-=this->more1;
		pc->chgDex(-1 * this->more2);
		pc->in-=this->more3;
		break;
	}
}

void cTmpEff::Off(P_CHAR pc)
{
	if (!pc)
		return;

	switch(this->num)
	{
	case 1:	pc->priv2 &= 0xFD;			break;
	case 2:	pc->fixedlight='\xFF';		break;
	case 3:	pc->chgDex(this->more1);	break;
	case 4:	pc->in+=this->more1;		break;
	case 5:	pc->st+=this->more1;		break;
	case 6:	pc->chgDex(-1 * this->more1);break;
	case 7:	pc->in-=this->more1;		break;
	case 8:	pc->st-=this->more1;		break;
	case 11:
		pc->st-=this->more1;
		pc->chgDex(-1 * this->more2);
		pc->in-=this->more3;
		break;
	case 12:
		pc->st+=this->more1;
		pc->chgDex(this->more2);
		pc->in+=this->more3;
		break;
	}
}


void cTmpEff::Reverse()
{
	P_CHAR pc_s = FindCharBySerial(getDest());

	switch(num)
	{
	case 1:	pc_s->priv2 &= 0xFD;	break;
	case 2:	pc_s->fixedlight='\xFF';break;
	case 3:	pc_s->chgDex(more1);	break;
	case 4:	pc_s->in+=more1;		break;
	case 5:	pc_s->st+=more1;		break;
	case 6:	pc_s->chgDex(-1 * more1);break;
	case 7:	pc_s->in-=more1;		break;
	case 8:	pc_s->st-=more1;		break;
	case 11:
		pc_s->st-=more1;
		pc_s->chgDex(-1 * more2);
		pc_s->in-=more3;
		break;
	case 12:
		pc_s->st+=more1;
		pc_s->chgDex(more2);
		pc_s->in+=more3;
		break;
	case 18: //Polymorph spell by AntiChrist
		if(pc_s->polymorph())
		{
			pc_s->setId(pc_s->xid);
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
		if ( pc_s == NULL )
		{
			return;		// just remove this effect
		} //End of TANiS' change
	}

	switch(num)
	{
	case 1:
		if (pc_s->priv2&0x02)
		{
			pc_s->priv2 &= 0xFD;
			UOXSOCKET sk=calcSocketFromChar((pc_s));
			if (sk!=-1) 
				sysmessage(sk, tr("You are no longer frozen.").latin1());
			Magic->afterParticles(38, pc_s); 			
		}
		break;
	case 2:
		pc_s->fixedlight='\xFF';
		dolight(calcSocketFromChar(pc_s), SrvParams->worldBrightLevel());
		break;
	case 3:
		pc_s->chgDex(more1);
		statwindow(calcSocketFromChar(pc_s), pc_s);
		break;
	case 4:
		pc_s->in+=more1;
		statwindow(calcSocketFromChar(pc_s), pc_s);
		break;
	case 5:
		pc_s->st+=more1;
		statwindow(calcSocketFromChar(pc_s), pc_s);
		break;
	case 6:
		pc_s->chgDex(-1 * more1);
		pc_s->stm=min(pc_s->stm, pc_s->effDex());
		statwindow(calcSocketFromChar(pc_s), pc_s);
		break;
	case 7:
		pc_s->in-=more1;
		pc_s->mn=min(pc_s->mn, pc_s->in);
		statwindow(calcSocketFromChar(pc_s), pc_s);
		break;
	case 8:
		pc_s->st-=more1;
		pc_s->hp=min(pc_s->hp, pc_s->st);
		statwindow(calcSocketFromChar(pc_s), pc_s);
		break;
	case 9:
		if (more1 == 0)
		{
			if (more2!=0)
			{
				sprintf((char*)temp, "*%s continues grinding.*", pc_s->name.c_str());
				npcemoteall(pc_s, (char*)temp,1);
			}
			soundeffect2(pc_s, 0x0242);
		}
		break;
	case 10:
		{
		pc_s = FindCharBySerial(getSour());
		P_ITEM pMortar = FindItemBySerial(getDest());
		if(pMortar != NULL) //AntiChrist - to prevent crashes
			Skills->CreatePotion(pc_s, more1, more2, pMortar);
		}
		break;
	case 11:
		pc_s->st-=more1;
		pc_s->hp=min(pc_s->hp, pc_s->st);
		pc_s->chgDex(-1 * more2);
		pc_s->stm=min(pc_s->stm, pc_s->effDex());
		pc_s->in-=more3;
		pc_s->mn=min(pc_s->mn, pc_s->in);
		statwindow(calcSocketFromChar(pc_s), pc_s);
		break;
	case 12:
		pc_s->st+=more1;
		pc_s->chgDex(more2);
		pc_s->in+=more3;
		statwindow(calcSocketFromChar(pc_s), pc_s);
		break;
	case 13:
		{
			P_ITEM pDoor = FindItemBySerial(getDest());// door
			if (pDoor)
			{
				if (pDoor->dooropen==0)
					break;
				pDoor->dooropen=0;
				dooruse(INVALID_UOXSOCKET, pDoor);
			}
			break;
		}
	case 14: //- training dummies Tauriel check to see if item moved or not before searching for it
		{
			P_ITEM pTrainDummy = FindItemBySerial(getDest());
			if (pTrainDummy)
			{
				if (pTrainDummy->id()==0x1071)
				{
					pTrainDummy->setId(0x1070);
					pTrainDummy->gatetime=0;
					RefreshItem(pTrainDummy);//AntiChrist
				}
				else if (pTrainDummy->id()==0x1075)
				{
					pTrainDummy->setId(0x1074);
					pTrainDummy->gatetime=0;
					RefreshItem(pTrainDummy);//AntiChrist
				}
			}
		}
		break;
	case 15: //reactive armor
		pc_s->setRa(0);
		break;
	case 16: //Explosion potion messages	Tauriel
		sprintf((char*)temp, "%i", more3);
		sysmessage(calcSocketFromChar(pc_s), (char*)temp); // crashfix, LB
		break;
	case 17: //Explosion potion explosion	Tauriel			
		pc_s = FindCharBySerial(getSour());
		explodeitem(calcSocketFromChar((pc_s)), FindItemBySerial(getDest())); //explode this item
		break;
	case 18: //Polymorph spell by AntiChrist 9/99
		if(pc_s->polymorph())//let's ensure it's under polymorph effect!
		{
			pc_s->setId(pc_s->xid);
			pc_s->setPolymorph(false);
			teleport(pc_s);
		}
		break;
	case 19: //Incognito spell by AntiChrist 12/99
		reverseIncognito(pc_s);
		break;
		
	case 20: // LSD potions, LB 5'th nov 1999
		{
			k=calcSocketFromChar((pc_s));
			if (k==-1) return;
			LSD[k]=0;
			sysmessage(k, tr("LSD has worn off").latin1());
			pc_s->stm=3; // stamina near 0
			pc_s->mn=3;
			pc_s->hp=pc_s->hp/7;
			impowncreate(k, pc_s, 0);
			all_items(k); // absolutely necassairy here !!!
			AllCharsIterator it;
			for (it.Begin(); !it.atEnd(); it++) // that hurts, but there's no other good way
			{
				P_CHAR pc = it.GetData();
				if (chardist( pc_s, pc ) < 15 && ( online(pc) || pc->isNpc() ) ) 
					updatechar(pc);
			}
		}
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
		k=calcSocketFromChar((pc_s));
		sysmessage(k, tr("You have hidden yourself well."));
		pc_s->setHidden( 1 );
		updatechar(pc_s);
		break;
		
	case 34: // delayed unhide for gms
		// Changed to be uniform with delayed hideing  (Aldur)
		k = calcSocketFromChar((pc_s)); 
		sysmessage(k, tr("You are now visible.")); 
		pc_s->setHidden( 0 ); 
		updatechar(pc_s); 
		break;
		
	case 35: //heals some pf - solarin
		int iHp;
		iHp=(int)more1;
		pc_s->hp+=iHp;
		updatestats(pc_s, 0);
		if (!more2)
			tempeffect(pc_s, pc_s, 35, more1+1, 1, more3, 0);
		break;
		
	default:
		LogErrorVar("Fallout of switch (num = %i).", num);
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

void cScriptEffect::Expire()
{
	;// here ya go darkstorm :P
}

void cScriptEffect::Serialize(ISerialization &archive)
{
	if( archive.isReading() )
	{
		archive.read( "scriptname",		this->scriptname );
		archive.read( "functionname",	this->functionname );
	}
	else if( archive.isWritting() )
	{
		archive.write( "scriptname",	this->scriptname );
		archive.write( "functionname",	this->functionname );
	}
	cTempEffect::Serialize(archive);
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

void cTempEffects::Check()
{
	if ( !teffects.empty() && (*teffects.begin())->expiretime <= uiCurrentTime )
	{
		cTempEffect *pTEs = *teffects.begin();

		pTEs->Expire();
		pop_heap( teffects.begin(), teffects.end(), ComparePredicate() ); // still sorted.
		delete pTEs;
	}
}

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

bool cTempEffects::Add(P_CHAR pc_source, P_CHAR pc_dest, int num, unsigned char more1, unsigned char more2, unsigned char more3, short dur)
{
	
	int color, color1, color2, socket; //used for incognito spell

	if ( pc_source == NULL || pc_dest == NULL )
		return false;

	cTempEffect *pTEs;
	cTmpEff *pTE;
	register unsigned int i;
	for ( i = 0; i < teffects.size(); ++i)	// If there is already an effect of the same or similar kind, reverse it first (Duke)
	{
		pTEs = teffects[i];
		if( typeid(*pTEs) == typeid(cTmpEff) )
			pTE = dynamic_cast<cTmpEff *>(pTEs);
		else
			continue;

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
				teffects.erase( teffects.begin() + i ); // Should we continue searching?
			}
		}
	}

	make_heap( teffects.begin(), teffects.end(), ComparePredicate() );

	pTE = new cTmpEff;
	pTE->Init();
	pTE->setSour(pc_source->serial);
	pTE->setDest(pc_dest->serial);
	pTE->num = num;

	switch (num)
	{
	case 1:
		pc_dest->priv2 |= 0x02;
		pTE->setExpiretime_s(pc_source->skill(MAGERY)/100);
		pTE->more1=0;
		pTE->more2=0;
		pTE->dispellable=1;
		break;
	case 2:	// night sight
		pc_dest->fixedlight=SrvParams->worldBrightLevel();
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
		pc_dest->stm=min(pc_dest->stm, pc_dest->effDex());
		statwindow(calcSocketFromChar(pc_dest), pc_dest);
		pTE->setExpiretime_s(pc_source->skill(MAGERY)/10);
		pTE->more1=more1;
		pTE->more2=0;
		pTE->dispellable=1;
		break;
	case 4:
		if (pc_dest->in<more1)
			more1=pc_dest->in;
		pc_dest->in-=more1;
		pc_dest->mn=min(pc_dest->mn, pc_dest->in);
		statwindow(calcSocketFromChar(pc_dest), pc_dest);
		pTE->setExpiretime_s(pc_source->skill(MAGERY)/10);
		pTE->more1=more1;
		pTE->more2=0;
		pTE->dispellable=1;
		break;
	case 5:
		if (pc_dest->st<more1)
			more1=pc_dest->st;
		pc_dest->st-=more1;
		pc_dest->hp=min(pc_dest->hp, pc_dest->st);
		statwindow(calcSocketFromChar(pc_dest), pc_dest);
		pTE->setExpiretime_s(pc_source->skill(MAGERY)/10);
		pTE->more1=more1;
		pTE->more2=0;
		pTE->dispellable=1;
		break;
	case 6:
		if (pc_dest->effDex()+more1>250)
			more1=250-pc_dest->effDex();
		pc_dest->chgDex(more1);
		statwindow(calcSocketFromChar(pc_dest), pc_dest);
		if(dur > 0)		// if a duration is given (potions), use that (Duke, 31.10.2000)
			pTE->setExpiretime_s(dur);
		else
			pTE->setExpiretime_s(pc_source->skill(MAGERY)/10);
		pTE->more1=more1;
		pTE->more2=0;
		pTE->dispellable=1;
		break;
	case 7:
		if (pc_dest->in+more1>255)
			more1=pc_dest->in-255;
		pc_dest->in+=more1;
		statwindow(calcSocketFromChar(pc_dest), pc_dest);
		pTE->setExpiretime_s(pc_source->skill(MAGERY)/10);
		pTE->more1=more1;
		pTE->more2=0;
		pTE->dispellable=1;
		break;
	case 8:
		if (pc_dest->st+more1>255)
			more1=pc_dest->st-255;
		pc_dest->st+=more1;
		statwindow(calcSocketFromChar(pc_dest), pc_dest);
		if(dur > 0)		// if a duration is given (potions), use that (Duke, 31.10.2000)
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
		if (pc_dest->st+more1>255)
			more1=pc_dest->st-255;
		if (pc_dest->effDex()+more2>250)
			more2=250-pc_dest->effDex();
		if (pc_dest->in+more3>255)
			more3=pc_dest->in-255;
		pc_dest->st+=more1;
		pc_dest->chgDex(more2);
		pc_dest->in+=more3;
		statwindow(calcSocketFromChar(pc_dest), pc_dest);
		pTE->setExpiretime_s(pc_source->skill(MAGERY)/10);
		pTE->more1=more1;
		pTE->more2=more2;
		pTE->more3=more3;
		pTE->dispellable=1;
		break;
	case 12: // Curse
		if (pc_dest->st<more1)
			more1=pc_dest->st;
		if (pc_dest->effDex()<more2)
			more2=pc_dest->effDex();
		if (pc_dest->in<more3)
			more3=pc_dest->in;
		pc_dest->st-=more1;
		pc_dest->chgDex(-1 * more2);
		pc_dest->in-=more3;
		statwindow(calcSocketFromChar(pc_dest), pc_dest);
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
		if(pc_dest->onHorse()) 
			k = unmounthorse(calcSocketFromChar(pc_dest));
		k=(more1<<8)+more2;

		pc_dest->xid = pc_dest->id();//let's backup previous id

		if (k>=0x000 && k<=0x3e1) // lord binary, body-values >0x3e crash the client
		{
			pc_dest->id1=k>>8; // allow only non crashing ones
			pc_dest->id2=k%256;

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
				pc_dest->setId(pc_dest->xid);
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
			pc_dest->xid = pc_dest->id();
			pc_dest->id1=0x01;
			//if we already have a beard..can't turn to female
			if(pc_dest->beardserial() != INVALID_SERIAL)
			{//if character has a beard...only male
				pc_dest->id2='\x90';//male
			} else
			{//if no beard let's randomly change
				if((rand()%2)==0) pc_dest->id2='\x90';//male
				else pc_dest->id2='\x91';//or female
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

			if(pc_dest->id2==0x90) 
				setrandomname(pc_dest, "1");//get a name from male list
			else 
				setrandomname(pc_dest, "2");//get a name from female list

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
			if(pc_dest->id2==0x90)// only if a man
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
		pc_source->hp=pc_source->st;
		pc_source->mn=pc_source->in;
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
		pc_dest->priv2 |= 0x02;
		pTE->setExpiretime_s(5);
		pTE->num=1;
		pTE->more1=0;
		pTE->more2=0;
		break;

	case 45: // special sword use concussion intelligence loss -Frazurbluu-
		if (pc_dest->in<more1)
			more1=pc_dest->in;
		pc_dest->in-=more1;
		pc_dest->mn=min(pc_dest->mn, pc_dest->in);
		statwindow(calcSocketFromChar(pc_dest), pc_dest);
		pTE->setExpiretime_s(30);
		pTE->num=4;
		pTE->more1=more1;
		pTE->more2=0;
		break;

	default:
		LogErrorVar("Fallout of switch! Value (%d)",num);
		return 0;
	}

	cTempEffects::getInstance()->Insert( pTE );
	return 1;
}

bool cTempEffects::Add(P_CHAR pc_source, P_ITEM piDest, int num, unsigned char more1, unsigned char more2, unsigned char more3)
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
	this->Insert(pTE);
	return true;
}

void cTempEffects::Insert(cTempEffect* pTE)
{
	teffects.push_back( pTE );
	push_heap( teffects.begin(), teffects.end(), ComparePredicate() );
}

void cTempEffects::Serialize(ISerialization &archive)
{
	std::vector< cTempEffect* >::iterator it = teffects.begin();
	while( it != teffects.end() )
	{
		if( (*it)->isSerializable() )
			archive.writeObject( (*it) );
		it++;
	}
}

void cTempEffects::Dispel( P_CHAR pc_dest )
{
	vector<cTempEffect *>::iterator i;
	for( i = teffects.begin(); i != teffects.end(); i++ )
		if( i != NULL && (*i) != NULL && (*i)->dispellable && (*i)->getDest() == pc_dest->serial )
		{
			(*i)->Off( pc_dest );
			teffects.erase( i );
		}
	make_heap( teffects.begin(), teffects.end(), ComparePredicate() );
}

unsigned int cTempEffects::size( void )
{
	return teffects.size();
}

unsigned char tempeffect(P_CHAR pc_source, P_CHAR pc_dest, int num, unsigned char more1, unsigned char more2, unsigned char more3, short dur)
{
	return cTempEffects::getInstance()->Add(pc_source, pc_dest, num, more1, more2, more3, dur);
}

unsigned char tempeffect2(P_CHAR source, P_ITEM piDest, int num, unsigned char more1, unsigned char more2, unsigned char more3)
{
	return cTempEffects::getInstance()->Add(source, piDest, num, more1, more2, more3);
}

// cTimedAction Methods
cTimedAction::cTimedAction( P_CHAR nChar, UI08 nAction, UI32 nDuration )
{
	objectid = "TimedAction";
	character = nChar->serial;
	action = nAction;
	duration = nDuration;
}

cTimedAction::cTimedAction( SERIAL serial, UI08 nAction, UI32 nDuration )
{
	objectid = "TimedAction";
	character = serial;
	action = nAction;
	duration = nDuration;
}

void cTimedAction::Serialize( ISerialization &archive )
{
	if( archive.isReading() )
	{
		archive.read( "serial",			character );
		archive.read( "action",			action );
		archive.read( "duration",		duration );
	}
	else if( archive.isWritting() )
	{
		archive.write( "serial",		character );
		archive.write( "action",		action );
		archive.write( "duration",		duration );
	}
	cTempEffect::Serialize(archive);
}

// Show the action and create a new action
// With length - anim-length (thats a mysterium !!)
void cTimedAction::Expire()
{
}

// Singleton
cTempEffects cTempEffects::instance;