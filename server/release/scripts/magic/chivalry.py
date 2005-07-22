
# http://uo.stratics.com/content/guides/paladinabilities.shtml
# todo: required chivalry skill (differs from spell to spell)

from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell
from wolfpack.consts import *
from magic.utilities import *
from system import poison
import random
import math
from wolfpack.utilities import energydamage, mayAreaHarm

def ComputePowerValue( char, div ):
	if not char:
		return False

	v = math.sqrt( char.karma + 20000 + (char.skill[CHIVALRY] * 10) )
	return v / div

class CleanseByFire(CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 5)
		self.skill = CHIVALRY
		self.requiredskill = 50
		self.damageskill = FOCUS
		self.mana = 10
		self.tithingpoints = 10
		self.mantra = 'Expor Flamus'

	def effect(self, char, target, mode, args, item):
		if target.poison == -1:
			char.socket.clilocmessage(1060176) # That creature is not poisoned!
		else:
			# Cures the target of poisons, but causes the caster to be burned by fire damage for 13-55 hit points.
			# The amount of fire damage is lessened if the caster has high Karma.
			chanceToCure = 10000 + char.skill[CHIVALRY] * 75 - ((target.poison + 1) * 2000);
			chanceToCure /= 100
			if chanceToCure > random.randint(0, 101):
				poison.cure( target )
				if not char == target and char.socket:
					char.socket.clilocmessage(1010058) # You have cured the target of all poisons!
				if target.socket:
					target.socket.clilocmessage( 1010059 ) # You have been cured of all poisons.
			else:
				char.socket.clilocmessage(1010060) # You have failed to cure your target!
				
			target.soundeffect( 0x1e0 );
			target.effect(0x373a, 1, 15)
			target.effect(0x374b, 5, 10) # should move from feet to head...
			char.soundeffect( 0x208 );
			char.effect(0x3709, 1, 30)

			damage = 50 - ComputePowerValue( char, 4 )

			if damage < 13:
				damage = 13
			elif damage > 55:
				damage = 55

			char.damage(1, damage, char)

class CloseWounds(CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 7)
		self.skill = CHIVALRY
		self.requiredskill = 0
		self.damageskill = FOCUS
		self.mana = 10
		self.tithingpoints = 10
		self.mantra = 'Obsu Vulni'

	def affectchar(self, char, mode, target, args=[]):
		if target.poison != -1 or ismortallywounded(target):
			if target == char:
				char.message(1005000)
			else:
				char.message(1010398)
			return False
		return True

	def effect(self, char, target, mode, args, item):
		if not char.canreach(target, 2):
			char.socket.clilocmessage( 1060178 ) # You are too far away to perform that action!

		elif target.dead:
			char.socket.clilocmessage( 1061654 ) # You cannot heal that which is not alive.
		elif target.hitpoints >= target.maxhitpoints:
			char.socket.clilocmessage( 500955 ) # That being is not damaged!
		else:
			# Heals the target for 7 to 39 points of damage.
			# The caster's Karma affects the amount of damage healed.
			toHeal = ComputePowerValue( char, 6 )
			if toHeal < 7:
				toHeal = 7
			elif toHeal > 39:
				toHeal = 39
			if (target.hitpoints + toHeal) > target.maxhitpoints:
				toHeal = target.maxhitpoints - target.hitpoints
			target.hitpoints += toHeal;
			if target.socket:
				target.socket.clilocmessage( 1060203, str(toHeal) ) # You have had ~1_HEALED_AMOUNT~ hit points of damage healed.
			target.soundeffect( 0x202 );
			target.effect( 0x376A, 1, 62 )
			target.effect( 0x3779, 1, 46 )

class ConsecrateWeapon(Spell):
	def __init__(self):
		Spell.__init__(self, 3)
		self.skill = CHIVALRY
		self.requiredskill = 150
		self.damageskill = FOCUS
		self.mana = 10
		self.tithingpoints = 10
		self.mantra = 'Consecrus Arma'
	def cast(self, char, mode, args=[], target=None, item=None):
		char.socket.sysmessage( tr("Not yet implemented.") )

class DispelEvil(Spell):
	def __init__(self):
		Spell.__init__(self, 2)
		self.skill = CHIVALRY
		self.requiredskill = 350
		self.damageskill = FOCUS
		self.mana = 10
		self.tithingpoints = 10
		self.mantra = 'Dispiro Malas'
	def cast(self, char, mode, args=[], target=None, item=None):
		char.socket.sysmessage( tr("Not yet implemented.") )

class DivineFury(Spell):
	def __init__(self):
		Spell.__init__(self, 5)
		self.skill = CHIVALRY
		self.requiredskill = 250
		self.damageskill = FOCUS
		self.mana = 15
		self.tithingpoints = 10
		self.mantra = 'Divinum Furis'
	def cast(self, char, mode, args=[], target=None, item=None):
		char.socket.sysmessage( tr("Not yet implemented.") )

class EnemyOfOne(Spell):
	def __init__(self):
		Spell.__init__(self, 3)
		self.skill = CHIVALRY
		self.requiredskill = 450
		self.damageskill = FOCUS
		self.mana = 20
		self.tithingpoints = 10
		self.mantra = 'Forul Solum'
	def cast(self, char, mode, args=[], target=None, item=None):
		char.socket.sysmessage( tr("Not yet implemented.") )

