
from magic import registerspell
from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell
from magic.utilities import *
import random
import wolfpack
from wolfpack.utilities import tobackpack, energydamage
from wolfpack import time
import math

class Dispel (CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 6)
		self.reagents = {REAGENT_GARLIC: 1, REAGENT_MANDRAKE: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'An Ort'
		self.casttime = 2750
		self.harmful = 1

	def affectchar(self, char, mode, target, args=[]):
		if not Spell.affectchar(self, char, mode, target):
			return 0

		# Dispelling ourself is forbidden
		# And we can only dispel summoned creatures
		if char == target or not target.npc or target.summontimer == 0:
			if char.socket:
				char.socket.clilocmessage(1005049)
			return 0
		return 1

	def effect(self, char, target):
		# Show an effect at the position
		if self.checkresist(char, target):
			target.effect(0x3779, 10, 20)
		else:
			wolfpack.effect(0x3728, target.pos, 8, 20)
			target.soundeffect(0x201)
			target.delete()

class EnergyBolt (DelayedDamageSpell):
	def __init__(self):
		DelayedDamageSpell.__init__(self, 6)
		self.reagents = {REAGENT_BLACKPEARL: 1, REAGENT_NIGHTSHADE: 1}
		self.mantra = 'Corp Por'
		self.sound = 0x20a
		self.missile = [ 0x379f, 0, 1, 7, 0 ]
		self.reflectable = 1
		self.casttime = 2750

	def damage(self, char, target):
		damage = self.scaledamage(char, target, 38, 1, 5)
		energydamage(target, char, damage, energy=100)

class Explosion (DelayedDamageSpell):
	def __init__(self):
		DelayedDamageSpell.__init__(self, 6)
		self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1}
		self.mantra = 'Vas Ort Flam'
		self.reflectable = 1
		self.delay = 3000
		self.casttime = 2750

	def damage(self, char, target):
		target.soundeffect(0x307)
		target.effect(0x36BD, 20, 10)
		damage = self.scaledamage(char, target, 38, 1, 5)
		energydamage(target, char, damage, fire=100)

def reveal_expire(char, arguments):
	char.reveal()

class Invisibility (CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 6)
		self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_NIGHTSHADE: 1}
		self.mantra = 'An Lor Xen'

	def effect(self, char, target):
		# Clean previous removal timers
		target.dispel(None, 1, "invisibility_reveal")

		# Hide the target
		target.removefromview()
		target.hidden = 1
		target.update()

		# Add a removal timer
		target.addtimer(5000, "magic.circle6.reveal_expire", [], 1, 0, "invisibility_reveal")

		wolfpack.effect(0x376a, target.pos, 10, 15)
		target.soundeffect(0x3c4)

class Mark (Spell):
	def __init__(self):
		Spell.__init__(self, 6)
		self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_BLACKPEARL: 1, REAGENT_MANDRAKE: 1}
		self.mantra = 'Kal Por Ylem'
		self.validtarget = TARGET_ITEM

	def cast(self, char, mode, args=[]):
		# No Mark Region
		if not char.region or char.region.nomark:
			if char.socket:
				char.socket.clilocmessage(501802)
			return

		return Spell.cast(self, char, mode)

	def target(self, char, mode, targettype, target, args=[]):
		char.turnto(target)

		if not char.region or char.region.nomark:
			if char.socket:
				char.socket.clilocmessage(501802)
			return

		# We can only mark recall runes
		if not 'magic.rune' in target.events:
			char.message(501797)
			return

		if target.getoutmostchar() != char:
			if char.socket:
				char.socket.clilocmessage(1062422)
			return

		if not self.consumerequirements(char, mode):
			return

		char.soundeffect(0x1fa)

		# Transfer our current coordinates and region name to
		# the rune.
		target.settag('marked', 1)
		target.settag('location', '%u,%u,%d,%u' % (char.pos.x, char.pos.y, char.pos.z, char.pos.map))
		target.name = char.region.name
		target.resendtooltip()

