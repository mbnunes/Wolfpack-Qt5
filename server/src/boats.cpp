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
//	Wolfpack Homepage: http://www.wpdev.com/
//========================================================================================

 //Boats->cpp by Zippy Started on 7/12/99

// Mapregion stuff + boat-blocking corrected/touched by LB 7/24/99

#include "boats.h"


#undef DBGFILE
#define DBGFILE "boats.cpp" 

#define X 0
#define Y 1

//============================================================================================
//UooS Item translations - You guys are the men! :o)

//[4]=direction of ship
//[4]=Which Item (PT Plank, SB Plank, Hatch, TMan)
//[2]=Coord (x,y) offsets
signed short int iSmallShipOffsets[4][4][2]=
// X  Y  X  Y  X  Y  X  Y
{ -2, 0, 2, 0, 0,-4, 1, 4,//Dir
   0,-2, 0, 2, 4, 0,-4, 0,
   2, 0,-2, 0, 0, 4, 0,-4,
   0, 2, 0,-2,-4, 0, 4, 0 };
//  P1    P2   Hold  Tiller
signed short int iMediumShipOffsets[4][4][2]=
// X  Y  X  Y  X  Y  X  Y
{ -2, 0, 2, 0, 0,-4, 1, 5,
   0,-2, 0, 2, 4, 0,-5, 0,
   2, 0,-2, 0, 0, 4, 0,-5,
   0, 2, 0,-2,-4, 0, 5, 0 };
signed short int iLargeShipOffsets[4][4][2]=
// X  Y  X  Y  X  Y  X  Y
{ -2,-1, 2,-1, 0,-5, 1, 5,
   1,-2, 1, 2, 5, 0,-5, 0,
   2, 1,-2, 1, 0, 5, 0,-5,
  -1, 2,-1,-2,-5, 0, 5, 0 };
//Ship Items
//[4] = direction
//[6] = Which Item (PT Plank Up,PT Plank Down, SB Plank Up, SB Plank Down, Hatch, TMan)
unsigned char cShipItems[4][6]=
{(unsigned char)0xB1,(unsigned char)0xD5,(unsigned char)0xB2,(unsigned char)0xD4,(unsigned char)0xAE,(unsigned char)0x4E,
 (unsigned char)0x8A,(unsigned char)0x89,(unsigned char)0x85,(unsigned char)0x84,(unsigned char)0x65,(unsigned char)0x53,
 (unsigned char)0xB2,(unsigned char)0xD4,(unsigned char)0xB1,(unsigned char)0xD5,(unsigned char)0xB9,(unsigned char)0x4B,
 (unsigned char)0x85,(unsigned char)0x84,(unsigned char)0x8A,(unsigned char)0x89,(unsigned char)0x93,(unsigned char)0x50 };
//============================================================================================


void sendinrange(ITEM i)//Send this item to all online people in range
{//(Decided this was better than writting 1000 for loops to send an item.
	for(int a=0;a<now;a++)
	{
		if(perm[a] && iteminrange(a,i,VISRANGE))
			senditem(a,i);
	}
}


int dist(int a, int b, int type)//Distance from A to B (type = 1 (a is a char) type=0 (a is an item))
{
	int xa,ya;

	if(type)
	{
		P_CHAR pcc_a = MAKE_CHARREF_LOGGED(a,err);	
        if (err) 
		{
          return 100;
		}

		xa = pcc_a->pos.x;
		ya = pcc_a->pos.y;

	} else {
		P_ITEM pcc_a = MAKE_ITEM_REF(a);
		if (pcc_a == NULL)
			return ~0;
		xa = pcc_a->pos.x;
		ya = pcc_a->pos.y;
	}
	int dx = abs(xa - items[b].pos.x);
	int dy = abs(ya - items[b].pos.y);
	return (int) (hypot(dx, dy));
}

P_ITEM findmulti(Coord_cl pos) //Sortta like getboat() only more general... use this for other multi stuff!
{
	int lastdist=30;
	P_ITEM multi = NULL;
	int ret;

	int	StartGrid=mapRegions->StartGrid(pos.x,pos.y);
			
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
					if (mapitem->id1>=0x40)
					{
						/*dx=abs(pos.x - items[mapitem].pos.x);
						dy=abs(pos.y - items[mapitem].pos.y);
						ret=(int)(hypot(dx, dy));
						*/
						ret = pos.distance(mapitem->pos);
						if (ret<=lastdist)
						{
							lastdist=ret;
							if (inmulti(pos, mapitem))
								multi = mapitem;
						}
					}
				}
			}
		}//For a
	}//For checkgrid
	return multi;
}

bool inmulti(Coord_cl pos, P_ITEM pi)//see if they are in the multi at these chords (Z is NOT checked right now)
{
	int j;
	SI32 length;			// signed long int on Intel
	st_multi multi;
	UOXFile *mfile;
	Map->SeekMulti(pi->id()-0x4000, &mfile, &length);
	length=length/sizeof(st_multi);
	if (length == -1 || length>=17000000)//Too big...
	{
		sprintf((char*)temp,"inmulti() - Bad length in multi file. Avoiding stall. (Item Name: %s)\n", pi->name );
		LogError( (char*)temp ); // changed by Magius(CHE) (1)
		length = 0;
	}
	for (j=0;j<length;j++)
	{
		mfile->get_st_multi(&multi);
		/*clConsole.send("DEBUG: Multi { vis=%i - (%i,%i) } check(%i,%i,%i)   -   total(%i,%i)\n",
		multi.visible,multi.x,multi.y,x,y,z,multi.x+items[m].x,items[m].y+multi.y);*/
		if ((multi.visible)&&(pi->pos.x+multi.x == pos.x) && (pi->pos.y+multi.y == pos.y))
		{
			return true;
		}
	}
	return false;
}


