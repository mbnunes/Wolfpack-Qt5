#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts		        #
#  ((    _/{  "-;  | Created by: Naddel			        #
#   )).-' {{ ;'`   | Revised by:				#
#  ( (  ;._ \\ ctr | Last Modification: Created		        #
#===============================================================#
# Bushido basic stuff					        #
#===============================================================#

# http://uo.stratics.com/content/skills/bushido.php

from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell
from wolfpack.consts import *
from magic.utilities import *
import wolfpack.time

class HonorableExecution(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = BUSHIDO
		self.requiredskill = 25
		self.mana = 0
		self.mantra = ''

class Confidence(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = BUSHIDO
		self.requiredskill = 25
		self.mana = 10
		self.mantra = ''

class Evasion(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = BUSHIDO
		self.requiredskill = 60
		self.mana = 10
		self.mantra = ''

class CounterAttack(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = BUSHIDO
		self.requiredskill = 40
		self.mana = 5
		self.mantra = ''

class LightningStrike(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = BUSHIDO
		self.requiredskill = 50
		self.mana = 5
		self.mantra = ''

class MomentumStrike(Spell):
	def __init__(self):
		Spell.__init__(self, 0)
		self.skill = BUSHIDO
		self.requiredskill = 70
		self.mana = 10
		self.mantra = ''

def onLoad():
	HonorableExecution().register(401)
	Confidence().register(402)
	Evasion().register(403)
	CounterAttack().register(404)
	LightningStrike().register(405)
	MomentumStrike().register(406)
