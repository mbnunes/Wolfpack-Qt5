
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

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return

		if char.player:
			party = char.party
			guild = char.guild
		else:
			party = None
			guild = None

		targets = []

		spellrange = 1 + int(char.skill[MAGERY] / 150.0)
		chars = wolfpack.chars(char.pos.x, char.pos.y, char.pos.map, spellrange)
		for target in chars:
			if target == char:
				continue

			if (guild and target.guild == guild) or (party and target.party == party):
				continue

			if not char.canreach(target, spellrange):
				continue

			targets.append(target)

		for target in targets:
			target.soundeffect(0x2F3)
			self.harmchar(char, target)

			damage = target.hitpoints / 2
			if target.player:
				damage += random.randint(0, 15)

			damage = min(100, max(15, damage))
			energydamage(target, char, damage, physical=100)

class EnergyVortex(Spell):
	def __init__(self):
		Spell.__init__(self, 8)
		self.reagents = {REAGENT_BLACKPEARL: 1, REAGENT_MANDRAKE: 1, REAGENT_BLOODMOSS: 1, REAGENT_NIGHTSHADE: 1}
		self.mantra = 'Vas Corp Por'
		self.validtarget = TARGET_GROUND
		self.casttime *= 5

	def target(self, char, mode, targettype, target, args, item):
		char.turnto(target)

		if not self.consumerequirements(char, mode, args, target, item):
			return

		ev = wolfpack.addnpc('summoned_energy_vortex', target)
		ev.summontime = wolfpack.time.currenttime() + 120000
		ev.summoned = 1
		ev.soundeffect(0x212)

class Resurrection(Spell):
	def __init__(self):
		Spell.__init__(self, 8)
		self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_GARLIC: 1, REAGENT_GINSENG: 1}
		self.mantra = 'An Corp'
		self.validtarget = TARGET_CHAR
		self.affectdead = True

	def target(self, char, mode, targettype, target, args, item):
		char.turnto(target)

		if not target.player:
			char.socket.clilocmessage(503348)
			return

		if not target.dead:
			char.socket.clilocmessage(501041)
			return

		if not self.consumerequirements(char, mode, args, target, item):
			return

		target.resurrect( char )
		target.soundeffect(0x214)

class SummonElementBase(Spell):
	def __init__(self):
		Spell.__init__(self, 8)
		self.elementid = ''
		self.validtarget = TARGET_GROUND
		self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1, REAGENT_SPIDERSILK: 1}
		self.casttime = 6000

	def target(self, char, mode, targettype, target, args, item):
		char.turnto(target)

		# Lowest controlslots we see is 2, Earth Elemental
		if char.player and char.controlslots + 2 > char.maxcontrolslots:
			char.socket.clilocmessage(1049645)
			return

		if not self.consumerequirements(char, mode, args, target, item):
			return

		creature = wolfpack.addnpc(self.elementid, target)
		# If the creature is out of our control, delete it.
		if char.player and char.controlslots + creature.controlslots > char.maxcontrolslots:
			creature.delete()
			char.socket.clilocmessage(1049645)
		else:
			creature.addscript('speech.pets') # This only adds if it has the event anyway
			creature.owner = char
			creature.tamed = True
			creature.summontime = wolfpack.time.currenttime() + 120000
			creature.summoned = True
			creature.soundeffect(0x217)

class SummonAirElement(SummonElementBase):
	def __init__(self):
		SummonElementBase.__init__(self)
		self.mantra = 'Kal Vas Xen Hur'
		self.elementid = 'summoned_air_elemental'
		self.casttime = 6000

class SummonEarthElement(SummonElementBase):
	def __init__(self):
		SummonElementBase.__init__(self)
		self.mantra = 'Kal Vas Xen Ylem'
		self.elementid = 'summoned_earth_elemental'
		self.casttime = 6000

class SummonFireElement(SummonElementBase):
	def __init__(self):
		SummonElementBase.__init__(self)
		self.mantra = 'Kal Vas Xen Flam'
		self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1, REAGENT_SPIDERSILK: 1, REAGENT_SULFURASH: 1}
		self.elementid = 'summoned_fire_elemental'
		self.casttime = 6000

class SummonWaterElement(SummonElementBase):
	def __init__(self):
		SummonElementBase.__init__(self)
		self.mantra = 'Kal Vas Xen An Flam'
		self.elementid = 'summoned_water_elemental'
		self.casttime = 6000

class SummonDaemon(SummonElementBase):
	def __init__(self):
		SummonElementBase.__init__(self)
		self.mantra = 'Kal Vas Xen Corp'
		self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1, REAGENT_SPIDERSILK: 1, REAGENT_SULFURASH: 1}
		self.elementid = 'summoned_daemon'
		self.casttime = 6000