cBoat::cBoat()//Consturctor
{
	return;
}

cBoat::~cBoat()//Destructor
{
}

void cBoat::PlankStuff(UOXSOCKET s, ITEM p)//If the plank is opened, double click Will send them here
{
	P_CHAR pc_cs,pc_b;

	pc_cs=MAKE_CHARREF_LR(currchar[s]);	

	int a,b,serhash=pc_cs->serial%HASHMAX;
	P_ITEM boat = MAKE_ITEM_REF(GetBoat(s));
	if(boat == NULL)//They aren't on a boat, so put then on the plank.
	{
		// LB, bugfix for tillerman not reacting if the boat was entered via plank !

		// we need to get the boat again after beaming the character to the boat's plank
		// otherweise only -1's will be added to the boat hash-table 
        
		pc_cs->MoveTo(items[p].pos.x,items[p].pos.y,items[p].pos.z+5);

		pc_cs->multis=-3; // we have to trick getboat to start the search !!!
		                              // will be corrected automatically by setserial...

		P_ITEM boat2 = MAKE_ITEM_REF(GetBoat(s));
		if (boat2 == NULL)
			return;
	
		for(a=0;a<cownsp[serhash].max;a++)//Put all their Pets/Hirlings on the boat too
		{
			b=cownsp[serhash].pointer[a];

			if (b!=-1) // never log -1's that indicate non existance !!!
			{
			   pc_b=MAKE_CHARREF_LOGGED(b,err);							   
			   if (!err && boat2 != NULL)
			   if (pc_b->isNpc() && pc_cs->Owns(pc_b))
			   {
				  pc_b->MoveTo(boat2->pos.x+1, boat2->pos.y+1, boat2->pos.z+2);
                  pc_b->SetMultiSerial(boat2->serial);
				  teleport(b);
			   }
			}
		}
	

        OpenPlank(p); //lb

		if (boat2 != NULL) // now set the char coords to the boat !!!
		{
			pc_cs->MoveTo(boat2->pos.x+1,boat2->pos.y+1,boat2->pos.z+2);
		}

		sysmessage(s,"you entered a boat");
        pc_cs->SetMultiSerial(boat2->serial);
	} else {
		LeaveBoat(s,p);//They are on a boat, get off
	}
	teleport(DEREF_P_CHAR(pc_cs));//Show them they moved.
}


void cBoat::LeaveBoat(UOXSOCKET s, ITEM p)//Get off a boat (dbl clicked an open plank while on the boat.
{
	P_CHAR pc_cs,pc_b;
	
	pc_cs=MAKE_CHARREF_LR(currchar[s]);

	//long int pos, pos2, length;
	int x,x2=items[p].pos.x;
	int y,y2=items[p].pos.y;
	signed char z=items[p].pos.z,mz,sz,typ;
	P_ITEM pBoat = MAKE_ITEM_REF(GetBoat(s));
	int a,b,serhash=pc_cs->serial%HASHMAX;
	// char o;
	
	if (pBoat == NULL) 
		return;
	
	for(x=x2-2;x<x2+3;x++)
	{
		for(y=y2-2;y<y2+3;y++)
		{
			sz = Map->StaticTop(x,y,z); // MapElevation() doesnt work cauz we are in a multi !!
			
			mz = Map->MapElevation(x,y);
			if (sz == illegal_z) 
				typ=0;
			else 
				typ=1;
			//o=Map->o_Type(x,y,z);
			
			if((typ==0 && mz!=5) || (typ==1 && sz!=-5))// everthing the blocks a boat is ok to leave the boat ... LB
			{
				for(a=0;a<cownsp[serhash].max;a++)//Put all their Pets/Hirlings on the boat too
				{
					b=cownsp[serhash].pointer[a];

					if (b!=-1) // never log -1's that indicate non existance !!!
					{
					   pc_b=MAKE_CHARREF_LOGGED(b,err);			           
					   if (!err)
					   {
						if (pc_b->isNpc() && pc_cs->Owns(pc_b) && inrange1p(currchar[s], b)<=15)
						{
							pc_b->MoveTo(x,y, typ ? sz : mz);
							
							if(pc_b->multis>-1)
							{
								if( pBoat->serial != INVALID_SERIAL ) 
									cmultisp.remove(pBoat->serial, pc_b->serial);
//									removefromptr(&cmultisp[items[boat].serial%HASHMAX], b);
								pc_b->multis=-1;
							}
							
							teleport(b);
						}
					   }
					}
				}
				
                if( pBoat->serial != INVALID_SERIAL ) 
					cmultisp.remove(pBoat->serial, chars[currchar[s]].serial);
//					removefromptr(&cmultisp[items[boat].serial%HASHMAX], currchar[s]);
				pc_cs->multis=-1;
				
				if (typ) 
					pc_cs->MoveTo(x, y, sz); 
				else  
					pc_cs->MoveTo(x, y, mz);
				
				sysmessage(s,"You left the boat.");			
				return;
			}
		}//for y
	}//for x
	sysmessage(s,"You cannot get off here!");
}

