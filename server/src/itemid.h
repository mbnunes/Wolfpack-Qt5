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
// Name:	itemid.h
// Purpose: interface for itemid.cpp
// History:	by Duke, 24.02.01
//			added some IsWeapon stuff (Duke, 20.4.2001)
//
#ifndef _ITEMID_H
#define _ITEMID_H

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

#endif