class HolyLight(Spell):
	def __init__(self):
		Spell.__init__(self, 5)
		self.skill = CHIVALRY
		self.requiredskill = 550
		self.damageskill = FOCUS
		self.mana = 10
		self.tithingpoints = 10
		self.mantra = 'Augus Luminos'
		self.harmful = 1

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return

		targets = []

		chars = wolfpack.chars(char.pos.x, char.pos.y, char.pos.map, 3)
		for target in chars:
			if not mayAreaHarm(char, target):
				continue

			if not char.canreach(target, 3):
				continue

			targets.append(target)

		char.soundeffect( 0x212 )
		char.soundeffect( 0x206 )
		char.effect(0x376A, 1, 29)
		char.effect(0x37C4, 1, 29)
		
		damage = ComputePowerValue( char, 10 ) + random.randint( 0, 3 )
		if damage < 8:
			damage = 8
		elif damage > 24:
			damage = 24

		for target in targets:
			energydamage(target, char, damage, 0, 0, 0, 0, 100)

class NobleSacrifice(Spell):
	def __init__(self):
		Spell.__init__(self, 7)
		self.skill = CHIVALRY
		self.requiredskill = 650
		self.damageskill = FOCUS
		self.mana = 20
		self.tithingpoints = 30
		self.mantra = 'Dium Prostra'

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return

		targets = []

		chars = wolfpack.chars(char.pos.x, char.pos.y, char.pos.map, 3)
		for target in chars:
			if target.npc:
				continue
			
			if not char == target:
				targets.append(target)

		char.soundeffect( 0x244 )
		char.effect( 0x3709, 1, 30 )
		char.effect( 0x376A, 1, 30 )

		# Attempts to Resurrect, Cure and Heal all targets in a radius around the caster.
		# If any target is successfully assisted, the Paladin's current
		# Hit Points, Mana and Stamina are set to 1.
		# Amount of damage healed is affected by the Caster's Karma, from 8 to 24 hit points.

		sacrifice = False

		# TODO: Is there really a resurrection chance?
		resChance = 0.1 + (0.9 * (double(char.karma / 10000)))

		for target in targets:
			if char.dead:
				if char.region and char.region.name == "Khaldun":
					char.socket.clilocmessage( 1010395 ) # The veil of death in this area is too strong and resists thy efforts to restore life.
				elif resChance > Utility.RandomDouble():
					target.effect( 0x375A, 1, 15 )
					# we need a resurrect gump here...
					#target.SendGump( new ResurrectGump( m, Caster ) )
					sacrifice = True
			else:
				sendEffect = False
				if target.poison:
					poison.cure( target )
					if not target == char:
						char.socket.clilocmessage( 1010058 ) # You have cured the target of all poisons!
					if target.socket:
						target.socket.clilocmessage( 1010059 ) # You have been cured of all poisons.
					sendEffect = True
					sacrifice = True
				if target.hitpoints < target.maxhitpoints:
					toHeal = ComputePowerValue( 10 ) + random.randint( 0, 3 )
					if toHeal < 8:
						toHeal = 8
					elif toHeal > 24:
						toHeal = 24

					if (char.hitpoints + toHeal) > char.maxhitpoints:
						char.hitpoints = char.maxhitpoints
					else:
						char.hitpoints += toHeal
					char.updatehealth()
					sendEffect = true

				# negative stat modifiers should be removed from the target

				if target.frozen:
					target.frozen = False
					target.resendtooltip()

				# ToDo:
				# remove from char:
				# EvilOmenSpell
				# StrangleSpell
				# CorpseSkinSpell

				if sendEffect:
					target.effect( 0x375A, 1, 15 )
					sacrifice = True
		if sacrifice:
			char.soundeffect( 0x423 );
			char.hitpoints = 1
			char.stamina = 1
			char.mana = 1
			char.updatestats()

class RemoveCurse(Spell):
	def __init__(self):
		Spell.__init__(self, 7)
		self.skill = CHIVALRY
		self.requiredskill = 50
		self.damageskill = FOCUS
		self.mana = 20
		self.tithingpoints = 10
		self.mantra = 'Extermo Vomica'
	def cast(self, char, mode, args=[], target=None, item=None):
		char.socket.sysmessage( tr("Not yet implemented.") )

class SacredJourney(Spell):
	def __init__(self):
		Spell.__init__(self, 7)
		self.skill = CHIVALRY
		self.requiredskill = 150
		self.damageskill = FOCUS
		self.mana = 10
		self.tithingpoints = 15
		self.mantra = 'Sanctum Viatas'
	def cast(self, char, mode, args=[], target=None, item=None):
		char.socket.sysmessage( tr("Not yet implemented.") )


def onLoad():
	CleanseByFire().register(201)
	CloseWounds().register(202)
	ConsecrateWeapon().register(203)
	DispelEvil().register(204)
	DivineFury().register(205)
	EnemyOfOne().register(206)
	HolyLight().register(207)
	NobleSacrifice().register(208)
	RemoveCurse().register(209)
	SacredJourney().register(210)