void cBoat::OpenPlank(ITEM p)//Open, or close the plank (called from keytarget() )
{
	switch(items[p].id2)
	{
		//Open plank->
		case (unsigned char)0xE9: items[p].id2=(unsigned char)0x84; break;
		case (unsigned char)0xB1: items[p].id2=(unsigned char)0xD5; break;
		case (unsigned char)0xB2: items[p].id2=(unsigned char)0xD4; break;
		case (unsigned char)0x8A: items[p].id2=(unsigned char)0x89; break;
		case (unsigned char)0x85: items[p].id2=(unsigned char)0x84; break;
		//Close Plank->
		case (unsigned char)0x84: items[p].id2=(unsigned char)0xE9; break;
		case (unsigned char)0xD5: items[p].id2=(unsigned char)0xB1; break;
		case (unsigned char)0xD4: items[p].id2=(unsigned char)0xB2; break;
		case (unsigned char)0x89: items[p].id2=(unsigned char)0x8A; break;
		default: { sprintf((char*)temp,"WARNING: Invalid plank ID called! Plank %i '%s' [%x %x]\n",p,items[p].name,items[p].id1,items[p].id2); LogWarning( (char*)temp ); break; }
	}
}

bool cBoat::Build(UOXSOCKET s, ITEM b, char id2)//Build a boat! (Do stuff NESSICARY for boats, called from buildhouse() )
{
	P_CHAR pc_cs=MAKE_CHARREF_LRV(currchar[s],false);

	int nid2=id2;
	tile_st tile;
	map_st map;
	land_st land;

	P_ITEM pBoat=MAKE_ITEMREF_LRV(b,false);
	if( !pBoat ) 
	{
		sysmessage(s, "There was an error creating that boat.");
		return false;
	}
	
	if(id2!=0x00 && id2!=0x04 && id2!=0x08 && id2!=0x0C && id2!=0x10 && id2!=0x14)//Valid boat ids (must start pointing north!)
	{
		sysmessage(s, "The deed is broken, please contact a Game Master.");
		return false;
	}
	//Start checking for a valid possition:

	map = Map->SeekMap0(pBoat->pos.x, pBoat->pos.y);
	switch(map.id)
	{
	//water tiles:
	case 0x00A8:
	case 0x00A9:
	case 0x00AA:
	case 0x00Ab:
	case 0x0136:
	case 0x0137:
	case 0x3FF0:
	case 0x3FF1:
	case 0x3FF2:
	case 0x2FF3:
		break;
	//Lava tiles:
	case 0x01F4:
	case 0x01F5:
	case 0x01F6:
	case 0x01F7:
		break;
	default:
		Map->SeekTile(((buffer[s][0x11]<<8)+buffer[s][0x12]), &tile);
		if(!(strstr((char *) tile.name, "water") || strstr((char *) tile.name, "lava")))
		{
			Map->SeekLand(map.id, &land);
			if (!(land.flag1&0x80))//not a "wet" tile
			{
				sysmessage(s, "You cannot place your boat there.");
				return false;
			}
		}
	}
	
	// Okay we found a good  place....

	pBoat->more1=id2;//Set min ID
	pBoat->more2=nid2+3;//set MAX id
	pBoat->type=117;//Boat type
	pBoat->pos.z=-5;//Z in water
	strcpy(pBoat->name,"a mast");//Name is something other than "%s's house"
	
	P_ITEM pTiller=Items->SpawnItem(DEREF_P_CHAR(pc_cs),1,"a tiller man",0,0x3E4E,0,0);
	if( !pTiller ) return false;
	pTiller->pos.z=-5;
	pTiller->priv=0;

	P_ITEM pPlankR=Items->SpawnItem(DEREF_P_CHAR(pc_cs),1,"#",0,0x3EB2,0,0);//Plank2 is on the RIGHT side of the boat
	if( !pPlankR ) return false;
	pPlankR->type=117;
	pPlankR->type2=3;
	pPlankR->more1=pBoat->ser1;//Lock this item!
	pPlankR->more2=pBoat->ser2;
	pPlankR->more3=pBoat->ser3;
	pPlankR->more4=pBoat->ser4;
	pPlankR->pos.z=-5;
	pPlankR->priv=0;//Nodecay

	P_ITEM pPlankL=Items->SpawnItem(DEREF_P_CHAR(pc_cs),1,"#",0,0x3EB1,0,0);//Plank1 is on the LEFT side of the boat
	if( !pPlankL ) return false;
	pPlankL->type=117;//Boat type
	pPlankL->type2=3;//Plank sub type
	pPlankL->more1=pBoat->ser1;
	pPlankL->more2=pBoat->ser2;//Lock this
	pPlankL->more3=pBoat->ser3;
	pPlankL->more4=pBoat->ser4;
	pPlankL->pos.z=-5;
	pPlankL->priv=0;

	P_ITEM pHold=Items->SpawnItem(DEREF_P_CHAR(pc_cs),1,"#",0,0x3EAE,0,0);
	if( !pHold ) return false;
	pHold->more1=pBoat->ser1;//Lock this too :-)
	pHold->more2=pBoat->ser2;
	pHold->more3=pBoat->ser3;
	pHold->more4=pBoat->ser4;
	pHold->type=1;//Conatiner
	pHold->pos.z=-5;
	pHold->priv=0;
	
	pBoat->moreb1=pTiller->ser1;//Tiller ser stored in boat's Moreb
	pBoat->moreb2=pTiller->ser2;
	pBoat->moreb3=pTiller->ser3;
	pBoat->moreb4=pTiller->ser4;
	pBoat->morex=pPlankL->serial;//Store the other stuff anywhere it will fit :-)
	pBoat->morey=pPlankR->serial;
	pBoat->morez=pHold->serial;
	
	switch(id2)//Give everything the right Z for it size boat
	{
	case 0x00:
	case 0x04:
		pTiller->pos.x=pBoat->pos.x+1;
		pTiller->pos.y=pBoat->pos.y+4;
		pPlankR->pos.x=pBoat->pos.x+2;
		pPlankR->pos.y=pBoat->pos.y;
		pPlankL->pos.x=pBoat->pos.x-2;
		pPlankL->pos.y=pBoat->pos.y;
		pHold->pos.x=pBoat->pos.x;
		pHold->pos.y=pBoat->pos.y-4;
		break;
	case 0x08:
	case 0x0C:
		pTiller->pos.x=pBoat->pos.x+1;
		pTiller->pos.y=pBoat->pos.y+5;
		pPlankR->pos.x=pBoat->pos.x+2;
		pPlankR->pos.y=pBoat->pos.y;
		pPlankL->pos.x=pBoat->pos.x-2;
		pPlankL->pos.y=pBoat->pos.y;
		pHold->pos.x=pBoat->pos.x;
		pHold->pos.y=pBoat->pos.y-4;
		break;
	case 0x10:
	case 0x14:
		pTiller->pos.x=pBoat->pos.x+1;
		pTiller->pos.y=pBoat->pos.y+5;
		pPlankR->pos.x=pBoat->pos.x+2;
		pPlankR->pos.y=pBoat->pos.y-1;
		pPlankL->pos.x=pBoat->pos.x-2;
		pPlankL->pos.y=pBoat->pos.y-1;
		pHold->pos.x=pBoat->pos.x;
		pHold->pos.y=pBoat->pos.y-5;
		break;
	}
	mapRegions->Add(pTiller);//Make sure everything is in da regions!
	mapRegions->Add(pPlankL);
	mapRegions->Add(pPlankR);
	mapRegions->Add(pHold);
	mapRegions->Add(pBoat);
	
	//their x pos is set by BuildHouse(), so just fix their Z...
	pc_cs->pos.z = pc_cs->dispz = pBoat->pos.z+3;//Char Z, try and keep it right.
    pc_cs->SetMultiSerial(DEREF_P_ITEM(pBoat));
	return true;
}

