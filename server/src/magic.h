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

#ifndef __MAGIC_H__
#define __MAGIC_H__

#include "typedefs.h"
#include "structs.h"

// Library includes
#include "qstring.h"
#include "qdom.h"
#include <map>

using namespace std;

// Handles the casting of a specific spell
class cSpell
{
// Private data
protected:

	// BIT | DESCRIPTION
	// -----------------
	//   0 | Enabled/Disabled
	//   1 | Reflectable / Not Reflectable
	//   2 | Agressive / Not Agressive
	//   3 | Spell has "runic" Words of Power
	UI08 bitfield_; 

	UI08 circle_;			// Circle the spell is in
	UI08 id_;				// ID of our spell
	
	// Put that into "requirements"
	UI08 mana_;				// Mana requirements
	UI08 stamina_;			// Stamina requirements
	UI08 health_;			// Health requirements (Sacrifice Spells)
	
	// Required reagents
	UI08 reqGinseng_;
	UI08 reqMoss_;
	UI08 reqDrake_;
	UI08 reqPearl_;
	UI08 reqSilk_;
	UI08 reqAsh_;
	UI08 reqShade_;
	UI08 reqGarlic_;

	UI16 lowSkill_;			// low magery skill req.
	UI16 highSkill_;		// high magery skill req.
	UI16 scrollLowSkill_;	// high magery skill req. if using scroll
	UI16 scrollHighSkill_;	// low magery skill req. if using scroll

	UI08 action_;			// The animation displayed when the character casts the spell
	UI16 delay_;			// Delay before the spell is really casted

	QString mantra_;		// The "Words of Power"
	QString target_;		// This message is sysmessage'd to the client when it casts the spell

public:
	// Bitfield getters
	bool			enabled()			const { return bitfield_&0x01; }
	bool			reflect()			const { return bitfield_&0x02; }
	bool			agressive()			const { return bitfield_&0x04; }
	bool			runic()				const { return bitfield_&0x08; }

	// Bitfield setters - Could be private ;o)
	void			setEnabled( bool data )			{ if( data ) bitfield_ |= 0x01; else bitfield_ &= 0xFE; }
	void			setReflect( bool data )			{ if( data ) bitfield_ |= 0x02; else bitfield_ &= 0xFD; }
	void			setAgressive( bool data )		{ if( data ) bitfield_ |= 0x04; else bitfield_ &= 0xFB; }
	void			setRunic( bool data )			{ if( data ) bitfield_ |= 0x08; else bitfield_ &= 0xF7; }

	//=Getters
	UI08			id()				const { return id_; }
	UI08			mana()				const { return mana_; }
	UI08			stamina()			const { return stamina_; }

	// Required Reagents
	UI08			reqGinseng()		const { return reqGinseng_; }
	UI08			reqMoss()			const { return reqMoss_; }
	UI08			reqDrake()			const { return reqDrake_; }
	UI08			reqPearl()			const { return reqPearl_; }
	UI08			reqSilk()			const { return reqSilk_; }
	UI08			reqAsh()			const { return reqAsh_; }
	UI08			reqShade()			const { return reqShade_; }
	UI08			reqGarlic()			const { return reqGarlic_; }

	UI08			health()			const { return health_; }
	UI08			action()			const { return action_; }
	UI16			delay()				const { return delay_; }
	UI16			lowSkill()			const { return lowSkill_; }
	UI16			highSkill()			const { return highSkill_; }
	UI16			scrollLowSkill()	const { return scrollLowSkill_; }
	UI16			scrollHighSkill()	const { return scrollHighSkill_; }
	const QString	&mantra()			const { return mantra_; }
	const QString	&target()			const { return target_; }

	// Setters are not required as we're loading ourself into memory by 
	// load( QDomElement &node );
	void			load( QDomElement &node );	// Loads the spell from a XML node

	// Spell is cast by the specified character
	// SOURCE | DESCRIPTION
	// --------------------
	//      0 | Spell is cast out of a spellbook
	//		1 | Spell is cast of a scroll
	//		2 | Spell is cast using a wand/rod/etc.
	bool			prepare( P_CHAR character, UI08 source );
	void			cast( P_CHAR character, UI08 source );

	cSpell( void );
};

class cMagic
{
// New interface
protected:
	map< UI08, cSpell* > loadedSpells; // Rename to "spells" later

public:
	void	load( void ); // Loads the magic system
	void	unload( void );
	void	reload( void ); // Reloads all spells

