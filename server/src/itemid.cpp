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

/////////////////////
// Name:	itemid.cpp
// Purpose: functions that wrap the checking for a list of item IDs
// History:	cut from various cpps and 'restyled' by Duke, 24.02.01
// Remarks:	this file should contain ONLY THAT type of functions
//			Feel free to collect more
//
#include "itemid.h"
#include "platform.h"

#include "debug.h"
#undef  DBGFILE
#define DBGFILE "itemid.cpp"

bool IsCorpse(short id)
{
	return (id==0x2006);
}

bool IsTree(short id)	// this is used in AxeTarget()
{
	if (id==0x0CD0 || id==0x0CD3 || id==0x0CD6 ||
		id==0x0CD8 || id==0x0CDA || id==0x0CDD ||
		id==0x0CE0 || id==0x0CE3 || id==0x0CE6 ||
		(id>=0x0CCA && id<=0x0CCD) ||
		(id>=0x12B8 && id<=0x12BB) ||
		id==0x0D42 || id==0x0D43 || id==0x0D58 ||
		id==0x0D59 || id==0x0D70 || id==0x0D85 ||
		id==0x0D94 || id==0x0D95 || id==0x0D98 ||
		id==0x0Da4 || id==0x0Da8 || id==0x0D58)
		return true;
	return false;
}

#if 0
bool IsTree2(short id)	// this is used in SwordTarget() to give kindling. Donno why it's different
{
	if (id==0x0CD0 || id==0x0CD3 || id==0x0CD6 ||
		id==0x0CD8 || id==0x0CDA || id==0x0CDD ||
		id==0x0CE0 || id==0x0CE3 || id==0x0CE6 ||
		(id>=0x0CCA && id<=0x0CCD) ||
		(id>=0x12B8 && id<=0x12BB) )
		return true;
	return false;
}
#endif

bool IsSpellScroll(short id)
{
	return (id>0x1F2C && id<0x1F6D);
}

bool IsSpellScroll72(short id)
{
	return (id >= 0x1F2D && id <= 0x1F72);
}

bool IsInstrument(short id)
{
	return (id==0x0E9C || id==0x0E9D || id==0x0E9E || id==0x0EB1 || id==0x0EB2 || id==0x0EB3 || id==0x0EB4);
}

bool IsSword(short id)
{
	return ((id>=0x0EC1 && id<=0x0EC5) ||	// cleaver & skinning knife
			(id>=0x0F5E && id<=0x0F61) ||	// broad & longsword
			(id>=0x13B5 && id<=0x13BA) ||	// scimitar, thin longsword & viking sword
			(id==0x13F6 || id==0x13F7) ||	// butcher knife
			(id==0x13FE || id==0x13FF) ||	// katana
			(id==0x1440 || id==0x1441) );	// cutlass
}

bool IsAxe(short id) // -Fraz- this includes all the special sword concussion weapons because they are all 2handed
{
	return ((id>=0x0F43 && id<=0x0F4E) ||	// axes
			(id==0x13FA || id==0x13FB) ||	// large battle axe
			(id==0x143E || id==0x143F) ||	// halberd
			(id==0x1442 || id==0x1443) );	// two handed axe
}

bool IsSwordType(short id)
{
	return ( IsSword(id) || IsAxe(id) );
}

bool IsMace1H(short id)
{
	return ((id>=0x0DF0 && id<=0x0DF5) ||
			(id==0x13B3 || id==0x13B4) ||
			(id==0x0F5C || id==0x0F5D) ||
			(id==0x0FB4 || id==0x0FB5) || 
			(id==0x13AF || id==0x13B0) ||
			(id==0x13E3 || id==0x13E4) ||
			(id==0x13F4 || id==0x13F5) ||
			(id==0x13F8 || id==0x13F9) ||
			(id==0x143A || id==0x143B) ||	// maul -Fraz-
			(id==0x1406 || id==0x1407) ||
			(id==0x0E89 || id==0x0E8A) ||
			(id==0x0E81 || id==0x0E82) );
}

bool IsMace2H(short id)
{
	return ( id>=0x1438 && id<=0x143C );	// war hammer & hammerpick -Fraz- so says OSI
}

bool IsMaceType(short id)
{
	return ( IsMace1H(id) || IsMace2H(id) );
}

bool IsFencing1H(short id)
{
	return ((id==0x1404 || id==0x1405) ||	// war fork -Fraz- adjusted
			(id==0x0F51 || id==0x0F52) ||   // dagger
			(id==0x1400 || id==0x1401) );	// kryss 
}

bool IsFencing2H(short id) // -Fraz- These weapons will all cause combat paralyzation
{
	return ((id==0x1402 || id==0x1403) ||	// short spear -Fraz- added to 2handed , so sayeth OSI
			(id==0x0E87 || id==0x0E88) ||	// pitchfork
			(id==0x0F62 || id==0x0F63) );	// spear
}

bool IsFencingType(short id)
{
	return ( IsFencing1H(id) || IsFencing2H(id) );
}

