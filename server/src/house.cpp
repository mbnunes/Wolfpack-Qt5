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
void cHouseManager::AddHome(int s,int i)
{
	int x,y,key,loopexit=0;//where they click, and the house/key items
	signed char z;
	int sx=0,sy=0;                                  //space around the house needed
	int k,l,tmp;                                            //Temps
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
	char name[512];
	P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]);

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
					tmp=hstr2num(script2);
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
			if(!(this->HomeBuildSite(x,y,z,sx,sy)))
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
		
		P_ITEM pHouse=Items->SpawnItem(DEREF_P_CHAR(pc_currchar), 1,(char*)temp,0,(id1<<8)+id2,0,0);
		if (!pHouse) return;		
		
		
		pc_currchar->making=0;
		
		pHouse->MoveTo(x,y,z);
		pHouse->priv=0;
		pHouse->more4 = itemsdecay; // set to 1 to make items in houses decay
		pHouse->morex=hdeed; // crackerjack 8/9/99 - for converting back *into* deeds
		pHouse->SetOwnSerial(pc_currchar->serial);
		if (!hitem[0] && !boat)
		{
			teleport(DEREF_P_CHAR(pc_currchar));
			all_items(s);
			return;//If there's no extra items, we don't really need a key, or anything else do we? ;-)
		}
		
		if(boat) //Boats
		{
			if(!Boats->Build(s,DEREF_P_ITEM(pHouse), id2))
			{
				Items->DeleItem(pHouse);
				return;
			} 
		}
		
		if (i)//Boats->.. Moved from up there ^
			Items->DeleItem(pc_currchar->fx1); // this will del the deed no matter where it is
		
		pc_currchar->fx1=-1; //reset fx1 so it does not interfere
		// bugfix LB ... was too early reseted 
		
		//Key...
		
		if (id2>=112&&id2<=115) key=Items->SpawnItem(s, DEREF_P_CHAR(pc_currchar), 1, "a tent key", 0, 0x10, 0x10,0, 0,1,1);//iron key for tents
		else if(id2<=0x18) key=Items->SpawnItem(s,DEREF_P_CHAR(pc_currchar),1,"a ship key",0,0x10,0x13,0,0,1,1);//Boats -Rusty Iron Key
		else key=Items->SpawnItem(s, DEREF_P_CHAR(pc_currchar), 1, "a house key", 0, 0x10, 0x0F, 0, 0,1,1);//gold key for everything else;
		
		items[key].more1=pHouse->ser1;//use the house's serial for the more on the key to keep it unique
		items[key].more2=pHouse->ser2;
		items[key].more3=pHouse->ser3;
		items[key].more4=pHouse->ser4;
		items[key].type=7;
		items[key].priv=2; // Newbify key..Ripper
        
		ITEM key2=Items->SpawnItem(s, DEREF_P_CHAR(pc_currchar), 1, "a house key", 0, 0x10, 0x0F, 0, 0,1,1);
		P_ITEM bankbox = pc_currchar->GetBankBox();
		P_ITEM p_key=MAKE_ITEMREF_LR(key2);
		p_key->more1=pHouse->ser1;
		p_key->more2=pHouse->ser2;
		p_key->more3=pHouse->ser3;
		p_key->more4=pHouse->ser4;
		p_key->type=7;
		p_key->priv=2;
		bankbox->AddItem(p_key);
		
		if(nokey) 
		{
			Items->DeleItem(key); // No key for .. nokey items
			Items->DeleItem(key2);
		}
		
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
							l=Items->CreateScriptItem(s,str2num(script2),0);//This opens the item script... so we gotta keep track of where we are with the other script.
							openscript("house.scp");
							fseek(scpfile, pos, SEEK_SET);
							strcpy((char*)script1, "ITEM");
							items[l].magic=2;//Non-Movebale by default
							items[l].priv=0;//since even things in houses decay, no-decay by default
							items[l].pos.x=x;
							items[l].pos.y=y;
							items[l].pos.z=z;
							items[l].SetOwnSerial(pc_currchar->serial);
						}
						if (!(strcmp((char*)script1,"DECAY")))
						{
							items[l].priv |= 0x01;
						}
						if (!(strcmp((char*)script1,"NODECAY")))
						{
							items[l].priv=0;
						}
						if (!(strcmp((char*)script1,"PACK")))//put the item in the Builder's Backpack
						{
							items[l].SetContSerial(items[packitem(DEREF_P_CHAR(pc_currchar))].serial);
							items[l].pos.x=rand()%90+31;
							items[l].pos.y=rand()%90+31;
							items[l].pos.z=9;
						}
						if (!(strcmp((char*)script1,"MOVEABLE")))
						{
							items[l].magic=1;
						}
						if (!(strcmp((char*)script1,"LOCK")))//lock it with the house key
						{
							items[l].more1=pHouse->ser1;
							items[l].more2=pHouse->ser2;
							items[l].more3=pHouse->ser3;
							items[l].more4=pHouse->ser4;
						}
						if (!(strcmp((char*)script1,"X")))//offset + or - from the center of the house:
						{
							items[l].pos.x=x+str2num(script2);
						}
						if (!(strcmp((char*)script1,"Y")))
						{ 
							items[l].pos.y=y+str2num(script2);
						}
						if (!(strcmp((char*)script1,"Z")))
						{
							items[l].pos.z=z+str2num(script2);
						}
					}
				}
				while ( (strcmp((char*)script1,"}")) && (++loopexit < MAXLOOPS) );
				
				if (items[l].isInWorld()) 
					mapRegions->Add(&items[l]);  //add to mapRegions
				closescript();
			}
		}
		all_items(s);//make sure they have all the items Sent....

		if (!(norealmulti))
		{
			pc_currchar->pos.x=x+cx; //move char inside house
			pc_currchar->pos.y=y+cy;
			pc_currchar->dispz=pc_currchar->pos.z=z+cz;
			//clConsole.send("Z: %i Offset: %i Char: %i Total: %i\n",z,cz,pc_currchar->pos.z,z+cz);
			teleport(DEREF_P_CHAR(pc_currchar));
		}
	}
}

