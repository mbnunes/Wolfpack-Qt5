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

/* House code for deed creation by Tal Strake, revised by Cironian */

#include "wolfpack.h"
#include "SndPkg.h"
#include "debug.h"
#include "utilsys.h"
#undef  DBGFILE
#define DBGFILE "house.cpp"

vector<cHouse *> House;

void cHouseManager::HomeTarget(int s, int a1, int a2, int a3, int a4, char b1, char b2, char *txt)
{
	char multitarcrs[27]="\x99\x01\x40\x01\x02\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x02\x00\x00\x00\x00\x00\x00";

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
void cHouseManager::AddHome(int s, int i)
{
	int x,y,loopexit=0;//where they click, and the house/key items
	P_ITEM pKey = NULL;
	signed char z;
	int sx=0,sy=0;                                  //space around the house needed
	int k,tmp;                                            //Temps
	int hitem[100],icount=0;//extra "house items" (up to 100)
	char sect[512];                         //file reading
	unsigned char id1,id2;                                   //house ID
	char itemsdecay = 0;            // set to 1 to make stuff decay in houses
	static int looptimes=0;         //for targeting
	long int pos;                                   //for files...
	int cx=0,cy=0,cz=8;             //where the char is moved to when they place the house (Inside, on the steps.. etc...)(Offset)
	int boat=0;//Boats
	int hdeed=0;//deed id #		
	int norealmulti=0,nokey=0,othername=0;
	int lockamount=0, secureamount=0;
	char name[512];
	P_CHAR pc_currchar = currchar[s];

	if(buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF) return; // do nothing if user cancels, avoids CRASH!
	
	hitem[0]=0;//avoid problems if there are no HOUSE_ITEMs by initializing the first one as 0
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
					tmp=hex2num(script2);
					id1 = (unsigned char)(tmp>>8);
					id2 = (unsigned char)(tmp%256);
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
					hitem[icount]=str2num(script2);
					icount++;
				}
				else if (!(strcmp((char*)script1, "HOUSE_DEED")))
				{
					hdeed=str2num(script2);
				}							
				else if (!(strcmp((char*)script1, "BOAT"))) boat=1;//Boats
				
				else if (!(strcmp((char*)script1, "NOREALMULTI"))) norealmulti=1; // LB bugfix for pentas crashing client
				else if (!(strcmp((char*)script1, "NOKEY"))) nokey=1;
				else if (!(strcmp((char*)script1, "NAME"))) 
				{ 
					strcpy((char*)name,(char*)script2);
					othername=1;
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
		
		if (!id1)
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
			if (norealmulti) target(s, 0, 1, 0, 245, "Select a place for your structure: "); else
				this->HomeTarget(s, 0, 1, 0, 0, id1-0x40, id2, "Select location for building.");
			
		}
		else
		{
			this->HomeTarget(s, 0, 1, 0, 0, addid1[s]-0x40, addid2[s], "Select location for building.");
		}
		looptimes++;//for when we come back after they target something
		return;
	}
	if(looptimes)
	{
		looptimes=0;
		if(!pc_currchar->isGM() && SrvParms->houseintown==0)
		{
		    if (pc_currchar->inGuardedArea() && ishouse(id1, id2) ) // popy
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
		
		
		
		if (ishouse(id1, id2)) // strict checking only for houses ! LB
		{
			if(false==HomeBuildSite(x,y,z,sx,sy))
			{
				sysmessage(s,"Can not build a house at that location (CBS)!");
				return;
			}
		}
		
		//Boats ->
		if(id2>=18) sprintf((char*)temp,"%s's house",pc_currchar->name);//This will make the little deed item you see when you have showhs on say the person's name, thought it might be helpful for GMs.
		else strcpy((char*)temp, "a mast");
		if(norealmulti) strcpy((char*)temp, name);
		//--^
		
		if (othername) strcpy((char*)temp,name);
		
		P_ITEM pMulti = Items->SpawnItem(pc_currchar, 1,(char*)temp,0,(id1<<8)+id2,0,0);
		if (!pMulti) return;		
		pMulti->MoveTo(x,y,z);
		pMulti->priv=0;
		pMulti->more4 = itemsdecay; // set to 1 to make items in houses decay
		pMulti->morex=hdeed; // crackerjack 8/9/99 - for converting back *into* deeds
		pMulti->SetOwnSerial(pc_currchar->serial);
		if (!hitem[0] && !boat)
		{
			teleport((pc_currchar));
			cRegion::RegionIterator4Items rii(pMulti->pos);
			for(rii.Begin();rii.GetData() != rii.End(); rii++)
			{
				P_ITEM sii = rii.GetData();
				senditem(s, sii);
			}
			return;//If there's no extra items, we don't really need a key, or anything else do we? ;-)
		}
		
		if(boat) //Boats
		{
			if(!Boats->Build(s, pMulti, id2))
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
		
		int houseSize=House.size();
		House.resize(House.size()+1);
		House[houseSize]=new cHouse;
		House[houseSize]->pos.x=x-sx;
		House[houseSize]->pos2.x=x+sx;
		House[houseSize]->pos.y=y-sy;
		House[houseSize]->pos2.y=y+sy;
		House[houseSize]->pos.z=z;
		House[houseSize]->id1=id1;
		House[houseSize]->id2=id2;
		House[houseSize]->serial=pMulti->serial;
		House[houseSize]->OwnerSerial=pc_currchar->serial;
		House[houseSize]->OwnerAccount=pc_currchar->account;
		House[houseSize]->LockAmount=lockamount;
		House[houseSize]->SecureAmount=secureamount;
	
		if (id2>=112&&id2<=115) pKey = Items->SpawnItem(s, pc_currchar, 1, "a tent key", 0, 0x10, 0x10,0, 0,1,1);//iron key for tents
		else if(id2<=0x18) pKey = Items->SpawnItem(s, pc_currchar, 1, "a ship key",0,0x10,0x13,0,0,1,1);//Boats -Rusty Iron Key
		else pKey = Items->SpawnItem(s, pc_currchar, 1, "a house key", 0, 0x10, 0x0F, 0, 0,1,1);//gold key for everything else;
		
		pKey->more1=pMulti->ser1;//use the house's serial for the more on the key to keep it unique
		pKey->more2=pMulti->ser2;
		pKey->more3=pMulti->ser3;
		pKey->more4=pMulti->ser4;
		pKey->type=7;
		pKey->priv=2; // Newbify key..Ripper
        
		P_ITEM pKey2 = Items->SpawnItem(s, pc_currchar, 1, "a house key", 0, 0x10, 0x0F, 0, 0,1,1);
		P_ITEM bankbox = pc_currchar->GetBankBox();
		pKey2->more1=pMulti->ser1;
		pKey2->more2=pMulti->ser2;
		pKey2->more3=pMulti->ser3;
		pKey2->more4=pMulti->ser4;
		pKey2->type=7;
		pKey2->priv=2;
		bankbox->AddItem(pKey2);
		
		if(nokey) 
		{
			Items->DeleItem(pKey); // No key for .. nokey items
			Items->DeleItem(pKey2);
		}
		
		P_ITEM pHouseItem = NULL;

		for (k=0;k<icount;k++)//Loop through the HOUSE_ITEMs
		{
			openscript("house.scp");
			sprintf(sect,"HOUSE ITEM %i",hitem[k]);
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
							pHouseItem->priv=0;
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
							pHouseItem->more1=pHouseItem->ser1;
							pHouseItem->more2=pHouseItem->ser2;
							pHouseItem->more3=pHouseItem->ser3;
							pHouseItem->more4=pHouseItem->ser4;
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
		cRegion::RegionIterator4Items ri(pHouseItem->pos);
		for(ri.Begin();ri.GetData() != ri.End(); ri++)
		{
			P_ITEM si = ri.GetData();
			sendinrange(si);
		}
		
		if (!(norealmulti))
		{
			pc_currchar->pos.x=x+cx; //move char inside house
			pc_currchar->pos.y=y+cy;
			pc_currchar->dispz=pc_currchar->pos.z=z+cz;
			//clConsole.send("Z: %i Offset: %i Char: %i Total: %i\n",z,cz,pc_currchar->pos.z,z+cz);
			teleport((pc_currchar));
		}
	}
}

// turn a house into a deed if possible. - crackerjack 8/9/99
// s = socket of player
// i = house item # in items[]
// morex on the house item must be set to deed item # in items.scp.
void deedhouse(UOXSOCKET s, P_ITEM pHouse) // Ripper & AB
{
	int loopexit=0;
	int x1, y1, x2, y2;
	unsigned char ser1, ser2, ser3, ser4;
	if( pHouse == NULL ) return;
	P_CHAR pc = currchar[s];
//	P_ITEM playerCont = Packitem( pc );
	int a,checkgrid,increment,StartGrid,getcell,ab;		
	if(pc->Owns(pHouse) || pc->isGM())
	{
		Map->MultiArea(pHouse, &x1,&y1,&x2,&y2);
		
		P_ITEM pDeed = Items->SpawnItemBackpack2(s, pHouse->morex, 0);        // need to make before delete
		if( pDeed == NULL ) return;
		sprintf((char*)temp, "Demolishing House %s", pHouse->name);
		sysmessage( s, (char*)temp );
		ser1 = pHouse->ser1;
		ser2 = pHouse->ser2;
		ser3 = pHouse->ser3;
		ser4 = pHouse->ser4;
		Items->DeleItem(pHouse);
		sprintf((char*)temp, "Converted into a %s.", pDeed->name);
		sysmessage(s, (char*)temp); 
		// door/sign delete
		StartGrid=mapRegions->StartGrid(pc->pos.x, pc->pos.y);
		getcell=mapRegions->GetCell(pc->pos.x, pc->pos.y);
		increment=0;
		ab=0;
		for (checkgrid=StartGrid+(increment*mapRegions->GetColSize());increment<3;increment++, checkgrid=StartGrid+(increment*mapRegions->GetColSize()))
		{       
			for (a=0;a<3;a++)
			{                                       
				loopexit=0;
				vector<SERIAL> vecEntries = mapRegions->GetCellEntries(checkgrid+a);
				for (unsigned int k = 0; k < vecEntries[k]; k++)
				{
					P_CHAR mapchar = FindCharBySerial(vecEntries[k]);
					P_ITEM mapitem = FindItemBySerial(vecEntries[k]);
					if (mapchar != NULL)
					{
						if( mapchar->pos.x >= x1 && mapchar->pos.y >= y1 && mapchar->pos.x <= x2 && mapchar->pos.y <= y2 )
						{
							if( mapchar->npcaitype == 17 ) // player vendor in right place
							{
								sprintf( (char*)temp, "A vendor deed for %s", mapchar->name );
								P_ITEM pPvDeed = Items->SpawnItem(pc, 1, (char*)temp, 0, 0x14F0, 0, 1);
								pPvDeed->type = 217;
								pPvDeed->value = 2000;
								RefreshItem( pPvDeed );
								sysmessage(s, "Packed up vendor %s.", mapchar->name);
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
		int h=HouseManager->GetHouseNum(pc);
		if(h>=0)
			HouseManager->RemoveHouse(h);
		sysmessage(s,"All house items and keys removed.");
		
		pc->pos.z = pc->dispz = Map->MapElevation(pc->pos.x, pc->pos.y);
		teleport((pc));
		return;
	}
}

// removes houses - without regions. slow but necassairy for house decay
// LB 19-September 2000
void cHouseManager::RemoveHouse(int h)
{
	cRegion::RegionIterator4Chars ri(House[h]->pos);
	for (ri.Begin(); ri.GetData() != ri.End(); ri++)
	{
		P_CHAR pc = ri.GetData();
		if(GetHouseNum(pc)==h)
			if(pc->npcaitype == 17)
				Npcs->DeleteChar(pc);
	}
	cRegion::RegionIterator4Items rii(House[h]->pos);
	for(rii.Begin(); rii.GetData() != rii.End(); ri++)
	{
		P_ITEM pi = rii.GetData();
		if(GetHouseNum(pi))
			if(pi->type != 202)
				Items->DeleItem(pi);
	}
	House.erase(House.begin() + h);
	RemoveKeys(House[h]->serial);
}
// helper functiion for houde-decay.
// cause houses have no special type or more value we have to check all
// house item'ids
// LB 19-September 2000

bool ishouse(P_ITEM pci)
{
  if (pci == NULL)
	  return false;

  if (pci->id1 < 0x40) return false;
  
  if ( (pci->id2 >= 0x64) && (pci->id2 <= 0x7f) ) return true;

  switch(pci->id2)
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


bool ishouse(int id1, int id2)
{

  if (id1 < 0x40) return false;
  
  if ( (id2 >= 0x64) && (id2 <= 0x7f) ) return true;

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

// does all the work for house decay.
// checks all items if they are houses 
// if so, check its time stamp. if its too old remove it
// LB 19-September 2000
int cHouseManager::CheckDecayStatus()
{
   int houses=0;   
   int decayed_houses=0;
   unsigned long int timediff;
   unsigned long int ct=getNormalizedTime();
   int housesize=House.size();
   for(int h=0;h<housesize;)
   {
	    if(House[h]->TimeUnused>SrvParms->housedecay_secs)
		{
		    decayed_houses++;
            sprintf((char*)temp,"House decayed! not refreshed for > %i seconds!\n",SrvParms->housedecay_secs);
			LogMessage((char*)temp);
			//HouseManager->RemoveHouse(h);
		}
		else // house ok -> update unused-time-attribute
		{
           timediff=(ct-House[h]->LastUsed);
		   House[h]->TimeUnused+=timediff; // might be over limit now, but it will be cought next check anyway
		   House[h]->TimeUnused=House[h]->TimeUnused/(MY_CLOCKS_PER_SEC*60);
		   House[h]->LastUsed=ct;	
		}	
		houses++; 
		h++;
   }
   return decayed_houses;
}

void cHouseManager::RemoveKeys(int serial) // Crackerjack 8/11/99
{
	if (serial == INVALID_SERIAL)
		return;
	AllItemsIterator iter_items;
	for (iter_items.Begin(); !iter_items.atEnd(); iter_items++)
	{
		P_ITEM pi = iter_items.GetData();
		if (pi->type == 7 && calcserial(pi->more1, pi->more2, pi->more3, pi->more4) == serial)
		{
			Items->DeleItem(pi);
		}
	}
	return;
}

// Handles house commands from friends of the house. - Crackerjack 8/12/99
void house_speech(int s, char *msg)	// msg must already be capitalized
{
	int h=-1;
	int hf=-1;

	P_CHAR pc_currchar = currchar[s];

	h=HouseManager->GetHouseNum(pc_currchar);
	if(h==-1)
		return;

	hf=House[h]->FindFriend(pc_currchar);
	if(hf==-1 && (House[h]->OwnerAccount!=pc_currchar->account))
		return;

	if(strstr(msg, "I BAN THEE")) { // house ban
		addid1[s] = House[h]->serial>>24;
		addid2[s] = House[h]->serial>>16;
		addid3[s] = House[h]->serial>>8;
		addid4[s] = House[h]->serial%256;
		target(s, 0, 1, 0, 229, "Select person to ban from house.");
		return;
	}
	if(strstr(msg, "REMOVE THYSELF")) { // kick out of house
		addid1[s] = House[h]->serial>>24;
		addid2[s] = House[h]->serial>>16;
		addid3[s] = House[h]->serial>>8;
		addid4[s] = House[h]->serial%256;
		target(s, 0, 1, 0, 228, "Select person to eject from house.");
		return;
	}
	//if ((pc_currchar->serial==items[i].ownserial) || (fr==H_FRIEND)) // strictly owner only as ripper demanded :-) !!!
	//{
	   if (strstr(msg,"I WISH TO LOCK THIS DOWN")) 
	   { // lock down code AB/LB
          target(s, 0, 1, 0, 232, "Select item to lock down");           
	   }

	   if (strstr(msg,"I WISH TO RELEASE THIS")) 
	   { // lock down code AB/LB
          target(s, 0, 1, 0, 233, "Select item to release"); 
	   }
	   if (strstr(msg,"I WISH TO SECURE THIS")) 
	   { // lock down code AB/LB
          target(s, 0, 1, 0, 234, "Select item to secure"); 
	   }
	   //} else if (strstr(msg,"I WISH TO ")) sysmessage(s,"Only the house owner can lock down items!");
	//} else sysmessage(s,"Only the house owner can lock down items!");
}

bool CheckBuildSite(int x, int y, int z, int sx, int sy)
{
signed int checkz;
//char statc;
int checkx;
int checky;
int ycount=0;
checkx=x-abs(sx/2);
for (;checkx<(x+abs(sx/2));checkx++)
{
	checky=y-(sy/2);
	for (;checky<(y+(sy/2));checky++)
	{
		checkz=Map->MapElevation(checkx,checky);
		if ((checkz<(z-7)) || (checkz>(z+7)))
		{
			return false;
		}
	}
}
return true;
}

int cHouseManager::GetHouseNum(P_CHAR pc)
{
	int i;
	for(i=0;i!=House.size();i++)
		if((pc->pos.x>=House[i]->pos.x) && (pc->pos.x<=House[i]->pos2.x))
			if((pc->pos.y>=House[i]->pos.y) && (pc->pos.y<=House[i]->pos2.y))
				return i;
	return -1;
}

int cHouseManager::GetHouseNum(P_ITEM pi)
{
	int i;
	for(i=0;i!=House.size();i++)
		if((pi->pos.x>=House[i]->pos.x) && (pi->pos.x<=House[i]->pos2.x))
			if((pi->pos.y>=House[i]->pos.y) && (pi->pos.y<=House[i]->pos2.y))
				return i;
	return -1;
}

void cHouseManager::SaveHouses()
{
	HouseFile=fopen("wphouses.wsc", "w");
	if (HouseFile == NULL) 
	{
		clConsole.send("Error, couldn't open wphouses.wsc for writing. Check file permissions.\n");
		return;
	}
	
	fprintf(HouseFile, "// Wolfpack World Script (WSC)[TEXT]\n");
	fprintf(HouseFile, "// Generated by %s Version %s\n",  wp_version.betareleasestring.c_str() , wp_version.verstring.c_str() );
	fprintf(HouseFile, "// Wolfpack Houses.\n");
	for(int h=0;h<House.size();h++)
	{
		fprintf(HouseFile, "SECTION HOUSE %i\n", h);
		fprintf(HouseFile, "{\n");
		if(House[h]->id1>0)
			fprintf(HouseFile, "ID1 %i\n", House[h]->id1);
		if(House[h]->id1>0)
			fprintf(HouseFile, "ID2 %i\n", House[h]->id2);
		if(House[h]->serial>0)
			fprintf(HouseFile, "SERIAL %i\n", House[h]->serial);
		if(House[h]->pos.x>0)
			fprintf(HouseFile, "X1 %i\n", House[h]->pos.x);
		if(House[h]->pos2.x>0)
			fprintf(HouseFile, "X2 %i\n", House[h]->pos2.x);
		if(House[h]->pos.y>0)
			fprintf(HouseFile, "Y1 %i\n", House[h]->pos.y);
		if(House[h]->pos2.y>0)
			fprintf(HouseFile, "Y2 %i\n", House[h]->pos2.y);
		if(House[h]->pos.z!=illegal_z)
			fprintf(HouseFile, "Z %i\n", House[h]->pos.z);
		if(House[h]->LockAmount>=0)
			fprintf(HouseFile, "LOCKAMT %i\n", House[h]->LockAmount);
		if(House[h]->SecureAmount>=0)
			fprintf(HouseFile, "SECUREAMT %i\n", House[h]->SecureAmount);
		if(House[h]->LockTotal>=0)
			fprintf(HouseFile, "LOCKTOTAL %i\n", House[h]->LockTotal);
		if(House[h]->SecureTotal>=0)
			fprintf(HouseFile, "SECURETOTAL %i\n", House[h]->SecureTotal);
		if(House[h]->OwnerSerial>0)
			fprintf(HouseFile, "OWNERSERIAL %i\n", House[h]->OwnerSerial);
		if(House[h]->OwnerAccount>=0)
			fprintf(HouseFile, "OWNERACCOUNT %i\n", House[h]->OwnerAccount);
		if(House[h]->TimeUnused>=0)
			fprintf(HouseFile, "TIMEUNUSED %i\n", House[h]->TimeUnused);
		if(House[h]->LastUsed>=0)
			fprintf(HouseFile, "LASTUSED %i\n", House[h]->LastUsed);
		for(int f=0;f!=House[h]->FriendList.size();f++)
			fprintf(HouseFile, "FRIEND %i\n",House[h]->FriendList[f]);
		for(int b=0;b!=House[h]->BanList.size();b++)
			fprintf(HouseFile, "BANNED %i\n",House[h]->BanList[b]);
		fprintf(HouseFile, "}\n");
	}
	fclose(HouseFile);
	return;
}

void cHouseManager::LoadHouses()
{
	int housecount=0;
	wscfile=fopen("wphouses.wsc", "r");
	if (wscfile == NULL) 
	{
		return;
	}
	do
	{
		readw2();
		if(!(strcmp((char*)script1,"SECTION")))
		{
			housecount=House.size();
			House.resize(House.size()+1);
			House[housecount]=new cHouse;
		}
		else
		{
			switch(script1[0])
			{
			case 'B':
			case 'b':
				if (!strcmp((char*)script1, "BANNED"))	
					House[housecount]->BanList.push_back(str2num(script2));
				break;
			case 'F':
			case 'f':
				if (!strcmp((char*)script1, "FRIEND"))	
					House[housecount]->FriendList.push_back(str2num(script2));
				break;
			case 'I':
			case 'i':
				if(!strcmp((char*)script1, "ID1"))
					House[housecount]->id1=str2num(script2);
				if(!strcmp((char*)script1, "ID2"))
					House[housecount]->id2=str2num(script2);
				break;
			case 'L':
			case 'l':
				if(!strcmp((char*)script1, "LOCKAMT"))
					House[housecount]->LockAmount=str2num(script2);
				if(!strcmp((char*)script1, "LOCKTOTAL"))
					House[housecount]->LockTotal=str2num(script2);
				if(!strcmp((char*)script1, "LASTUSED"))
					House[housecount]->LastUsed=getNormalizedTime();
				break;
			case 'O':
			case 'o':
				if(!strcmp((char*)script1, "OWNERSERIAL"))
					House[housecount]->OwnerSerial=str2num(script2);
				if(!strcmp((char*)script1, "OWNERACCOUNT"))
					House[housecount]->OwnerAccount=str2num(script2);
				break;
			case 'S':
			case 's':
				if(!strcmp((char*)script1, "SERIAL"))
					House[housecount]->serial=str2num(script2);
				if(!strcmp((char*)script1, "SECUREAMT"))
					House[housecount]->SecureAmount=str2num(script2);
				if(!strcmp((char*)script1, "SECURETOTAL"))
					House[housecount]->SecureTotal=str2num(script2);
				break;
			case 'T':
			case 't':
				if(!strcmp((char*)script1, "TIMEUNUSED"))
					House[housecount]->TimeUnused=str2num(script2);
				break;
			case 'X':
			case 'x':
				if(!strcmp((char*)script1, "X1"))
					House[housecount]->pos.x=str2num(script2);
				if(!strcmp((char*)script1, "X2"))
					House[housecount]->pos2.x=str2num(script2);
				break;
			case 'Y':
			case 'y':
				if(!strcmp((char*)script1, "Y1"))
					House[housecount]->pos.y=str2num(script2);
				if(!strcmp((char*)script1, "Y2"))
					House[housecount]->pos2.y=str2num(script2);
				break;
			case 'Z':
			case 'z':
				if(!strcmp((char*)script1, "Z"))
					House[housecount]->pos.z=str2num(script2);
				break;
			}
		}
	} while (strcmp((char*)script1,"EOF") && !feof(wscfile));
	fclose(wscfile);
	return;
}

int cHouse::FindFriend(P_CHAR pc)
{
	if(pc->serial==OwnerSerial)
		return -2;
	for(int i=0;i!=FriendList.size();i++)
		if(pc->serial==FriendList[i])
			return i;
	return -1;
}

int cHouse::FindBan(P_CHAR pc)
{
	if(pc->serial==OwnerSerial)
		return -2;
	for(int i=0;i!=BanList.size();i++)
		if(pc->serial==BanList[i])
			return i;
	return -1;
}

int cHouse::AddFriend(P_CHAR pc)
{
	int i=FindFriend(pc);
	UOXSOCKET s=calcSocketFromChar(pc);
	if(i==-1)
	{
		FriendList.push_back(pc->serial);
		sysmessage(s,"You are now a Friend of the house!");
		return 1;
	}
	else if(i==-2)
	{
		sysmessage(s, "You are the Owner of this house!");
		return 3;
	}
	else
	{
		sysmessage(s,"You are already a Friend of the house!");
		return 2;
	}
	return 3;
}

int cHouse::AddBan(P_CHAR pc)
{
	int i=FindBan(pc);
	UOXSOCKET s=calcSocketFromChar(pc);
	if(i==-1)
	{
		BanList.push_back(pc->serial);
		sysmessage(s,"You are now banned from the house!");
		return 1;
	}
	else if(i==-2)
	{
		sysmessage(s,"You are the Owner of this house!");
		return 3;
	}
	else
	{
		sysmessage(s,"You are already banned from the house!");
		return 2;
	}
	return 3;
}

bool cHouse::RemoveFriend(P_CHAR pc)
{
	int i=FindFriend(pc);
	if(i>-1)
	{
		FriendList.erase(FriendList.begin() + i);
		return 1;
	}
	else
		return 0;
}

bool cHouse::RemoveBan(P_CHAR pc)
{
	int i=FindBan(pc);
	if(i>-1)
	{
		BanList.erase(BanList.begin()+i);
		return 1;
	}
	else
		return 0;
}
