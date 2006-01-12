#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Radiant                        #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Necromancy basic stuff                                        #
#===============================================================#

# http://uo.stratics.com/content/guides/necromancy1.shtml

from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell, TransformationSpell
from wolfpack.consts import *
from magic.utilities import *
import wolfpack.time

def transformed(char):
	if char.hasscript('magic.horrificbeast') or char.hasscript('magic.lichform') or char.hasscript('magic.wraithform') \
	or char.hasscript('magic.vampiricembrace'):
		return True
	return False

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

		target.addscript('magic.corpseskin') # Resistance mod is done through this script!
		target.updatestats()

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
		weapon.settag('cursed', 1)
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
		char.dispel(char, True, 'EVILOMEN')
		target.soundeffect( 0xFC )
		target.effect( 0x3728, 1, 13, 1150, 7 )
		target.effect( 0x3779, 1, 15, 67, 7 )

		target.addscript('magic.evilomen')
		if target.skill[MAGICRESISTANCE] > 500:
			target.settag('magicresistance', target.skill[MAGICRESISTANCE])
			target.skill[MAGICRESISTANCE] = 500
		duration = ( 3.5 + (3.5 * (char.skill[self.damageskill] / 10.0)) ) * 100
		target.addtimer( duration, magic.evilomen.expire, [], True, False, 'EVILOMEN', magic.evilomen.dispel )

class HorrificBeast(TransformationSpell):
	def __init__(self):
		TransformationSpell.__init__(self, 6)
		self.skill = NECROMANCY
		self.requiredskill = 40
		self.damageskill = SPIRITSPEAK
		self.mana = 11
		self.reagents = {REAGENT_BATWING: 1, REAGENT_DAEMONBLOOD: 1}
		self.mantra = 'Rel Xen Vas Bal'

	#def precast(self, char, mode=0, args=[], target = None, item = None):	
	#	if polymorphed(char):
	##		return False
	#	return Spell.precast(self, char, mode, args, target, item)

	def cast(self, char, mode, args=[], target=None, item=None):
		char.soundeffect( 0x165 )
		char.effect( 0x3728, 1, 13, 92, 3 )
		if char.hasscript('magic.horrificbeast'):
			char.id = char.orgid
			char.removescript('magic.horrificbeast')
		else:
			char.id = 746
			char.addscript('magic.horrificbeast')
		char.update()

class LichForm(TransformationSpell):
	def __init__(self):
		TransformationSpell.__init__(self, 6)
		self.skill = NECROMANCY
		self.requiredskill = 70
		self.damageskill = SPIRITSPEAK
		self.mana = 23
		self.reagents = {REAGENT_GRAVEDUST: 1, REAGENT_DAEMONBLOOD: 1, REAGENT_NOXCRYSTAL: 1}
		self.mantra = 'Rel Xen Corp Ort'

	def cast(self, char, mode, args=[], target=None, item=None):
		char.soundeffect( 0x19c )
		char.effect( 0x3709, 1, 30, 1108, 6 )
		if char.hasscript('magic.lichform'):
			char.dispel(char, True, 'LICHFORM')
			char.id = char.orgid
			# reverse increased mana regeneration
			if char.hastag('regenmana'):
				if (char.gettag('regenmana') - 3) <= 0:
					char.deltag('regenmana')
				else:
					char.settag('regenmana', char.gettag('regenmana') - 3)

		else:
			char.id = 749
			# increase mana regeneration
			if char.hastag('regenmana'):
				char.settag('regenmana', char.gettag('regenmana') + 3)
			else:
				char.settag('regenmana', 3)
			char.addscript('magic.lichform')

			char.addtimer( 2500, magic.lichform.expire, [], True, False, 'LICHFORM', magic.lichform.dispel )
		char.update()

