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
//	GNU General Public Li678cense for more details.
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

/* House code for deed creation by Tal Strake, revised by Cironian */

//#include "wolfpack.h"
#include "house.h"
#include "iserialization.h"
#include "regions.h"
#include "srvparams.h"
//#include "SndPkg.h"
//#include "debug.h"
//#include "utilsys.h"

// System Include Files
#include <algorithm>

// Namespaces for this module
using namespace std;

#undef  DBGFILE
#define DBGFILE "house.cpp"

void HomeTarget(int s, int a1, int a2, int a3, int a4, char b1, char b2, char *txt)
{
	unsigned char multitarcrs[27]="\x99\x01\x40\x01\x02\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x02\x00\x00\x00\x00\x00\x00";

	targetok[s]=1; 
	multitarcrs[2]=a1;
	multitarcrs[3]=a2;
	multitarcrs[4]=a3;
	multitarcrs[5]=a4;
	multitarcrs[18]=b1;
	multitarcrs[19]=b2;
	sysmessage(s, txt);
	Xsend(s, multitarcrs, 26);
}

bool CheckBuildSite(const Coord_cl& pos, int sx, int sy)
{
	signed int checkz;
	//char statc;
	int checkx;
	int checky;
	int ycount=0;
	checkx=pos.x-abs(sx/2);
	for (;checkx<(pos.x+abs(sx/2));checkx++)
	{
		checky=pos.y-(sy/2);
		for (;checky<(pos.y+(sy/2));checky++)
		{
			checkz=Map->MapElevation( Coord_cl( checkx,checky, pos.z, pos.map));
			if ((checkz<(pos.z-7)) || (checkz>(pos.z+7)))
			{
				return false;
			}
		}
	}
	return true;
}