bool IsBow			(short id)	{return (id==0x13B2);}
bool IsCrossbow		(short id)	{return (id==0x13FD);}
bool IsHeavyCrossbow(short id)	{return (id==0x0F4F);}

bool IsBowType(short id)
{
	return (IsBow(id) || IsCrossbow(id) || IsHeavyCrossbow(id));
}

bool IsStave(short id) // -Fraz- added
{
	return ((id==0x13F8 || id==0x13F9) || // Gnarled staff
			(id==0x0E89 || id==0x0E8A) || // Quarter Staff
			(id==0x0DF0 || id==0x0DF1) || // Black Staff
			(id==0x0E81 || id==0x0E82)); //  Crook
}

bool IsSpecialMace(short id) // -Fraz- The OSI macing weapons that do stamina and armor damage 2handed only
{
	return ( IsStave(id) || IsMace2H(id) );
}

bool IsChaosOrOrderShield(short id)
{
	return (id>=0x1BC3 && id<=0x1BC5);
}

bool IsShield(short id)
{
	return ((id>=0x1B72 && id<=0x1B7B) || IsChaosOrOrderShield(id));
}

bool IsLog(short id)
{
	return ( id>=0x1BDD && id<=0x1BE2 );
}

bool IsShaft(short id)
{
	return ( id>=0x1BD4 && id<=0x1BD6 );
}

bool IsBoard(short id)
{
	return ( id>=0x1BD7 && id<=0x1BDC );
}

bool IsFeather(short id)
{
	return ( id>=0x1BD1 && id<=0x1BD3 );
}

bool IsCutLeather(short id)
{
	return ( id==0x1067 || id==0x1068 || id==0x1081 || id==0x1082 );
}

bool IsHide(short id)
{
	return ( id==0x1078 || id==0x1079 );
}

bool IsForge(short id)
{
	return ( id==0x0FB1 || ( id>=0x197A && id<=0x19A9 ) );
}

bool IsAnvil(short id)
{
	return ( id==0x0FAF || id==0x0FB0 );
}

bool IsCookingPlace(short id)
{
	return ((id>=0x0DE3 && id<=0x0DE9)||
			(id==0x0FAC || id==0x0FB1)||
			(id>=0x197A && id<=0x19B6)||
			(id>=0x0461 && id<=0x0480)||
			(id>=0x0E31 && id<=0x0E33)||
			(id==0x19BB || id==0x1F2B)||
			(id>=0x092B && id<=0x0934)||
			(id>=0x0937 && id<=0x0942)||
			(id>=0x0945 && id<=0x0950)||
			(id>=0x0953 && id<=0x095E)||
			(id>=0x0961 && id<=0x096C) );
}

bool IsCutCloth(short id)
{
	return ( id>=0x1766 && id<=0x1768 );
}

bool IsCloth(short id)
{
	return ((id>=0x175D && id<=0x1765)|| IsCutCloth(id) );
}

bool IsBoltOfCloth(short id)
{
	return ( id>=0x0F95 && id<=0x0F9C );
}

bool IsChest(short id)
{
	return (( id>=0x09A8 && id<=0x09AB) ||
		    ( id>=0x0E40 && id<=0x0E43) ||
		    ( id==0x0E7C) || (id==0x0E7D) ||
		    ( id==0x0E80));
}

bool IsFishWater(short id) // -Fraz- needed water tiles for fishing
{
	return ((id>=0x00A8 && id<=0x00AB) ||
			(id>=0x0136 && id<=0x0137) ||
			(id>=0x1797 && id<=0x179C) ||
			(id>=0x346E && id<=0x346F) ||
			(id>=0x3470 && id<=0x3485) ||
			(id>=0x3494 && id<=0x34AB) ||
			(id>=0x34B8 && id<=0x34BB) ||
			(id>=0x34BD && id<=0x34BF) ||
			(id>=0x34C0 && id<=0x34C2) ||
			(id>=0x34C3 && id<=0x34C5) ||
			(id>=0x34C7 && id<=0x34CA) ||
			(id>=0x34D1 && id<=0x34D5) ||
			(id>=0x3FF0 && id<=0x3FF3));
}

bool IsFish(short id)
{
	return ( id>=0x09CC && id<=0x09CF );
}

bool IsPlateArmour(short id)
{
	return ( id>=0x1410 && id<=0x141A );
}

bool IsBoneArmour(short id)
{
	return ( id>=0x144E && id<=0x1457 );
}

bool IsChainArmour(short id)
{
	return ( id>=0x13BB && id<=0x13C4 );
}

bool IsRingArmour(short id)
{
	return ( id>=0x13E5 && id<=0x13F2 );
}

bool IsStuddedArmour(short id)
{
	return ( id>=0x13D4 && id<=0x13E2 );
}

bool IsLeatherArmour(short id)
{
	return (( id>=0x13C5 && id<=0x13D3) ||
			( id>=0x1DB9 && id<=0x1DBA));
}

bool IsMetalArmour(short id)
{
	return ( IsPlateArmour(id) || IsChainArmour(id) || IsRingArmour(id) );
}

