#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts		        #
#  ((    _/{  "-;  | Created by: Naddel			        #
#   )).-' {{ ;'`   | Revised by:				#
#  ( (  ;._ \\ ctr | Last Modification: Created		        #
#===============================================================#
# NINJITSU basic stuff					        #
#===============================================================#

# http://uo.stratics.com/content/skills/NINJITSU.php

from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell
from wolfpack.consts import *
from magic.utilities import *
import wolfpack.time

class FocusAttack(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = NINJITSU
		self.requiredskill = 60
		self.mana = 20
		self.mantra = ''

class DeathStrike(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = NINJITSU
		self.requiredskill = 50
		self.mana = 85
		self.mantra = ''

class AnimalForm(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = NINJITSU
		self.requiredskill = 0
		self.mana = 0
		self.mantra = ''

class KiAttack(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = NINJITSU
		self.requiredskill = 50
		self.mana = 80
		self.mantra = ''

class SurpriseAttack(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = NINJITSU
		self.requiredskill = 30
		self.mana = 20
		self.mantra = ''

class Backstab(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = NINJITSU
		self.requiredskill = 20
		self.mana = 30
		self.mantra = ''

class Shadowjump(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = NINJITSU
		self.requiredskill = 50
		self.mana = 15
		self.mantra = ''

class MirrorImage(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = NINJITSU
		self.requiredskill = 40
		self.mana = 10
		self.mantra = ''

def onLoad():
	FocusAttack().register(501)
	DeathStrike().register(502)
	AnimalForm().register(503)
	KiAttack().register(504)
	SurpriseAttack().register(505)
	Backstab().register(506)
	Shadowjump().register(507)
	MirrorImage().register(508)