// turn a house into a deed if possible. - crackerjack 8/9/99
// s = socket of player
// i = house item # in items[]
// morex on the house item must be set to deed item # in items.scp.
void deedhouse(UOXSOCKET s, int i) // Ripper & AB
{
	int ii,loopexit=0;
	int x1, y1, x2, y2;
	unsigned char ser1, ser2, ser3, ser4;
	int playerCont;
	if( i == -1 ) return;
	P_CHAR pc = MAKE_CHARREF_LR(currchar[s]);
	playerCont = packitem( DEREF_P_CHAR(pc) );
	int mapitemptr,mapitem,mapchar,a,checkgrid,increment,StartGrid,getcell,ab;		
	if(pc->Owns(&items[i]) || pc->isGM())
	{
		Map->MultiArea(i, &x1,&y1,&x2,&y2);
		
		ii=Items->SpawnItemBackpack2(s, items[i].morex, 0);        // need to make before delete
		if( ii == -1 ) return;
		sprintf((char*)temp, "Demolishing House %s", items[i].name);
		sysmessage( s, (char*)temp );
		ser1 = items[i].ser1;
		ser2 = items[i].ser2;
		ser3 = items[i].ser3;
		ser4 = items[i].ser4;
		Items->DeleItem(i);
		sprintf((char*)temp, "Converted into a %s.", items[ii].name);
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
				mapitemptr=-1;
				mapitem=-1;
				mapchar=-1;
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
								P_ITEM pPvDeed = Items->SpawnItem(DEREF_P_CHAR(pc), 1, (char*)temp, 0, 0x14F0, 0, 1);
								pPvDeed->type = 217;
								pPvDeed->value = 2000;
								RefreshItem( pPvDeed );
								sprintf((char*)temp, "Packed up vendor %s.", mapchar->name);
								Npcs->DeleteChar( DEREF_P_CHAR(mapchar) );
								sysmessage(s, (char*)temp);
							}
						}
					}
					else if( mapitem != NULL )
					{
						if( mapitem->pos.x >= x1 && mapitem->pos.y >= y1 && mapitem->pos.x <= x2 && mapitem->pos.y <= y2 )
						{
							Items->DeleItem(DEREF_P_ITEM(mapitem));
						}
					}
				} 
			}
		}
		killkeys(items[i].serial);
		sysmessage(s,"All house items and keys removed.");
		
		pc->pos.z = pc->dispz = Map->MapElevation(pc->pos.x, pc->pos.y);
		teleport(DEREF_P_CHAR(pc));
		return;
	}
}

