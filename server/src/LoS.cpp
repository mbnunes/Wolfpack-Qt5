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

#undef  DBGFILE
#define DBGFILE "LoS.cpp"


int line_of_sight(int s, Coord_cl &source, Coord_cl &target, int checkfor)
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
	
if (line_of_sight(s, source.x, source.y, source.z, target.x, target.y, target.z, WALLS_CHIMNEYS | DOORS | ROOFING_SLANTED))
	  
		
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
	double b1_incrament, b2_incrament, aplus=0, bplus=0, a2plus=0, b2plus=0;	// the length of b1 at a1,c1
	int checkcount, dyncount;
	char blocked=0;					// could potentially replace returning a char with a bool... you either see it or you don't (Abaddon)
	char not_blocked=1;
	double a2, b2, c2;
	UOXFile *mfile;
	st_multi multi;
	map_st map1;
	SI32 length, i,j;
	/////what is checked for
	int checkitemcount=0;
	int checkthis[ITEM_TYPE_CHOICES];
	int checkthistotal;
	int itemtype;
	/////item cahcing until item lookup is implimented
	int loscachecount=0;
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
	int StartGrid=mapRegions->StartGrid(source.x,source.y);
	unsigned long loopexit=0;
	
	// - This needs to use a pointer/array to cut down on walking lag...
	cRegion::RegionIterator4Items ri(source);
	for (ri.Begin(); ri.GetData() != ri.End(); ri++)
	{
		P_ITEM mapitem = ri.GetData();
		if (mapitem != NULL)
		{
			if (
				(mapitem->pos.x<= source.x +20)&&
				(mapitem->pos.x>= source.x -20)&&
				(mapitem->pos.y<= source.y +20)&&
				(mapitem->pos.y>= source.y -20)
				)
			{
				loscache[loscachecount] = DEREF_P_ITEM(mapitem);
				loscachecount++;
			}
		}
	}

	for (i=0;i<ITEM_TYPE_CHOICES; i++) checkthis[i] = 0 ; //Null is for pointer types
	// Zippy moved ----^ to main for initalization.
	
	
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
	i = 0;
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
			map1 = Map->SeekMap0( xcheck, ycheck );
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
					//sysmessage(s, "There seems to be something in the way!");
					return blocked;
				}
			}
			
			// Statics
			MapStaticIterator msi(xcheck, ycheck);
			tile_st tile;
			staticrecord *stat;
			loopexit=0;
			while ( (stat = msi.Next()) && (++loopexit < MAXLOOPS) )
			{
				msi.GetTile(&tile);
				if (	(zcheck>=stat->zoff)&&
					(zcheck<=(stat->zoff+tile.height)))
				{
					itemids[checkitemcount]=stat->itemid;
					checkitemcount++;
				}// if
			}
			
			// Items
			for (i=0;i<loscachecount;i++)
			{
				dyncount=loscache[i];
				if (items[dyncount].id1<0x40)
				{ // Dynamic items
					Map->SeekTile(items[dyncount].id(), &tile);
					if ((items[dyncount].pos.x==xcheck)&&
						(items[dyncount].pos.y==ycheck)&&
						(zcheck>=items[dyncount].pos.z)&&
						(zcheck<=(items[dyncount].pos.z+tile.height))&&
						(items[dyncount].visible==0))
					{
						itemids[checkitemcount]=items[dyncount].id();
						checkitemcount++;
					}
				}
				else
				{// Multi's
					if ((abs(source.x-target.x)<=BUILDRANGE)&&(abs(source.y-target.y)<=BUILDRANGE))
					{
						Map->SeekMulti(items[dyncount].id()-0x4000, &mfile, &length);
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
								(items[dyncount].pos.x+multi.x == xcheck)&&
								(items[dyncount].pos.y+multi.y == ycheck))
							{
								//pos=mfile->tell();
								Map->SeekTile(multi.tile, &tile);
								//mfile->seek(pos, SEEK_SET);
								if ((zcheck>=items[dyncount].pos.z+multi.z)&&
									(zcheck<=(items[dyncount].pos.z+multi.z + tile.height)))
								{
									itemids[checkitemcount]=multi.tile;
									checkitemcount++;
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
 
	for (i=0;i<checkitemcount;i++)
	{
		for (j=0;j<checkthistotal;j++)
		{
			switch(checkthis[j])
			{
			case 1 : // Trees, Shrubs, bushes
				if ((itemids[i]==3240)||(itemids[i]==3242)||((itemids[i]>=3215)&&(itemids[i]<=3218))||
					((itemids[i]>=3272)&&(itemids[i]<=3280))||(itemids[i]==3283)||(itemids[i]==3286)||
					(itemids[i]==3288)||(itemids[i]==3290)||(itemids[i]==3293)||(itemids[i]==3296)||
					(itemids[i]==3299)||(itemids[i]==3302)||(itemids[i]==3305)||(itemids[i]==3306)||
					(itemids[i]==3320)||(itemids[i]==3323)||(itemids[i]==3326)||(itemids[i]==3329)||
					(itemids[i]==3381)||(itemids[i]==3383)||(itemids[i]==3384)||(itemids[i]==3394)||
					(itemids[i]==3395)||((itemids[i]>=3416)&&(itemids[i]<=3418))||
					(itemids[i]==3440)||(itemids[i]==3461)||(itemids[i]==3476)||(itemids[i]==3480)||
					(itemids[i]==3484)||(itemids[i]==3488)||(itemids[i]==3492)||(itemids[i]==3496)||
					(itemids[i]==3512)||(itemids[i]==3513)||((itemids[i]>=4792)&&(itemids[i]<=4795)))
				{
					//			sprintf(temp, "You can't see the forest for the trees!");
					//			sysmessage(s, temp);
					return blocked;
				}
				break;
			case 2 : // Walls, Chimneys, ovens, not fences
				if (((itemids[i]>=6)&&(itemids[i]<=748))||((itemids[i]>=761)&&(itemids[i]<=881))||
					((itemids[i]>=895)&&(itemids[i]<=1006))||((itemids[i]>=1057)&&(itemids[i]<=1061))||
					(itemids[i]==1072)||(itemids[i]==1073)||((itemids[i]>=1080)&&(itemids[i]<=1166))||
					((itemids[i]>=2347)&&(itemids[i]<=2412))||((itemids[i]>=16114)&&(itemids[i]<=16134))||
					((itemids[i]>=8538)&&(itemids[i]<=8553))||((itemids[i]>=9535)&&(itemids[i]<=9555))||
					(itemids[i]==12583))
				{
					//			sprintf(temp, "There seems to be some sort of wall in the way!");
					//			sysmessage(s, temp);
					return blocked;
				}
				break;
			case 4 : // Doors, not gates
				if (((itemids[i]>=1653)&&(itemids[i]<=1782))||((itemids[i]>=8173)&&(itemids[i]<=8188)))
				{
					//			sprintf(temp, "Only ghosts do things through doors!");
					//			sysmessage(s, temp);
					return blocked;
				}
				break;
			case 8 : // Roofing Slanted
				if (((itemids[i]>=1414)&&(itemids[i]<=1578))||((itemids[i]>=1587)&&(itemids[i]<=1590))||
					((itemids[i]>=1608)&&(itemids[i]<=1617))||((itemids[i]>=1630)&&(itemids[i]<=1652))||
					((itemids[i]>=1789)&&(itemids[i]<=1792)))
				{
					//			sprintf(temp, "The roof is too steep!");
					//			sysmessage(s, temp);
					return blocked;
				}
				break;
			case 16 : // Floors & Flat Roofing (Attacking through floors Roofs)
				if (((itemids[i]>=1169)&&(itemids[i]<=1413))||((itemids[i]>=1508)&&(itemids[i]<=1514))||
					((itemids[i]>=1579)&&(itemids[i]<=1586))||((itemids[i]>=1591)&&(itemids[i]<=1598)))
				{
					if (source.z==target.z) // in case of char and target on same roof
					{
						return not_blocked;
					}
					else
					{
						//	sprintf(temp, "You would love to do that, but the is a floor in the way!");
						//	sysmessage(s, temp);
						return blocked;
					}
				}
				break;
			case 32 :	// Lava, water
				if (((itemids[i]>=4846)&&(itemids[i]<=4941))||((itemids[i]>=6038)&&(itemids[i]<=6066))||
					((itemids[i]>=12934)&&(itemids[i]<=12977))||((itemids[i]>=13371)&&(itemids[i]<=13420))||
					((itemids[i]>=13422)&&(itemids[i]<=13638))||((itemids[i]>=13639)&&(itemids[i]<=13665)))
				{
					//	sprintf(temp, "Yah, you wish!");
					//	sysmessage(s, temp);
					return blocked;
				}
				break;
			default:
				//clConsole.send("ERROR: Fallout of switch statement without default. uox3.cpp, line_of_sight()"); //Morrolan
				return not_blocked;
			} // switch
		} //for
	} //for
	return not_blocked;
} //function

