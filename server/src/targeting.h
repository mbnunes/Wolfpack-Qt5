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

#ifndef _TARGETING_INCLUDED
#define _TARGETING_INCLUDED

// Platform specifics
#include "platform.h"

//system includes
#include <iostream>

using namespace std ;

//Forward class
class cTarget ;
class cCharTarget ;
class cItemTarget ;
class cWpObjTarget ;
class cTargets ;
class cRenameTarget ;
class cRemoveTarget ;
class cNewzTarget ;
class cBoltTarget ;
class cSetAmountTarget ;


//Wolfpack includes
#include "wolfpack.h"
#include "basics.h"
#include "cmdtable.h"
#include "itemid.h"
#include "SndPkg.h"

#include "Client.h"
#include "debug.h"


//##ModelId=3C5D92B600A7
typedef struct _PKGx6C
{
//Clicking Commands (19 bytes) 
//* BYTE cmd 
//* BYTE type 
//* 0x00 = server set cursor to pointer 
//* 0x01 = client return pointer position 
//* BYTE[4] charID
	short Tnum;		// we only use the low byte anyway, so short
//* BYTE Cursor Type 
//* 0x00 - Select Object 
//* 0x01 - Choose x, y, z 
//* The following are always sent but are only valid if sent by client 
//* BYTE[4] Clicked On ID
	long Tserial;
//* BYTE[2] click xLoc
	short TxLoc;
//* BYTE[2] click yLoc 
	short TyLoc;
//* BYTE unknown2 (0x00) 
//* BYTE click zLoc
	signed char TzLoc;
//* BYTE[2] model # (if a static tile, 0 if a map/landscape tile)
	short model;
//Note: the model # and charID should NEVER be trusted.
} PKGx6C;
	
