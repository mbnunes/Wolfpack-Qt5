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

class cCombat
{
private:
	void ItemCastSpell( P_CHAR pAttacker, P_CHAR pDefender, P_ITEM pItem );
	int TimerOk(P_CHAR pc);
	void ItemSpell(cChar* Attacker, cChar* Defender);
	void doSoundEffect(P_CHAR pc, int fightskill, P_ITEM pWeapon);//AntiChrist
	void doMissedSoundEffect(P_CHAR pc);//AntiChrist
	void DoCombatAnimations(P_CHAR pc_attacker, P_CHAR pc_defender, int fightskill, int bowtype, int los);
public:
	int GetBowType(P_CHAR pc);
	int CalcAtt(P_CHAR pc);
	int CalcDef(P_CHAR pc, int x);
	void CombatOnHorse(P_CHAR pc);
	void CombatOnFoot(P_CHAR pc);
	void CombatHitCheckLoS(P_CHAR pAttacker, unsigned int currenttime);
	void CombatHit(P_CHAR pc_attacker, P_CHAR pc_deffender, unsigned int currenttime, short los);
	void DoCombat(P_CHAR pc_attacker, unsigned int currenttime);
	void SpawnGuard( P_CHAR pc_offender, P_CHAR pc_caller, const Coord_cl &pos );
};

#endif // __COMBAT_H__

