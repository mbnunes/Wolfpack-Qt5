#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Mysticism Magic System                     #
#   )).-' {{ ;'`   | Ported & Enhanced from ServUO              #
#  ( (  ;._ \\ ctr | Spells 677 to 692                          #
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

# Helper to scale damage using Mysticism + Imbuing/Focus
def calcMysticPower(char):
	myst = char.skill[MYSTICISM] / 10.0
	other = max(char.skill[IMBUING], char.skill[FOCUS]) / 10.0
	return myst, other

# 677: Nether Bolt (Circle 1)
class NetherBolt(DelayedDamageSpell):
	def __init__(self):
		DelayedDamageSpell.__init__(self, 1)
		self.skill = MYSTICISM
		self.requiredskill = 1
		self.mana = 4
		self.mantra = 'In Corp Ylem'
		self.missile = [0x36D4, 7, 0, False]
		self.sound = 0x211
		self.delay = 500

	def damage(self, char, target):
		myst, other = calcMysticPower(char)
		dmg = int(6 + (myst * 0.1) + (other * 0.1) + random.randint(1, 4))
		target.damage(dmg, 0, 0, 0, 0, 0)
		target.effect(0x36D4, 7, 10)

# 678: Healing Stone (Circle 1)
class HealingStone(Spell):
	def __init__(self):
		Spell.__init__(self, 1)
		self.skill = MYSTICISM
		self.requiredskill = 1
		self.mana = 4
		self.mantra = 'Kal In Mani'

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False
		char.soundeffect(0x650)
		char.effect(0x3779, 10, 20)
		# Heal caster directly or cure minor poison
		heal = int(12 + (char.skill[MYSTICISM] / 100) * 8)
		if char.poison != -1:
			from system import poison
			poison.cure(char)
		if char.hitpoints < char.maxhitpoints:
			char.hitpoints = min(char.maxhitpoints, char.hitpoints + heal)
			char.updatehealth()
		if char.socket:
			char.socket.sysmessage("The healing stone knits your wounds.")
		return True

# 679: Purge Magic (Circle 2)
class PurgeMagic(CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 2)
		self.skill = MYSTICISM
		self.requiredskill = 80
		self.mana = 6
		self.mantra = 'An Ort Sanct'
		self.validtarget = TARGET_CHAR
		self.harmful = 1

	def effect(self, char, target, mode, args, item):
		target.soundeffect(0x654)
		target.effect(0x375A, 10, 15)
		# Removes curses or beneficial spells based on alignment
		for tag in ['bless', 'curse', 'agility', 'cunning', 'strength', 'protection', 'archprotection']:
			if target.hastag(tag):
				target.deltag(tag)
		if char.socket:
			char.socket.sysmessage("You purge magical enchantments from your target.")

# 680: Enchant (Circle 2)
class Enchant(Spell):
	def __init__(self):
		Spell.__init__(self, 2)
		self.skill = MYSTICISM
		self.requiredskill = 80
		self.mana = 6
		self.mantra = 'In Ort Ylem'

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False
		char.soundeffect(0x656)
		char.effect(0x376A, 9, 32)
		char.settag('enchanted_weapon', 1)
		char.settag('enchant_expire', wolfpack.time.currenttime() + 60000)
		if char.socket:
			char.socket.sysmessage("Your weapon is infused with mystic power.")
		return True

# 681: Sleep (Circle 3)
class Sleep(CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 3)
		self.skill = MYSTICISM
		self.requiredskill = 200
		self.mana = 9
		self.mantra = 'In Zu'
		self.validtarget = TARGET_CHAR
		self.harmful = 1

	def effect(self, char, target, mode, args, item):
		target.soundeffect(0x653)
		target.effect(0x374A, 10, 15)
		# Slow target stamina and paralyze briefly
		duration = min(6000, max(2000, int(char.skill[MYSTICISM] * 40)))
		target.paralyze(duration)
		if target.socket:
			target.socket.sysmessage("You feel overcome by profound slumber.")

# 682: Eagle Strike (Circle 3)
class EagleStrike(DelayedDamageSpell):
	def __init__(self):
		DelayedDamageSpell.__init__(self, 3)
		self.skill = MYSTICISM
		self.requiredskill = 200
		self.mana = 9
		self.mantra = 'Kal Por Xen'
		self.missile = [0x407A, 5, 0, False]
		self.sound = 0x2EE
		self.delay = 500

	def damage(self, char, target):
		myst, other = calcMysticPower(char)
		dmg = int(14 + (myst * 0.15) + (other * 0.15) + random.randint(1, 6))
		target.damage(dmg, 0, 0, 0, 0, 0)
		target.soundeffect(0x64D)
		target.effect(0x3779, 10, 15)