int cBoat::GetBoat(UOXSOCKET s)//get the closest boat to the player and check to make sure they are on it
{	
	
	PC_CHAR pcc_cs=MAKE_CHARREF_LOGGED(currchar[s],err);
	if (err)
	{
		return -1;
	}

	ITEM boat=-1;
	P_ITEM pi_boat = NULL;

    if (pcc_cs->multis>0) return calcItemFromSer( pcc_cs->multis );
    else if (pcc_cs->multis==-1) return -1;
	else 
	{
		pi_boat = findmulti(pcc_cs->pos);
		if (pi_boat != NULL)//if we found a boat, make sure they are in it
			if(!inmulti(pcc_cs->pos, pi_boat)) 
				pi_boat = NULL;

		return DEREF_P_ITEM(pi_boat);
	}
}


// This  Boat-blocking method is now WATER PROOF :-)
// Please don't TOUCH !!!
// Lord Binary 21 August 1999

// it doesnt check against dynamics yet, especially against other ships.
// hopefully coming soon

bool cBoat::Block(ITEM b, short int xmove, short int ymove, int dir)//Check to see if the boat is blocked in front of, behind, or next to it (Depending on direction)
// PARAM WARNING: xmove and ymove is unreferenced
{
	int ser, sz, zt, loopexit=0;
	short x = 0, y = 0, c;
	bool blocked = false;
	char type, size = 0, typ = 0;
	
	map_st map;
	land_st land;
	tile_st tile;
	cItem p1, p2, t, h;

    ser = calcserial(items[b].moreb1,items[b].moreb2,items[b].moreb3,items[b].moreb4);
	t	= items[calcItemFromSer( ser )];
	p1	= items[calcItemFromSer( items[b].morex )];
	p2	= items[calcItemFromSer( items[b].morey )];
	h	= items[calcItemFromSer( items[b].morez )];

	switch(dir)
	{
	case 6: // L
	case 7: // U & L
	case 0: // U
	case 1: // U & R
		x = min( t.pos.x, min( h.pos.x, min( p1.pos.x, p2.pos.x ) ) );
		y = min( t.pos.y, min( h.pos.y, min( p1.pos.y, p2.pos.y ) ) );
		if ( dir != 6 )
			type = 1;
		else if ( dir == 0 )
			type = 2;
		else 
			type = 3;
		break;

	case 2: // R
	case 3: // D & R
	case 4: // D
	case 5: // D & L
		x = max( t.pos.x, max( h.pos.x, max( p1.pos.x, p2.pos.x ) ) );
		y = min( t.pos.y, min( h.pos.y, min( p1.pos.y, p2.pos.y ) ) );
		if ( dir != 2 )
			type = 1;
		else if ( dir == 4 )
			type = 2;
		else 
			type = 3;
		break;
	}
	//small = 10x5, med = 11x5 large = 12x5
	switch(items[b].more1)//Now set what size boat it is and move the specail items
	{
	case 0x00:
	case 0x04:
		if ( type == 1 )
			size = 10;
		else if ( type == 2 )
			size = 5;
		else
			size = 7;
	case 0x08:
	case 0x0C:
		if ( type == 1 )
			size = 11;
		else if ( type == 2 )
			size = 5;
		else
			size = 7;
	case 0x10:
	case 0x14:
		if ( type == 1 )
			size = 12;
		else if ( type == 2 )
			size = 5;
		else
			size = 7;
		break;
	}

	if ( type == 1)
		y -= (size/2)-1;
	else
		x -= (size/2)-1;

	for ( c=0 ; c<size ; c++ )
	{
		if ( type == 1 )
			y++;
		else if ( type == 2 )
			x++;
		else // type == 3
		{
			x++;
			y++;
		}

		sz=Map->StaticTop(x,y, items[b].pos.z);

		if (sz==illegal_z) 
			typ=0; //0: map-tile 
		else 
			typ=1; //1: static-tile
		
		if (typ==0)
		{
			map=Map->SeekMap0( x, y );
			Map->SeekLand(map.id, &land);
			//clConsole.send("map type, water bit: %i\n",land.flag1&0x80);
			if (!(land.flag1&0x80)) 
				blocked = true;
		} else { // go through all statics of a given x,y...
			MapStaticIterator msi(x, y);
			staticrecord *stat;

			while ( (stat = msi.Next()) && (++loopexit < MAXLOOPS) )
			{
				msi.GetTile(&tile);
				zt=stat->zoff+tile.height;
					
				//for this part...: Bridges can be shown not valid,
				//so we will keep setting false until we hit a valid point,
				//when we hit a valid place, we'll stop, leave block as it was, 
				//if all points are invalid, block is true and we exit as normal.
				if (!(tile.flag1&0x80) && zt<=70) blocked = true;
				else if (strcmp((char*)tile.name, "water")) blocked = true;
				//if (zt>70) water = 1; // every static til with z>70 (mast height?) doesnt block, no matter what water-bit is has
			}
		}//if type....
		if ( blocked )
			return true;
	}//for c=soze
	return false;
}

