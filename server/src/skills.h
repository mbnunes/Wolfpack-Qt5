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

#ifndef __SKILLS_H
#define __SKILLS_H

#include "defines.h"
#include "globals.h"
#include "typedefs.h"
#include "wptargetrequests.h"

// Forward Declaration
class cUOSocket;



class cSkills
{
private:
	void Tracking(int s, int selection);
	int GetSubIngotAmt(int p, char id1, char id2, char color1, char color2);
	int DeleSubIngot(int p, int id1, int id2, int color1, int color2, int amount);
	void AnvilTarget(int s, int m);
	void AnvilTargetGold(int s, int m);
	void AnvilTargetSilver(int s, int m);
	void AnvilTargetAdam(int s, int m);
	void AnvilTargetMyth(int s, int m);
	void AnvilTargetBronze(int s, int m);
	void AnvilTargetVerite(int s, int m);
	void AnvilTargetMerk(int s, int m);
	void AnvilTargetCop(int s, int m);
	void AnvilTargetAgap(int s, int m);
	void Hide( cUOSocket* );
	void Stealth( cUOSocket* );
	void PeaceMaking( cUOSocket* );
public:
	void PlayInstrumentWell(cUOSocket*, P_ITEM pi);
	void PlayInstrumentPoor(cUOSocket*, P_ITEM pi);
	P_ITEM GetInstrument( cUOSocket* );
	void TellScroll(char *menu_name, int player, long item_param);
	void CollectAmmo(int s, int a, int b);
	void Meditation(cUOSocket* s);
	// skills using crafting menus
	void Blacksmithing( cUOSocket* socket );
	void Carpentry( cUOSocket* socket );
	void Cartography( cUOSocket* socket );
	void Fletching( cUOSocket* socket );
	void Tailoring( cUOSocket* socket );
	void Tinkering( cUOSocket* socket );

	static void RandomSteal( cUOSocket*, SERIAL );	
	void RepairTarget(UOXSOCKET s); // Ripper
	void SmeltItemTarget(UOXSOCKET s); // Ripper
	int TrackingDirection(UOXSOCKET s, P_CHAR pc_i);
	void CreatePotion(P_CHAR pc, char type, char sub, P_ITEM pi_mortar);
	char AdvanceSkill(P_CHAR pc, int sk, char skillused);
	void AdvanceStats(P_CHAR pc, UINT16 sk);
	void TinkerAxel(int s);
	void TinkerAwg(int s);
	void TinkerClock(int s);
	void Track(P_CHAR pc_i);
	void DoPotion(int s, int type, int sub, P_ITEM mortar);
	void CookOnFire(int s, short id1, short id2, char* matname);
	void BottleTarget(int s);
	void PotionToBottle(P_CHAR pc, P_ITEM pi_mortar);
	void CreateBandageTarget(int s);
	void HealingSkillTarget(UOXSOCKET s);
	void SpiritSpeak(int s);
	void ArmsLoreTarget(int s);
	void SkillUse( cUOSocket*, UINT16 );
	void CreateTrackingMenu(int s, int m);
	void TrackingMenu(int s, int gmindex);
	int Inscribe(int s, long snum);
	int EngraveAction(int s, P_ITEM pi, int cir, int spl);
	void updateSkillLevel(P_CHAR pc, int s);
	void Persecute(cUOSocket*);//AntiChrist persecute stuff
	void Decipher(P_ITEM tmap, int s); // By Polygon - attempt to decipher a tattered treasure map
	int GetAntiMagicalArmorDefence(P_CHAR pc); // blackwind meditation armor stuff
	void Snooping(P_CHAR, P_ITEM);
};

// Repair Item
class cSkRepairItem : public cTargetRequest
{
private:
	cMakeSection* makesection_;
public:
	cSkRepairItem( cMakeSection* makesection = NULL ) : cTargetRequest(), makesection_( makesection ) {}
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target );
};

#endif