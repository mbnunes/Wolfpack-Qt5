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

#if !defined(__BOATS_H__)
#define __BOATS_H__

// Platform specifics
#include "platform.h"


// System Includes

#include <iostream>
#include <cmath>

using namespace std;


// Forward class Declaration

class cBoat;

// Wolfpack files
#include "wolfpack.h"
#include "SndPkg.h"

//For iSizeShipOffsets->
#define PORT_PLANK 0
#define STARB_PLANK 1
#define HOLD 2
#define TILLER 3

//For iShipItems->
#define PORT_P_C 0//Port Plank Closed
#define PORT_P_O 1//Port Planl Opened
#define STAR_P_C 2//Starboard Plank Closed
#define STAR_P_O 3//Starb Plank Open
#define TILLERID 5//Tiller
#define HOLDID 4//Hold

extern signed short int iSmallShipOffsets[4][4][2];
extern signed short int iMediumShipOffsets[4][4][2];
extern signed short int iLargeShipOffsets[4][4][2];
extern unsigned char cShipItems[4][6];

class cBoat 
{
	private:
		void LeaveBoat(int, P_ITEM);		
		bool Block(P_ITEM, short int, short int,int);
		void TurnStuff(P_ITEM pBoat, P_CHAR pc_i, int dir);
		void TurnStuff(P_ITEM pBoat, P_ITEM pi, int dir);

	public:
		P_ITEM GetBoat(P_CHAR);
		char Speech(int, char *);
		void OpenPlank(P_ITEM);
		void PlankStuff(UOXSOCKET, P_ITEM);
		bool Build(int, P_ITEM pBoat, char);
		void Move(int, int, P_ITEM pBoat);
		void Turn(P_ITEM pBoat, int);
};
#endif