class MindRot(CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 4)
		self.skill = NECROMANCY
		self.requiredskill = 30
		self.damageskill = SPIRITSPEAK
		self.mana = 17
		self.reagents = {REAGENT_BATWING: 1, REAGENT_DAEMONBLOOD: 1, REAGENT_PIGIRON: 1}
		self.mantra = 'Wis An Ben'

	def affectchar(self, char, mode, target, args=[]):
		if target.hasscript('magic.mindrot'):
			char.socket.clilocmessage( 1005559 ) # This spell is already in effect.
			return False
		return True

	def effect(self, char, target, mode, args, item):
		target.soundeffect( 0x1fb )
		target.soundeffect( 0x258 );
		target.effect( 0x373A, 1, 17, 15, 4 )

		char.addscript('magic.mindrot')
		percent = 25
		if target.npc:
			percent = 100
		if char.hastag(''):
			char.settag('', char.gettag('') + percent)
		duration = ( ((char.skill[self.damageskill] - target.skill[MAGICRESISTANCE]) / 5.0) + 20.0 ) * 100
		target.addtimer( duration, magic.mindrot.expire, [percent], True, False, 'MINDROT', magic.mindrot.dispel )

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
		self.harmful = 1

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

class Strangle(CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 6)
		self.skill = NECROMANCY
		self.requiredskill = 65
		self.damageskill = SPIRITSPEAK
		self.mana = 29
		self.reagents = {REAGENT_NOXCRYSTAL: 1, REAGENT_DAEMONBLOOD: 1}
		self.mantra = 'In Bal Nox'

	def effect(self, char, target, mode, args, item):
		target.soundeffect( 0x22f )
		target.effect( 0x36CB, 1, 9, 67, 5 )
		target.effect( 0x374A, 1, 17, 1108, 4 )

		spiritLevel = char.skill[self.damageskill] / 100
		MinBaseDamage = spiritLevel - 2
		MaxBaseDamage = spiritLevel + 1
		HitDelay = 5
		NextHit = HitDelay * 1000
		Count = spiritLevel
		if Count < 4:
			Count = 4
		MaxCount = Count

		target.addtimer(100, magic.strangle.dodamage, [spiritLevel, MinBaseDamage, MaxBaseDamage, HitDelay, MaxCount, Count], \
		True, False, 'STRANGLE', magic.strangle.dispel )

class SummonFamiliar(Spell):
	def __init__(self):
		Spell.__init__(self, 6)
		self.skill = NECROMANCY
		self.requiredskill = 30
		self.damageskill = SPIRITSPEAK
		self.mana = 17
		self.reagents = {REAGENT_BATWING: 1, REAGENT_GRAVEDUST: 1, REAGENT_DAEMONBLOOD: 1}
		self.mantra = 'Kal Xen Bal'

class VampiricEmbrace(TransformationSpell):
	def __init__(self):
		TransformationSpell.__init__(self, 6)
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
		if char.controlslots + 3 > char.maxcontrolslots:
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
		self.harmful = 1

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

class WraithForm(TransformationSpell):
	def __init__(self):
		TransformationSpell.__init__(self, 6)
		self.skill = NECROMANCY
		self.requiredskill = 20
		self.damageskill = SPIRITSPEAK
		self.mana = 17
		self.reagents = {REAGENT_NOXCRYSTAL: 1, REAGENT_PIGIRON: 1}
		self.mantra = 'Rel Xen Um'

	def cast(self, char, mode, args=[], target=None, item=None):
		char.soundeffect( 0x17f )
		char.effect( 0x374a, 1, 15, 1108, 4 )
		if char.hasscript('magic.wraithform'):
			char.removescript('magic.wraithform')
			char.id = char.orgid
			char.skin = char.orgskin
		else:
			char.addscript('magic.wraithform')
			if char.gender: # female
				char.id = 747
				char.skin = 0
			else: # male
				char.id = 748
				char.skin = 0x4001
		char.updatestats()
		char.update()

class Exorzism(Spell):
	def __init__(self):
		Spell.__init__(self, 6)
		self.skill = NECROMANCY
		self.requiredskill = 1000
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
