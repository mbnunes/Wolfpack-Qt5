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

// Platform Includes
#include "platform.h"

// Wolfpack Includes
#include "wolfpack.h"
#include "itemid.h"
#include "territories.h"
#include "SndPkg.h"
#include "debug.h"
#include "utilsys.h"
#include "walking2.h"
#include "regions.h"
#include "wpdefmanager.h"
#include "wpdefaultscript.h"
#include "network.h"
#include "classes.h"
#include "mapstuff.h"

#include <string>

#undef  DBGFILE
#define DBGFILE "npcs.cpp"

#define CHAR_RESERVE 100	// minimum of free slots that should be left in the array.
							// otherwise, more memory will be allocated in the mainloop (Duke)

unsigned short addrandomcolor(P_CHAR pc_s, char *colorlist)
{
	char sect[512];
	int i = 0,j = 0,storeval = 0;
	openscript("colors.scp");
	sprintf(sect, "RANDOMCOLOR %s", colorlist);
	if (!i_scripts[colors_script]->find(sect))
	{
		closescript();

		//sprintf(pc_s->name, "Error Colorlist %s Not Found(1)", colorlist);
		// LB: wtf should this do apart from crashing ? copying an error message in a chars name ??
		// very weired! think it should look like this:
		if( pc_s != NULL )
			clConsole.send("Error Colorlist %s not found on character: %s\n",colorlist,pc_s->name.c_str());

		return 0;
	}
	unsigned long loopexit=0;
	do
	{
		read1();
		if (script1[0]!='}')
		{
			i++;
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	closescript();
	if(i>0)
	{
		i=rand()%i;
		i++;
		openscript("colors.scp");
		if(!i_scripts[colors_script]->find(sect))
		{
			closescript();
			//sprintf(pc_s->name, "Error Colorlist %s Not Found(2)", colorlist);
			if( pc_s != NULL )
				clConsole.send("Error Colorlist %s not found on character: %s\n",colorlist,pc_s->name.c_str());
			return 0;
		}
		loopexit=0;
		do
		{
			read1();
			if (script1[0]!='}')
			{
				j++;
				if(j==i)
				{
					storeval=hex2num(script1);
				}
			}
		}
		while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
		closescript();
	}
	return (storeval);
}

static int addrandomhaircolor(P_CHAR pc_s, char *colorlist)
{
	char sect[512];
	int i,j,haircolor = 0x044e;
	i=0; j=0;
	openscript("colors.scp");
	sprintf(sect, "RANDOMCOLOR %s", colorlist);
	if (!i_scripts[colors_script]->find(sect))
	{
		closescript();
		clConsole.send("Error Colorlist %s not found on character: %s\n",colorlist,pc_s->name.c_str());
		return 0;
	}
	unsigned long loopexit=0;
	do
	{
		read1();
		if (script1[0]!='}')
		{
			i++;
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
	closescript();
	if(i>0)
	{
		i=rand()%i;
		i++;
		openscript("colors.scp");
		if(!i_scripts[colors_script]->find(sect))
		{
			closescript();
			//sprintf(pc_s->name, "Error Colorlist %s Not Found(2)", colorlist);
			clConsole.send("Error Colorlist %s not found on character: %s\n",colorlist,pc_s->name.c_str());
			return 0;
		}
		loopexit=0;
		do
		{
			read1();
			if (script1[0]!='}')
			{
				j++;
				if(j==i)
				{
					haircolor=hex2num(script1);
				}
			}
		}
		while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
		closescript();
	}
	return (haircolor);
}

void setrandomname(P_CHAR pc_s, char * namelist)
{
	char sect[512];
	int i=0,j=0;

	sprintf(sect, "RANDOMNAME %s", namelist);
	Script *pScpBase=i_scripts[npc_script];
	Script *pScp=pScpBase->Select(sect,custom_npc_script);
	if (!pScp)
	{
		pc_s->name = "Error Namelist";
		return;
	}

	unsigned long loopexit=0;
	do
	{
		pScp->NextLine();
		if (script1[0]!='}')
		{
			i++;
		}
	}
	while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
	pScp->Close();

	if(i>0)
	{
		i=rand()%(i);
		pScp=pScpBase->Select(sect,custom_npc_script);
		if (!pScp) return;

		loopexit=0;
		do
		{
			pScp->NextLine();
			if (script1[0]!='}')
			{
				if(j==i)
				{
					pc_s->name = (char*)script1;
					break;
				}
				else j++;
			}
		}
		while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
		pScp->Close();
	}
}

void cCharStuff::DeleteChar (P_CHAR pc_k) // Delete character
{
	int j;//,serial; //Zippy lag
	//int ptr,ci;

	LongToCharPtr(pc_k->serial, &removeitem[1]);

	if (pc_k->spawnSerial() != INVALID_SERIAL) 
		cspawnsp.remove(pc_k->spawnSerial(), pc_k->serial);
	if (pc_k->ownserial != INVALID_SERIAL) 
		cownsp.remove(pc_k->ownserial, pc_k->serial);
	
	for (j=0;j<now;j++)
	{
		if (perm[j]) 
			Xsend(j, removeitem, 5);		
	}
	
	if (pc_k != NULL) 
		mapRegions->Remove(pc_k); // taking it out of mapregions BEFORE x,y changed, LB

	delete pc_k->tags;
	
	pc_k->free = true;
	cCharsManager::getInstance()->deleteChar( pc_k );
}

P_CHAR cCharStuff::MemCharFree()			// Find a free char slot
{
	P_CHAR pc = new cChar;
	return pc;
}

P_ITEM cCharStuff::AddRandomLoot(P_ITEM pBackpack, char * lootlist)
{
	char sect[512];
	int i,j, storeval,loopexit=0;
	P_ITEM retitem = NULL;
	storeval=-1;
	i=0; j=0;

	sprintf(sect, "LOOTLIST %s", lootlist);
	
	Script *pScpBase=i_scripts[npc_script];
	Script *pScp=pScpBase->Select(sect,custom_npc_script);
	if (!pScp) return NULL;

	loopexit=0;
	do
	{
		pScp->NextLine();
		if (script1[0]!='}')
		{
			i++; // Count number of entries on list.
		}
	} while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
	pScp->Close();

	if(i>0)
	{
		i=rand()%(i);
		pScp=pScpBase->Select(sect,custom_npc_script);

		loopexit=0;
		do
		{
			pScp->NextLine();
			if (script1[0]!='}')
			{
				if(j==i)
				{
					storeval=str2num(script1);	//script1 = ITEM#

					scpMark m=pScp->Suspend();
					retitem = Targ->AddMenuTarget(-1, 0, storeval);
					pScp->Resume(m);

					if(retitem!=NULL)
					{
						retitem->pos.x=50+(rand()%80);
						retitem->pos.y=50+(rand()%80);
						retitem->pos.z=9;
						retitem->setContSerial(pBackpack->serial);
					}
					break;;    
				}
				else j++;
			}
		}	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
		pScp->Close();
	}
	return retitem;
}

void cCharStuff::Split(P_CHAR pc_k) // For NPCs That Split during combat
{
	int z;

	if ( pc_k == NULL ) return;
	
	P_CHAR pc_c = new cChar(*pc_k);
	if ( pc_c == NULL ) return;
//	pc_c->Init();
	pc_c->setSerial(cCharsManager::getInstance()->getUnusedSerial());
	pc_c->ftarg = INVALID_SERIAL;
	pc_c->moveTo(pc_k->pos + Coord_cl(1, 0, 0) );
	pc_c->kills = 0;
	pc_c->hp = pc_k->st;
	pc_c->stm = pc_k->realDex();
	pc_c->mn = pc_k->in;
	z=rand()%35;
	if (z == 5) 
		pc_c->setSplit(1); 
	else 
		pc_c->setSplit(0);	
	updatechar(pc_c);
}

P_CHAR cCharStuff::createScriptNpc( int s, P_ITEM pi_i, QString Section, int posx, int posy, signed char posz )
{
	if( Section.length() == 0 )
		return NULL;

	QDomElement* DefSection = DefManager->getSection( WPDT_NPC, Section );

	if( DefSection->isNull() )
	{
		clConsole.log( QString("Unable to create unscripted npc: %1\n").arg(Section).latin1() );
		return NULL;
	}

	int k=0, xos=0, yos=0, lb;
	short postype;				// determines how xyz of the new NPC are set, see below
	unsigned long loopexit=0;

	P_CHAR nChar = MemCharFree();
	nChar->Init( true );
	cCharsManager::getInstance()->registerChar( nChar );

	nChar->setPriv(0x10);
	nChar->npc=1;
	nChar->att=1;
	nChar->def=1;
	nChar->setSpawnSerial( INVALID_SERIAL );	

	if (posx > 0 && posy > 0)
 		postype = 3;	// take position from parms
	else if ( s != -1 && pi_i == NULL)
		postype = 2;	// take position from socket's buffer
	else if ( s == -1 && pi_i != NULL)
		postype = 1;	// take position from items[i]
	else
	{
		clConsole.send("ERROR: bad parms in call to createScriptNpc\n");
		return NULL;
	}

	// Now that we have created the NPC, lets place him
	switch (postype)
	{
	case 1:					// take position from (spawning) item
		if (triggerx)
		{
			nChar->pos.x=triggerx;
			nChar->pos.y=triggery;
			nChar->dispz=nChar->pos.z=triggerz;
			triggerx = nChar->serial;
		} else
		{
	   /*Zippy's Code chages for area spawns --> (Type 69) xos and yos (X OffSet, Y OffSet) 
	   are used to find a random number that is then added to the spawner's x and y (Using 
	   the spawner's z) and then place the NPC anywhere in a square around the spawner. 
	   This square is random anywhere from -10 to +10 from the spawner's location (for x and 
	   y) If the place chosen is not a valid position (the NPC can't walk there) then a new 
	   place will be chosen, if a valid place cannot be found in a certain # of tries (50), 
		   the NPC will be placed directly on the spawner and the server op will be warned. */
			if ((pi_i->type() == 69 || pi_i->type() == 125)&& pi_i->isInWorld())
			{
				if (pi_i->more3==0) pi_i->more3=10;
				if (pi_i->more4==0) pi_i->more4=10;
			   //signed char z, ztemp, found;
	   
				k=0;
				do
				{
					if (k>=50) //this CAN be a bit laggy. adjust as nessicary
					{
						clConsole.send("WOLFPACK: Problem area spawner found at [%i,%i,%i]. NPC placed at default location.\n",pi_i->pos.x,pi_i->pos.y,pi_i->pos.z);
						xos=0;
						yos=0;
						break;
					}
					xos=RandomNum(-pi_i->more3,pi_i->more3);
					yos=RandomNum(-pi_i->more4,pi_i->more4);
					// clConsole.send("Spawning at Offset %i,%i (%i,%i,%i) [-%i,%i <-> -%i,%i]. [Loop #: %i]\n",xos,yos,pi_i->x+xos,pi_i->y+yos,pi_i->z,pi_i->more3,pi_i->more3,pi_i->more4,pi_i->more4,k); /** lord binary, changed %s to %i, crash when uncommented ! **/
					k++;
					if ((pi_i->pos.x+xos<1) || (pi_i->pos.y+yos<1)) lb=0; /* lord binary, fixes crash when calling npcvalid with negative coordiantes */
					else lb = Movement->validNPCMove(pi_i->pos.x+xos,pi_i->pos.y+yos,pi_i->pos.z, nChar);				 
				   
					//Bug fix Monsters spawning on water:
					MapStaticIterator msi(pi_i->pos + Coord_cl(xos, yos, 0));

					staticrecord *stat;
					loopexit=0;
					while ( (stat = msi.Next()) && (++loopexit < MAXLOOPS) )
					{
						tile_st tile;
						msi.GetTile(&tile);
						if(!(strcmp((char *) tile.name, "water")))//Water
						{//Don't spawn on water tiles... Just add other stuff here you don't want spawned on.
							lb=0;
						}
					}
				} while (!lb);
			} // end Zippy's changes (exept for all the +xos and +yos around here....)

			nChar->pos.x=pi_i->pos.x+xos;
			nChar->pos.y=pi_i->pos.y+yos;
			nChar->dispz=nChar->pos.z=pi_i->pos.z;
			nChar->SetSpawnSerial(pi_i->serial);
			if ( pi_i->type() == 125 )
			{
				MsgBoardQuestEscortCreate( nChar );
			}
		} // end of if !triggerx
		break;
	case 2: // take position from Socket
		if (s!=-1)
		{
			nChar->pos.x=(buffer[s][11]<<8)+buffer[s][12];
			nChar->pos.y=(buffer[s][13]<<8)+buffer[s][14];
			nChar->dispz=nChar->pos.z=buffer[s][16]+Map->TileHeight((buffer[s][17]<<8)+buffer[s][18]);
		}
		break;
	case 3: // take position from Parms
		nChar->pos.x=posx;
		nChar->pos.y=posy;
		nChar->dispz=nChar->pos.z=posz;
		break;
	} // no default coz we tested on entry to function
   
	cTerritory* Region = cAllTerritories::getInstance()->region( nChar->pos.x, nChar->pos.y );
	if( Region != NULL )
		nChar->region = Region->name();
	else
		nChar->region = QString();

	nChar->applyDefinition( *DefSection );
	mapRegions->Add( nChar );

	cRegion::RegionIterator4Chars ri(nChar->pos);
	for (ri.Begin(); !ri.atEnd(); ri++)
	{
		P_CHAR pc = ri.GetData();
		if ( pc != NULL && calcSocketFromChar( pc ) != -1 && pc->dist( nChar ) <= VISRANGE )
			impowncreate( s, nChar, 1 );
	}

	return nChar;
}

////////////
// Name:	inGuardedArea
// history:	by Duke, 13.1.2002
// Purpose:	checks if the char is in a guarded region
// Remark:	the recalculation of the region is necessary because it is not maintained properly :(
//			I think it is better to do this only when needed
//
bool cChar::inGuardedArea()
{
	cTerritory* Region = cAllTerritories::getInstance()->region( this->pos.x, this->pos.y );
	if( Region != NULL )
		return Region->isGuarded();
	else
		return false;
}

