
import magic
from wolfpack.consts import *
from magic.utilities import *
import wolfpack.console
import random
from combat import properties

# Recursive Function for counting reagents
def countReagents(item, items):
	for key in items.keys():
		if key == item.id and item.color == 0:
			items[ key ] = max(0, items[ key ] - item.amount)
			return items # Reagents normally dont have content
			
	for subitem in item.content:
		items = countReagents(subitem, items)
		
	return items
	
# Recursive Function for removing reagents
def consumeReagents(item, items):
	for (key, value) in items.items():
		if key == item.id and item.color == 0:
			if item.amount <= value:
				items[ key ] -= item.amount
				item.delete()
			else:
				item.amount -= value
				item.update()
				del items[key]

			return items # Reagents normally dont have content

	for subitem in item.content:
		items = consumeReagents(subitem, items)
		
	return items

def callback(char, args):
	eventlist = char.events
	eventlist.remove('magic')
	char.events = eventlist

	args[0].cast(char, args[1], args[2])

# Basic Spell Class
class Spell:
	# We affect another character
	def affectchar(self, char, mode, target, args=[]):
		return 1
	
	def register(self, id):
		self.spellid = id
		magic.registerspell(self.spellid, self)
	
	def __init__(self, circle):
		# Set Mana
		self.mana = 0
		self.reagents = {}
		self.validtarget = TARGET_IGNORE
		self.spellid = None
		self.mantra = None
		self.harmful = 0
		self.affectdead = 0
		self.resistable = 0
		self.range = 12
		self.skill = MAGERY
		self.damageskill = EVALUATINGINTEL
		self.circle = circle
		self.reflectable = 0
		self.inherent = 0
		mana_table = [ 4, 6, 9, 11, 14, 20, 40, 50 ]
		self.mana = mana_table[ self.circle - 1 ]

	#
	# Prepare the casting of this spell.
	#
	def precast(self, char, mode=0, args=[]):
		eventlist = char.events
		socket = char.socket

		# We are frozen
		if char.frozen:
			char.socket.clilocmessage(502643)
			return 0
	
		# We are already casting a spell
		if 'magic' in eventlist or (socket and socket.hastag('cast_target')):
			char.socket.clilocmessage(502642)
			return 0

		# If we are using a spellbook to cast, check if we do have
		# the spell in our spellbook (0-based index)
		if not self.inherent and mode == 0 and not hasSpell(char, self.spellid - 1):
			char.message("You don't know the spell you want to cast.")
			return 0

		if not self.checkrequirements(char, mode, args):
			return 0
		
		# Unhide the Caster
		char.reveal()
		
		if self.mantra:
			char.say(self.mantra)
		
		# Precasting
		char.events = ['magic'] + eventlist
		char.action(ANIM_CASTAREA)
		char.addtimer(self.calcdelay(), 'magic.spell.callback', [self, mode, args], 0, 0, "cast_delay")	
		return 1

	def calcdelay(self):
		return 250 + (250 * self.circle)
	
	def checkrequirements(self, char, mode, args=[]):
		if char.dead:
			return 0

		if mode == MODE_BOOK:
			# Check for Mana
			if char.mana < self.mana:
				char.message(502625)
				return 0

			# Check for Reagents	
			if len(self.reagents) > 0:
				items = countReagents(char.getbackpack(), self.reagents.copy())
	
				for item in items.keys():
					if items[item] > 0:
						char.message(502630)
						return 0
	
		return 1

	def consumerequirements(self, char, mode, args=[]):
		# Check Basic Requirements before proceeding (Includes Death of Caster etc.)
		if not self.checkrequirements(char, mode):
			fizzle(char)
			return 0
	
		# Consume Mana
		if mode == MODE_BOOK:
			if self.mana != 0:
				char.mana -= self.mana
				char.updatemana()
	
			# Consume Reagents
			if len(self.reagents) > 0:
				consumeReagents(char.getbackpack(), self.reagents.copy())

		# Check Skill
		if self.skill != None:
			minskill = max(0, int((1000 / 7) * self.circle - 200))
			maxskill = min(1200, int((1000 / 7) * self.circle + 200))

			if not char.checkskill(self.skill, minskill, maxskill):
				char.message(502632)
				fizzle(char)
				return 0
			
		return 1
		
	# Not implemented yet
	def checkreflect(self, char, mode, targettype, target):
		return 0
	
	#
	# bonus Fixed bonus to the throw.
	# dice The number of dices to roll.
	# sides How many sides has each dice.	
	#
	def scaledamage(self, char, target, bonus, dice, sides):
		damage = rolldice(dice, sides, bonus) * 100.0
		
		bonus = char.skill[INSCRIPTION] / 100.0
		bonus += char.intelligence / 10.0
		bonus += properties.fromchar(char, SPELLDAMAGEBONUS)
		damage *= 1.0 + bonus / 100.0
		
		char.checkskill(self.damageskill, 0, 1200)
		damage *= (30 + (9 * char.skill[self.damageskill]) / 100.0) / 100.0

		return max(1, int(damage / 100.0))

	#
	# Calculate the chance the given target has to resist
	# this spell.
	#	
	def resistchance(self, char, target):
		basechance = target.skill[MAGICRESISTANCE] / 50.0
		evalchance = target.skill[MAGICRESISTANCE] / 10.0 - (((char.skill[self.skill] - 200) / 50.0) + self.circle * 5.0)
		if basechance > evalchance:
			return basechance / 2.0
		else:
			return evalchance / 2.0

	#
	# Check if the target is successful in resisting this spell
	# cast by char.
	#
	def checkresist(self, char, target):
		chance = self.resistchance(char, target)
		chance /= 100.0
		
		# No chance to resist
		if chance <= 0.0:
			return 0
			
		if chance >= 1.0:
			return 1
			
		maxskill = self.circle * 100
		maxskill += (1 + ((self.circle - 1) / 6)) * 250

		if target.skill[MAGICRESISTANCE] < maxskill:
			target.checkskill(MAGICRESISTANCE, 0, 1200)
			
		return chance >= random.random()

	def cast(self, char, mode, args=[]):
		if char.socket:	
			char.socket.settag('cast_target', 1)
			char.socket.attachtarget('magic.target_response', [ self, mode, args ], 'magic.target_cancel', 'magic.target_timeout', 8000) # Don't forget the timeout later on
		else:
			# Callback to the NPC AI ??
			wolfpack.console.log(LOG_ERROR, "A NPC is trying to cast a spell.")

	def target(self, char, mode, targettype, target, args=[]):
		raise Exception, "Spell without target method: " + str(self.__class__.__name__)
		
	#
	# Call this if you harm another character directly
	#
	def harmchar(self, char, victim):
		pass
		
