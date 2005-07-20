
# http://uo.stratics.com/content/guides/paladinabilities.shtml
# todo: required chivalry skill (differs from spell to spell)

from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell
from wolfpack.consts import *
from magic.utilities import *

class CloseWounds(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = CHIVALRY
		# req skill = 0
		self.damageskill = FOCUS
		self.mana = 10
		self.tithingpoints = 10
		self.mantra = 'Obsu Vulni'

class CleanseByFire(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = CHIVALRY
		# req skill = 50
		self.damageskill = FOCUS
		self.mana = 10
		self.tithingpoints = 10
		self.mantra = 'Expor Flamus'

class RemoveCurse(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = CHIVALRY
		# req skill = 50
		self.damageskill = FOCUS
		self.mana = 20
		self.tithingpoints = 10
		self.mantra = 'Extermo Vomica'

class ConsecrateWeapon(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = CHIVALRY
		# req skill = 150
		self.damageskill = FOCUS
		self.mana = 10
		self.tithingpoints = 10
		self.mantra = 'Consecrus Arma'

class SacredJourney(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = CHIVALRY
		# req skill = 150
		self.damageskill = FOCUS
		self.mana = 10
		self.tithingpoints = 15
		self.mantra = 'Sanctum Viatas'

class DivineFury(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = CHIVALRY
		# req skill = 250
		self.damageskill = FOCUS
		self.mana = 15
		self.tithingpoints = 10
		self.mantra = 'Divinum Furis'

class DispelEvil(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = CHIVALRY
		# req skill = 350
		self.damageskill = FOCUS
		self.mana = 10
		self.tithingpoints = 10
		self.mantra = 'Dispiro Malas'

class EnemyOfOne(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = CHIVALRY
		# req skill = 450
		self.damageskill = FOCUS
		self.mana = 20
		self.tithingpoints = 10
		self.mantra = 'Forul Solum'

class HolyLight(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = CHIVALRY
		# req skill = 550
		self.damageskill = FOCUS
		self.mana = 10
		self.tithingpoints = 10
		self.mantra = 'Augus Luminos'

class NobleSacrifice(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = CHIVALRY
		# req skill = 650
		self.damageskill = FOCUS
		self.mana = 20
		self.tithingpoints = 30
		self.mantra = 'Dium Prostra'


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