void cBoat::Move(UOXSOCKET s, int dir, ITEM boat)
{//Move the boat and all it's items 1 square
	int tx=0,ty=0;
	int a, serial;
     
	P_ITEM pBoat = MAKE_ITEM_REF(boat);
	if (pBoat == NULL)
		return;

	serial = calcserial(pBoat->moreb1, pBoat->moreb2, pBoat->moreb3, pBoat->moreb4);
	if (serial == INVALID_SERIAL) return;
	P_ITEM pTiller = FindItemBySerial( serial );
	if(pTiller == NULL)
		return;
	
	P_ITEM pi_p1 = FindItemBySerial( pBoat->morex );
	if(pi_p1 == NULL) 
		return;

	P_ITEM pi_p2 = FindItemBySerial( pBoat->morey );
	if(pi_p2 == NULL) 
		return;

	P_ITEM pHold = FindItemBySerial( pBoat->morez );
	if(pHold == NULL) 
		return;

	Xsend(s,wppause,2);

	switch(dir&0x0F)//Which DIR is it going in?
	{
	case '\x00' : 
		ty--;
		break;
	case '\x01' : 
		tx++; 
		ty--;
		break;
	case '\x02' :
		tx++;
		break;
	case '\x03' :
		tx++;
		ty++;
		break;
	case '\x04' : 
		ty++;
		break;
	case '\x05' :
		tx--;
		ty++;
		break;
	case '\x06' : 
		tx--;
		break;
	case '\x07' : 
		tx--; 
		ty--;
		break;
	default:
		{
		  sprintf((char*)temp,"warning: Boat direction error: %i int boat %i\n",pBoat->dir&0x0F,pBoat->serial);
		  LogWarning((char*)temp);
		  break;
		}
	}

	if((pBoat->pos.x+tx<=200 || pBoat->pos.x+tx>=6000) && (pBoat->pos.y+ty<=200 || pBoat->pos.y+ty>=4900)) //bugfix LB
	{
		pBoat->type2=0;
		itemtalk(s,DEREF_P_ITEM(pTiller),"Arr, Sir, we've hit rough waters!");
		Xsend(s,restart,2);
		return;
	}
	if(Block(DEREF_P_ITEM(pBoat),tx,ty,dir))
	{
		pBoat->type2=0;
		itemtalk(s, DEREF_P_ITEM(pTiller), "Arr, somethings in the way!");
		Xsend(s,restart,2);
		return;
	}

	//Move all the special items
	pBoat->MoveTo(pBoat->pos.x+tx,pBoat->pos.y+ty,pBoat->pos.z);
	pTiller->MoveTo(pTiller->pos.x+tx, pTiller->pos.y+ty, pTiller->pos.z);
	pi_p1->MoveTo(pi_p1->pos.x+tx, pi_p1->pos.y+ty, pi_p1->pos.z);
	pi_p2->MoveTo(pi_p2->pos.x+tx, pi_p2->pos.y+ty, pi_p2->pos.z);
	pHold->MoveTo(pHold->pos.x+tx, pHold->pos.y+ty, pHold->pos.z);

    serial = pBoat->serial;
	
	vector<SERIAL> vecEntries = imultisp.getData(pBoat->serial);
	for (a = 0; a < vecEntries.size(); a++)
	{
		P_ITEM pi = FindItemBySerial(vecEntries[a]);
		if(pi != NULL)
		{
			pi->MoveTo(pi->pos.x+=tx, pi->pos.y+=ty, pi->pos.z);
			sendinrange(DEREF_P_ITEM(pi));
		}
	}

	vecEntries.clear();
	vecEntries = cmultisp.getData(pBoat->serial);
	for (a = 0; a < vecEntries.size(); a++)
	{
		P_CHAR pc_c = FindCharBySerial(vecEntries[a]);
		if (pc_c != NULL)
		{
			pc_c->MoveTo(pc_c->pos.x+=tx, pc_c->pos.y+=ty, pc_c->pos.z);
			teleport(DEREF_P_CHAR(pc_c));
		}
	}
	Xsend(s,restart,2);
}

