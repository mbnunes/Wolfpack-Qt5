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

// WOLFPACK - Bounty System
// FILE - bounty.cpp
// DATE - July 17, 2000
//   By - Dupois (dupois@home.com)
//
#include "bounty.h"

#undef  DBGFILE
#define DBGFILE "bounty.cpp"

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:    BountyAskVictim( int nVictimSerial, int nMurdererSerial )
//
// PURPOSE:     Used to ask the murder victim whether they would like to place
//              a bounty on the head of the murderer.
// 
// PARAMETERS:  nVictimSerial   Victim characters serial number
//              nMurdererSerial Murderer characters serial number
//
// RETURNS:     void
//////////////////////////////////////////////////////////////////////////////
void cBounty::BountyAskVictim( int nVictimSerial, int nMurdererSerial )
{
  int nAmount     = 0;
  PC_CHAR pcc_nVictimIdx  = FindCharBySerial( nVictimSerial );
  PC_CHAR pcc_nMurderIdx  = FindCharBySerial( nMurdererSerial );
  int err2;
 
  if (pcc_nVictimIdx == NULL || pcc_nMurderIdx == NULL) return;

  // Indicate that the victim is being asked if they want to place a bounty
  sprintf((char*)temp, "BountyAskVictim():  %s is attempting to place a bounty of %i on %s\n",
          pcc_nVictimIdx->name, nAmount, pcc_nMurderIdx->name );
  LogMessage((char*)temp);

  // If the amount of the bounty is larger than zero, create a bounty posting
  if( nAmount > 0 )
  {
     if( BountyCreate( pcc_nMurderIdx->serial, nAmount ) )
	 {
         sprintf((char*)temp, "BountyAskVictim():  %s has placed a bounty of %i on %s\n",
                pcc_nVictimIdx->name,
                nAmount,
                pcc_nMurderIdx->name );
		 LogMessage((char*)temp);
	 }

     else
	 {
       sprintf((char*)temp, "BountyAskVictim():  %s FAILED to place a bounty of %i on %s\n",
               pcc_nVictimIdx->name,
               nAmount,
               pcc_nMurderIdx->name );
	   LogMessage((char*)temp);
	 }
  }

  return;

} // BountyAskVictim()


//////////////////////////////////////////////////////////////////////////////
// FUNCTION:    BountyCreate( int nMurdererSerial, int nRewardAmount )
//
// PURPOSE:     Used to create the bounty posting message on the global
//              bulletin board for all to see.
// 
// PARAMETERS:  nMurdererSerial Murderer characters serial number
//              nRewardAmount   Bounty placed on the murderers head
//
// RETURNS:     TRUE  Bounty post successfully created
//              FALSE Bounty post could not be created
//////////////////////////////////////////////////////////////////////////////
bool cBounty::BountyCreate( int nMurdererSerial, int nRewardAmount )
{
  P_CHAR pc_nIndex  = FindCharBySerial( nMurdererSerial );
  int   nPostSerial = INVALID_SERIAL;
  
  if (pc_nIndex == NULL) return false;

  // Check that we have a reward amount greater than zero
  if( nRewardAmount > 0 )
  {
    // Check that this murderer doesn't already have a bounty on them
    if( pc_nIndex->questBountyReward > 0 )
    {
      // This murderer already has a bounty on them because they 
      // have a reward amount on their head, so delete old bounty
      // and add the new (updated) one
      nRewardAmount += pc_nIndex->questBountyReward;
      BountyDelete( nMurdererSerial );
    }

    // Attempt to post the message first
    pc_nIndex->questBountyReward = nRewardAmount;
    nPostSerial = MsgBoardPostQuest( nMurdererSerial, BOUNTYQUEST );

    // If we received a valid serial number then the post was successfull
    if( nPostSerial > 0 )
    {
      pc_nIndex->questBountyPostSerial = nPostSerial;
      return true;
    }
  }

  // Failed to post bounty
  sprintf((char*)temp, "BountyCreate():  FAILED to place a bounty of %i on %s (PostSerial=%x)\n",
          nRewardAmount,
          pc_nIndex->name,
          nPostSerial );
  LogWarning((char*)temp);

  // Post must have failed
  return false;

} // BountyCreate()


bool cBounty::BountyDelete( int nMurdererSerial )
{
  bool  bReturn = true;
  P_CHAR pc_nIndex  = FindCharBySerial( nMurdererSerial );

  if ( pc_nIndex == NULL ) return false;


  // Find and mark the post associated with this bounty as deleted
  // so that the bulletin board maintenance routine can clean it up
  bReturn = MsgBoardRemoveGlobalPostBySerial( pc_nIndex->questBountyPostSerial );

  // Reset all bounty values for this character
  pc_nIndex->questBountyReward     = 0;
  pc_nIndex->questBountyPostSerial = INVALID_SERIAL;

  return bReturn;

} // BountyDelete()


bool cBounty::BountyWithdrawGold( P_CHAR pVictim, int nAmount )
{
	int has = pVictim->CountBankGold();
	if (has < nAmount)
		return false;

	P_ITEM pBox = pVictim->GetBankBox();
	if (!pBox)
		return false;	// shouldn't happen coz it's needed in CountBankGold...

	pBox->DeleteAmount(nAmount,0x0EED);
	return true;
}