// removes houses - without regions. slow but necassairy for house decay
// LB 19-September 2000
void killhouse(ITEM i)
{
	P_ITEM pi;
	P_CHAR pc;
	int x1, y1, x2, y2;
	Map->MultiArea(i, &x1, &y1, &x2, &y2);
	
	pi = MAKE_ITEM_REF(i);
	SERIAL serial = pi->serial;
	
	unsigned int a;
	for (a = 0; a < charcount; a++) // deleting npc-vendors attched to the decying house
	{
		pc = MAKE_CHARREF_LR(a);
		if (pc->pos.x >= x1 && pc->pos.y >= y1 && pc->pos.x <= x2 && pc->pos.y <= y2 && !pc->free)
		{
			if (pc->npcaitype == 17) // player vendor in right place, delete !
			{
				Npcs->DeleteChar(a);
			}
		}                                           
	}
	
	for (a = 0; a < itemcount; a++) // deleting itmes inside house
	{
		pi = MAKE_ITEM_REF(a);
		if (pi->pos.x >= x1 && pi->pos.y >= y1 && pi->pos.x <= x2 && pi->pos.y <= y2 && !pi->free)
		{
			if (pi->type != 202) // dont delete guild stones !
			{
				Items->DeleItem(a);        
			}
		}
	}
	
	// deleting house keys
	killkeys(serial);   
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
int check_house_decay()
{
   bool is_house;
   int houses=0;   
   int decayed_houses=0;
   unsigned long int timediff;
   unsigned long int ct=getNormalizedTime();
   
   AllItemsIterator iter_items;
   for (iter_items.Begin(); iter_items.GetData() != iter_items.End(); iter_items++) 
   {   
	 P_ITEM pi = iter_items.GetData(); // there shouldnt be an error here !		 
     is_house = ishouse(pi);	 
	 if (is_house && !pi->free)
	 {
       
		// its a house -> check its unused time

		//clConsole.send("id2: %x time_unused: %i max: %i\n",pi->id2,pi->time_unused,server_data.housedecay_secs);

		if (pi->time_unused>SrvParms->housedecay_secs) // not used longer than max_unused time ? delete the house
		{          
			decayed_houses++;
            sprintf((char*)temp,"%s decayed! not refreshed for > %i seconds!\n",pi->name,SrvParms->housedecay_secs);
			LogMessage((char*)temp);
			killhouse(DEREF_P_ITEM(pi));
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

void killkeys(SERIAL serial) // Crackerjack 8/11/99
{
	if (serial == INVALID_SERIAL)
		return;
	AllItemsIterator iter_items;
	for (iter_items.Begin(); iter_items.GetData() != iter_items.End(); iter_items++)
	{
		P_ITEM pi = iter_items.GetData();
		if (pi->type == 7 && calcserial(pi->more1, pi->more2, pi->more3, pi->more4) == serial)
		{
			Items->DeleItem(DEREF_P_ITEM(pi));
		}
	}
	return;
}

// Crackerjack 8/12/99 - House List Functions

// Checks if somebody is on the house list.
// on_hlist(int h (items[] index for house), unsigned char s1, s2, s3, s4 (char serial),
//		int *li (pointer to variable to put items[] index of list item in or NULL))
// Returns:
// 0 - Character is not on house list
// Anything else - Character is on house list, type # is returned.
int on_hlist(int h, unsigned char s1, unsigned char s2, unsigned char s3, unsigned char s4, int *li)
{
	if(h<0)
		return 0;
	int  StartGrid=mapRegions->StartGrid(items[h].pos.x,items[h].pos.y);
	unsigned int increment=0;
	for (unsigned int checkgrid=StartGrid+(increment*mapRegions->GetColSize());increment<3;increment++, checkgrid=StartGrid+(increment*mapRegions->GetColSize()))
	{
		for (int a=0;a<3;a++)
		{
			vector<SERIAL> vecEntries = mapRegions->GetCellEntries(checkgrid+a);
			for ( unsigned int k = 0; k < vecEntries.size(); k++)
			{
				P_ITEM mapitem = FindItemBySerial(vecEntries[k]);
				if (mapitem != NULL)
				{
				   	if((mapitem->morey==items[h].serial)&&
				       (mapitem->more1==s1)&&(mapitem->more2==s2)&&
				       (mapitem->more3==s3)&&(mapitem->more4==s4)) 
					{
						if(li!=NULL) *li = DEREF_P_ITEM(mapitem);
						return mapitem->morex;
					}		          
				}								
			}
		}
	}// end of mapregions loop
	
	return 0;

}
/*
int on_hlist(int h, unsigned char s1, unsigned char s2, unsigned char s3, unsigned char s4, int *li)
{
	int cc;
	int cl=-1;
	int ci=-1;

	cc=mapRegions->GetCell(items[h].x,items[h].y);
	do {
		cl=mapRegions->GetNextItem(cc, cl);
		if(cl==-1) break;
		ci=mapRegions->GetItem(cc, cl);
		if(ci<1000000) {
			if((items[ci].contserial==items[h].serial)&&
				(items[ci].more1==s1)&&(items[ci].more2==s2)&&
				(items[ci].more3==s3)&&(items[ci].more4==s4)) 
			{
				if(li!=NULL) *li=ci;
				return items[ci].morex;
			}
		}
	} while(ci!=-1);
	return 0;
}
*/
// Adds somebody to a house list.
// add_hlist(int c (chars[] index), int h (items[] index for house), int t (list type))
// Returns:
// 1 - Successful addition to house list
// 2 - Character is already on a house list
// 3 - Character is not on property
int add_hlist(int c, int h, int t)
{
	int sx, sy, ex, ey;
	
	P_CHAR pc = MAKE_CHAR_REF(c);

	if(on_hlist(h, pc->ser1, pc->ser2, pc->ser3, pc->ser4, NULL))
		return 2;

	Map->MultiArea(h, &sx,&sy,&ex,&ey);
	// Make an object with the character's serial & the list type
	// and put it "inside" the house item.
	if(pc->pos.x>=sx&&pc->pos.y>=sy&&pc->pos.x<=ex&&pc->pos.y<=ey)
	{
		ITEM i = Items->MemItemFree();	
		P_ITEM pi = MAKE_ITEM_REF(i);
		pi->Init();

		pi->morex=t;
		pi->more1 = pc->ser1;
		pi->more2 = pc->ser2;
		pi->more3 = pc->ser3;
		pi->more4 = pc->ser4;
		pi->morey=items[h].serial;

		pi->priv=0; // no decay !!
		pi->visible=0;
		strcpy(pi->name,"friend of house");

		pi->pos.x=items[h].pos.x;
		pi->pos.y=items[h].pos.y;
		pi->pos.z=items[h].pos.z;

		mapRegions->Add(pi);
		return 1;
	}

	return 3;
}
// Removes somebody from a house list.
// del_hlist(int c (chars[] index), int h (items[] index for house))
// Returns:
// 0 - Player was not on a list
// # - What list the player was on if any.
int del_hlist(int c, int h)
{
	int hl, li;

	hl=on_hlist(h, chars[c].ser1, chars[c].ser2, chars[c].ser3, chars[c].ser4, &li);
	if(hl) {
		mapRegions->Remove(&items[li]);
		Items->DeleItem(li);
	}
	return(hl);
}

// Handles house commands from friends of the house. - Crackerjack 8/12/99
void house_speech(int s, char *msg)	// msg must already be capitalized
{
	int fr;
	P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]);
	if(s<0 || s>MAXCLIENT) return;
	P_ITEM pi_multi = findmulti(pc_currchar->pos);
	if(pi_multi == NULL) return; // not in a house, so we don't care.
	fr=on_hlist(DEREF_P_ITEM(pi_multi), pc_currchar->ser1, pc_currchar->ser2,
		pc_currchar->ser3, pc_currchar->ser4, NULL);

	if(fr!=H_FRIEND && !pc_currchar->Owns(pi_multi) )
		return; // not a friend or owner, so we don't care.

	if(strstr(msg, "I BAN THEE")) { // house ban
		addid1[s] = pi_multi->ser1;
		addid2[s] = pi_multi->ser2;
		addid3[s] = pi_multi->ser3;
		addid4[s] = pi_multi->ser4;
		target(s, 0, 1, 0, 229, "Select person to ban from house.");
		return;
	}
	if(strstr(msg, "REMOVE THYSELF")) { // kick out of house
		addid1[s] = pi_multi->ser1;
		addid2[s] = pi_multi->ser2;
		addid3[s] = pi_multi->ser3;
		addid4[s] = pi_multi->ser4;
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

bool cHouseManager::HomeBuildSite(int x, int y, int z, int sx, int sy)
{
	signed int checkz;
	//char statc;
	int checkx;
	int checky;
	int ycount=0;
	checkx=x-(sx/2);
	for (;checkx<(x+(sx/2));checkx++)
	{
		for (checky=y-(sy/2);checky<(y+(sy/2));checky++)
		{
			checkz=Map->MapElevation(checkx,checky);
			if ((checkz>(z-7))&&(checkz<(z+7)))
			{
				ycount++;
			}
			//	statc=Map->StaHeight(checkx,checky,checkz);
			//	if (statc>0)
			//		statb=true;
		}
	}
	if (ycount==(sx*sy)) //&& (statb==false))
		return true;
	else
		return false;
}