void cBoat::TurnStuff(ITEM b, int i, int dir, int type)//Turn an item that was on the boat when the boat was turned.
{
	int dx, dy;	
    P_CHAR pc_i;

    if (i<0 || i>=imem || b<0 || b>=imem) return;

	if(type)//item
	{
		dx=items[i].pos.x-items[b].pos.x;//get their distance x
		dy=items[i].pos.y-items[b].pos.y;//and distance Y

        mapRegions->Remove(&items[i]);

		items[i].pos.x=items[b].pos.x;
		items[i].pos.y=items[b].pos.y;

		if(dir)//turning right
		{
			items[i].pos.x+=dy*-1;
			items[i].pos.y+=dx;
		} else {//turning left
			items[i].pos.x+=dy;
			items[i].pos.y+=dx*-1;
		}

		
		mapRegions->Add(&items[i]);

		sendinrange(i);

	} else {//Character


        pc_i=MAKE_CHARREF_LOGGED(i,err);	
		if (err) return;

		dx=pc_i->pos.x-items[b].pos.x;
		dy=pc_i->pos.y-items[b].pos.y;

		mapRegions->Remove(pc_i);
        
		pc_i->pos.x = items[b].pos.x;
		pc_i->pos.y = items[b].pos.y;
		
		if(dir)
		{
			pc_i->pos.x+=dy*-1;
			pc_i->pos.y+=dx;
		} else {
			pc_i->pos.x+=dy;
			pc_i->pos.y+=dx*-1;
		}
		//Set then in their new cell

		mapRegions->Add(pc_i);

		teleport(i);
	}
}