	// This casts a specified -spell- 
	// SOURCE | DESCRIPTION
	// --------------------
	//      0 | Spell is cast out of a spellbook
	//		1 | Spell is cast of a scroll
	//		2 | Spell is cast using a wand/rod/etc.
	bool	prepare( P_CHAR caster, UI08 spellId, UI08 sourceType, P_ITEM sourceItem = NULL );
	void	cast( P_CHAR caster );

	bool	checkReagents( P_CHAR caster, cSpell *spell );
	bool	checkStats( P_CHAR caster, cSpell *spell );
	void	speakMantra( P_CHAR caster, cSpell *spell );
	
	cSpell	*getSpell( UI08 spellId );
	
	void	openSpellBook( P_CHAR mage, P_ITEM spellbook );
	P_ITEM	findSpellBook( P_CHAR mage );

	SI16	calcSpellId( UI16 model );

public:
	char CheckResist(P_CHAR attacker, P_CHAR defender, int circle);
	void PoisonDamage(P_CHAR pc, int poison);
	void CheckFieldEffects2(unsigned int currenttime, P_CHAR pc, char timecheck);
	void SpellBook(UOXSOCKET s) ;
	void SpellBook(UOXSOCKET s, P_ITEM pi);
	int  SpellsInBook(P_ITEM pi);
	char GateCollision(P_CHAR pc_player);
	bool requireTarget( unsigned char num );
	bool reqItemTarget( int num );
	bool reqLocTarget( int num );
	bool reqCharTarget( int num );
	bool spellReflectable( int num );
	bool travelSpell( int num );
	stat_st getStatEffects( int num );
	stat_st getStatEffects_pre( int num);
	stat_st getStatEffects_after( int num);
	stat_st getStatEffects_item( int num);
	void preParticles(int num, P_CHAR pc);
	void afterParticles(int num, P_CHAR pc);
	void itemParticles(int num, P_ITEM pi);
	void invisibleItemParticles(P_CHAR pc, int num, short x, short y, signed char z);
	move_st getMoveEffects( int num );
	sound_st getSoundEffects( int num );
	bool aggressiveSpell( int num );
	bool fieldSpell( int num );
	void doMoveEffect( int num, P_CHAR target, P_CHAR source );
	void doStaticEffect( P_CHAR source, int num );
	void playSound( P_CHAR pc_source, int num );
	void DelReagents( P_CHAR pc, int num );
	void NewCastSpell( UOXSOCKET s );
	void SbOpenContainer(int s);
	void PFireballTarget(P_CHAR pc_i, P_CHAR pc, int j);
	void LightningSpell(P_CHAR pc_Attacker, P_CHAR pc_Defender, bool usemana = true);
	void EnergyBoltSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana = true);
	void NPCHeal(P_CHAR pc);
	void NPCCure(P_CHAR pc);
	void NPCDispel(P_CHAR pc_s, P_CHAR pc_i);
	char CheckParry(P_CHAR pc_player, int circle);
	void MagicArrow(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana = true);
	void ClumsySpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana = true);
	void FeebleMindSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana = true);
	void WeakenSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana = true);
	void HarmSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana = true);
	void FireballSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana = true);
	void CurseSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana = true);
	void MindBlastSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana = true);
	void ParalyzeSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usenama = true);
	void ExplosionSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana = true);
	void FlameStrikeSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana = true);
	bool CheckBook(int circle, int spell, P_ITEM pi);
	char CheckMana(P_CHAR pc, int num);
    void Recall(UOXSOCKET s); // we need this four ones for the commands (separatly) !!!
    void Mark(UOXSOCKET s);
    void Gate(UOXSOCKET s);
    void Heal(UOXSOCKET s);
	bool CheckMagicReflect(P_CHAR pc);
	P_CHAR CheckMagicReflect(P_CHAR &attacker, P_CHAR &defender);
	void MagicDamage(P_CHAR pc, int amount);
	void SpellFail(int s);
	char SubtractMana(P_CHAR pc, int mana);
	void MagicTrap(P_CHAR pc, P_ITEM pTrap);	// moved here by AntiChrist (9/99)
	void Polymorph( int s, int gmindex, int creaturenumer); // added by AntiChrist (9/99)
	void Action4Spell(UOXSOCKET s, int num);
	char SubtractMana4Spell(P_CHAR pc, int num);
	void AfterSpellDelay(UOXSOCKET s, P_CHAR pc);


private:
	void SummonMonster(int s, unsigned char d1, unsigned char id2, char *monstername, unsigned char color1, unsigned char color2, int x, int y, int z, int spellnum);
	void BoxSpell(int s, int& x1, int& x2, int& y1, int& y2, int& z1, int& z2);
	void PolymorphMenu( int s, int gmindex ); // added by AntiChrist (9/99)
};

#endif