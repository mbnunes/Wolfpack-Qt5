
from magic import registerspell
from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell
from magic.utilities import *
import random
import wolfpack
from wolfpack.utilities import tobackpack
from system import poison

class Agility (CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 2)
		self.reagents = {REAGENT_MANDRAKE: 1, REAGENT_BLOODMOSS: 1}
		self.mantra = 'Ex Uus'
		
	def effect(self, char, target):
		statmodifier(char, target, 1, 0)				
		target.effect(0x375a, 10, 15)
		target.soundeffect(0x28e)		
		
class Cunning (CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 2)
		self.reagents = {REAGENT_MANDRAKE: 1, REAGENT_NIGHTSHADE: 1}
		self.mantra = 'Uus Wis'
		
	def effect(self, char, target):
		statmodifier(char, target, 2, 0)
				
		target.effect(0x375a, 10, 15)
		target.soundeffect(0x1eb)				

class Strength (CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 2)
		self.reagents = {REAGENT_MANDRAKE: 1, REAGENT_NIGHTSHADE: 1}
		self.mantra = 'Uus Mani'
		
	def effect(self, char, target):
		statmodifier(char, target, 0, 0)
				
		target.effect(0x375a, 10, 15)
		target.soundeffect(0x1ee)

class Harm (DelayedDamageSpell):
	def __init__(self):
		DelayedDamageSpell.__init__(self, 2)
		self.delay = None
		self.reagents = {REAGENT_SPIDERSILK: 1, REAGENT_NIGHTSHADE: 1}
		self.mantra = 'An Mani'
		self.sound = 0xfc
		self.reflectable = 1
		
	def damage(self, char, target):
		distance = target.distanceto(char)
		damage = self.scaledamage(char, target, 6, 3, 6.5)

		if distance > 2:
			damage = max(1, damage * 0.25)
		elif distance == 2:
			damage = max(1, damage * 0.5)		

		energydamage(target, char, damage, cold=100)
		target.effect(0x374a, 10, 30)

class MagicTrap (Spell):
	def __init__(self):
		Spell.__init__(self, 2)
		self.validtarget = TARGET_ITEM
		self.reagents = {REAGENT_GARLIC: 1, REAGENT_SPIDERSILK: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'In Jux'
		
	def target(self, char, mode, targettype, target):
		char.turnto(target)
	
		# Only containers are trappable
		if target.type != 1:
			char.message('You can''t trap that.')
			return
	
		if not self.consumerequirements(char, mode):
			return
		
		pos = target.pos		
		wolfpack.effect(0x376a, wolfpack.coord(pos.x + 1, pos.y, pos.z, pos.map), 9, 10)
		wolfpack.effect(0x376a, wolfpack.coord(pos.x, pos.y - 1, pos.z, pos.map), 9, 10)
		wolfpack.effect(0x376a, wolfpack.coord(pos.x - 1, pos.y, pos.z, pos.map), 9, 10)
		wolfpack.effect(0x376a, wolfpack.coord(pos.x, pos.y + 1, pos.z, pos.map), 9, 10)
		target.soundeffect(0x1ef)
		
		# Add the Trap Properties to the Item
		target.settag('trap_owner', char.serial)
		target.settag('trap_damage', random.randint(10, 50))
		target.settag('trap_type', 'magic')
		target.events = target.events + ['magic.trap']
		
class RemoveTrap (Spell):
	def __init__(self):
		Spell.__init__(self, 2)
		self.validtarget = TARGET_ITEM
		self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'An Jux'
		
	def target(self, char, mode, targettype, target):
		char.turnto(target)
	
		if not self.consumerequirements(char, mode):
			return
	
		# Already Trapped?
		if not'magic.trap' in target.events:
			fizzle(char)
			return

		wolfpack.effect(0x376a, target.pos, 9, 32)
		target.soundeffect(0x1f0)
		
		# Add the Trap Properties to the Item
		target.deltag('trap_owner', char.serial)
		target.deltag('trap_damage', random.randint(10, 50))
		target.deltag('trap_type', 'magic')
		events = target.events
		events.remove('magic.trap')
		target.events = events

class Protection(Spell):
	def __init__(self):
		Spell.__init__(self, 2)
		self.reagents = {REAGENT_GARLIC: 1, REAGENT_GINSENG: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'Uus Sanct'

	def cast(self, char, mode):
		if not self.consumerequirements(char, mode):
			return

		# Toggle Protection
		if char.propertyflags & 0x20000:
			char.propertyflags &= ~ 0x20000
			char.effect(0x375a, 9, 20)
			char.soundeffect(0x1ed)

			try:
				if char.hastag('protection_skill'):
					skill = int(char.gettag('protection_skill'))					
					if char.skill[MAGICRESISTANCE] + skill > char.skillcap[MAGICRESISTANCE]:
						char.skill[MAGICRESISTANCE] = char.skillcap[MAGICRESISTANCE]
					else:
						char.skill[MAGICRESISTANCE] += skill
			except:
				pass
			char.deltag('protection_skill')
		else:
			char.propertyflags |= 0x20000
			char.effect(0x375a, 9, 20)
			char.soundeffect(0x1e9)
			
			# Remove Magic Resistance Skill
			amount = 35 - int(char.skill[INSCRIPTION] / 200)
			if char.skill[MAGICRESISTANCE] - amount < 0:
				amount = char.skill[MAGICRESISTANCE]
				
			char.settag('protection_skill', amount)
			char.skill[MAGICRESISTANCE] -= amount

		if char.socket:
			char.socket.resendstatus()

class Cure (CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 2)
		self.reagents = {REAGENT_GARLIC: 1, REAGENT_GINSENG: 1}
		self.mantra = 'An Nox'
		
	def effect(self, char, target):
		if target.poison != -1:
			poison.cure(target)
			if char != target and char.socket:
				char.socket.clilocmessage(1010058)
				
			if target.socket:
				target.socket.clilocmessage(1010059)
		
		target.effect(0x373a, 10, 15)
		target.soundeffect(0x1e0)

def onLoad():
	Agility().register(9)
	Cunning().register(10)
	Cure().register(11)
	Harm().register(12)
	MagicTrap().register(13)
	RemoveTrap().register(14)
	Protection().register(15)
	Strength().register(16)