void cBoat::Turn(ITEM b, int turn)//Turn the boat item, and send all the people/items on the boat to turnboatstuff()
{
	int id2=items[b].id2 ,olddir = items[b].dir;
	unsigned short int Send[MAXCLIENT];
	int serial;
	int tiller, p1, p2, hold;
	int a,dir, d=0;

	if (b<0 || b>=imem) return; 

	for (a=0;a<now;a++)
	{
		if (iteminrange(a,b,BUILDRANGE) && perm[a])
		{
			Send[d]=a;
			Xsend(a,wppause,2);
			d++; 
		} 
		
	}

	//Of course we need the boat items!
	serial=calcserial(items[b].moreb1,items[b].moreb2,items[b].moreb3,items[b].moreb4);
	if(serial == INVALID_SERIAL) return;
	tiller = calcItemFromSer( serial );
	if(tiller<0) return;
	p1 = calcItemFromSer( items[b].morex );
	if(p1<0) return;
	p2 = calcItemFromSer( items[b].morey );
	if(p2<0) return;
	hold = calcItemFromSer( items[b].morez );
	if(hold<0) return;

    if (tiller<0 || tiller>=imem || p1<0 || p1>=imem || p2<0 || p2>imem || hold<0 || hold >=imem) return;

	if(turn)//Right
	{
		items[b].dir+=2;
		id2++;
	} else {//Left
		items[b].dir-=2;
		id2--;
	}
	if(items[b].dir>7) items[b].dir-=4;//Make sure we dont have any DIR errors
	//if(items[b].dir<0) items[b].dir+=4;
	if(id2<items[b].more1) id2+=4;//make sure we don't have any id errors either
	if(id2>items[b].more2) id2-=4;//Now you know what the min/max id is for :-)
	
	items[b].id2=id2;//set the id

	if(items[b].id2==items[b].more1) items[b].dir=0;//extra DIR error checking
	if(items[b].id2==items[b].more2) items[b].dir=6;


	if( Block( b, 0, 0, items[b].dir ) )
        {
                items[b].dir = olddir;
                for( a = 0; a < d; a++ )
                {
                        Xsend( Send[a], restart, 2 );
                        itemtalk( Send[a], tiller, "Arr, something's in the way!" );
                }
                return;
        }
        items[b].id2=id2;//set the id
 
        if(items[b].id2==items[b].more1) items[b].dir=0;//extra DIR error checking
        if(items[b].id2==items[b].more2) items[b].dir=6;    
	
	    
		
		    
    serial=items[b].serial; // lb !!!

	vector<SERIAL> vecEntries = imultisp.getData(serial);
    for (a = 0; a < vecEntries.size(); a++)
	{
		P_ITEM pi = FindItemBySerial(vecEntries[a]);
//		c=imultisp[serial%HASHMAX].pointer[a];
		if (pi != NULL)
			TurnStuff(b,DEREF_P_ITEM(pi),turn,1);
	}

	vecEntries.clear();
	vecEntries = cmultisp.getData(serial);
	for (a = 0; a < vecEntries.size(); a++)
	{
		P_CHAR pc = FindCharBySerial(vecEntries[a]);
//		c=cmultisp[serial%HASHMAX].pointer[a];
		if (pc != NULL)
			TurnStuff(b, DEREF_P_CHAR(pc), turn, 0);
	}
	
	//Set the DIR for use in the Offsets/IDs array
	dir=(items[b].dir&0x0F)/2;

	//set it's Z to 0,0 inside the boat

	items[p1].MoveTo(items[b].pos.x,items[b].pos.y,items[p1].pos.z);
	items[p1].id2=cShipItems[dir][PORT_P_C];//change the ID
	
	items[p2].MoveTo(items[b].pos.x,items[b].pos.y,items[p2].pos.z);
	items[p2].id2=cShipItems[dir][STAR_P_C];
	
	items[tiller].MoveTo(items[b].pos.x,items[b].pos.y,items[tiller].pos.z);
	items[tiller].id2=cShipItems[dir][TILLERID];
	
	items[hold].MoveTo(items[b].pos.x,items[b].pos.y,items[hold].pos.z);
	items[hold].id2=cShipItems[dir][HOLDID];

	switch(items[b].more1)//Now set what size boat it is and move the specail items
	{
	case 0x00:
	case 0x04:
		items[p1].MoveTo(items[p1].pos.x + iSmallShipOffsets[dir][PORT_PLANK][X], items[p1].pos.y + iSmallShipOffsets[dir][PORT_PLANK][Y], items[p1].pos.z);
		items[p2].MoveTo(items[p2].pos.x + iSmallShipOffsets[dir][STARB_PLANK][X], items[p2].pos.y + iSmallShipOffsets[dir][STARB_PLANK][Y], items[p2].pos.z);
		items[tiller].MoveTo(items[tiller].pos.x + iSmallShipOffsets[dir][TILLER][X], items[tiller].pos.y + iSmallShipOffsets[dir][TILLER][Y], items[tiller].pos.z);
		items[hold].MoveTo(items[hold].pos.x + iSmallShipOffsets[dir][HOLD][X], items[hold].pos.y + iSmallShipOffsets[dir][HOLD][Y], items[hold].pos.z);
		break;
	case 0x08:
	case 0x0C:
		items[p1].MoveTo(items[p1].pos.x + iMediumShipOffsets[dir][PORT_PLANK][X], items[p1].pos.y + iMediumShipOffsets[dir][PORT_PLANK][Y], items[p1].pos.z );
		items[p2].MoveTo(items[p2].pos.x + iMediumShipOffsets[dir][STARB_PLANK][X], items[p2].pos.y + iMediumShipOffsets[dir][STARB_PLANK][Y], items[p2].pos.z);
		items[tiller].MoveTo(items[tiller].pos.x + iMediumShipOffsets[dir][TILLER][X], items[tiller].pos.y + iMediumShipOffsets[dir][TILLER][Y], items[tiller].pos.z);
		items[hold].MoveTo(items[hold].pos.x + iMediumShipOffsets[dir][HOLD][X], items[hold].pos.y + iMediumShipOffsets[dir][HOLD][Y], items[hold].pos.z);

		break;
	case 0x10:
	case 0x14:
		items[p1].MoveTo(items[p1].pos.x + iLargeShipOffsets[dir][PORT_PLANK][X], items[p1].pos.y + iLargeShipOffsets[dir][PORT_PLANK][Y], items[p1].pos.z);
        items[p2].MoveTo(items[p2].pos.x + iLargeShipOffsets[dir][STARB_PLANK][X], items[p2].pos.y + iLargeShipOffsets[dir][STARB_PLANK][Y], items[p2].pos.z);
		items[tiller].MoveTo(items[tiller].pos.x + iLargeShipOffsets[dir][TILLER][X], items[tiller].pos.y + iLargeShipOffsets[dir][TILLER][Y], items[tiller].pos.z);
		items[hold].MoveTo(items[hold].pos.x + iLargeShipOffsets[dir][HOLD][X], items[hold].pos.y + iLargeShipOffsets[dir][HOLD][Y], items[hold].pos.z);

		break;

	default: { sprintf((char*)temp,"Turnboatstuff() more1 error! more1 = %c not found!\n",items[b].more1); 
		       LogWarning((char*)temp);
			 }  
	}

	sendinrange(p1);
	sendinrange(p2);
	sendinrange(hold);
	sendinrange(tiller);

	for (a=0;a<d;a++) 
	{ 
		Xsend(Send[a],restart,2);
	}
}