//o---------------------------------------------------------------------------o
//|   Function    -  void buildhouse(int s, int i)
//|   Date        -  UnKnown - Rewrite Date 1/24/99
//|   Programmer  -  UnKnown - Rewrite by Zippy (onlynow@earthlink.net)
//|   Lots of client crash fixes by LB, 25-dec 1999
//o---------------------------------------------------------------------------o
//|   Purpose     -  Triggered by double clicking a deed-> the deed's moreX is read
//|                  for the house section in house.scp. Extra items can be added
//|                  using HOUSE ITEM, (this includes all doors!) and locked "LOCK"
//|                  Space around the house with SPACEX/Y and CHAR offset CHARX/Y/Z
//o---------------------------------------------------------------------------o
void BuildHouse(UOXSOCKET s, int i)
{
	int x,y,loopexit=0;//where they click, and the house/key items
	P_ITEM pKey = NULL;
	signed char z;
	int sx=0,sy=0;                                  //space around the house needed
	
	//int hitem[100],icount=0;//extra "house items" (up to 100)
	vector<int> houseItems;
	char sect[512];                         //file reading
	unsigned short id;                                   //house ID
	char itemsdecay = 0;            // set to 1 to make stuff decay in houses
	static int looptimes=0;         //for targeting
	long int pos;                                   //for files...
	int cx=0,cy=0,cz=8;             //where the char is moved to when they place the house (Inside, on the steps.. etc...)(Offset)
	bool boat = false;//Boats
	int hdeed=0;//deed id #		
	bool norealmulti = false, nokey = false, othername = false;
	int lockamount=0, secureamount=0;
	char name[512];
	P_CHAR pc_currchar = currchar[s];

	if(buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF) return; // do nothing if user cancels, avoids CRASH!
	
	if (i)
	{
		openscript("house.scp");
		sprintf(sect, "HOUSE %d", i);//and BTW, .find() adds SECTION on there for you....
		if (!(i_scripts[house_script]->find(sect)))
		{
			closescript();
			return;
		}
		do
		{
			read2();
			if (script1[0]!='}')
			{
				if (!(strcmp((char*)script1,"ID")))
				{
					id = static_cast<unsigned short>(hex2num(script2));
				}
				else if (!(strcmp((char*)script1,"SPACEX")))
				{
					sx=str2num(script2)+1;
				}
				else if (!(strcmp((char*)script1,"SPACEY")))
				{
					sy=str2num(script2)+1;
				}
				else if (!(strcmp((char*)script1,"CHARX")))
				{
					cx=str2num(script2);
				}
				else if (!(strcmp((char*)script1,"CHARY")))
				{
					cy=str2num(script2);
				}
				else if (!(strcmp((char*)script1,"CHARZ")))
				{
					cz=str2num(script2);
				}
				else if( !(strcmp((char*)script1, "ITEMSDECAY" )))
				{
					itemsdecay = str2num( script2 );
				}
				else if (!(strcmp((char*)script1,"HOUSE_ITEM")))
				{
					houseItems.push_back(str2num(script2));
				}
				else if (!(strcmp((char*)script1, "HOUSE_DEED")))
				{
					hdeed=str2num(script2);
				}							
				else if (!(strcmp((char*)script1, "BOAT"))) boat = true;//Boats
				else if (!(strcmp((char*)script1, "NOREALMULTI"))) norealmulti = true;
				else if (!(strcmp((char*)script1, "NOKEY"))) nokey = true;
				else if (!(strcmp((char*)script1, "NAME"))) 
				{ 
					strcpy((char*)name,(char*)script2);
					othername = true;
				}
				else if(!(strcmp((char*)script1, "LOCKDOWNAMOUNT")))
				{
					lockamount=str2num(script2);
				}
				else if(!(strcmp((char*)script1, "SECUREAMOUNT")))
				{
					secureamount=str2num(script2);
				}
			}
		}
		while ( (strcmp((char*)script1,"}")) && (++loopexit < MAXLOOPS) );
		closescript();
		
		if (!id)
		{
			clConsole.send("ERROR: Bad house script # %i!\n",i);
			return;
		}
	}
	
	if(!looptimes)
	{
		if (i)
		{
			
			addid1[s]=0x40;addid2[s]=100;//Used in addtarget
			if (norealmulti) 
				target(s, 0, 1, 0, 245, "Select a place for your structure: "); 
			else
				HomeTarget(s, 0, 1, 0, 0, (id>>8)-0x40, static_cast<unsigned char>(id&0x00FF), "Select location for building.");
			
		}
		else
		{
			HomeTarget(s, 0, 1, 0, 0, addid1[s]-0x40, addid2[s], "Select location for building.");
		}
		looptimes++;//for when we come back after they target something
		return;
	}
	if(looptimes)
	{
		looptimes=0;
		if(!pc_currchar->isGM() && SrvParams->houseInTown()==0)
		{
		    if (pc_currchar->inGuardedArea() && ishouse(id) ) // popy
			{
			    sysmessage(s," You cannot build houses in town!");
			    return;
			}
		}
		
		x=(buffer[s][11]<<8)+buffer[s][12];//where they targeted
		y=(buffer[s][13]<<8)+buffer[s][14];
		z=buffer[s][16]+Map->TileHeight((buffer[s][17]<<8)+buffer[s][18]);
		
		
		if ( (( x<200 && y <200 ) || ( x>6200 || y >4200 ))  )			
		{
			sysmessage(s, "You cannot build your structure there!");
			return;
		}
		
		if (ishouse(id)) // strict checking only for houses ! LB
		{
			if(!CheckBuildSite(Coord_cl(x,y,z,pc_currchar->pos.map),sx,sy))
			{
				sysmessage(s,"Can not build a house at that location (CBS)!");
				return;
			}
		}
		
		//Boats ->
		if(!boat) 
			sprintf((char*)temp,"%s's house", pc_currchar->name.c_str());//This will make the little deed item you see when you have showhs on say the person's name, thought it might be helpful for GMs.
		else 
			strcpy((char*)temp, "a mast");
		if(norealmulti) 
			strcpy((char*)temp, name);
		//--^
		
		if (othername) 
			strcpy((char*)temp, name);
		
		P_ITEM pMulti = NULL;
		if (!boat)
		{
			pMulti = dynamic_cast<P_ITEM>( new cHouse );
			pMulti->Init();
			pMulti->setId(id);
			pMulti->setName( QString( "%1's house" ).arg( pc_currchar->name.c_str() ) );
			Items->GetScriptItemSetting(pMulti);
			//pMulti->setId(id);
			RefreshItem( pMulti );
		}
		else
			pMulti = Items->SpawnItem(pc_currchar, 1,(char*)temp,0,id,0,0);
			
		if (!pMulti) return;		
		pMulti->MoveTo(x,y,z);
		pMulti->priv = 0;
		pMulti->more4 = itemsdecay; // set to 1 to make items in houses decay
		pMulti->morex = hdeed; // crackerjack 8/9/99 - for converting back *into* deeds
		pMulti->SetOwnSerial(pc_currchar->serial);
		
		if (houseItems.empty() && !boat)
		{
			teleport(pc_currchar);
			cRegion::RegionIterator4Items rii(pMulti->pos);
			for(rii.Begin(); !rii.atEnd(); rii++)
			{
				P_ITEM sii = rii.GetData();
				senditem(s, sii);
			}
			return;//If there's no extra items, we don't really need a key, or anything else do we? ;-)
		}
		
		if(boat) //Boats
		{
			if(!Boats->Build(s, pMulti, static_cast<char>(id&0x00FF)))
			{
				Items->DeleItem(pMulti);
				return;
			} 
		}
		
		if (i)//Boats->.. Moved from up there ^
		{
			P_ITEM pDeed = FindItemBySerial(pc_currchar->fx1);
			Items->DeleItem(pDeed); // this will del the deed no matter where it is
		}

		pc_currchar->fx1=INVALID_SERIAL; //reset fx1 so it does not interfere
		// bugfix LB ... was too early reseted 
		
		//Key...
		
/*		House[houseSize]->OwnerSerial=pc_currchar->serial;
		House[houseSize]->OwnerAccount=pc_currchar->account;
		House[houseSize]->LockAmount=lockamount;
		House[houseSize]->SecureAmount=secureamount;
*/	
		unsigned char id2 = static_cast<unsigned char>(id&0x00FF);
		if (id2>=112&&id2<=115) pKey = Items->SpawnItem(s, pc_currchar, 1, "a tent key", 0, 0x10, 0x10,0, 1,1);//iron key for tents
		else if(id2<=0x18) pKey = Items->SpawnItem(s, pc_currchar, 1, "a ship key",0,0x10,0x13,0,1,1);//Boats -Rusty Iron Key
		else pKey = Items->SpawnItem(s, pc_currchar, 1, "a house key", 0, 0x10, 0x0F, 0, 1,1);//gold key for everything else;
		
		pKey->more1 = static_cast<unsigned char>((pMulti->serial&0xFF000000)>>24);
		pKey->more2 = static_cast<unsigned char>((pMulti->serial&0x00FF0000)>>16);
		pKey->more3 = static_cast<unsigned char>((pMulti->serial&0x0000FF00)>>8);
		pKey->more4 = static_cast<unsigned char>((pMulti->serial&0x000000FF));
		pKey->setType( 7 );
		pKey->priv=2; // Newbify key..Ripper
        
		P_ITEM pKey2 = Items->SpawnItem(s, pc_currchar, 1, "a house key", 0, 0x10, 0x0F, 0,1,1);
		P_ITEM bankbox = pc_currchar->GetBankBox();
		pKey2->more1 = static_cast<unsigned char>((pMulti->serial&0xFF000000)>>24);
		pKey2->more2 = static_cast<unsigned char>((pMulti->serial&0x00FF0000)>>16);
		pKey2->more3 = static_cast<unsigned char>((pMulti->serial&0x0000FF00)>>8);
		pKey2->more4 = static_cast<unsigned char>((pMulti->serial&0x000000FF));
		pKey2->setType( 7 );
		pKey2->priv=2;
		bankbox->AddItem(pKey2);
		
		if(nokey) 
		{
			Items->DeleItem(pKey); // No key for .. nokey items
			Items->DeleItem(pKey2);
		}
		
		P_ITEM pHouseItem = NULL;
		unsigned int k;
		for (k=0; k < houseItems.size(); ++k)//Loop through the HOUSE_ITEMs
		{
			openscript("house.scp");
			sprintf(sect,"HOUSE ITEM %i", houseItems[k]);
			if (!i_scripts[house_script]->find(sect))
			{
				closescript();
			}
			else
			{
				loopexit=0;
				do 
				{
					read2();
					if (script1[0]!='}')
					{
						if (!(strcmp((char*)script1,"ITEM")))
						{
							pos=ftell(scpfile);// To prevent accidental exit of loop.
							closescript();
							pHouseItem = Items->CreateScriptItem(s,str2num(script2),0);//This opens the item script... so we gotta keep track of where we are with the other script.
							openscript("house.scp");
							fseek(scpfile, pos, SEEK_SET);
							strcpy((char*)script1, "ITEM");
							pHouseItem->setGMMovable();//Non-Movebale by default
							pHouseItem->priv=0;//since even things in houses decay, no-decay by default
							pHouseItem->pos.x=x;
							pHouseItem->pos.y=y;
							pHouseItem->pos.z=z;
							pHouseItem->SetOwnSerial(pc_currchar->serial);
						}
						else if (!(strcmp((char*)script1,"DECAY")))
						{
							pHouseItem->priv |= 0x01;
						}
						else if (!(strcmp((char*)script1,"NODECAY")))
						{
							pHouseItem->priv = 0;
						}
						else if (!(strcmp((char*)script1,"PACK")))//put the item in the Builder's Backpack
						{
							P_ITEM pBackpack = Packitem(pc_currchar);
							pBackpack->AddItem(pHouseItem);
						}
						else if (!(strcmp((char*)script1,"MOVEABLE")))
						{
							pHouseItem->setAllMovable();
						}
						else if (!(strcmp((char*)script1,"LOCK")))//lock it with the house key
						{
							pHouseItem->more1 = static_cast<unsigned char>((pMulti->serial&0xFF000000)>>24);
							pHouseItem->more2 = static_cast<unsigned char>((pMulti->serial&0x00FF0000)>>16);
							pHouseItem->more3 = static_cast<unsigned char>((pMulti->serial&0x0000FF00)>>8);
							pHouseItem->more4 = static_cast<unsigned char>((pMulti->serial&0x000000FF));
						}
						else if (!(strcmp((char*)script1,"X")))//offset + or - from the center of the house:
						{
							pHouseItem->pos.x=x+str2num(script2);
						}
						else if (!(strcmp((char*)script1,"Y")))
						{ 
							pHouseItem->pos.y=y+str2num(script2);
						}
						else if (!(strcmp((char*)script1,"Z")))
						{
							pHouseItem->pos.z=z+str2num(script2);
						}
					}
				}
				while ( (strcmp((char*)script1,"}")) && (++loopexit < MAXLOOPS) );
				
				if (pHouseItem->isInWorld()) 
					mapRegions->Add(pHouseItem);  //add to mapRegions
				closescript();
			}
		}
		cRegion::RegionIterator4Items ri(pMulti->pos);
		for(ri.Begin(); !ri.atEnd(); ri++)
		{
			P_ITEM si = ri.GetData();
			sendinrange(si);
		}
		
		if (!norealmulti)
		{
			pc_currchar->pos.x=x+cx; //move char inside house
			pc_currchar->pos.y=y+cy;
			pc_currchar->dispz = pc_currchar->pos.z = z+cz;
			teleport((pc_currchar));
		}
	}
}

