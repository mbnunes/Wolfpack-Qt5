
from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell
from magic.utilities import *
import random
import wolfpack
from wolfpack.utilities import tobackpack, energydamage

def onLoad():
	Earthquake().register(57)
	EnergyVortex().register(58)
	Resurrection().register(59)
	SummonAirElement().register(60)
	SummonDaemon().register(61)
	SummonEarthElement().register(62)
	SummonFireElement().register(63)
	SummonWaterElement().register(64)

class Earthquake(Spell):
	def __init__(self):
		Spell.__init__(self, 8)
		self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1, REAGENT_GINSENG: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'In Vas Por'

	def cast(self, char, mode, args):
		if not self.consumerequirements(char, mode):
			return

		if char.player:
			party = char.party
			guild = char.guild
		else:
			party = None
			guild = None

		targets = []

		damage = self.scaledamage(char, None, 48, 45, 5)

		chars = wolfpack.chars(char.pos.x, char.pos.y, char.pos.map, 8)
		for target in chars:
			if target == char:
				continue

			if (guild and target.guild == guild) or (party and target.party == party):
				continue

			targets.append(target)

		for target in targets:
			target.soundeffect(0x2F3)
			self.harmchar(char, target)
			energydamage(target, char, damage, physical=100)

class EnergyVortex(Spell):
	def __init__(self):
		Spell.__init__(self, 8)
		self.reagents = {REAGENT_BLACKPEARL: 1, REAGENT_MANDRAKE: 1, REAGENT_BLOODMOSS: 1, REAGENT_NIGHTSHADE: 1}
		self.mantra = 'Vas Corp Por'
		self.validtarget = TARGET_GROUND

	def target(self, char, mode, targettype, target, args=[]):
		char.turnto(target)

		if not self.consumerequirements(char, mode):
			return

		ev = wolfpack.addnpc('energy_vortex', target)
		ev.summontime = wolfpack.time.servertime() + 120000
		ev.summoned = 1
		ev.soundeffect(0x212)

class Resurrection(Spell):
	def __init__(self):
		Spell.__init__(self, 8)
		self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_GARLIC: 1, REAGENT_GINSENG: 1}
		self.mantra = 'An Corp'
		self.validtarget = TARGET_CHAR

	def target(self, char, mode, targettype, target, args=[]):
		char.turnto(target)

		if not target.player:
			char.socket.clilocmessage(503348)
			return

		if not target.dead:
			char.socket.clilocmessage(501041)
			return

		if not self.consumerequirements(char, mode):
			return

		target.resurrect()
		target.soundeffect(0x214)

class SummonElementBase(Spell):
	def __init__(self):
		Spell.__init__(self, 8)
		self.elementid = ''
		self.validtarget = TARGET_GROUND
		self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1, REAGENT_SPIDERSILK: 1}
		self.casttime = 6000

	def target(self, char, mode, targettype, target, args=[]):
		char.turnto(target)

		if char.player and char.controlslots >= 5:
			char.socket.clilocmessage(1049645)
			return

		if not self.consumerequirements(char, mode):
			return

		creature = wolfpack.addnpc(self.elementid, target)
		creature.controlslots = 1
		creature.owner = char
		creature.summontime = wolfpack.time.servertime() + 120000
		creature.summoned = 1
		creature.soundeffect(0x217)

class SummonAirElement(SummonElementBase):
	def __init__(self):
		SummonElementBase.__init__(self)
		self.mantra = 'Kal Vas Xen Hur'
		self.elementid = 'air_elemental'
		self.casttime = 6000

class SummonEarthElement(SummonElementBase):
	def __init__(self):
		SummonElementBase.__init__(self)
		self.mantra = 'Kal Vas Xen Ylem'
		self.elementid = 'earth_elemental'
		self.casttime = 6000

class SummonFireElement(SummonElementBase):
	def __init__(self):
		SummonElementBase.__init__(self)
		self.mantra = 'Kal Vas Xen Flam'
		self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1, REAGENT_SPIDERSILK: 1, REAGENT_SULFURASH: 1}
		self.elementid = 'fire_elemental'
		self.casttime = 6000

class SummonWaterElement(SummonElementBase):
	def __init__(self):
		SummonElementBase.__init__(self)
		self.mantra = 'Kal Vas Xen An Flam'
		self.elementid = 'water_elemental'
		self.casttime = 6000

class SummonDaemon(SummonElementBase):
	def __init__(self):
		SummonElementBase.__init__(self)
		self.mantra = 'Kal Vas Xen Corp'
		self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1, REAGENT_SPIDERSILK: 1, REAGENT_SULFURASH: 1}
		self.elementid = 'daemon'
		self.casttime = 6000