# 683: Animated Weapon (Circle 4)
class AnimatedWeapon(Spell):
	def __init__(self):
		Spell.__init__(self, 4)
		self.skill = MYSTICISM
		self.requiredskill = 330
		self.mana = 11
		self.mantra = 'In Jux Por Ylem'

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False
		char.soundeffect(0x64B)
		char.effect(0x3728, 10, 10)
		pos = wolfpack.coord(char.pos.x + 1, char.pos.y, char.pos.z, char.pos.map)
		npc = wolfpack.addnpc('animated_weapon', pos)
		if not npc:
			npc = wolfpack.addnpc('blade_spirit', pos)
		if npc:
			npc.summoned = 1
			npc.wandermode = 5
			npc.leader = char
			npc.addtimer(30000, summon_expire, [npc.serial], 0, 0)
		return True

# 684: Stone Form (Circle 4)
class StoneForm(Spell):
	def __init__(self):
		Spell.__init__(self, 4)
		self.skill = MYSTICISM
		self.requiredskill = 330
		self.mana = 11
		self.mantra = 'In Rel Ylem'

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False
		char.soundeffect(0x655)
		char.effect(0x376A, 1, 15)
		char.color = 0x386
		char.settag('stone_form', 1)
		char.settag('stone_expire', wolfpack.time.currenttime() + 60000)
		if char.socket:
			char.socket.sysmessage("Your body hardens into living granite.")
		return True

# 685: Spell Trigger (Circle 5)
class SpellTrigger(Spell):
	def __init__(self):
		Spell.__init__(self, 5)
		self.skill = MYSTICISM
		self.requiredskill = 450
		self.mana = 14
		self.mantra = 'In Vas Ort Ex'

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False
		char.soundeffect(0x657)
		char.effect(0x375A, 10, 15)
		char.settag('spell_trigger_active', 1)
		if char.socket:
			char.socket.sysmessage("A mystic spell trigger weaves into your aura.")
		return True

# 686: Mass Sleep (Circle 5)
class MassSleep(Spell):
	def __init__(self):
		Spell.__init__(self, 5)
		self.skill = MYSTICISM
		self.requiredskill = 450
		self.mana = 14
		self.mantra = 'Vas Zu'

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False
		char.soundeffect(0x653)
		char.effect(0x374A, 10, 20)
		targets = wolfpack.chars(char.pos.x, char.pos.y, char.pos.map, 6)
		for t in targets:
			if t.serial != char.serial and mayAreaHarm(char, t):
				t.effect(0x374A, 10, 15)
				t.paralyze(4000)
		return True

# 687: Cleansing Winds (Circle 6)
class CleansingWinds(Spell):
	def __init__(self):
		Spell.__init__(self, 6)
		self.skill = MYSTICISM
		self.requiredskill = 580
		self.mana = 20
		self.mantra = 'In Vas Mani Hur'

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False
		char.soundeffect(0x64C)
		char.effect(0x3709, 10, 30)
		targets = wolfpack.chars(char.pos.x, char.pos.y, char.pos.map, 5)
		heal = int(20 + (char.skill[MYSTICISM] / 100) * 12)
		for t in targets:
			if t.serial == char.serial or not mayAreaHarm(char, t):
				t.soundeffect(0x64C)
				t.effect(0x376A, 9, 32)
				if t.poison != -1:
					from system import poison
					poison.cure(t)
				if t.hitpoints < t.maxhitpoints:
					t.hitpoints = min(t.maxhitpoints, t.hitpoints + heal)
					t.updatehealth()
		return True

# 688: Bombard (Circle 6)
class Bombard(DelayedDamageSpell):
	def __init__(self):
		DelayedDamageSpell.__init__(self, 6)
		self.skill = MYSTICISM
		self.requiredskill = 580
		self.mana = 20
		self.mantra = 'Corp Por Ylem'
		self.missile = [0x11B6, 7, 0, False]
		self.sound = 0x64B
		self.delay = 600

	def damage(self, char, target):
		myst, other = calcMysticPower(char)
		dmg = int(28 + (myst * 0.25) + (other * 0.25) + random.randint(1, 10))
		target.damage(dmg, 0, 0, 0, 0, 0)
		target.soundeffect(0x220)
		target.effect(0x36BD, 20, 10)
		target.paralyze(2000)