bool ishouse(unsigned short id)
{
	if (id < 0x4000) return false;
	register unsigned short id2 = id&0x00FF;
	if ( id2 >= 0x64 && id2 <= 0x7f ) 
		return true;

	switch(id2)
	{
		case 0x87:
		case 0x8c:
		case 0x8d:
		case 0x96:
		case 0x98:
		case 0x9a:
		case 0x9c:
		case 0x9e:
		case 0xa0:
		case 0xa2:
		case 0xbb8:
		case 0x1388: return true;
	}  
	return false;
}

void RemoveKeys(SERIAL serial) // Crackerjack 8/11/99
{
	if (serial == INVALID_SERIAL)
		return;
	AllItemsIterator iter_items;
	for (iter_items.Begin(); !iter_items.atEnd(); ++iter_items)
	{
		P_ITEM pi = iter_items.GetData();
		if (pi->type() == 7 && calcserial(pi->more1, pi->more2, pi->more3, pi->more4) == serial)
		{
			Items->DeleItem(pi);
		}
	}
	return;
}

// removes houses - without regions. slow but necassairy for house decay
// LB 19-September 2000
void RemoveHouse(P_ITEM pHouse)
{
	cRegion::RegionIterator4Chars ri(pHouse->pos);
	for (ri.Begin(); !ri.atEnd(); ri++)
	{
		P_CHAR pc = ri.GetData();
		if(pc->npcaitype() == 17 && pc->multis == pHouse->serial)
			Npcs->DeleteChar(pc);
	}
	cRegion::RegionIterator4Items rii(pHouse->pos);
	for(rii.Begin(); !rii.atEnd(); ri++)
	{
		P_ITEM pi = rii.GetData();
		if(pi->multis == pHouse->serial && pi->type() != 202)
			Items->DeleItem(pi);
	}
	RemoveKeys(pHouse->serial);
}


