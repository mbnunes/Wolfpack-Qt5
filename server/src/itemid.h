//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2004 by holders identified in authors.txt
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
//	Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================

/////////////////////
// Name:	itemid.h
// Purpose: interface for itemid.cpp
// History:	by Duke, 24.02.01
//			added some IsWeapon stuff (Duke, 20.4.2001)
//
#ifndef _ITEMID_H
#define _ITEMID_H

#include <qglobal.h>

bool IsCorpse		(short id);
bool IsSpellScroll	(short id);	// predefined spells
bool IsSpellScroll72(short id);	// includes the scrolls with a variable name
bool IsTree			(short id);	// this is used in AxeTarget()
//bool IsTree2		(short id);	// this is used in SwordTarget() to give kindling.
bool IsInstrument	(short id);

bool IsAxe			(short id);// -Fraz- for intelligence concussion
bool IsSword		(short id);
bool IsSwordType	(short id);
bool IsMace1H		(short id);
bool IsMace2H		(short id);
bool IsDagger       (short id);
bool IsMaceType		(short id);
bool IsFencing1H	(short id);
bool IsFencing2H	(short id);// -Fraz- for paralyzing
bool IsFencingType	(short id);
bool IsBow			(short id);
bool IsCrossbow		(short id);
bool IsHeavyCrossbow(short id);
bool IsBowType		(short id);
bool IsStave		(short id);
bool IsSpecialMace	(short id);// -Fraz- for stamina and armor damage
bool IsChaosOrOrderShield	(short id);
bool IsShield		(short id);
bool IsLog		    (short id);
bool IsShaft		(short id);
bool IsBoard		(short id);
bool IsFeather		(short id);
bool IsCutLeather	(short id);
bool IsHide			(short id);
bool IsBoltOfCloth	(short id);
bool IsCutCloth		(short id);
bool IsCloth		(short id);
bool IsChest        (short id);

bool IsForge		(short id);
bool IsAnvil		(short id);
bool IsCookingPlace	(short id);

bool IsFish			(short id);
bool IsFishWater	(short id);

bool IsPlateArmour	(short id);
bool IsBoneArmour	(short id);
bool IsChainArmour	(short id);
bool IsRingArmour	(short id);
bool IsStuddedArmour(short id);
bool IsLeatherArmour(short id);
bool IsMetalArmour	(short id);

bool IsHouse		(unsigned short id);

bool IsTree_Bush	( int id );
bool IsWall_Chimney	( int id );
bool IsDoor			( int id );
bool IsRoofing_Slanted( int id );
bool IsFloor_Flat_Roofing( int id );
bool IsLavaWater	( int id );

inline bool isHair( Q_UINT16 model )
{
	return	( ( ( model >= 0x203B ) && ( model <= 0x203D ) ) || ( ( model >= 0x2044 ) && ( model <= 0x204A ) ) ) ? true : false;
}

inline bool isBeard( Q_UINT16 model )
{
	return	( ( ( model >= 0x203E ) && ( model <= 0x2041 ) ) || ( ( model >= 0x204B ) && ( model <= 0x204D ) ) ) ? true : false;
}

inline bool isNormalColor( Q_UINT16 color )
{
	return ( ( color >= 2 ) && ( color < 0x3ea ) ) ? true : false;
}

inline bool isSkinColor( Q_UINT16 color )
{
	return ( ( color >= 0x3EA ) && ( color <= 0x422 ) ) ? true : false;
}

inline bool isHairColor( Q_UINT16 color )
{
	return ( ( color >= 0x44E ) && ( color <= 0x47D ) ) ? true : false;
}

#endif
