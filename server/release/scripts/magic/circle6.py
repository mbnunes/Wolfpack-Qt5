
from magic import registerspell
from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell
from magic.utilities import *
import random
import wolfpack
from wolfpack.utilities import tobackpack
from wolfpack import time
import math

class Dispel (CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 6)
		self.reagents = { REAGENT_GARLIC: 1, REAGENT_MANDRAKE: 1, REAGENT_SULFURASH: 1 }
		self.mantra = 'An Ort'
		self.harmful = 1

	def affectchar(self, char, mode, target):
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

def onLoad():
	Dispel().register(41)
