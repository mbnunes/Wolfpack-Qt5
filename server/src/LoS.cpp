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

/////////////////////
// Name:	LoS.cpp
// History:	cut from wolfpack.cpp by Duke, 26.10.00
//

#include "wolfpack.h"
#include "SndPkg.h"
#include "debug.h"
#include "mapobjects.h"
#include "mapstuff.h"
#include "tilecache.h"

#undef  DBGFILE
#define DBGFILE "LoS.cpp"


#define CORREA_LOS_OPTIMIZATION

bool isTree_Bushe(int id)
{
	if ((id==0xCA8)||(id==0xCAA)||((id>=0xC8F)&&(id<=0xC92))||
		((id>=0xCC8)&&(id<=0xCD0))||(id==0xCD3)||(id==0xCD6)||
		(id==0xCD8)||(id==0xCDA)||(id==0xCDD)||(id==0xCE0)||
		(id==0xCE3)||(id==0xCE6)||(id==0xCE9)||(id==0xCEA)||
		(id==0xCF8)||(id==0xCFB)||(id==0xCFE)||(id==0xD01)||
		(id==0xD35)||(id==0xD37)||(id==0xD38)||(id==0xD42)||
		(id==0xD43)||((id>=0xD58)&&(id<=0xD5A))||
		(id==0xD70)||(id==0xD85)||(id==0xD94)||(id==0xD98)||
		(id==0xD9C)||(id==0xDA0)||(id==0xDA4)||(id==0xDA8)||
		(id==0xDB8)||(id==0xDB9)||((id>=0x12B8)&&(id<=0x12BB)))
	{
		return true;
	}
	else 
		return false;

}

bool isWall_Chimney(int id)
{
	if (((id>=0x6)&&(id<=0x2EC))||((id>=0x2F9)&&(id<=0x371))||
		((id>=0x37F)&&(id<=0x3EE))||((id>=0x421)&&(id<=0x425))||
		(id==0x430)||(id==0x431)||((id>=0x438)&&(id<=0x48E))||
		((id>=0x92B)&&(id<=0x96C))||((id>=0x3EF2)&&(id<=0x3F06))||
		((id>=0x215A)&&(id<=0x2169))||((id>=0x253F)&&(id<=0x2553))||
		(id == 0x3127))
	{
		return true;
	}
	else 
		return false;

}

bool isDoor(int id)
{
	if (((id>=0x675)&&(id<=0x6F6))||((id>=0x1FED)&&(id<=0x1FFC)))
		return true;
	else
		return false;
}

bool isRoofing_Slanted(int id)
{
	if (((id>=1414)&&(id<=1578))||((id>=1587)&&(id<=1590))||
		((id>=1608)&&(id<=1617))||((id>=1630)&&(id<=1652))||
		((id>=1789)&&(id<=1792)))
		return true;
	else
		return false;
}

bool isFloor_Flat_Roofing(int id)
{
	 if (((id>=1169)&&(id<=1413))||((id>=1508)&&(id<=1514))||
		 ((id>=1579)&&(id<=1586))||((id>=1591)&&(id<=1598)))
		 return true;
	 else
		 return false;

}

bool isLavaWater(int id)
{
	if (((id>=0x12EE)&&(id<=0x134D))||((id>=0x1796)&&(id<=0x17B2))||
		((id>=0x3286)&&(id<=0x32B1))||((id>=0x343B)&&(id<=0x346C))||
		((id>=0x346E)&&(id<=0x3546))||((id>=0x3547)&&(id<=0x3561)))
		return true;
	else
		return false;
}

