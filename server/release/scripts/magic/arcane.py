#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts		        #
#  ((    _/{  "-;  | Created by: Naddel			        #
#   )).-' {{ ;'`   | Revised by:				#
#  ( (  ;._ \\ ctr | Last Modification: Created		        #
#===============================================================#
# Arcane basic stuff					        #
#===============================================================#

# http://uo.stratics.com/content/guides/SPELLWEAVING1.shtml

from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell
from wolfpack.consts import *
from magic.utilities import *
import wolfpack.time

class ArcaneCircle(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = SPELLWEAVING
		self.damageskill = MAGERY
		self.mana = 24
		self.reagents = {}
		self.mantra = ''

class GiftOfRenewal(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = SPELLWEAVING
		self.damageskill = MAGERY
		self.mana = 24
		self.reagents = {}
		self.mantra = ''

class ImmolatingWeapon(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = SPELLWEAVING
		self.damageskill = MAGERY
		self.mana = 32
		self.reagents = {}
		self.mantra = ''

class Attunement(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = SPELLWEAVING
		self.damageskill = MAGERY
		self.mana = 24
		self.reagents = {}
		self.mantra = ''

class Thunderstorm(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = SPELLWEAVING
		self.damageskill = MAGERY
		self.mana = 32
		self.reagents = {}
		self.mantra = ''

class NaturesFury(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = SPELLWEAVING
		self.damageskill = MAGERY
		self.mana = 24
		self.reagents = {}
		self.mantra = ''

class SummonFey(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = SPELLWEAVING
		self.damageskill = MAGERY
		self.mana = 10
		self.reagents = {}
		self.mantra = ''

class SummonFiend(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = SPELLWEAVING
		self.damageskill = MAGERY
		self.mana = 10
		self.reagents = {}
		self.mantra = ''

class ReaperForm(Spell):
	def __init__(self):
		Spell.__init__(self, 1)
		self.skill = SPELLWEAVING
		self.damageskill = MAGERY
		self.mana = 34
		self.reagents = {}
		self.mantra = ''

class Wildfire(Spell):
	def __init__(self):
		Spell.__init__(self, 1)
		self.skill = SPELLWEAVING
		self.damageskill = MAGERY
		self.mana = 50
		self.reagents = {}
		self.mantra = ''

class EssenceOfWind(Spell):
	def __init__(self):
		Spell.__init__(self, 1)
		self.skill = SPELLWEAVING
		self.damageskill = MAGERY
		self.mana = 40
		self.reagents = {}
		self.mantra = ''

class DryadAllure(Spell):
	def __init__(self):
		Spell.__init__(self, 1)
		self.skill = SPELLWEAVING
		self.damageskill = MAGERY
		self.mana = 40
		self.reagents = {}
		self.mantra = ''

class EtherealVoyage(Spell):
	def __init__(self):
		Spell.__init__(self, 1)
		self.skill = SPELLWEAVING
		self.damageskill = MAGERY
		self.mana = 32
		self.reagents = {}
		self.mantra = ''

class WordOfDeath(Spell):
	def __init__(self):
		Spell.__init__(self, 1)
		self.skill = SPELLWEAVING
		self.damageskill = MAGERY
		self.mana = 50
		self.reagents = {}
		self.mantra = ''

class GiftOfLife(Spell):
	def __init__(self):
		Spell.__init__(self, 1)
		self.skill = SPELLWEAVING
		self.damageskill = MAGERY
		self.mana = 70
		self.reagents = {}
		self.mantra = ''

class ArcaneEmpowerment(Spell):
	def __init__(self):
		Spell.__init__(self, 1)
		self.skill = SPELLWEAVING
		self.damageskill = MAGERY
		self.mana = 50
		self.reagents = {}
		self.mantra = ''

def onLoad():
	ArcaneCircle().register(601)
	GiftOfRenewal().register(602)
	ImmolatingWeapon().register(603)
	Attunement().register(604)
	Thunderstorm().register(605)
	NaturesFury().register(606)
	SummonFey().register(607)
	SummonFiend().register(608)
	ReaperForm().register(609)
	Wildfire().register(610)
	EssenceOfWind().register(611)
	DryadAllure().register(612)
	EtherealVoyage().register(613)
	WordOfDeath().register(614)
	GiftOfLife().register(615)
	ArcaneEmpowerment().register(616)
