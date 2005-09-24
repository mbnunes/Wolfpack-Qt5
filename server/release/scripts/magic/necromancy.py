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
from wolfpack.utilities import changeResistance
import magic.corpseskin
import wolfpack.time

class AnimateDead(Spell):
	def __init__(self):
		Spell.__init__(self, 4)
		self.skill = NECROMANCY
		self.requiredskill = 40
		self.damageskill = SPIRITSPEAK
		self.mana = 23
		self.reagents = {REAGENT_GRAVEDUST: 1, REAGENT_DAEMONBLOOD: 1}
		self.mantra = 'Uus Corp'
		self.validtarget = TARGET_ITEM

	def precast(self, char, mode, args, target, item):
		if Spell.precast(self, char, mode, args):
			char.socket.clilocmessage( 1061083 ) # Animate what corpse?

	def target(self, char, mode, targettype, target, args, item):
		if not target.corpse:
			char.socket.clilocmessage( 1061084 ) # You cannot animate that.
			return False

		char.turnto(target)
		if target.id != 0x2006:
			char.socket.clilocmessage(1042600)
			return

		if char.player and char.controlslots >= 5:
			char.socket.clilocmessage(1049645)
			return

		if not self.consumerequirements(char, mode, args, target, item):
			return

		char.socket.sysmessage('Not yet implented')

class BloodOath(CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 4)
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

		char.effect( 0x375A, 1, 17, 33, 7 )
		char.effect( 0x3728, 1, 13, 33, 7 )

		target.effect( 0x375A, 1, 17, 33, 7 )
		target.effect( 0x3728, 1, 13, 33, 7 )

		duration = (((char.skill[self.damageskill] - target.skill[MAGICRESISTANCE]) / 8) + 8) * 1000

		char.addscript('magic.bloodoath')
		target.addscript('magic.bloodoath')
		char.settag('bloodoath_caster', target.serial)
		target.settag('bloodoath', char.serial)
		char.addtimer( duration, magic.bloodoath.expire, [target.serial], True, False, 'BLOODOATH', magic.bloodoath.dispel )

class CorpseSkin(CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 4)
		self.skill = NECROMANCY
		self.requiredskill = 0
		self.damageskill = SPIRITSPEAK
		self.mana = 7
		self.reagents = {REAGENT_GRAVEDUST: 1, REAGENT_BATWING: 1}
		self.mantra = 'In Agle Corp Ylem'

	def effect(self, char, target, mode, args, item):
		if target.hasscript('magic.corpseskin'):
			target.dispel(char, True, 'CORPSESKIN', [1])
		elif target.socket:
			target.socket.clilocmessage( 1061689 ) # Your skin turns dry and corpselike.
		target.effect( 0x373A, 1, 15, 67, 7 )
		target.soundeffect( 0x1BB )

		ss = char.skill[self.damageskill]
		mr = 0
		if not target == char:
			mr = target.skill[MAGICRESISTANCE]

		duration = ( ((ss - mr) / 2.5) + 40.0 ) * 100

		#changeResistance( char, 'res_fire', -10 )
		#changeResistance( char, 'res_poison', -10 )
		#changeResistance( char, 'res_cold', 10 )
		#changeResistance( char, 'res_physical', 10 )
		target.addscript('magic.corpseskin')
		#target.updatestats()

		target.addtimer( duration, magic.corpseskin.expire, [], True, False, 'CORPSESKIN', magic.corpseskin.dispel )

class CurseWeapon(Spell):
	def __init__(self):
		Spell.__init__(self, 1)
		self.skill = NECROMANCY
		self.requiredskill = 0
		self.damageskill = SPIRITSPEAK
		self.mana = 0
		self.reagents = {REAGENT_PIGIRON: 1}
		self.mantra = 'An Sanct Gra Char'

	def cast(self, char, mode, args=[], target=None, item=None):
		weapon = char.getweapon()
		if not weapon:
			char.socket.clilocmessage( 501078 ) # You must be holding a weapon.
			return False

		char.soundeffect( 0x387 )
		char.effect( 0x3779, 1, 15, 32, 2 )
		char.effect( 0x37B9, 1, 14, 32, 5 )
		duration = ( (char.skill[self.damageskill] / 7.5) + 1.0 ) * 100
		weapon.addscript('magic.curseweapon')
		weapon.addtimer( duration, magic.curseweapon.expire, [])