int lineOfSight( const Coord_cl &source, const Coord_cl &target, int checkfor )
{
/*
Char (source.x, source.y, source.z) is the char(pc/npc), Target (target.x, target.y, target.z) is the target.
s is for pc's, in case a message needs to be sent.
the checkfor is what is checked for along the line of sight.
Look at uox3.h to see options. Works like npc magic.

#define TREES_BUSHES 1 // Trees and other large vegetaion in the way
#define WALLS_CHIMNEYS 2	// Walls, chimineys, ovens, etc... in the way
#define DOORS 4 // Doors in the way
#define ROOFING_SLANTED 8	// So can't tele onto slanted roofs, basically
#define FLOORS_FLAT_ROOFING 16	// For attacking between floors
#define LAVA_WATER 32	// Don't know what all to use this for yet
  
Just or (|) the values for the diff things together to get what to search for.
So put in place of the paramater checkfor for example
	
if (lineOfSight( source.x, source.y, source.z, target.x, target.y, target.z, WALLS_CHIMNEYS | DOORS | ROOFING_SLANTED))
	  
		
This whole thing is based on the Pythagorean Theorem.  What I have done is
taken the coordinates from both chars and created a right triange with the
hypotenuse as the line of sight.  Then by sectioning off side "a" into a number
of equal lengths and finding the other sides lengths according to that one,  I
have been able to find the coordinates of the tiles that lie along the line of
sight(line "c").  Then these tiles are searched from an item that would block
the line of sight.
*/
	
	int a, b;	// Lengths of sides a & b
	int zcheck=source.z, prexcheck=-128, preycheck=-128, prezcheck=-128;
	short int xcheck = source.x, ycheck = source.y;
	double c;	// Length of side c, Line of sight
	float asquared, bsquared, csquared;	// Squares of a, b, c
	double a_divide;	// this is how many times the entire line is checked
	double a1_incrament= .1;	// the line is checked by this length incrament, can be chenged, the larger the num less checks, more misses
	double a2_incrament= a1_incrament;
	double c1_incrament, c2_incrament;	// c/a_divide, this give the length incrament along c
	double b1_incrament, b2_incrament, aplus=0, bplus=0, /*a2plus=0,*/ b2plus=0;	// the length of b1 at a1,c1
	int checkcount;
	char blocked=0;					// could potentially replace returning a char with a bool... you either see it or you don't (Abaddon)
	char not_blocked=1;
	double a2, b2, c2;
	UOXFile *mfile;
	st_multi multi;
	map_st map1;
	SI32 length, j;
	/////what is checked for
	vector<short> itemids;
	int checkthis[ITEM_TYPE_CHOICES] = {0,};
	int checkthistotal;
	int itemtype;
	/////item cahcing until item lookup is implimented
	vector<P_ITEM> loscache;
	if( ( source.x <= 200 && source.y<= 200 ) || ( target.x <= 200 && target.y <= 200 ) ) return not_blocked;
	if( source.distance(target) > 18 ) return blocked;
	///////////////////////////////////////////////////////////
	///////////////// These next lines initialize arrays
	/*
	This function has to search the items array a number of times which
	caused a bit of lag. I made this item cache to be used instead.
	The items array is only search once for items in a 40 tile area.
	if an item is found, it is put into the cache, which is then used
	by the rest of the function. This way it doesn't have to check the
	entire array each time.
	*/
	// - Tauriel's region stuff 3/6/99
	unsigned long loopexit=0;
	
	// - This needs to use a pointer/array to cut down on walking lag...
	RegionIterator4Items ri(source);
	for (ri.Begin(); !ri.atEnd(); ri++)
	{
		P_ITEM mapitem = ri.GetData();
		if (mapitem != NULL)
		{
			if ( mapitem->pos.distance( source ) < 20 )
			{
				loscache.push_back(mapitem);
			}
		}
	}
	
	////////////End Initilzations
	//////////////////////////////////////////////////////////////
	/*
	char(player/npc)
	1 2  *_____a_____
	1      \          |
	2        \        |
	c      b   x -->
	\    |   y  ^
	\  |      |
	\|
	*
	target
	*/
	
	if ((target.x==65535)&&(target.y==65535)) return not_blocked;	// target cancled
	
	
	a=abs(source.x-target.x)+1;	// length of side a
	b=abs(source.y-target.y)+1;	// length of side b
	asquared = static_cast<float>((a * a));
	bsquared = static_cast<float>((b * b));
	csquared=(asquared + bsquared);
	c=sqrt(csquared);// length of c(hypotenuse==line of sight)
	
	a_divide = (a / a1_incrament);
	c1_incrament = (c / a_divide);
	
	if(a==1)
	{
		c1_incrament=1;
		a1_incrament=0;
		a_divide = b;
	}
	
	b1_incrament=sqrt(fabs((c1_incrament*c1_incrament)-(a1_incrament*a1_incrament)));
	if (b==1)
	{
		a1_incrament=1;
		a_divide=a;
		b1_incrament=0;
	}
	//////////////////////////////////////////////////////////
	// X position
	if (target.x == source.x) // Target has same x value
	{
		a1_incrament = 0; //(a1_incrament * 0); // sets x incraments to zero, no incrament
	}
	else if (target.x > source.x) // Target has greater x value
	{
		//a1_incrament = (a1_incrament * 1); // sets x incrament positive
	}
	else // Target has lesser x value
	{
		a1_incrament *= -1; //(a1_incrament * -1); // sets x incrament negative, postitive initially
	}
	
	// Y position
	if (target.y == source.y) // Target has same y value
	{
		b1_incrament = 0; //(b1_incrament * 0); // sets y incrament to zero, no incrament
	}
	else if (target.y > source.y) // Target has greater y value
	{
		//b1_incrament = (b1_incrament * 1); // sets y incraments positive
	}
	else if (target.y < source.y) // Target has lesser y value
	{
		b1_incrament *= -1; //(b1_incrament * -1);	// sets y incrament negative, it's positive initially
	}
	//////////////////////////////////////////////////////////////////////////////
	/*
	To find the tiles along the z axis another right triangle is formed. This triangle
	is formed along the line of sight between Char1 & Target, where the length of that line
	is the length of the base of the triange. The hiegth of the triangle is the targets z
	position. We now have two sides, and to find the third we use the pathagorean theorem
	again, and that gives us the true line of sight in 3 dimentions: X, Y, Z.
	
	*/
	a2=c;	// length of base
	b2=abs(target.z-source.z)+1;	// hieght of side
	asquared = static_cast<float>((a2 * a2));
	bsquared = static_cast<float>((b2 * b2));
	csquared=(asquared + bsquared);
	c2=sqrt(csquared); // length of true line of sight 3D
	
	a_divide = (a2 / a2_incrament);
	c2_incrament = (c2 / a_divide);
	b2_incrament=sqrt(fabs((c2_incrament*c2_incrament)-(a2_incrament*a2_incrament)));
	
	if (source.z>target.z) // going down
	{
		b2_incrament = (b2_incrament * -1);
	}
	else if (source.z==target.z) // level ground
	{
		b2_incrament = 0;
	}
	
	aplus = a1_incrament;
	bplus = b1_incrament;
	b2plus = b2_incrament; // going up or down
	////////////////////////////////////////////////////////
	//////////////// This determines what to check for
	unsigned int i = 0;
	itemtype = 1;
	checkthistotal = 0;
	loopexit = 0;
	while ( (checkfor) && (++loopexit < MAXLOOPS) )
	{
		if ( ( checkfor >= itemtype ) && ( checkfor < ( itemtype * 2 ) ) && ( checkfor ) )
		{
			checkthis[i] = itemtype;
			i++;
			checkfor = (checkfor - itemtype);
			checkthistotal++;
			itemtype = 1;
		}
		else
			if ( checkfor )
			{
				itemtype = (itemtype * 2);
			}
	}
	///////////////////////////////////////////////////////////////////////////
	//////////////////// This next stuff is what searches each tile for things
	for (checkcount = 1; checkcount <= a_divide; checkcount++)
	{
		if (xcheck != target.x) xcheck = int(source.x + aplus); // x coord to check
		if (ycheck != target.y) ycheck = int(source.y + bplus); // y coord to check
		if (zcheck != target.z) zcheck = int(source.z + b2plus); // z coord to check
		if ((xcheck!=prexcheck)||(ycheck!=preycheck)||(zcheck!=prezcheck))
		{
			// Texture mapping
			//source.xcheck=xcheck/8; // Block
			//source.ycheck=ycheck/8;
			//xoff=(xcheck-(source.xcheck*8)); // Offset
			//yoff=(ycheck-(y1check*8));
			map1 = Map->SeekMap( Coord_cl(xcheck, ycheck, source.map) );
			if (map1.id!=2)
			{
				if ( // Mountain walls
					((map1.id>=431)&&(map1.id<=432))||
					((map1.id>=467)&&(map1.id<=475))||
					((map1.id>=543)&&(map1.id<=560))||
					((map1.id>=1754)&&(map1.id<=1757))||
					((map1.id>=1787)&&(map1.id<=1789))||
					((map1.id>=1821)&&(map1.id<=1824))||
					((map1.id>=1851)&&(map1.id<=1854))||
					((map1.id>=1881)&&(map1.id<=1884))
					)
				{
					return blocked;
				}
			}
			
			// Statics
			MapStaticIterator msi(Coord_cl(xcheck, ycheck, source.z, source.map));
			tile_st tile;
			staticrecord *stat;
			loopexit=0;
			while ( (stat = msi.Next()) && (++loopexit < MAXLOOPS) )
			{
				msi.GetTile(&tile);
				if ( (zcheck >= stat->zoff ) && ( zcheck <= ( stat->zoff + tile.height ) ) )
				{
#ifdef CORREA_LOS_OPTIMIZATION
					if (cMapStuff::DoesTileBlock( tile ))
						return blocked;
					else
					{
						itemids.push_back(stat->itemid);
					}
#else
					itemids[checkitemcount]=stat->itemid;
					checkitemcount++;
#endif
				}// if
			}
			
			// Items
			unsigned int i;
			for (i = 0; i < loscache.size(); i++)
			{
				P_ITEM pi = loscache[i];
				if( !pi->isMulti() )
				{ // Dynamic items
					if ( (pi->pos.x==xcheck) && (pi->pos.y==ycheck) && (zcheck >= pi->pos.z) && (pi->visible==0)) // Seek file only when necessary
					{
						tile = cTileCache::instance()->getTile( pi->id() );
						if ( ( zcheck <= ( pi->pos.z + tile.height ) ) )
						{
#ifdef CORREA_LOS_OPTIMIZATION
							if (cMapStuff::DoesTileBlock( tile ))
								return blocked;
							else
							{
								itemids.push_back(pi->id());
							}
#else
							itemids[checkitemcount] = pi->id();
							checkitemcount++;
#endif
						}
					}
				}
				else
				{// Multi's
					if ((abs(source.x-target.x)<=BUILDRANGE)&&(abs(source.y-target.y)<=BUILDRANGE))
					{
						Map->SeekMulti(pi->id()-0x4000, &mfile, &length);
						length=length/sizeof(st_multi);
						if (length == -1 || length>=17000000)//Too big... bug fix hopefully (Abaddon 13 Sept 1999)
							//							if (length == -1)
						{
							clConsole.send("LoS - Bad length in multi file. Avoiding stall.\n");
							length = 0;
						}
						for (j=0;j<length;j++)
						{
							mfile->get_st_multi(&multi);
							if ((multi.visible)&&
								(pi->pos.x+multi.x == xcheck)&&
								(pi->pos.y+multi.y == ycheck))
							{
								//pos=mfile->tell();
								tile = cTileCache::instance()->getTile( multi.tile );
								//mfile->seek(pos, SEEK_SET);
								if ((zcheck>=pi->pos.z+multi.z)&&
									(zcheck<=(pi->pos.z+multi.z + tile.height)))
								{
									itemids.push_back(multi.tile);
								}
							}
						}
					}
				}// end else
			} // for
			if ((xcheck==target.x)&&(ycheck==target.y)&&(zcheck==target.z)) checkcount=int(a_divide+1);
			prexcheck=xcheck;
			preycheck=ycheck;
			prezcheck=zcheck;
		} // if statment
		if (xcheck!=target.x) aplus=aplus+a1_incrament;
		if (ycheck!=target.y) bplus=bplus+b1_incrament;
		if (zcheck!=target.z) b2plus=b2plus+b2_incrament;
	} // for loop
 
	for (i=0;i<itemids.size();i++)
	{
		for (j=0;j<checkthistotal;j++)
		{
			switch(checkthis[j])
			{
			case 1 : // Trees, Shrubs, bushes
				if (isTree_Bushe(itemids[i]))
				{
					return blocked;
				}
				break;
			case 2 : // Walls, Chimneys, ovens, not fences
				if (isWall_Chimney(itemids[i]))
				{
					return blocked;
				}
				break;
			case 4 : // Doors, not gates
				if (isDoor(itemids[i]))
				{
					return blocked;
				}
				break;
			case 8 : // Roofing Slanted
				if (isRoofing_Slanted(itemids[i]))
				{
					return blocked;
				}
				break;
			case 16 : // Floors & Flat Roofing (Attacking through floors Roofs)
				if (isFloor_Flat_Roofing(itemids[i]))
				{
					if (source.z==target.z) // in case of char and target on same roof
					{
						return not_blocked;
					}
					else
					{
						return blocked;
					}
				}
				break;
			case 32 :	// Lava, water
				if (isLavaWater(itemids[i]))
				{
					return blocked;
				}
				break;
			default:
				clConsole.send("ERROR: Fallout of switch statement without default. LoS.cpp, line_of_sight()"); //Morrolan
				return not_blocked;
			} // switch
		} //for
	} //for
	return not_blocked;
} //function

