
from magic import registerspell
from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell
from magic.utilities import *
import random
import wolfpack
from wolfpack.utilities import tobackpack, energydamage
from wolfpack import time
import math

class Clumsy (CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 1)
		self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_NIGHTSHADE: 1}
		self.mantra = 'Uus Jux'
		self.harmful = 1
		self.reflectable = 1

	def effect(self, char, target, mode, args, item):
		statmodifier(char, target, 1, 1)

		target.effect(0x3779, 10, 15)
		target.soundeffect(0x1df)
		target.disturb()

class Feeblemind (CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 1)
		self.reagents = {REAGENT_GINSENG: 1, REAGENT_NIGHTSHADE: 1}
		self.mantra = 'Rel Wis'
		self.harmful = 1
		self.reflectable = 1

	def effect(self, char, target, mode, args, item):
		statmodifier(char, target, 2, 1)

		target.effect(0x3779, 10, 15)
		target.soundeffect(0x1e5)
		target.disturb()

class Weaken (CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 1)
		self.reagents = {REAGENT_GARLIC: 1, REAGENT_NIGHTSHADE: 1}
		self.mantra = 'Des Mani'
		self.harmful = 1
		self.reflectable = 1

	def effect(self, char, target, mode, args, item):
		statmodifier(char, target, 0, 1)

		target.effect(0x3779, 10, 15)
		target.soundeffect(0x1e6)
		target.disturb()

class CreateFood(Spell):
	def __init__(self):
		Spell.__init__(self, 1)
		self.reagents = {REAGENT_GARLIC: 1, REAGENT_GINSENG: 1, REAGENT_MANDRAKE: 1}
		self.mantra = 'In Mani Ylem'

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return

		# Randomly select one id of food
		foodinfo = [["9d1", "a grape bunch"], ["9d2", "a peach"], ["9c9", "a ham"], ["97c", "a wedge of cheese"],
								["9eb", "muffins"], ["9f2", "cut of ribs"], ["97b", "a fish steak"], ["9b7", "a cooked bird"]]

		food = random.choice(foodinfo)

		item = wolfpack.additem(food[0])
		foodname = " " + food[1]
		if not tobackpack(item, char):
			item.update()

		if char.socket:
			char.socket.clilocmessage(1042695, "", 0x3b2, 3, None, foodname)
		char.soundeffect(0x1e2)

class Heal (CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 1)
		self.reagents = {REAGENT_GARLIC: 1, REAGENT_GINSENG: 1, REAGENT_SPIDERSILK: 1}
		self.mantra = 'In Mani'

	def affectchar(self, char, mode, target, args=[]):
		if target.poison != -1:
			if target == char:
				char.message(1005000)
			else:
				char.message(1010398)
			return 0
		return 1

	def effect(self, char, target, mode, args, item):
		# 10% of Magery + 1-5
		#amount = int(0.01 * char.skill[MAGERY]) + random.randint(1, 5)
		amount = int(char.skill[MAGERY] / 100) + random.randint(1, 3)

		target.hitpoints = min(target.maxhitpoints, target.hitpoints + amount)
		target.updatehealth()

		target.effect(0x376A, 9, 32)
		target.soundeffect(0x1f2)

class NightSight (CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 1)
		self.reagents = {REAGENT_SPIDERSILK: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'In Lor'
		self.range = 10

	def affectchar(self, char, mode, target, args=[]):
		if not target.player:
			if char.socket:
				char.socket.sysmessage('This spell only works on players.')
			return 0

		if target.hasevent( 'magic.nightsight' ):
			if char.socket:
				if char == target:
					char.socket.sysmessage('You already have nightsight.')
				else:
					char.socket.sysmessage('They already have nightsight.')
			return 0
		return 1

	def effect(self, char, target, mode, args, item):
		# Remove an old bonus
		if target.hastag('nightsight'):
			bonus = target.gettag('nightsight')
			target.lightbonus = max(0, target.lightbonus - bonus)

		# With 100% magery you gain a 18 light level bonus
		bonus = min(18, math.floor(18 * (char.skill[MAGERY] / 1000.0)))

		target.addevent( 'magic.nightsight' )
		target.settag("nightsight", bonus)
		target.settag("nightsight_start", time.minutes())
		target.lightbonus += bonus

		if target.socket:
			target.socket.updatelightlevel()

		target.soundeffect(0x1e3)
		target.effect(0x376a, 9, 32)

class MagicArrow (DelayedDamageSpell):
	def __init__(self):
		DelayedDamageSpell.__init__(self, 1)
		self.reagents = {REAGENT_SULFURASH: 1}
		self.mantra = 'In Por Ylem'
		self.missile = [0x36e4, 0, 1, 5, 0]
		self.reflectable = 1
		self.sound = 0x1e5
		self.delay = 0

	def damage(self, char, target):
		damage = self.scaledamage(char, target, 10, 1, 4)
		energydamage(target, char, damage, fire=100)

class ReactiveArmor(Spell):
	def __init__(self):
		Spell.__init__(self, 1)
		self.reagents = {REAGENT_GARLIC: 1, REAGENT_SPIDERSILK: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'Flam Sanct'

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return

		# Toggle ReactiveArmor
		if char.propertyflags & 0x10000:
			char.propertyflags &= ~ 0x10000
			char.effect(0x376a, 9, 32)
			char.soundeffect(0x1ed)
		else:
			char.propertyflags |= 0x10000
			char.effect(0x376a, 9, 32)
			char.soundeffect(0x1e9)

		if char.socket:
			char.socket.resendstatus()

# Register the spells in this module (magic.circle1)
def onLoad():
	Clumsy().register(1)
	CreateFood().register(2)
	Feeblemind().register(3)
	Heal().register(4)
	MagicArrow().register(5)
	NightSight().register(6)
	ReactiveArmor().register(7)
	Weaken().register(8)