class EvilOmen(CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 1)
		self.skill = NECROMANCY
		self.requiredskill = 20
		self.damageskill = SPIRITSPEAK
		self.mana = 11
		self.reagents = {REAGENT_BATWING: 1, REAGENT_NOXCRYSTAL: 1}
		self.mantra = 'Pas Tym An Sanct'

	def effect(self, char, target, mode, args, item):
		target.soundeffect( 0xFC )
		target.effect( 0x3728, 1, 13, 1150, 7 )
		target.effect( 0x3779, 1, 15, 67, 7 )

		target.addscript('magic.evilomen')
		if target.skill[MAGICRESISTANCE] > 500:
			target.settag('magicresistance', target.skill[MAGICRESISTANCE])
			target.skill[MAGICRESISTANCE] = 500
		duration = ( 3.5 + (3.5 * (char.skill[self.damageskill] / 100.0)) ) * 100
		target.addtimer( duration, magic.evilomen.expire, [], True, False, 'CORPSESKIN', magic.evilomen.dispel )

class HorrificBeast(Spell):
	def __init__(self):
		Spell.__init__(self, 6)
		self.skill = NECROMANCY
		self.requiredskill = 40
		self.damageskill = SPIRITSPEAK
		self.mana = 11
		self.reagents = {REAGENT_BATWING: 1, REAGENT_DAEMONBLOOD: 1}
		self.mantra = 'Rel Xen Vas Bal'

	def cast(self, char, mode, args=[], target=None, item=None):
		#char.id = 746
		#char.update()
		char.soundeffect( 0x165 )
		char.effect( 0x3728, 1, 13, 92, 3 )

class LichForm(Spell):
	def __init__(self):
		Spell.__init__(self, 6)
		self.skill = NECROMANCY
		self.requiredskill = 70
		self.damageskill = SPIRITSPEAK
		self.mana = 23
		self.reagents = {REAGENT_GRAVEDUST: 1, REAGENT_DAEMONBLOOD: 1, REAGENT_NOXCRYSTAL: 1}
		self.mantra = 'Rel Xen Corp Ort'

class MindRot(Spell):
	def __init__(self):
		Spell.__init__(self, 4)
		self.skill = NECROMANCY
		self.requiredskill = 30
		self.damageskill = SPIRITSPEAK
		self.mana = 17
		self.reagents = {REAGENT_BATWING: 1, REAGENT_DAEMONBLOOD: 1, REAGENT_PIGIRON: 1}
		self.mantra = 'Wis An Ben'

class PainSpike(CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 2)
		self.skill = NECROMANCY
		self.requiredskill = 20
		self.damageskill = SPIRITSPEAK
		self.mana = 5
		self.reagents = {REAGENT_PIGIRON: 1, REAGENT_GRAVEDUST: 1}
		self.mantra = 'In Sar'

	def effect(self, char, target, mode, args, item):
		target.effect( 0x37C4, 1, 8, 39, 3 )
		target.effect( 0x37C4, 1, 8, 39, 4 )
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

class PoisonStrike(CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 4)
		self.skill = NECROMANCY
		self.requiredskill = 50
		self.damageskill = SPIRITSPEAK
		self.mana = 17
		self.reagents = {REAGENT_NOXCRYSTAL: 1}
		self.mantra = 'In Vas Nox'

	def affectchar(self, char, mode, target, args=[]):
		return True

	def effect(self, char, target, mode, args, item):
		target.effect( 0x36B0, 1, 14, 63 )
		target.soundeffect( 0x229 )

		damage = random.randint( 36, 40 ) * ((300 + (char.skill[self.damageskill] * 9)) / 1000)

		chars = wolfpack.chars(target.pos.x, target.pos.y, target.pos.map, 2)
		for aim in chars:
			if target.distanceto(aim) == 0:
				num = 1
			elif target.distanceto(aim) == 0:
				num = 1
			elif target.distanceto(aim) == 0:
				num = 2
			else:
				num = 3
			energydamage(aim, char, damage/num, 0, 0, 0, 100, 0, 0, DAMAGE_POISON)

class Strangle(Spell):
	def __init__(self):
		Spell.__init__(self, 6)
		self.skill = NECROMANCY
		self.requiredskill = 65
		self.damageskill = SPIRITSPEAK
		self.mana = 29
		self.reagents = {REAGENT_NOXCRYSTAL: 1, REAGENT_DAEMONBLOOD: 1}
		self.mantra = 'In Bal Nox'

class SummonFamiliar(Spell):
	def __init__(self):
		Spell.__init__(self, 6)
		self.skill = NECROMANCY
		self.requiredskill = 30
		self.damageskill = SPIRITSPEAK
		self.mana = 17
		self.reagents = {REAGENT_BATWING: 1, REAGENT_GRAVEDUST: 1, REAGENT_DAEMONBLOOD: 1}
		self.mantra = 'Kal Xen Bal'

