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

/****************************************************
 *	      All weight function are coded here		*										
 *               Done by Moongraven					*
 ****************************************************/

#include "weight.h"

#undef  DBGFILE
#define DBGFILE "weight.cpp"


/**************************************************************************************************************************
void newcalcweight(int);

  Ison 2-20-99 - re-wrote by Tauriel 3/20/99
  
	calcweight will search player's paperdoll and then backpacks for items with item.weight set to a value.  Values are 
	added until total weight of character is determined.  The called character's weight is first set to zero then re-calculated
	during the function.  Since it is a global variable I just modifed .weight instead of returning a value.

  pass the current character number to the function

**************************************************************************************************************************/

void cWeight::NewCalc(P_CHAR pc)
{
	float totalweight=0.0;
	//P_CHAR pc = MAKE_CHAR_REF(p);

	//get weight for items on players
	unsigned int ci = 0;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(pc->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi == NULL || (pi != NULL && pi->id() == 0x1E5E))	// trade window ?
			continue;
		if ((pi->layer!=0x0B) && (pi->layer!=0x10) && //no weight for hair/beard
			(pi->layer!=0x1D) && (pi->layer!=0x19))   //no weight for steed/bank box
		{
			totalweight+=(pi->getWeight()/100.0f);
		}
	}

	// Items in players pack
	P_ITEM pBackpack = Packitem(pc);
	if (pBackpack != NULL) totalweight += RecursePacks(pBackpack); //LB

	pc->weight = (int)totalweight;

	return;
}

//////////////////
// name:	RecursePacks
// Purpose:	recurses through the container given by bp to calculate the total weight
// History:	Ison 2-20-99  - rewrote by Tauriel 3/20/99
//			rewritten by Duke 4.11.2k
//
float cWeight::RecursePacks(P_ITEM bp)
{
	float totalweight=0.0;

	if (bp == NULL) return 0.0f;
	
	unsigned int ci = 0;
	vector<SERIAL> vecContainer = contsp.getData(bp->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		int debug = vecContainer[ci];
		P_ITEM pi = FindItemBySerial(vecContainer[ci]);
		int itemsweight=pi->getWeight();
		if (pi->type==1) //item is another container
		{
			totalweight += (itemsweight/100.0f);		// the weight of container itself
			totalweight += RecursePacks(pi);			//find the item's weight within this container
		}
		
		if (pi->id() == 0x0EED)
			totalweight += (pi->amount*SrvParms->goldweight);
		else
			totalweight += (float)((itemsweight*pi->amount)/100.0f);
	}
	return totalweight;
}

int cWeight::CheckWeight(int s, int k) // Check when player is walking if overloaded
{
	P_CHAR pc = currchar[s];
	if (pc != NULL)
	if ((pc->weight > (pc->st*WEIGHT_PER_STR)+30))
	{
		float res=float(pc->weight - ((pc->st*WEIGHT_PER_STR)+30))*2;

		pc->stm -= (int)res;
		if (pc->stm<=0)
		{
			pc->stm=0;
			//AntiChrist - displays a message
			sysmessage(calcSocketFromChar(s),"You are overloaded! You can't hold all this weight..");
			return 0;
		}
	}
	return 1;
}

int cWeight::CheckWeight2(int s) // Morrolan - Check when player is teleporting if overloaded
{
	P_CHAR pc = currchar[s];
	if (pc != NULL)
	if ((pc->weight > (pc->st*WEIGHT_PER_STR)+30))
	{
		pc->mn -= 30;
		if (pc->mn<=0)
		{
			pc->mn=0;
		}
		return 1;
	}
	return 0;
}

//	history:	added containersearch Duke, 4.11.2k
float cWeight::LockeddownWeight(P_ITEM pItem, int *total, int *total2 )
{
	float totalweight=0.0;
	if (!pItem) 
	{
		*total=0;
		return 0.0;
	}
	
	unsigned int ci = 0;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(pItem->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		int itemsweight=pi->getWeight();
		total2=total2+pi->amount;
		*total=*total+1;
		if (pi->type==1 || pi->type==63 || pi->type==65 || pi->type==87) //item is another container
		{
			totalweight+=(itemsweight/100.0f); //(pi->weight/100);
			totalweight+=LockeddownWeight(pi, total, total2); //find the item's weight within this container
		}
		
		if ( pi->id() == 0x0EED )
			totalweight+=(pi->amount*SrvParms->goldweight);
		else
			totalweight+=(float)((itemsweight*pi->amount)/100.0f); //((pi->weight*pi->amount)/100);  // Ison 2-21-99
	}

	if (*total==0) 
	{ 
		*total=pItem->amount;
		*total=*total*-1; // Indicate that not a pack ! on osi servers in that case weigt/items count isnt shown
		                  // thus i set it negative, if you want to show it anyway, add something like if (weight<0) weight*=-1; 
		return ((((float)pItem->getWeight())/100)*pItem->amount); // if no pack return single item weight*/        		
		
	}
	else
		return totalweight;
}

