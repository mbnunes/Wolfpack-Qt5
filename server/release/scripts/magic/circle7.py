
from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell
from magic.utilities import *
import random
import wolfpack
from wolfpack.utilities import tobackpack, energydamage
from math import ceil
from magic import polymorph

class ChainLightning (Spell):
	def __init__(self):
		Spell.__init__(self, 7)
		self.reagents = {REAGENT_BLACKPEARL: 1, REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'Vas Ort Grav'
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

		targets = []
		damage = self.scaledamage(char, None, 48, 1, 5)

		# Enumerate chars
		chars = wolfpack.chars(target.x, target.y, char.pos.map, 2)
		for target in chars:
			if target == char:
				continue

			# We also ignore people from the same guild or party.
			# Or targets who are innocent.
			if (guild and target.guild == guild) or (party and target.party == party):
				continue

			targets.append(target)

		# Re-scale the damage
		if len(targets) > 1:
			damage = ceil((damage * 2.0) / len(targets))

		for target in targets:
			target.lightning()
			target.soundeffect(0x29)

			self.harmchar(char, target)
			energydamage(target, char, damage, energy=100)

class EnergyField(Spell):
	def __init__(self):
		Spell.__init__(self, 7)
		self.reagents = {REAGENT_BLACKPEARL: 1, REAGENT_MANDRAKE: 1, REAGENT_SPIDERSILK: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'In Sanct Grav'
		self.validtarget = TARGET_GROUND

	def target(self, char, mode, targettype, target, args=[]):
		char.turnto(target)

		if not self.consumerequirements(char, mode):
			return

		xdiff = abs(target.x - char.pos.x)
		ydiff = abs(target.y - char.pos.y)

		positions = []

		# West / East
		if xdiff > ydiff:
			itemid = 0x3956
			for i in range(-2, 3):
				positions.append(wolfpack.coord(target.x, target.y + i, target.z, target.map))

		# North South
		else:
			itemid = 0x3946
			for i in range(-2, 3):
				positions.append(wolfpack.coord(target.x + i, target.y, target.z, target.map))

		serials = []

		char.soundeffect(0x20b)

		for pos in positions:
			newitem = wolfpack.newitem(1)
			newitem.id = itemid
			newitem.moveto(pos)
			newitem.decay = 0
			newitem.direction = 29
			newitem.settag('dispellable_field', 1)
			newitem.update()
			serials.append(newitem.serial)

		duration = int((15 + (char.skill[MAGERY] / 5)) / 0.007)
		wolfpack.addtimer(duration, "magic.utilities.field_expire", serials, 1)

class FlameStrike (DelayedDamageSpell):
	def __init__(self):
		DelayedDamageSpell.__init__(self, 7)
		self.reagents = {REAGENT_SPIDERSILK: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'Kal Vas Flam'
		self.reflectable = 1

	def damage(self, char, target):
		target.effect(0x3709, 10, 30)
		target.soundeffect(0x208)
		damage = self.scaledamage(char, target, 48, 1, 5)
		energydamage(target, char, damage, fire=100)

class GateTravel (Spell):
	def __init__(self):
		Spell.__init__(self, 7)
		self.reagents = {REAGENT_BLACKPEARL: 1, REAGENT_MANDRAKE: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'Vas Rel Por'
		self.validtarget = TARGET_ITEM

	def cast(self, char, mode, args=[]):
		if not char.gm:
			if char.iscriminal():
				if char.socket:
					char.socket.clilocmessage(1005561)
				return

			if char.attacktarget:
				if char.socket:
					char.socket.clilocmessage(1005564)
				return

		return Spell.cast(self, char, mode)

	def target(self, char, mode, targettype, target, args=[]):
		char.turnto(target)

		# We can only recall from recall runes
		if not 'magic.rune' in target.events:
			char.message(502357)
			return

		if not self.consumerequirements(char, mode):
			return

		if not target.hastag('marked') or target.gettag('marked') != 1:
			char.message(502354)
			fizzle(char)
			return

		region = wolfpack.region(char.pos.x, char.pos.y, char.pos.map)

		if region.nogate:
			char.message(501802)
			fizzle(char)
			return

		location = target.gettag('location')
		location = location.split(",")
		location = wolfpack.coord(int(location[0]), int(location[1]), int(location[2]), int(location[3]))

		region = wolfpack.region(location.x, location.y, location.map)

		if not location.validspawnspot():
			char.message(501942)
			fizzle(char)
			return

		if region.nogate:
			char.message(1019004)
			fizzle(char)
			return

		if char.socket:
			char.socket.clilocmessage(501024)

		serials = []

		# Create the two gates
		gate = wolfpack.newitem(1)
		gate.id = 0xf6c
		gate.moveto(char.pos)
		gate.settag('dispellable_field', 1)
		gate.events = ['magic.gate']
		gate.settag('target', target.gettag('location'))
		gate.direction = 9
		gate.update()
		gate.soundeffect(0x20e)
		serials.append(gate.serial)

		gate = wolfpack.newitem(1)
		gate.id = 0xf6c
		gate.moveto(location)
		gate.settag('target', '%u,%u,%d,%u' % (char.pos.x, char.pos.y, char.pos.z, char.pos.map))
		gate.settag('dispellable_field', 1)
		gate.events = ['magic.gate']
		gate.direction = 9
		gate.update()
		gate.soundeffect(0x20e)
		serials.append(gate.serial)

		wolfpack.addtimer(30000, "magic.utilities.field_expire", serials, 1)

class ManaVampire (DelayedDamageSpell):
	def __init__(self):
		DelayedDamageSpell.__init__(self, 7)
		self.reagents = {REAGENT_BLACKPEARL: 1, REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1, REAGENT_SPIDERSILK: 1}
		self.mantra = 'Ort Sanct'
		self.reflectable = 1
		self.delay = None

	def damage(self, char, target):
		target.disturb()
		if target.frozen:
			target.frozen = 0
			target.resendtooltip()

		char.checkskill(self.damageskill, 0, 1200)

		# See if we can gain skill
		maxskill = self.circle * 100
		maxskill += (1 + ((self.circle - 1) / 6)) * 250

		if target.skill[MAGICRESISTANCE] < maxskill:
			target.checkskill(MAGICRESISTANCE, 0, 1200)

		drain = min(target.mana, int((char.skill[self.damageskill] - target.skill[MAGICRESISTANCE]) / 10.0))

		if drain > char.maxmana - char.mana:
			drain = char.maxmana - char.mana

		if target.player:
			drain /= 2

		if drain > 0:
			char.mana += drain
			char.updatemana()
			target.mana -= drain
			target.updatemana()

		target.effect(0x374a, 1, 15)
		target.soundeffect(0x1f9)

class MassDispel (Spell):
	def __init__(self):
		Spell.__init__(self, 7)
		self.reagents = {REAGENT_GARLIC: 1, REAGENT_BLACKPEARL: 1, REAGENT_MANDRAKE: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'Vas An Ort'
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
		chars = wolfpack.chars(target.x, target.y, char.pos.map, 8)
		for target in chars:
			if char == target or not target.npc or target.summontimer == 0:
				continue

			wolfpack.effect(0x3728, target.pos, 8, 20)
			target.soundeffect(0x201)
			target.delete()

class MeteorSwarm (Spell):
	def __init__(self):
		Spell.__init__(self, 7)
		self.reagents = {REAGENT_SPIDERSILK: 1, REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'Flam Kal Des Ylem'
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

		targets = []
		damage = self.scaledamage(char, None, 48, 1, 5)

		# Enumerate chars
		chars = wolfpack.chars(target.x, target.y, char.pos.map, 2)
		for target in chars:
			if target == char:
				continue

			# We also ignore people from the same guild or party.
			# Or targets who are innocent.
			if (guild and target.guild == guild) or (party and target.party == party):
				continue

			targets.append(target)

		# Re-scale the damage
		if len(targets) > 1:
			damage = ceil((damage * 2.0) / len(targets))

		for target in targets:
			target.soundeffect(0x160)
			char.movingeffect(0x36D4, target, 7, 0)
			self.harmchar(char, target)
			energydamage(target, char, damage, fire=100)

def polymorph_expire(char, arguments):
	char.id = char.orgid
	char.skin = char.orgskin
	char.polymorph = 0
	char.update()

class Polymorph (Spell):
	def __init__(self):
		Spell.__init__(self, 7)
		self.reagents = {REAGENT_GARLIC: 1, REAGENT_GINSENG: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'Vas Ylem Rel'

	def checkrequirements(self, char, mode, args=[]):
		if char.polymorph:
			if char.socket:
				char.socket.clilocmessage(1005559)
			return 0
		if char.incognito:
			if char.socket:
				char.socket.clilocmessage(502167)
			return 0

		if len(args) == 0:
			polymorph.showmenu(char)
			return 0
		return Spell.checkrequirements(self, char, mode, args)

	def cast(self, char, mode, args):
		if char.polymorph:
			if char.socket:
				char.socket.clilocmessage(1005559)
			return 0

		if char.incognito:
			if char.socket:
				char.socket.clilocmessage(502167)
			return 0

		# Select a creature to polymorph into
		char.unmount()
		char.orgid = char.id
		char.id = args[0]
		char.orgskin = char.skin
		if char.id == 0x190 or char.id == 0x191:
			char.skin = random.randint(1002, 1059)
		else:
			char.skin = 0
		char.polymorph = 1
		char.update()

		duration = int(300 + char.skill[MAGERY] * 100)
		char.addtimer(duration, "magic.circle7.polymorph_expire", [], 1)

def onLoad():
	ChainLightning().register(49)
	EnergyField().register(50)
	FlameStrike().register(51)
	GateTravel().register(52)
	ManaVampire().register(53)
	MassDispel().register(54)
	MeteorSwarm().register(55)
	Polymorph().register(56)