class CharEffectSpell (Spell):
	def __init__(self, circle):
		Spell.__init__(self, circle)
		self.validtarget = TARGET_CHAR
		self.resistable = 1 # Most of them are resistable

	def effect(self, char, target):
		raise Exception, "CharEffectSpell with unimplemented effect method: " + str(self.__clas__.__name__)

	def target(self, char, mode, targettype, target, args=[]):
		if not self.consumerequirements(char, mode):
			return

		if not self.affectchar(char, mode, target):
			return 

		char.turnto(target)

		if self.reflectable and self.checkreflect(char, mode, targettype, target):
			target = char

		if self.harmful:
			self.harmchar(char, target)

		self.effect(char, target)

#
# A damage spell. It can be delayed but doesnt need
# to be.
#
class DelayedDamageSpell(CharEffectSpell):
	def __init__(self, circle):
		CharEffectSpell.__init__(self, circle)
		self.validtarget = TARGET_CHAR
		self.harmful = 1 # All of them are harmful
		self.missile = None # If set, a missile will be shot [ id, fixed-direction, explode, speed ]
		self.delay = 1000 # Default Delay
		self.sound = None
		
	def effect(self, char, target):
		# Shoot a missile?
		if self.missile:
			char.movingeffect(self.missile[0], target, self.missile[1], self.missile[2], self.missile[3])
	
		if self.sound:
			char.soundeffect(self.sound)
	
		# The damage will be dealt in one second
		if not self.delay:
			self.damage(char, target)
		else:
			target.addtimer(1000, 'magic.spell.damage_callback', [ self.spellid, char.serial ], 0, 0)

#
# Callback for delayed damage spells
#
def damage_callback(target, args):
	spell = magic.spells[ args[0] ]
	char = wolfpack.findchar(args[1])
	
	# Something went out of scope
	if not char or not spell:
		wolfpack.console.log(LOG_WARNING, "Either Caster or Spell went out of scope in damage_callback.\n")
		return

	spell.damage(char, target)