# 689: Spell Plague (Circle 7)
class SpellPlague(CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 7)
		self.skill = MYSTICISM
		self.requiredskill = 700
		self.mana = 40
		self.mantra = 'Vas Rel Jux Ort'
		self.validtarget = TARGET_CHAR
		self.harmful = 1

	def effect(self, char, target, mode, args, item):
		target.soundeffect(0x658)
		target.effect(0x3728, 10, 20)
		target.settag('spell_plague', 3) # 3 strikes
		target.settag('spell_plague_caster', char.serial)
		target.damage(15, 0, 0, 0, 0, 0)
		if char.socket:
			char.socket.sysmessage("Spell plague grips your foe's life force!")

# 690: Hail Storm (Circle 7)
class HailStorm(Spell):
	def __init__(self):
		Spell.__init__(self, 7)
		self.skill = MYSTICISM
		self.requiredskill = 700
		self.mana = 40
		self.mantra = 'Kal Des Ylem'

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False
		char.soundeffect(0x64F)
		char.effect(0x375A, 10, 30)
		targets = wolfpack.chars(char.pos.x, char.pos.y, char.pos.map, 6)
		myst, other = calcMysticPower(char)
		dmg = int(24 + (myst * 0.2) + (other * 0.2))
		for t in targets:
			if t.serial != char.serial and mayAreaHarm(char, t):
				t.effect(0x374A, 10, 15)
				t.soundeffect(0x64F)
				t.damage(dmg, 0, 0, 0, 0, 0)
		return True

# 691: Nether Cyclone (Circle 8)
class NetherCyclone(Spell):
	def __init__(self):
		Spell.__init__(self, 8)
		self.skill = MYSTICISM
		self.requiredskill = 830
		self.mana = 50
		self.mantra = 'Grav Hur'

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False
		char.soundeffect(0x64E)
		char.effect(0x37CC, 1, 30)
		targets = wolfpack.chars(char.pos.x, char.pos.y, char.pos.map, 8)
		myst, other = calcMysticPower(char)
		dmg = int(35 + (myst * 0.3) + (other * 0.3))
		for t in targets:
			if t.serial != char.serial and mayAreaHarm(char, t):
				t.effect(0x3709, 10, 30)
				t.soundeffect(0x64E)
				t.damage(dmg, 0, 0, 0, 0, 0)
				t.stamina = max(0, t.stamina - 20)
				t.mana = max(0, t.mana - 20)
		return True

# 692: Rising Colossus (Circle 8)
class RisingColossus(Spell):
	def __init__(self):
		Spell.__init__(self, 8)
		self.skill = MYSTICISM
		self.requiredskill = 830
		self.mana = 50
		self.mantra = 'Kal Vas Xen Corp Ylem'

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False
		char.soundeffect(0x651)
		char.effect(0x3709, 10, 30)
		pos = wolfpack.coord(char.pos.x + 1, char.pos.y, char.pos.z, char.pos.map)
		colossus = wolfpack.addnpc('rising_colossus', pos)
		if not colossus:
			colossus = wolfpack.addnpc('earth_elemental', pos)
		if colossus:
			colossus.summoned = 1
			colossus.wandermode = 5
			colossus.leader = char
			duration = min(60000, int(15000 + char.skill[MYSTICISM] * 35))
			colossus.addtimer(duration, summon_expire, [colossus.serial], 0, 0)
		if char.socket:
			char.socket.sysmessage("A mighty colossus emerges from the earth to fight for you!")
		return True

def summon_expire(char, args):
	colossus = wolfpack.findchar(args[0])
	if colossus:
		colossus.soundeffect(0x1FA)
		colossus.effect(0x3728, 10, 15)
		colossus.delete()

def onLoad():
	NetherBolt().register(677)
	HealingStone().register(678)
	PurgeMagic().register(679)
	Enchant().register(680)
	Sleep().register(681)
	EagleStrike().register(682)
	AnimatedWeapon().register(683)
	StoneForm().register(684)
	SpellTrigger().register(685)
	MassSleep().register(686)
	CleansingWinds().register(687)
	Bombard().register(688)
	SpellPlague().register(689)
	HailStorm().register(690)
	NetherCyclone().register(691)
	RisingColossus().register(692)