char cBoat::Speech(UOXSOCKET s, char *msg)//See if they said a command. msg must already be capitalized
{
	int boat=GetBoat(s);
	if(boat==-1) return 0;//if they aren't on a boat, then we don't care what they said
	int dir=items[boat].dir&0x0F;
	int serial, tiller;
	char msg2[512];
//	char msg[512];

	if (s<0 || s>=MAXCLIENT) return 0;

	//get the tiller man's item #
	serial=calcserial(items[boat].moreb1,items[boat].moreb2,items[boat].moreb3,items[boat].moreb4);
	if(serial == INVALID_SERIAL) return 0;
	tiller = calcItemFromSer(serial);
	if(tiller<0) return 0;

	//if(dist(currchar[s],tiller,1)>4) return;

//	strcpy(msg, talk);//Capitalize the msg
//	strcpy(msg, strupr(msg));
	/*forward, backward, right, left, anchor down, raise anchor, one left, one right, one */
	if(strstr(msg,"FORWARD") || strstr(msg,"UNFURL SAIL"))
	{
		items[boat].type2=1;//Moving
		Move(s,dir,boat);
		itemtalk(s, tiller, "Aye, sir.");
		return 1;
	} else if(strstr(msg,"BACKWARD"))
	{
		items[boat].type2=2;//Moving backward
		if(dir>=4) dir-=4; 
		else dir+=4;
		Move(s,dir,boat);		
		itemtalk(s, tiller, "Aye, sir.");
		return 1;
	}  else if(strstr(msg,"ONE") || strstr(msg,"DRIFT"))
	{
		if(strstr(msg,"LEFT"))
		{
			dir-=2;
			if(dir<0) dir+=8;			
			Move(s,dir,boat);
			itemtalk(s, tiller, "Aye, sir.");
			return 1;

		} else if(strstr(msg,"RIGHT"))
		{
			dir+=2;
			if(dir>=8) dir-=8; 			
			Move(s,dir,boat);
		
			itemtalk(s, tiller, "Aye, sir.");
			return 1;
		}
	} 
	else if(strstr(msg,"STOP") || strstr(msg,"FURL SAIL")) { items[boat].type2=0;itemtalk(s, tiller, "Aye, sir."); }//Moving is type2 1 and 2, so stop is 0 :-)
	else if((strstr(msg,"TURN") && (strstr(msg,"AROUND") || strstr(msg,"LEFT") || strstr(msg,"RIGHT")))
		|| strstr(msg,"PORT") || strstr(msg,"STARBOARD") || strstr(msg,"COME ABOUT"))
	{
		if(strstr(msg,"RIGHT") || strstr(msg,"STARBOARD")) 
		{
			dir-=2; if(dir<0) dir+=8;
			int tx=0,ty=0;

	        switch(dir&0x0F) // little reminder for myself: move this swtich to a function to have less code ... LB
			{
	           case '\x00' : 
		       ty--;
		       break;
	           case '\x01' : 
		       tx++; 
		       ty--;
		       break;
	           case '\x02' :
		       tx++;
		       break;
	           case '\x03' :
		       tx++;
		       ty++;
		       break;
	           case '\x04' : 
		       ty++;
		       break;
	           case '\x05' :
		       tx--;
		       ty++;
		       break;
	           case '\x06' : 
		       tx--;
		       break;
	           case '\x07' : 
		       tx--; 
		       ty--;
		       break;
			}


			if (!Block(boat,tx,ty,dir))
			{
			  Turn(boat,1);
			  itemtalk(s, tiller, "Aye, sir.");
			  return 1;
			} else { 
				items[boat].type2=0;
			    itemtalk(s,tiller,"Arr,somethings in the way"); 
				return 1;
			}
		}
		else if(strstr(msg, "LEFT") || strstr(msg,"PORT")) 
		{
			dir+=2; if(dir>7) dir-=8;
			int tx=0,ty=0;

	        switch(dir&0x0F)
			{
	           case '\x00' : 
		       ty--;
		       break;
	           case '\x01' : 
		       tx++; 
		       ty--;
		       break;
	           case '\x02' :
		       tx++;
		       break;
	           case '\x03' :
		       tx++;
		       ty++;
		       break;
	           case '\x04' : 
		       ty++;
		       break;
	           case '\x05' :
		       tx--;
		       ty++;
		       break;
	           case '\x06' : 
		       tx--;
		       break;
	           case '\x07' : 
		       tx--; 
		       ty--;
		       break;
			}


			if (!Block(boat,tx,ty,dir))
			{
			  Turn(boat,0);			
			  itemtalk(s, tiller, "Aye, sir.");
			  return 1;
			} else 
			{ 
				items[boat].type2=0;
				itemtalk(s,tiller,"Arr,somethings in the way"); 
				return 1;
			}
		}
		else if(strstr(msg,"COME ABOUT") || strstr(msg,"AROUND"))
		{
			Turn(boat,1);
			Turn(boat,1);
			itemtalk(s, tiller, "Aye, sir.");
			return 1;
		}
	}
	else if(strstr(msg,"SET NAME"))
	{
		strcpy(items[tiller].name,"a ship named ");
		for(unsigned int a=8;a<strlen(msg2);a++)
		{
			sprintf(items[tiller].name,"%s%c",items[tiller].name,msg2[a]);
		}
		return 1;
	}

  return 0;
}