// turn a house into a deed if possible. - crackerjack 8/9/99
// s = socket of player
// i = house item # in items[]
// morex on the house item must be set to deed item # in items.scp.

void deedhouse(UOXSOCKET s, P_ITEM pHouse) // Ripper & AB
{
	int x1, y1, x2, y2;
	if( pHouse == NULL ) return;
	P_CHAR pc = currchar[s];
	int a,checkgrid,increment,StartGrid,getcell,ab;		
	if(pc->Owns(pHouse) || pc->isGM())
	{
		Map->MultiArea(pHouse, &x1,&y1,&x2,&y2);
		
		P_ITEM pDeed = Items->SpawnItemBackpack2(s, pHouse->morex, 0);        // need to make before delete
		if( pDeed == NULL ) return;
		sprintf((char*)temp, "Demolishing %s", pHouse->name().ascii() );
		sysmessage( s, (char*)temp );
		sprintf((char*)temp, "Converted into a %s.", pDeed->name().ascii() );
		sysmessage(s, (char*)temp); 
		// door/sign delete
		StartGrid=mapRegions->StartGrid(pHouse->pos);
		getcell=mapRegions->GetCell(pHouse->pos);
		increment=0;
		ab=0;
		for (checkgrid=StartGrid+(increment*mapRegions->GetColSize());increment<3;increment++, checkgrid=StartGrid+(increment*mapRegions->GetColSize()))
		{       
			for (a=0;a<3;a++)
			{                                       
				cRegion::raw vecEntries = mapRegions->GetCellEntries(checkgrid+a);
				cRegion::rawIterator it = vecEntries.begin();
				for (; it != vecEntries.end(); ++it )
				{
					P_CHAR mapchar = FindCharBySerial(*it);
					P_ITEM mapitem = FindItemBySerial(*it);
					if (mapchar != NULL)
					{
						if( mapchar->pos.x >= x1 && mapchar->pos.y >= y1 && mapchar->pos.x <= x2 && mapchar->pos.y <= y2 )
						{
							if( mapchar->npcaitype() == 17 ) // player vendor in right place
							{
								sprintf( (char*)temp, "A vendor deed for %s", mapchar->name.c_str() );
								P_ITEM pPvDeed = Items->SpawnItem(pc, 1, (char*)temp, 0, 0x14F0, 0, 1);
								pPvDeed->setType( 217 );
								pPvDeed->value = 2000;
								RefreshItem( pPvDeed );
								sysmessage(s, "Packed up vendor %s.", mapchar->name.c_str());
								Npcs->DeleteChar( mapchar );
							}
						}
					}
					else if( mapitem != NULL )
					{
						if( mapitem->pos.x >= x1 && mapitem->pos.y >= y1 && mapitem->pos.x <= x2 && mapitem->pos.y <= y2 )
						{
							Items->DeleItem(mapitem);
						}
					}
				} 
			}
		}
		Items->DeleItem(pHouse);
		killkeys( pHouse->serial );
		sysmessage(s, "All house items and keys removed.");
		
		pc->pos.z = pc->dispz = Map->MapElevation(pc->pos);
		teleport(pc);
		return;
	}
}