class MassCurse (Spell):
	def __init__(self):
		Spell.__init__(self, 6)
		self.reagents = {REAGENT_GARLIC: 1, REAGENT_NIGHTSHADE: 1, REAGENT_MANDRAKE: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'Vas Des Sanct'
		self.validtarget = TARGET_GROUND

	def target(self, char, mode, targettype, target, args=[]):
		if not self.consumerequirements(char, mode):
			return

		char.turnto(target)

		if char.player:
			party = char.party
			guild = char.guild
		else:
			party = None
			guild = None

		# Enumerate chars
		chars = wolfpack.chars(target.x, target.y, char.pos.map, 3)
		for target in chars:
			if target == char:
				continue

			# We also ignore people from the same guild or party.
			# Or targets who are innocent.
			if (guild and target.guild == guild) or (party and target.party == party):
				continue

			if target.invulnerable or target.notoriety(char) == 1:
				continue

			target.effect(0x374a, 10, 15)
			target.soundeffect(0x1fb)

			self.harmchar(char, target)
			statmodifier(char, target, 3, 1)

class ParalyzeField(Spell):
	def __init__(self):
		Spell.__init__(self, 6)
		self.reagents = {REAGENT_BLACKPEARL: 1, REAGENT_SPIDERSILK: 1, REAGENT_GINSENG: 1}
		self.mantra = 'In Ex Grav'
		self.validtarget = TARGET_GROUND
		self.harmful = 1
		self.resistable = 1

	def target(self, char, mode, targettype, target, args=[]):
		char.turnto(target)

		if not self.consumerequirements(char, mode):
			return

		xdiff = abs(target.x - char.pos.x)
		ydiff = abs(target.y - char.pos.y)

		positions = []

		# West / East
		if xdiff > ydiff:
			itemid = 0x3967
			for i in range(-2, 3):
				positions.append(wolfpack.coord(target.x, target.y + i, target.z, target.map))

		# North South
		else:
			itemid = 0x3979
			for i in range(-2, 3):
				positions.append(wolfpack.coord(target.x + i, target.y, target.z, target.map))

		serials = []

		char.soundeffect(0x20b)

		for pos in positions:
			newitem = wolfpack.newitem(1)
			newitem.id = itemid
			newitem.moveto(pos)
			newitem.decay = 0 # Dont decay. TempEffect will take care of them
			newitem.settag('dispellable_field', 1)
			newitem.settag('strength', char.skill[self.damageskill])
			newitem.events = ['magic.paralyzefield']
			newitem.direction = 29
			newitem.update()
			serials.append(newitem.serial)
			wolfpack.effect(0x376A, newitem.pos, 9, 10)

			# Affect chars who are occupying the field cells
			chars = wolfpack.chars(newitem.pos.x, newitem.pos.y, newitem.pos.map)
			for affected in chars:
				if affected.pos.z >= newitem.pos.z - 10 and affected.pos.z <= newitem.pos.z + 10:
					newitem.callevent(EVENT_COLLIDE, (affected, newitem))

		duration = int((3 + char.skill[MAGERY] / 30.0) * 1000)
		wolfpack.addtimer(duration, "magic.utilities.field_expire", serials, 1)

class Reveal (Spell):
	def __init__(self):
		Spell.__init__(self, 6)
		self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'Wis Quas'
		self.validtarget = TARGET_GROUND

	def target(self, char, mode, targettype, target, args=[]):
		if not self.consumerequirements(char, mode):
			return

		char.turnto(target)

		if char.player:
			party = char.party
			guild = char.guild
		else:
			party = None
			guild = None

		# Enumerate chars
		range = 1 + int(char.skill[MAGERY] / 200.0)

		chars = wolfpack.chars(target.x, target.y, char.pos.map, range)
		for target in chars:
			if target == char:
				continue

			# We also ignore people from the same guild or party.
			# Or targets who are innocent.
			if (guild and target.guild == guild) or (party and target.party == party):
				continue

			if target.hidden:
				target.reveal()
				target.effect(0x375a, 9, 20)
				target.soundeffect(0x1fd)

def onLoad():
	Dispel().register(41)
	EnergyBolt().register(42)
	Explosion().register(43)
	Invisibility().register(44)
	Mark().register(45)
	MassCurse().register(46)
	ParalyzeField().register(47)
	Reveal().register(48)
