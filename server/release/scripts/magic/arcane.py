#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Spellweaving (Arcane) Magic System         #
#   )).-' {{ ;'`   | Ported & Enhanced from ServUO              #
#  ( (  ;._ \\ ctr |                                            #
#===============================================================#

import random
import math
import wolfpack
import wolfpack.time
from wolfpack.consts import *
from wolfpack.utilities import energydamage, mayAreaHarm
from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell
from magic.utilities import *
import magic

class ArcaneCircle(Spell):
	def __init__(self):
		Spell.__init__(self, 1)
		self.skill = SPELLWEAVING
		self.requiredskill = 1
		self.mana = 24
		self.mantra = ''

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False
		char.soundeffect(0x5CA)
		char.effect(0x375A, 1, 15)
		char.settag('arcane_focus', min(6, 1 + random.randint(1, 3)))
		if char.socket:
			char.socket.sysmessage("You feel the arcane forces focus around you.")
		return True

class GiftOfRenewal(CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 1)
		self.skill = SPELLWEAVING
		self.requiredskill = 1
		self.mana = 24
		self.mantra = ''
		self.validtarget = TARGET_CHAR

	def effect(self, char, target, mode, args, item):
		target.soundeffect(0x5C9)
		target.effect(0x3779, 10, 20)
		heal = int(10 + (char.skill[SPELLWEAVING] / 100) * 3)
		if target.hitpoints < target.maxhitpoints:
			target.hitpoints = min(target.maxhitpoints, target.hitpoints + heal)
			target.updatehealth()
		if target.poison != -1:
			from system import poison
			poison.cure(target)
		target.settag('gift_of_renewal', 1)
		target.settag('renewal_expire', wolfpack.time.currenttime() + 30000)

class ImmolatingWeapon(Spell):
	def __init__(self):
		Spell.__init__(self, 1)
		self.skill = SPELLWEAVING
		self.requiredskill = 100
		self.mana = 32
		self.mantra = ''

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False
		char.soundeffect(0x5CA)
		char.effect(0x3709, 1, 30)
		char.settag('immolating_weapon', int(10 + char.skill[SPELLWEAVING] / 100))
		char.settag('immolating_expire', wolfpack.time.currenttime() + 15000)
		if char.socket:
			char.socket.sysmessage("Your weapon is wreathed in roaring flames.")
		return True

class Attunement(Spell):
	def __init__(self):
		Spell.__init__(self, 1)
		self.skill = SPELLWEAVING
		self.requiredskill = 1
		self.mana = 24
		self.mantra = ''

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False
		char.soundeffect(0x5C3)
		char.effect(0x3728, 1, 13)
		absorb = int(18 + (char.skill[SPELLWEAVING] / 100) * 3)
		char.settag('melee_absorb', absorb)
		if char.socket:
			char.socket.sysmessage(f"A shielding aura absorbs up to {absorb} damage.")
		return True

class Thunderstorm(DelayedDamageSpell):
	def __init__(self):
		DelayedDamageSpell.__init__(self, 2)
		self.skill = SPELLWEAVING
		self.requiredskill = 100
		self.mana = 32
		self.mantra = ''
		self.harmful = 1
		self.validtarget = TARGET_CHAR

	def effect(self, char, target, mode, args, item):
		target.soundeffect(0x5CE)
		target.effect(0x376A, 1, 20)
		damage = int(15 + (char.skill[SPELLWEAVING] / 100) * 5)
		target.damage(DAMAGE_ENERGY, damage, char)

class NaturesFury(Spell):
	def __init__(self):
		Spell.__init__(self, 2)
		self.skill = SPELLWEAVING
		self.requiredskill = 1
		self.mana = 24
		self.mantra = ''
		self.validtarget = TARGET_GROUND

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False
		pos = target if target else char.pos
		char.soundeffect(0x5CC)
		npc = wolfpack.addnpc('natures_fury' if wolfpack.hasdefinition(WPDT_NPC, 'natures_fury') else 'air_elemental', pos)
		if npc:
			npc.tamed = True
			npc.tamer = char
			npc.settag('decay_time', wolfpack.time.currenttime() + 25000)
		return True

class SummonFey(Spell):
	def __init__(self):
		Spell.__init__(self, 3)
		self.skill = SPELLWEAVING
		self.requiredskill = 380
		self.mana = 10
		self.mantra = ''

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False
		char.soundeffect(0x217)
		char.effect(0x3728, 1, 10)
		mob = 'pixie' if wolfpack.hasdefinition(WPDT_NPC, 'pixie') else 'wisp'
		npc = wolfpack.addnpc(mob, char.pos)
		if npc:
			npc.tamed = True
			npc.tamer = char
			npc.wander = 4
			npc.following = char
			npc.settag('decay_time', wolfpack.time.currenttime() + 120000)
		return True

class SummonFiend(Spell):
	def __init__(self):
		Spell.__init__(self, 3)
		self.skill = SPELLWEAVING
		self.requiredskill = 380
		self.mana = 10
		self.mantra = ''

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False
		char.soundeffect(0x217)
		char.effect(0x3728, 1, 10)
		mob = 'imp' if wolfpack.hasdefinition(WPDT_NPC, 'imp') else 'mongbat'
		npc = wolfpack.addnpc(mob, char.pos)
		if npc:
			npc.tamed = True
			npc.tamer = char
			npc.wander = 4
			npc.following = char
			npc.settag('decay_time', wolfpack.time.currenttime() + 120000)
		return True

