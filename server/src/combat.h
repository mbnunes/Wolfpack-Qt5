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

#if !defined(__COMBAT_H__)
#define __COMBAT_H__

#include "typedefs.h"
class Coord_cl;
class cUOSocket;

namespace Combat 
{
	enum enBowTypes
	{
		INVALID_BOWTYPE = 0xFF,
		BOW = 0,
		XBOW,
		HEAVYXBOW
	};
	
	UI16 weaponSkill( P_ITEM pi );
	enBowTypes bowType( P_ITEM pi );
	void checkandhit( P_CHAR pAttacker );
	void hit( P_CHAR pAttacker, P_CHAR pDefender, bool los );
	void combat( P_CHAR pAttacker );
	void setWeaponTimeout( P_CHAR pAttacker, P_ITEM pWeapon );	
	bool isTimerOk( P_CHAR pc );
	void doFootCombatAnimation( P_CHAR pc );
	void doHorseCombatAnimation( P_CHAR pc );

	// Sounds + Animations
	void playMissedSoundEffect( P_CHAR pChar, P_ITEM pWeapon );
	void playSoundEffect( P_CHAR pChar, P_ITEM pWeapon );
	void playGetHitSoundEffect( P_CHAR pChar );
	void playGetHitAnimation( P_CHAR pChar );

	void doCombatAnimations( P_CHAR pAttacker, P_CHAR pDefender, P_ITEM pWeapon );

	void spawnGuard( P_CHAR pOffender, P_CHAR pCaller, const Coord_cl &pos );
};

#endif // __COMBAT_H__

