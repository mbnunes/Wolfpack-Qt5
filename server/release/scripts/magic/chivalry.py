
# http://uo.stratics.com/content/guides/paladinabilities.shtml

from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell
from wolfpack.consts import *
from magic.utilities import *

class CloseWounds(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = CHIVALRY
		self.damageskill = FOCUS
		self.mana = 0
		#self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1, REAGENT_GINSENG: 1, REAGENT_SULFURASH: 1}
		self.mantra = ''

class CleanseByFire(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = CHIVALRY
		self.damageskill = FOCUS
		self.mana = 0
		#self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1, REAGENT_GINSENG: 1, REAGENT_SULFURASH: 1}
		self.mantra = ''

class RemoveCurse(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = CHIVALRY
		self.damageskill = FOCUS
		self.mana = 0
		#self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1, REAGENT_GINSENG: 1, REAGENT_SULFURASH: 1}
		self.mantra = ''

class ConsecrateWeapon(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = CHIVALRY
		self.damageskill = FOCUS
		self.mana = 0
		#self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1, REAGENT_GINSENG: 1, REAGENT_SULFURASH: 1}
		self.mantra = ''

class SacredJourney(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = CHIVALRY
		self.damageskill = FOCUS
		self.mana = 0
		#self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1, REAGENT_GINSENG: 1, REAGENT_SULFURASH: 1}
		self.mantra = ''

class DivineFury(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = CHIVALRY
		self.damageskill = FOCUS
		self.mana = 0
		#self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1, REAGENT_GINSENG: 1, REAGENT_SULFURASH: 1}
		self.mantra = ''

class DispelEvil(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = CHIVALRY
		self.damageskill = FOCUS
		self.mana = 0
		#self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1, REAGENT_GINSENG: 1, REAGENT_SULFURASH: 1}
		self.mantra = ''

class EnemyOfOne(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = CHIVALRY
		self.damageskill = FOCUS
		self.mana = 0
		#self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1, REAGENT_GINSENG: 1, REAGENT_SULFURASH: 1}
		self.mantra = ''

class HolyLight(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = CHIVALRY
		self.damageskill = FOCUS
		self.mana = 0
		#self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1, REAGENT_GINSENG: 1, REAGENT_SULFURASH: 1}
		self.mantra = ''

class NobleSacrifice(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = CHIVALRY
		self.damageskill = FOCUS
		self.mana = 0
		#self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1, REAGENT_GINSENG: 1, REAGENT_SULFURASH: 1}
		self.mantra = ''


def onLoad():
	CloseWounds().register(101)
	CleanseByFire().register(102)
	RemoveCurse().register(103)
	ConsecrateWeapon().register(104)
	SacredJourney().register(105)
	DivineFury().register(106)
	DispelEvil().register(107)
	EnemyOfOne().register(108)
	HolyLight().register(109)
	NobleSacrifice().register(110)
