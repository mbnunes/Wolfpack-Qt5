//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001-2003 by holders identified in authors.txt
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

#ifndef _TARGETING_INCLUDED
#define _TARGETING_INCLUDED

// Platform specifics
#include "platform.h"

//system includes
#include <iostream>

//Forward class
class cTarget ;
class cCharTarget ;
class cItemTarget ;
class cWpObjTarget ;
class cTargets ;
class cRenameTarget ;
class cNewzTarget ;
class cBoltTarget ;
class cSetAmountTarget ;


//Wolfpack includes
#include "wolfpack.h"
#include "basics.h"
#include "itemid.h"
#include "debug.h"

class cTargets
{
private:
	void HouseSecureDown( UOXSOCKET s ); // Ripper
	void HouseLockdown( UOXSOCKET s ); // Abaddon
    void HouseRelease( UOXSOCKET s ); // Abaddon
	void PlVBuy(int s);
	void RenameTarget(int s);
	void RemoveTarget(int s);
	void NewzTarget(int s);
	void TypeTarget(int s);
	void IstatsTarget(int s);
	void AmountTarget(int s);
	void VisibleTarget(int s);
	void DvatTarget(int s);
	void LoadCannon(int s);
	void NpcTarget(int s);
	void NpcTarget2(int s);
	void NpcRectTarget(int s);
	void NpcCircleTarget(int s);
	void NpcWanderTarget(int s);
	void NpcAITarget(int s);
	void xBankTarget(int s);
	void xSpecialBankTarget(int s);//AntiChrist
	void MoveToBagTarget(int s);
	void SellStuffTarget(int s);
	void GmOpenTarget(int s);
	void StaminaTarget(int s);
	void ManaTarget(int s);
	void AttackTarget(int s);
	void FollowTarget(int s);
	void TransferTarget(int s);
	void BuyShopTarget(int s);
	void SetSpeechTarget(int s);
	void SetPoisonTarget(int s);
	void CanTrainTarget(int s);
	void SetSpaDelayTarget(int s);
	void NewXTarget(int s);
	void NewYTarget(int s);
	void IncXTarget(int s);
	void IncYTarget(int s);
	void BoltTarget(int s);
	void HouseOwnerTarget(int s);
	void HouseEjectTarget(int s);
	void HouseBanTarget(int s);
	void HouseFriendTarget(int s);
	void HouseUnBanTarget(int s);
	void HouseUnFriendTarget(int s);
	void TargetsMenuPrivTarg(int s);
	void ResurrectionTarget( UOXSOCKET s );
	//taken from 6904t2(5/10/99) - AntiChrist
	void GuardTarget( UOXSOCKET s );
	void FetchTarget( UOXSOCKET s );
	void SetMurderCount( int s );	// Abaddon 12 Sept 1999
public:
	void Wiping(int s);
	P_CHAR NpcMenuTarget(int s);
	bool NpcResurrectTarget(P_CHAR pc);
	void JailTarget(int s, int c);
	void ReleaseTarget(int s, int c);
	P_ITEM AddMenuTarget(int s, int x, int addmitem);
	int BuyShop(UOXSOCKET s, P_CHAR pc);
};
#endif	// _TARGETING_INCLUDED
