
import magic.spellbook
from wolfpack.utilities import *
from wolfpack import properties
from wolfpack.consts import EVALUATINGINTEL, MAGICRESISTANCE \

TARGET_CHAR = 1
TARGET_ITEM = 2
TARGET_GROUND = 3
TARGET_IGNORE = 4

# Common Reagents
REAGENT_BLACKPEARL = 'f7a'
REAGENT_BLOODMOSS = 'f7b'
REAGENT_GARLIC = 'f84'
REAGENT_GINSENG = 'f85'
REAGENT_MANDRAKE = 'f86'
REAGENT_NIGHTSHADE = 'f88'
REAGENT_SULFURASH = 'f8c'
REAGENT_SPIDERSILK = 'f8d'

# Rarer Reagents
REAGENT_BATWING = 'f78'
REAGENT_BLACKMOOR = 'f79'
REAGENT_BLOODSPAWN = 'f7c'
REAGENT_DAEMONBLOOD = 'f7d'
REAGENT_BONE = 'f7e'
REAGENT_BRIMSTONE = 'f7f'
REAGENT_DAEMONBONE = 'f80'
REAGENT_FERTILEDIRT = 'f81'
REAGENT_DRAGONSBLOOD = 'f82'
REAGENT_EXECUTIONERSCAP = 'f83'
REAGENT_EYEOFNEWT = 'f87'
REAGENT_OBSIDIAN = 'f89'
REAGENT_PIGIRON = 'f8a'
REAGENT_PUMICE = 'f8b'
REAGENT_NOXCRYSTAL = 'f8e'
REAGENT_GRAVEDUST = 'f8f'
REAGENT_DEADWOOD = 'f90'
REAGENT_WYRMSHEART = 'f91'

# Casting modes
MODE_BOOK = 0
MODE_SCROLL = 1
MODE_WAND = 2
MODE_CMD = 3

#
# Remove a possible timer/targetrequest
#
def fizzle(char):
	char.dispel(char, 1, 'cast_delay')

	char.removescript('magic')

	if char.socket:
		char.socket.deltag('cast_target')

	char.effect(0x3735, 1, 30)
	char.soundeffect(0x5c)

# Check whether the spellbook's of a char have that specific spell
def hasSpell(char, spell):
	book = char.itemonlayer(1)

	if magic.spellbook.hasspell(book, spell):
		return True

	for book in char.getbackpack().content:
		if magic.spellbook.hasspell(book, spell):
			return True

	return False

#
# A general purpose function for chaning some users stats
# This is shared among: curse, bless, all single stat modifying spells
#
def statmodifier_dispel(char, args, source, dispelargs):
	stat = args[0]
	amount = args[1]

	if stat == 0:
		char.strength2 -= amount
		if char.strength - amount > 0:
			char.strength -= amount
		else:
			char.strength = 1
		char.hitpoints = min(char.hitpoints, char.maxhitpoints)

	elif stat == 1:
		char.dexterity2 -= amount
		if char.dexterity - amount > 0:
			char.dexterity -= amount
		else:
			char.dexterity = 1
		char.stamina = min(char.stamina, char.maxstamina)

	elif stat == 2:
		char.intelligence2 -= amount
		if char.intelligence - amount > 0:
			char.intelligence -= amount
		else:
			char.intelligence = 1
		char.mana = min(char.mana, char.maxmana)

	# If we're not dispelling it silently, update
	# health and stats
	if not "silent" in dispelargs:
		if stat == 0:
			char.updatehealth()
		char.updatestats()

#
# A stat enhancing spell has expired
#
def statmodifier_expire(char, args):
	stat = args[0]
	amount = args[1]

	if stat == 0:
		char.strength2 -= amount
		if char.strength - amount > 0:
			char.strength -= amount
		else:
			char.strength = 1
		char.hitpoints = min(char.hitpoints, char.maxhitpoints)
		char.updatehealth()

	elif stat == 1:
		char.dexterity2 -= amount
		if char.dexterity - amount > 0:
			char.dexterity -= amount
		else:
			char.dexterity = 1
		char.stamina = min(char.stamina, char.maxstamina)

	elif stat == 2:
		char.intelligence2 -= amount
		if char.intelligence - amount > 0:
			char.intelligence -= amount
		else:
			char.intelligence = 1
		char.mana = min(char.mana, char.maxmana)

	char.updatestats()

#
# Statmodifier including given percentage
#
def statmodifier_raw(target, stat, curse, percent, duration):
	amount1 = int((target.strength - target.strength2) * percent)
	amount2 = int((target.dexterity - target.dexterity2) * percent)
	amount3 = int((target.intelligence - target.intelligence2) * percent)

	# Reverse if it's a curse
	if curse:
		amount1 *= -1
		amount2 *= -1
		amount3 *= -1

	if stat == 0 or stat == 3:
		target.dispel(None, 0, "magic_statmodifier_0", ["silent"])

		# Adjust amount
		if target.strength + amount1 < 1:
			amount1 = -(target.strength - 1)
		target.strength2 += amount1
		target.strength += amount1
		target.hitpoints = min(target.hitpoints, target.maxhitpoints)
		target.updatehealth()

		target.addtimer(duration, "magic.utilities.statmodifier_expire", [0, amount1], \
			1, 1, "magic_statmodifier_0", "magic.utilities.statmodifier_dispel")

	if stat == 1 or stat == 3:
		target.dispel(None, 0, "magic_statmodifier_1", ["silent"])

		if target.dexterity + amount2 < 1:
			amount2 = -(target.dexterity - 1)
		target.dexterity2 += amount2
		target.dexterity += amount2
		target.stamina = min(target.stamina, target.maxstamina)

		target.addtimer(duration, "magic.utilities.statmodifier_expire", [1, amount2], \
			1, 1, "magic_statmodifier_1", "magic.utilities.statmodifier_dispel")

	if stat == 2 or stat == 3:
		target.dispel(None, 0, "magic_statmodifier_2", ["silent"])

		if target.intelligence + amount3 < 1:
			amount3 = -(target.intelligence - 1)
		target.intelligence2 += amount3
		target.intelligence += amount3
		target.mana = min(target.mana, target.maxmana)

		target.addtimer(duration, "magic.utilities.statmodifier_expire", [2, amount3], \
			1, 1, "magic_statmodifier_2", "magic.utilities.statmodifier_dispel")

	target.updatestats()

#
# Apply a stat modifying spell
#
def statmodifier(char, target, stat, curse):
	# Calculate the amount
	char.checkskill(EVALUATINGINTEL, 0, 1200)
	if curse:
		target.checkskill(MAGICRESISTANCE, 0, 1200)

	if curse:
		percent = 8 + (char.skill[EVALUATINGINTEL] - target.skill[MAGICRESISTANCE]) / 100.0
	else:
		percent = 1 + char.skill[EVALUATINGINTEL] / 100.0

	percent = max(0, percent / 100)

	# Calculate the duration
	duration = (char.skill[EVALUATINGINTEL] / 50 + 1) * 6000

	statmodifier_raw(target, stat, curse, percent, duration)

#
# When a fieldeffect spell expires.
#
def field_expire(object, args):
	for serial in args:
		item = wolfpack.finditem(serial)
		if item:
			item.delete()