bool cHouse::isBanned(P_CHAR pc)
{
	return binary_search(bans.begin(), bans.end(), pc->serial);
}

bool cHouse::isFriend(P_CHAR pc)
{
	return binary_search(friends.begin(), friends.end(), pc->serial);
}

void cHouse::addBan(P_CHAR pc)
{
	bans.push_back(pc->serial);
	sort(bans.begin(), bans.end());
}

void cHouse::addFriend(P_CHAR pc)
{	
	friends.push_back(pc->serial);
	sort(friends.begin(), friends.end());
}

void cHouse::removeBan(P_CHAR pc)
{
	vector<SERIAL>::iterator it = find(bans.begin(), bans.end(), pc->serial);
	bans.erase(it);
}

void cHouse::removeFriend(P_CHAR pc)
{
	vector<SERIAL>::iterator it = find(friends.begin(), friends.end(), pc->serial);
	friends.erase(it);
}

// Handles house commands from friends of the house. - Crackerjack 8/12/99
void house_speech(int s, string& msg)	// msg must already be capitalized
{
	P_CHAR pc_currchar = currchar[s];
	
	if ( pc_currchar->multis == INVALID_SERIAL )
		return; // Not inside a multi

	P_ITEM pMulti = FindItemBySerial(pc_currchar->multis);
		
	if ( pMulti && ishouse(pMulti->id()) )
	{
		cHouse* pHouse = dynamic_cast<cHouse*>(pMulti);
		if ( !(pc_currchar->Owns(pHouse) || pHouse->isFriend(pc_currchar)))
			return; // Not (Friend or Owner)
	}
	else
		return;	

	if(msg.find("I BAN THEE")!=string::npos) 
	{ // house ban
		addid1[s] = pMulti->serial>>24;
		addid2[s] = pMulti->serial>>16;
		addid3[s] = pMulti->serial>>8;
		addid4[s] = pMulti->serial%256;
		target(s, 0, 1, 0, 229, "Select person to ban from house.");
	}
	else if(msg.find("REMOVE THYSELF")!=string::npos) 
	{ // kick out of house
		addid1[s] = pMulti->serial>>24;
		addid2[s] = pMulti->serial>>16;
		addid3[s] = pMulti->serial>>8;
		addid4[s] = pMulti->serial%256;
		target(s, 0, 1, 0, 228, "Select person to eject from house.");
	}
	else if (msg.find("I WISH TO LOCK THIS DOWN")!=string::npos) 
	{ // lock down code AB/LB
         target(s, 0, 1, 0, 232, "Select item to lock down");
	}
	else if (msg.find("I WISH TO RELEASE THIS")!=string::npos) 
	{ // lock down code AB/LB
          target(s, 0, 1, 0, 233, "Select item to release");
	}
	else if (msg.find("I WISH TO SECURE THIS")!=string::npos) 
	{ // lock down code AB/LB
		target(s, 0, 1, 0, 234, "Select item to secure"); 
	}
}

