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

#ifndef __SKILLS_H
#define __SKILLS_H

#include "defines.h"
#include "globals.h"
#include "typedefs.h"
#include "wptargetrequests.h"

// Library Includes
#include <qmap.h>
#include <qstringlist.h>
#include <qvaluevector.h>

// Forward Declaration
class cUOSocket;

struct stAdvancement
{
	UINT16 base;
	UINT16 success;
	UINT16 failure;
};

struct stSkill
{
	UINT16 strength, dexterity, intelligence;
	QString name, defname, title;
	QValueVector< stAdvancement > advancement;
};

class cSkills
{
private:
	void Tracking(int s, int selection);
	int GetSubIngotAmt(int p, char id1, char id2, char color1, char color2);
	int DeleSubIngot(int p, int id1, int id2, int color1, int color2, int amount);
	void Hide( cUOSocket* );
	void Stealth( cUOSocket* );
	void PeaceMaking( cUOSocket* );

	QStringList skillRanks;
	QValueVector< stAdvancement > advStrength;
	QValueVector< stAdvancement > advDexterity;
	QValueVector< stAdvancement > advIntelligence;
	QValueVector< stSkill > skills;
public:
	// Skill management methods
	void load();
	void unload();
	void reload() { unload(); load(); }
	QString getSkillTitle( P_CHAR pChar ) const;
	const QString &getSkillName( UINT16 skill ) const;
	const QString &getSkillDef( UINT16 skill ) const;
	INT16 findSkillByDef( const QString &defname ) const; // -1 = Not Found

	bool advanceSkill( P_CHAR pChar, UINT16 skill, SI32 min, SI32 max, bool success ) const;
	bool advanceStats( P_CHAR pChar, UINT16 skill ) const;

	// Skill Usage methods
	void PlayInstrumentWell(cUOSocket*, P_ITEM pi);
	void PlayInstrumentPoor(cUOSocket*, P_ITEM pi);
	P_ITEM GetInstrument( cUOSocket* );
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
	void Track(P_CHAR pc_i);
	void PotionToBottle(P_PLAYER pc, P_ITEM pi_mortar);
	void SpiritSpeak(int s);
	void SkillUse( cUOSocket*, UINT16 );
	void CreateTrackingMenu(int s, int m);
	void TrackingMenu(int s, int gmindex);
	void Persecute(cUOSocket*);//AntiChrist persecute stuff
	int GetAntiMagicalArmorDefence(P_CHAR pc); // blackwind meditation armor stuff
	void Snooping(P_PLAYER, P_ITEM);
};

// Repair Item

#endif