//##ModelId=3C5D92BC01D2
class cTargets
{
private:
	//##ModelId=3C5D92BC01F0
	void CharTarget(P_CLIENT ps, PKGx6C *pt);
	//##ModelId=3C5D92BC020F
	void HouseSecureDown( UOXSOCKET s ); // Ripper
	//##ModelId=3C5D92BC022C
	void HouseLockdown( UOXSOCKET s ); // Abaddon
	//##ModelId=3C5D92BC0240
    void HouseRelease( UOXSOCKET s ); // Abaddon
	//##ModelId=3C5D92BC025E
	void Priv3XTarget(int s);
	//##ModelId=3C5D92BC0272
	void ShowPriv3Target(int s); // Whose been screwing with this? cj 8/11/99
	//##ModelId=3C5D92BC0286
	void PlVBuy(int s);
	//##ModelId=3C5D92BC02A4
	void RenameTarget(int s);
	//##ModelId=3C5D92BC02B9
	void RemoveTarget(int s);
	//##ModelId=3C5D92BC02D7
	void NewzTarget(int s);
	//##ModelId=3C5D92BC02EB
	void TypeTarget(int s);
	//##ModelId=3C5D92BC02FF
	void IstatsTarget(int s);
	//##ModelId=3C5D92BC031D
	void GhostTarget(int s);
	//##ModelId=3C5D92BC0331
	void AmountTarget(int s);
	//##ModelId=3C5D92BC0345
	void CloseTarget(int s);
	//##ModelId=3C5D92BC0363
	void VisibleTarget(int s);
	//##ModelId=3C5D92BC0377
	void DvatTarget(int s);
	//##ModelId=3C5D92BC0395
	void AllSetTarget(int s);
	//##ModelId=3C5D92BC03A9
	void TweakTarget(int s);
	//##ModelId=3C5D92BC03C7
	void LoadCannon(int s);
//	void SetInvulFlag(int s);
	//##ModelId=3C5D92BC03DB
	void SquelchTarg(int s);
	//##ModelId=3C5D92BD0011
	void SwordTarget(P_CLIENT pC, PKGx6C *pt);
	//##ModelId=3C5D92BD0026
	void NpcTarget(int s);
	//##ModelId=3C5D92BD0043
	void NpcTarget2(int s);
	//##ModelId=3C5D92BD00ED
	void NpcRectTarget(int s);
	//##ModelId=3C5D92BD0115
	void NpcCircleTarget(int s);
	//##ModelId=3C5D92BD013D
	void NpcWanderTarget(int s);
	//##ModelId=3C5D92BD0151
	void NpcAITarget(int s);
	//##ModelId=3C5D92BD016F
	void xBankTarget(int s);
	//##ModelId=3C5D92BD0183
	void xSpecialBankTarget(int s);//AntiChrist
	//##ModelId=3C5D92BD01AC
	void DupeTarget(int s);
	//##ModelId=3C5D92BD01C0
	void MoveToBagTarget(int s);
	//##ModelId=3C5D92BD0206
	void SellStuffTarget(int s);
	//##ModelId=3C5D92BD021A
	void GmOpenTarget(int s);
	//##ModelId=3C5D92BD0238
	void StaminaTarget(int s);
	//##ModelId=3C5D92BD0256
	void ManaTarget(int s);
	//##ModelId=3C5D92BD026A
	void MakeShopTarget(int s);
	//##ModelId=3C5D92BD0288
	void AttackTarget(int s);
	//##ModelId=3C5D92BD029C
	void FollowTarget(int s);
	//##ModelId=3C5D92BD02BA
	void TransferTarget(int s);
	//##ModelId=3C5D92BD02CE
	void BuyShopTarget(int s);
	//##ModelId=3C5D92BD02E2
	void permHideTarget(int s);
	//##ModelId=3C5D92BD0300
	void unHideTarget(int s);
	//##ModelId=3C5D92BD0314
	void SetSpeechTarget(int s);
	//##ModelId=3C5D92BD0329
	void SetPoisonTarget(int s);
	//##ModelId=3C5D92BD0346
	void SetPoisonedTarget(int s);
	//##ModelId=3C5D92BD035A
	void FullStatsTarget(int s);
	//##ModelId=3C5D92BD0378
	void SetAdvObjTarget(int s);
	//##ModelId=3C5D92BD0396
	void CanTrainTarget(int s);
	//##ModelId=3C5D92BD03C8
	void SetSplitTarget(int s);
	//##ModelId=3C5D92BE00C7
	void SetSplitChanceTarget(int s);
	//##ModelId=3C5D92BE00EF
	void SetSpaDelayTarget(int s);
	//##ModelId=3C5D92BE0153
	void NewXTarget(int s);
	//##ModelId=3C5D92BE0167
	void NewYTarget(int s);
	//##ModelId=3C5D92BE017B
	void IncXTarget(int s);
	//##ModelId=3C5D92BE0199
	void IncYTarget(int s);
	//##ModelId=3C5D92BE01AD
	void BoltTarget(int s);
	//##ModelId=3C5D92BE01C2
	void SetDirTarget(int s);
	//##ModelId=3C5D92BE01DF
	void HouseOwnerTarget(int s);
	//##ModelId=3C5D92BE0207
	void HouseEjectTarget(int s);
	//##ModelId=3C5D92BE021B
	void HouseBanTarget(int s);
	//##ModelId=3C5D92BE022F
	void HouseFriendTarget(int s);
	//##ModelId=3C5D92BE024D
	void HouseUnBanTarget(int s);
	//##ModelId=3C5D92BE0261
	void HouseUnFriendTarget(int s);
	//##ModelId=3C5D92BE028A
	void BanTarg(int s);
	//##ModelId=3C5D92BE02A7
	void triggertarget(int ts); // By Magius(CHE)
	//##ModelId=3C5D92BE02C5
	void GlowTarget(int s);
	//##ModelId=3C5D92BE0301
    void UnglowTaget(int s); 
	//##ModelId=3C5D92BE0320
	void TargetsMenuPrivTarg(int s);
	//##ModelId=3C5D92BE0334
	void ResurrectionTarget( UOXSOCKET s );
	//##ModelId=3C5D92BE0348
	void MenuPrivTarg(int s);
	//##ModelId=3C5D92BE0370
	void ShowSkillTarget(int s);
	//taken from 6904t2(5/10/99) - AntiChrist
	//##ModelId=3C5D92BE03A2
	void GuardTarget( UOXSOCKET s );
	//##ModelId=3C5D92BE03C0
	void FetchTarget( UOXSOCKET s );
	//##ModelId=3C5D92BE03D4
	void SetMurderCount( int s );	// Abaddon 12 Sept 1999
	//##ModelId=3C5D92BF003C
	void ShowAccountCommentTarget(int s);//AntiChrist
	//##ModelId=3C5D92BF005A
	void SetHome(int s);
	//##ModelId=3C5D92BF006E
	void SetWork(int s);
	//##ModelId=3C5D92BF0083
	void SetFood(int s);
	//##ModelId=3C5D92BF00B4
	void TargIdTarget(UOXSOCKET s); // added -Fraz- 
public:
	//##ModelId=3C5D92BF00C9
	void IDtarget(int s);
	//##ModelId=3C5D92BF00E6
	void MultiTarget(P_CLIENT ps);
	//##ModelId=3C5D92BF00FA
	void Wiping(int s);
	//##ModelId=3C5D92BF0140
	P_CHAR NpcMenuTarget(int s);
	//##ModelId=3C5D92BF0154
	bool NpcResurrectTarget(P_CHAR pc);
	//##ModelId=3C5D92BF0190
	void JailTarget(int s, int c);
	//##ModelId=3C5D92BF01AE
	void ReleaseTarget(int s, int c);
	//##ModelId=3C5D92BF01C2
	P_ITEM AddMenuTarget(int s, int x, int addmitem);
	//##ModelId=3C5D92BF0231
	void XTeleport(int s, int x);
	//##ModelId=3C5D92BF0245
	int BuyShop(UOXSOCKET s, P_CHAR pc);
};
#endif	// _TARGETING_INCLUDED