class VampiricEmbrace(Spell):
	def __init__(self):
		Spell.__init__(self, 6)
		self.skill = NECROMANCY
		self.requiredskill = 99
		self.damageskill = SPIRITSPEAK
		self.mana = 23
		self.reagents = {REAGENT_BATWING: 1, REAGENT_NOXCRYSTAL: 1, REAGENT_PIGIRON: 1}
		self.mantra = 'Rel Xen An Sanct'

	def cast(self, char, mode, args=[], target=None, item=None):
		char.pos.effect( 0x373A, 1, 17, 1108, 7 )
		char.pos.effect( 0x376A, 1, 22, 67, 7)
		char.pos.soundeffect( 0x4B1 )
		# reverse the transformation
		if char.hasscript('magic.vampiricembrace'):
			char.removescript('magic.vampiricembrace')
			char.id = char.orgid
			char.skin = char.orgskin
			# reverse increased stamina regeneration
			if char.hastag('regenstamina'):
				if (char.gettag('regenstamina') - 15) <= 0:
					char.deltag('regenstamina')
				else:
					char.settag('regenstamina', char.gettag('regenstamina') - 15)
			# reverse increased mana regeneration
			if char.hastag('regenmana'):
				if (char.gettag('regenmana') - 3) <= 0:
					char.deltag('regenmana')
				else:
					char.settag('regenmana', char.gettag('regenmana') - 3)
			# reverse decreased fire resistance
			if char.hastag('vampemb_res_fire'):
				if char.hastag('res_fire'):
					fire_res = char.gettag('res_fire')
					char.settag('res_fire', fire_res + char.gettag('vampemb_res_fire'))
				else:
					char.settag('res_fire', char.gettag('vampemb_res_fire'))
		# do the transformation
		else:
			if char.gender:
				char.id = 745
			else:
				char.id = 744
			char.skin = 0x847e
			# decrease fire resistance
			if char.hastag('res_fire'):
				fire_res = char.gettag('res_fire')
				char.settag('vampemb_res_fire',fire_res)
				if fire_res - 25 <= 0:
					char.deltag('res_fire')
				else:
					char.settag('res_fire', fire_res - 25)

			char.addscript('magic.vampiricembrace')
			# increase stamina regeneration
			if char.hastag('regenstamina'):
				char.settag('regenstamina', char.gettag('regenstamina') + 15)
			else:
				char.settag('regenstamina', 15)
			# increase mana regeneration
			if char.hastag('regenmana'):
				char.settag('regenmana', char.gettag('regenmana') + 3)
			else:
				char.settag('regenmana', 3)

class VengefulSpirit(CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 6)
		self.skill = NECROMANCY
		self.requiredskill = 80
		self.damageskill = SPIRITSPEAK
		self.mana = 41
		self.reagents = {REAGENT_BATWING: 1, REAGENT_GRAVEDUST: 1, REAGENT_PIGIRON: 1}
		self.mantra = 'Kal Xen Bal Beh'

	def affectchar(self, char, mode, target, args=[]):
		if (len(char.followers) + 3) > char.maxcontrolslots:
			char.socket.clilocmessage(1049645) # You have too many followers to summon that creature.
			return False
		elif target == char:
			char.socket.clilocmessage( 1061832 ) # You cannot exact vengeance on yourself.
			return False
		return True

	def effect(self, char, target, mode, args, item):
		duration = ( ((char.skill[self.damageskill] * 80) / 120) + 10 ) * 100

		revenant = wolfpack.addnpc('summoned_revenant', target.pos)
		revenant.summoned = True
		revenant.owner = char
		revenant.summontime = wolfpack.time.currenttime() + duration
		revenant.fight(target)
		revenant.addscript('magic.vengefulspirit')
		revenant.settag('target', target.serial)
		revenant.effect( 0x373A, 1, 15 )
		revenant.addtimer( 2000, magic.vengefulspirit.checktarget, [])
		return

class Wither(Spell):
	def __init__(self):
		Spell.__init__(self, 2)
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
		char.effect( 0x37CC, 1, 40, 97 )

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
		Spell.__init__(self, 6)
		self.skill = NECROMANCY
		self.requiredskill = 20
		self.damageskill = SPIRITSPEAK
		self.mana = 17
		self.reagents = {REAGENT_NOXCRYSTAL: 1, REAGENT_PIGIRON: 1}
		self.mantra = 'Rel Xen Um'

class Exorzism(Spell):
	def __init__(self):
		Spell.__init__(self, 6)
		self.skill = NECROMANCY
		self.requiredskill = 80
		self.damageskill = SPIRITSPEAK
		self.mana = 40
		self.reagents = {REAGENT_GRAVEDUST: 1, REAGENT_NOXCRYSTAL: 1}
		self.mantra = 'Ort Corp Grav'

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
	Exorzism().register(117)