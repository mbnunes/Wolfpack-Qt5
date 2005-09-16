#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Radiant                        #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Necromancy basic stuff                                        #
#===============================================================#

# http://uo.stratics.com/content/guides/necromancy1.shtml

from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell
from wolfpack.consts import *
from magic.utilities import *

class AnimateDead(Spell):
	def __init__(self):
		Spell.__init__(self, 1)
		self.skill = NECROMANCY
		self.requiredskill = 40
		self.damageskill = SPIRITSPEAK
		self.mana = 23
		self.reagents = {REAGENT_GRAVEDUST: 1, REAGENT_DAEMONBLOOD: 1}
		self.mantra = 'Uus Corp'

	def target(self, char, mode, targettype, target, args, item):
		char.turnto(target)
		if target.id != 0x2006:
			char.socket.clilocmessage(1042600)
			return

		if char.player and char.controlslots >= 5:
			char.socket.clilocmessage(1049645)
			return

		if target.owner:
			char.socket.sysmessage('You may not use this on players!')
			return

		if not self.consumerequirements(char, mode, args, target, item):
			return

		char.socket.sysmessage('Not yet implented')

class BloodOath(CharEffectSpell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = NECROMANCY
		self.requiredskill = 20
		self.damageskill = SPIRITSPEAK
		self.mana = 13
		self.reagents = {REAGENT_DAEMONBLOOD: 1}
		self.mantra = 'In Jux Mani Xen'

	def affectchar(self, char, mode, target, args=[]):
		if target == char:
			char.socket.clilocmessage( 1060508 ) # You can't curse that.
			return False
		elif char.hasscript('magic.bloodoath'):
			char.socket.clilocmessage( 1061607 ) # You are already bonded in a Blood Oath.
			return False
		elif target.hasscript('magic.bloodoath'):
			if target.player:
				char.socket.clilocmessage( 1061608 ) # That player is already bonded in a Blood Oath.
			else:
				char.socket.clilocmessage( 1061609 ) # That creature is already bonded in a Blood Oath.
			return False
		return True

	def effect(self, char, target, mode, args, item):
		char.soundeffect( 0x175 )

		char.effect( 0x375A, 1, 17 )
		char.effect( 0x3728, 1, 13 )

		target.effect( 0x375A, 1, 17 )
		target.effect( 0x3728, 1, 13 )

		duration = (((char.skill[self.damageskill] - target.skill[MAGICRESISTANCE]) / 8) + 8) * 1000

		char.addscript('magic.bloodoath')
		target.addscript('magic.bloodoath')
		char.settag('bloodoath_caster', target.serial)
		target.settag('bloodoath', char.serial)
		char.addtimer( duration, expire_bloodoath, [target.serial], True, False, 'BLOODOATH', magic.necromancy.dispel_bloodoath )

def dispel_bloodoath(char, args, source, dispelargs):
	expire_bloodoath(char, dispelargs)

def expire_bloodoath(char, args):
	victim = wolfpack.findchar(args[0])
	if char:
		if char.hasscript('magic.bloodoath'):
			char.removescript('magic.bloodoath')
		if char.hastag('bloodoath_caster'):
			char.deltag('bloodoath_caster')	
		if char.socket:
			char.socket.clilocmessage( 1061620 ) # Your Blood Oath has been broken.
	if victim:
		if victim.hasscript('magic.bloodoath'):
			victim.removescript('magic.bloodoath')
		if victim.hastag('bloodoath'):
			victim.deltag('bloodoath')
		if victim.socket:
			victim.socket.clilocmessage( 1061620 ) # Your Blood Oath has been broken.
	return

class CorpseSkin(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = NECROMANCY
		self.requiredskill = 0
		self.damageskill = SPIRITSPEAK
		self.mana = 7
		self.reagents = {REAGENT_GRAVEDUST: 1, REAGENT_BATWING: 1}
		self.mantra = 'In Agle Corp Ylem'

class CurseWeapon(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = NECROMANCY
		self.requiredskill = 0
		self.damageskill = SPIRITSPEAK
		self.mana = 0
		self.reagents = {REAGENT_PIGIRON: 1}
		self.mantra = 'An Sanct Gra Char'

class EvilOmen(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = NECROMANCY
		self.requiredskill = 20
		self.damageskill = SPIRITSPEAK
		self.mana = 11
		self.reagents = {REAGENT_BATWING: 1, REAGENT_NOXCRYSTAL: 1}
		self.mantra = 'Pas Tym An Sanct'

class HorrificBeast(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = NECROMANCY
		self.requiredskill = 40
		self.damageskill = SPIRITSPEAK
		self.mana = 11
		self.reagents = {REAGENT_BATWING: 1, REAGENT_DAEMONBLOOD: 1}
		self.mantra = 'Rel Xen Vas Bal'

class LichForm(Spell):
	def __init__(self):
		Spell.__init__(self, 1)
		self.skill = NECROMANCY
		self.requiredskill = 70
		self.damageskill = SPIRITSPEAK
		self.mana = 23
		self.reagents = {REAGENT_GRAVEDUST: 1, REAGENT_DAEMONBLOOD: 1, REAGENT_NOXCRYSTAL: 1}
		self.mantra = 'Rel Xen Corp Ort'

class MindRot(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = NECROMANCY
		self.requiredskill = 30
		self.damageskill = SPIRITSPEAK
		self.mana = 17
		self.reagents = {REAGENT_BATWING: 1, REAGENT_DAEMONBLOOD: 1, REAGENT_PIGIRON: 1}
		self.mantra = 'Wis An Ben'

class PainSpike(CharEffectSpell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = NECROMANCY
		self.requiredskill = 20
		self.damageskill = SPIRITSPEAK
		self.mana = 5
		self.reagents = {REAGENT_PIGIRON: 1, REAGENT_GRAVEDUST: 1}
		self.mantra = 'In Sar'

	def effect(self, char, target, mode, args, item):
		target.effect( 0x37C4, 1, 8 )
		target.effect( 0x37C4, 1, 8 )
		target.soundeffect( 0x210 )

		damage = ((char.skill[self.damageskill] - target.skill[MAGICRESISTANCE]) / 10) + 30
		if damage < 1:
			damage = 1
		if target.hastag('painspike'):
			damage /= 10
		else:
			target.addtimer(10000, painspike_expire, [damage], True)
		target.settag('painspike', 0)
		target.damage(DAMAGE_MAGICAL, damage, char)

def painspike_expire(char, args):
	toRestore = args[0]
	if char.hastag('painspike'):
		char.deltag('painspike')
	if not char.dead:
		char.hitpoints += toRestore
		char.updatehealth()
	return

class PoisonStrike(Spell):
	def __init__(self):
		Spell.__init__(self, 1)
		self.skill = NECROMANCY
		self.requiredskill = 50
		self.damageskill = SPIRITSPEAK
		self.mana = 17
		self.reagents = {REAGENT_NOXCRYSTAL: 1}
		self.mantra = 'In Vas Nox'

class Strangle(Spell):
	def __init__(self):
		Spell.__init__(self, 1)
		self.skill = NECROMANCY
		self.requiredskill = 65
		self.damageskill = SPIRITSPEAK
		self.mana = 29
		self.reagents = {REAGENT_NOXCRYSTAL: 1, REAGENT_DAEMONBLOOD: 1}
		self.mantra = 'In Bal Nox'

class SummonFamiliar(Spell):
	def __init__(self):
		Spell.__init__(self, 1)
		self.skill = NECROMANCY
		self.requiredskill = 30
		self.damageskill = SPIRITSPEAK
		self.mana = 17
		self.reagents = {REAGENT_BATWING: 1, REAGENT_GRAVEDUST: 1, REAGENT_DAEMONBLOOD: 1}
		self.mantra = 'Kal Xen Bal'

class VampiricEmbrace(Spell):
	def __init__(self):
		Spell.__init__(self, 1)
		self.skill = NECROMANCY
		self.requiredskill = 99
		self.damageskill = SPIRITSPEAK
		self.mana = 23
		self.reagents = {REAGENT_BATWING: 1, REAGENT_NOXCRYSTAL: 1, REAGENT_PIGIRON: 1}
		self.mantra = 'Rel Xen An Sanct'

class VengefulSpirit(Spell):
	def __init__(self):
		Spell.__init__(self, 1)
		self.skill = NECROMANCY
		self.requiredskill = 80
		self.damageskill = SPIRITSPEAK
		self.mana = 41
		self.reagents = {REAGENT_BATWING: 1, REAGENT_GRAVEDUST: 1, REAGENT_PIGIRON: 1}
		self.mantra = 'Kal Xen Bal Beh'

class Wither(Spell):
	def __init__(self):
		Spell.__init__(self, 1)
		self.skill = NECROMANCY
		self.requiredskill = 60
		self.damageskill = SPIRITSPEAK
		self.mana = 23
		self.reagents = {REAGENT_GRAVEDUST: 1, REAGENT_NOXCRYSTAL: 1, REAGENT_PIGIRON: 1}
		self.mantra = 'Kal Vas An Flam'

	def cast(self, char, mode, args=[], target=None, item=None):
		targets = []

		chars = wolfpack.chars(char.pos.x, char.pos.y, char.pos.map, 5)
		for target in chars:
			if not mayAreaHarm(char, target):
				continue

			elif not char.canreach(target, 3):
				continue

		char.soundeffect( 0x1FB )
		char.soundeffect( 0x10B )
		char.effect( 0x37CC, 1, 40 )

		for target in targets:
			if target == char:
				continue
			target.effect( 0x374A, 1, 15 )
			damage = random.randint( 30, 35 )
			damage *= (300 + (target.karma / 100) + (char.skill[self.damageskill] * 10))
			damage /= 1000

			energydamage(target, char, damage, 0, 0, 100, 0, 0, 0, DAMAGE_COLD)

class WraithForm(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = NECROMANCY
		self.requiredskill = 20
		self.damageskill = SPIRITSPEAK
		self.mana = 17
		self.reagents = {REAGENT_NOXCRYSTAL: 1, REAGENT_PIGIRON: 1}
		self.mantra = 'Rel Xen Um'

def onLoad():
	AnimateDead().register(101)
	BloodOath().register(102)
	CorpseSkin().register(103)
	CurseWeapon().register(104)
	EvilOmen().register(105)
	HorrificBeast().register(106)
	LichForm().register(107)
	MindRot().register(108)
	PainSpike().register(109)
	PoisonStrike().register(110)
	Strangle().register(111)
	SummonFamiliar().register(112)
	VampiricEmbrace().register(113)
	VengefulSpirit().register(114)
	Wither().register(115)
	WraithForm().register(116)