int check_house_decay()
{
	int houses=0;   
	int decayed_houses=0;
	unsigned long int timediff;
	unsigned long int ct=getNormalizedTime();
	
	AllItemsIterator iter_items;
	for (iter_items.Begin(); !iter_items.atEnd(); iter_items++) 
	{   
		P_ITEM pi = iter_items.GetData(); // there shouldnt be an error here !		 
		if (!pi->free && ishouse(pi->id()))
		{
			if (pi->time_unused>SrvParams->housedecay_secs()) // not used longer than max_unused time ? delete the house
			{          
				decayed_houses++;
				sprintf((char*)temp,"%s decayed! not refreshed for > %i seconds!\n",pi->name().ascii(), SrvParams->housedecay_secs());
				LogMessage((char*)temp);
				RemoveHouse(pi);
			}
			else // house ok -> update unused-time-attribute
			{
				timediff=(ct-pi->timeused_last)/MY_CLOCKS_PER_SEC;
				pi->time_unused+=timediff; // might be over limit now, but it will be cought next check anyway
				
				pi->timeused_last=ct;	// if we don't do that and housedecay is checked every 11 minutes,
				// it would add 11,22,33,... minutes. So now timeused_last should in fact
				// be called timeCHECKED_last. but as there is a new timer system coming up
				// that will make things like this much easier, I'm too lazy now to rename
				// it (Duke, 16.2.2001)
			}
			
			houses++;
			
		}
		
	}
	
	//delete Watch;
	return decayed_houses;
}

void cHouse::Serialize(ISerialization &archive)
{
	if (archive.isReading())
	{
		unsigned int amount = 0;
		register unsigned int i;
		SERIAL readData;
		archive.read("banamount", amount);
		for (i = 0; i < amount; ++i)
		{
			archive.read("ban", readData);
			bans.push_back(readData);			
		}
		archive.read("friendamount", amount);
		for (i = 0; i < amount; ++i)
		{
			archive.read("friend", readData);
			friends.push_back(readData);
		}
	}
	else if ( archive.isWritting())
	{
		register unsigned int i;
		archive.write("banamount", bans.size());
		for ( i = 0; i < bans.size(); ++i )
			archive.write("ban", bans[i]);
		archive.write("friendamount", friends.size());
		for ( i = 0; i < friends.size(); ++i )
			archive.write("friend", friends[i]);
	}
	cItem::Serialize(archive); // Call base class method too.
}

inline string cHouse::objectID()
{
	return string("HOUSE");
}

// This function is horrible.
void killkeys( SERIAL serial )
{
	if (serial == INVALID_SERIAL)
		return;
	AllItemsIterator iter_items;
	for (iter_items.Begin(); !iter_items.atEnd(); ++iter_items)
	{
		P_ITEM pi = iter_items.GetData();
		if (pi->type() == 7 && calcserial(pi->more1, pi->more2, pi->more3, pi->more4) == serial)
		{
			--iter_items;
			Items->DeleItem(pi);
		}
	}
	return;
}