class ReaperForm(Spell):
	def __init__(self):
		Spell.__init__(self, 4)
		self.skill = SPELLWEAVING
		self.requiredskill = 240
		self.mana = 34
		self.mantra = ''

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False
		char.soundeffect(0x1B0)
		char.effect(0x373A, 1, 15)
		char.settag('reaper_form', 1)
		char.settag('reaper_expire', wolfpack.time.currenttime() + 180000)
		if char.socket:
			char.socket.sysmessage("You assume the form and power of a grim reaper.")
		return True

class Wildfire(Spell):
	def __init__(self):
		Spell.__init__(self, 5)
		self.skill = SPELLWEAVING
		self.requiredskill = 660
		self.mana = 50
		self.mantra = ''
		self.validtarget = TARGET_GROUND

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False
		pos = target if target else char.pos
		char.soundeffect(0x5CF)
		# Area fire damage
		chars = wolfpack.chars(pos, 5)
		for c in chars:
			if c != char and mayAreaHarm(char, c):
				c.effect(0x3709, 1, 30)
				c.damage(DAMAGE_FIRE, int(15 + char.skill[SPELLWEAVING] / 50), char)
		return True

class EssenceOfWind(Spell):
	def __init__(self):
		Spell.__init__(self, 5)
		self.skill = SPELLWEAVING
		self.requiredskill = 520
		self.mana = 40
		self.mantra = ''

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False
		char.soundeffect(0x5C6)
		char.effect(0x37CC, 1, 20)
		chars = wolfpack.chars(char.pos, 6)
		for c in chars:
			if c != char and mayAreaHarm(char, c):
				c.effect(0x37CC, 1, 15)
				c.damage(DAMAGE_COLD, int(18 + char.skill[SPELLWEAVING] / 50), char)
		return True

class DryadAllure(CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 5)
		self.skill = SPELLWEAVING
		self.requiredskill = 520
		self.mana = 40
		self.mantra = ''
		self.validtarget = TARGET_CHAR

	def effect(self, char, target, mode, args, item):
		target.soundeffect(0x5C4)
		target.effect(0x375A, 1, 15)
		if not target.player and target.karma >= 0:
			target.tamed = True
			target.tamer = char
			target.wander = 4
			target.following = char
			if char.socket:
				char.socket.sysmessage("The creature is charmed by your dryad allure.")

class EtherealVoyage(Spell):
	def __init__(self):
		Spell.__init__(self, 6)
		self.skill = SPELLWEAVING
		self.requiredskill = 240
		self.mana = 32
		self.mantra = ''

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False
		char.soundeffect(0x5C5)
		char.effect(0x375A, 1, 15)
		char.settag('ethereal_voyage', 1)
		char.settag('ethereal_expire', wolfpack.time.currenttime() + 25000)
		if char.socket:
			char.socket.sysmessage("You wander into an ethereal voyage.")
		return True

class WordOfDeath(DelayedDamageSpell):
	def __init__(self):
		DelayedDamageSpell.__init__(self, 7)
		self.skill = SPELLWEAVING
		self.requiredskill = 800
		self.mana = 50
		self.mantra = ''
		self.harmful = 1
		self.validtarget = TARGET_CHAR

	def effect(self, char, target, mode, args, item):
		target.soundeffect(0x211)
		target.effect(0x3709, 1, 30)
		# Execute below 30% hp
		if target.hitpoints <= (target.maxhitpoints * 0.3):
			target.damage(DAMAGE_MAGICAL, min(300, target.hitpoints), char)
		else:
			dmg = int(35 + (char.skill[SPELLWEAVING] / 100) * 10)
			target.damage(DAMAGE_ENERGY, dmg, char)

class GiftOfLife(CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 7)
		self.skill = SPELLWEAVING
		self.requiredskill = 380
		self.mana = 70
		self.mantra = ''
		self.validtarget = TARGET_CHAR

	def effect(self, char, target, mode, args, item):
		target.soundeffect(0x5C7)
		target.effect(0x375A, 1, 15)
		target.settag('gift_of_life', 1)
		target.settag('gift_of_life_expire', wolfpack.time.currenttime() + 600000) # 10 mins
		if char.socket:
			char.socket.sysmessage("Gift of Life is placed upon the target.")

class ArcaneEmpowerment(Spell):
	def __init__(self):
		Spell.__init__(self, 8)
		self.skill = SPELLWEAVING
		self.requiredskill = 240
		self.mana = 50
		self.mantra = ''

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False
		char.soundeffect(0x5C8)
		char.effect(0x373A, 1, 15)
		char.settag('arcane_empowerment', 1)
		char.settag('arcane_empowerment_expire', wolfpack.time.currenttime() + 30000)
		if char.socket:
			char.socket.sysmessage("Your magical potential surges with arcane empowerment.")
		return True

def onLoad():
	ArcaneCircle().register(601)
	GiftOfRenewal().register(602)
	ImmolatingWeapon().register(603)
	Attunement().register(604)
	Thunderstorm().register(605)
	NaturesFury().register(606)
	SummonFey().register(607)
	SummonFiend().register(608)
	ReaperForm().register(609)
	Wildfire().register(610)
	EssenceOfWind().register(611)
	DryadAllure().register(612)
	EtherealVoyage().register(613)
	WordOfDeath().register(614)
	GiftOfLife().register(615)
	ArcaneEmpowerment().register(616)
