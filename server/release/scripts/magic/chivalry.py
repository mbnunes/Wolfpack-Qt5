
# http://uo.stratics.com/content/guides/paladinabilities.shtml

from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell
from wolfpack.consts import *
from magic.utilities import *

def onLoad():
	CloseWounds().register(81)
	CleanseByFire().register(82)
	RemoveCurse().register(83)
	ConsecrateWeapon().register(84)
	SacredJourney().register(85)
	DivineFury().register(86)
	DispelEvil().register(87)
	EnemyOfOne().register(88)
	HolyLight().register(89)
	NobleSacrifice().register(90)

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
