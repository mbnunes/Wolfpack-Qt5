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
//	Wolfpack Homepage: http://www.wpdev.sf.net/
//========================================================================================

#if !defined (__BOUNTY_H__)
#define __BOUNTY_H__

// WOLFPACK - Bounty System
// FILE - bounty.h
// DATE - July 17, 2000
//   By - Dupois (dupois@home.com)
//
// Platform Specific
#include "platform.h"

// System includes

#include <iostream>

using namespace std ;

// Forward Class Declaration
class cBounty;


// Wolfpack includes
#include "wolfpack.h"
#include "bounty.h"

class cBounty
{
public:
     // Ask victim if they want to post a bounty on the
     // murderer, and if so, then return the amount of the
     // reward they want to place on the murderers head.
     void BountyAskVictim  ( int nVictimSerial,
                             int nMurdererSerial );
     // Create the bounty on the murderer
     bool BountyCreate     ( int nMurdererSerial,
                             int nRewardAmount );
     // Remove the bounty from the murderer
     bool BountyDelete     ( int nMurdererSerial );
     // Check and then withdraw the bounty amount
     bool BountyWithdrawGold( P_CHAR pVictim, int nAmount );
};

extern cBounty    *Bounty;

#endif

