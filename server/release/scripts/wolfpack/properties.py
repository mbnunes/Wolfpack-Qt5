
from math import floor, sqrt, ceil
import random
#from wolfpack import weaponinfo
#from wolfpack import armorinfo
import wolfpack.armorinfo
import wolfpack.weaponinfo
from wolfpack import console
from wolfpack.consts import *
from system.lootlists import *

#
# Get the delay for the next swing from this attacker and his weapon.
# The return value is in miliseconds.
#
def getdelay(attacker, weapon):
	speed = fromitem(weapon, SPEED)	
	
	value = (attacker.stamina + 100) * speed

	bonus = fromchar(attacker, SPEEDBONUS)

	# Scale value according to bonus		
	value += bonus * value / 100

	if value <= 0:
		value = 1

	return floor(40000.0 / float(value)) * 500

#
# Known item properties and the tags they are stored in.
# The default value is saved here too (Wrestling values).
# The third value specifies whether the value is cumulative
# and can be retrieved from characters.
#
PROPERTIES = {
	# Resistances
	RESISTANCE_PHYSICAL: ['res_physical', 0, 1],
	RESISTANCE_FIRE: ['res_fire', 0, 1],
	RESISTANCE_COLD: ['res_cold', 0, 1],
	RESISTANCE_POISON: ['res_poison', 0, 1],
	RESISTANCE_ENERGY: ['res_energy', 0, 1],

	# Damage Distribution (only weapons)
	DAMAGE_PHYSICAL: ['dmg_physical', 100, 0],
	DAMAGE_FIRE: ['dmg_fire', 0, 0],
	DAMAGE_COLD: ['dmg_cold', 0, 0],
	DAMAGE_POISON: ['dmg_poison', 0, 0],
	DAMAGE_ENERGY: ['dmg_energy', 0, 0],

	# Flags (Weapons):
	SPELLCHANNELING: ['spellchanneling', 0, 0],
	MAGEARMOR: ['magearmor', 0, 0],
	
	# % Boni
	LOWERREQS: ['lower_reqs', 0, 1],
	DAMAGEBONUS: ['aos_boni_damage', 0, 1],
	SPEEDBONUS: ['aos_boni_speed', 0, 1],
	HITBONUS: ['aos_boni_hit_chance', 0, 1],
	DEFENSEBONUS: ['aos_defense_chance', 0, 1],
	SPELLDAMAGEBONUS: ['spelldamagebonus', 0, 1],
	CASTRECOVERYBONUS: ['castrecoverybonus', 0, 1],
	CASTSPEEDBONUS: ['castspeedbonus', 0, 1],
	LOWERMANACOST: ['lowermanacost', 0, 1],
	LOWERREAGENTCOST: ['lowerreagentcost', 0, 1],

	# Bonus Hitpoints etc.
	BONUSHITPOINTS: ['bonushitpoints', 0, 1],
	BONUSSTAMINA: ['bonusstamina', 0, 1],
	BONUSMANA: ['bonusmana', 0, 1],
	BONUSSTRENGTH: ['bonusstrength', 0, 1],
	BONUSDEXTERITY: ['bonusdexterity', 0, 1],
	BONUSINTELLIGENCE: ['bonusintelligence', 0, 1],

	# Regeneration rates
	REGENHITPOINTS: ['regenhitpoints', 0, 1],
	REGENSTAMINA: ['regenstamina', 0, 1],
	REGENMANA: ['regenmana', 0, 1],
	
	# Misc
	LUCK: ['luck', 0, 1],
	SELFREPAIR: ['selfrepair', 0, 0],
	ENHANCEPOTIONS: ['enhancepotions', 0, True],

	# Requirements
	REQSTR: ['req_strength', 0, 0],
	REQDEX: ['req_dexterity', 0, 0],
	REQINT: ['req_intelligence', 0, 0],

	# Regular Combat Properties
	BESTSKILL: ['bestskill', 0, 0],
	MAGEWEAPON: ['mageweapon', 0, 0],
	MINDAMAGE: ['mindamage', 1, 0],
	MAXDAMAGE: ['maxdamage', 4, 0],
	SPEED: ['speed', 50, 0],
	HITSOUND: ['hitsound', [0x135, 0x137, 0x13b], 0],
	MISSSOUND: ['misssound', [0x238, 0x23a], 0],
	AMMUNITION: ['ammunition', '', 0],
	PROJECTILE: ['projectile', 0, 0],
	PROJECTILEHUE: ['projectilehue', 0, 0],
	SWING: ['swing', [0xa, 0x9, 0x1f] ],
	REFLECTPHYSICAL: ['reflectphysical', 0, 1],
	DURABILITYBONUS: ['durabilitybonus', 0, 0],
	WEIGHTBONUS: ['weightbonus', 0, 0],
}

#
# Get a certain property for an item.
#
def fromitem(item, property):
	if property == MAGEARMOR and item.allowmeditation:
		return True
	
	if not PROPERTIES.has_key(property):
		raise Exception, "Unknown property value %u" % property

	info = PROPERTIES[property]

	# Fists are no items.
	if not item:
		return info[1]

	# Tags Always go first, override system.
	if item.hastag(info[0]):
		# HITSOUND and MISSSOUND need special treatment beacuse of
		# the list character.
		if property in [HITSOUND, MISSSOUND, SWING]:
			return str(item.gettag(info[0])).split(',')
		elif property == AMMUNITION:
			return str(item.gettag(info[0]))
		else:
			return int(item.gettag(info[0]))

	else:
		# Take three properties from the hardcoded property list for weapons
		if property in [HITSOUND, MISSSOUND, SWING]:
			if not wolfpack.weaponinfo.WEAPONINFO.has_key(item.baseid):
				return info[1]
			else:
				return wolfpack.weaponinfo.WEAPONINFO[item.baseid][property]
	
		# Get the base property from the item
		if type(info[1]) == int:
			value = item.getintproperty( info[0], info[1] )
		elif type(info[1]) == str:
			value = item.getstrproperty( info[0], info[1] )
		else:
			raise Exception, "Unsupported property type %s for property %u." % (type(info[1]).__name__, property)	

		resboni = None

		# Get Resname boni from the weapon list.
		if itemcheck(item, ITEM_WEAPON):
			resboni = wolfpack.weaponinfo.WEAPON_RESNAME_BONI
		
		# Get Resname boni from the ARMOR list
		elif itemcheck(item, ITEM_ARMOR) or itemcheck(item, ITEM_SHIELD):
			resboni = wolfpack.armorinfo.ARMOR_RESNAME_BONI

		if resboni and item.hastag('resname'):
			resname = str(item.gettag('resname'))
			if resboni.has_key(resname) and resboni[resname].has_key(property):
				value += resboni[resname][property]
				
		if resboni and item.hastag('resname2'):
			resname = str(item.gettag('resname2'))
			if resboni.has_key(resname) and resboni[resname].has_key(property):
				value += resboni[resname][property]

		return value

	return info[1]

#
# Calculates a certain property for the character by
# recursing trough all of his equipment.
#
def fromchar(char, property):
	# Check for unknown property
	if not PROPERTIES.has_key(property):
		raise Exception, "Unknown property value %u" % property

	info = PROPERTIES[property]

	value = 0 # Default
	minvalue = 0
	resistance = 0

	# Special calculations for resistance properties
	# They depend on Magic Resistance
	if property in [RESISTANCE_COLD, RESISTANCE_ENERGY, RESISTANCE_FIRE, RESISTANCE_PHYSICAL, RESISTANCE_POISON]:
		resistance = 1
		magicresist = char.skill[MAGICRESISTANCE]

		if magicresist >= 1000:
			minvalue = int(40 + (magicresist - 1000) / 50)
		elif magicresist >= 400:
			minvalue = int((magicresist - 400) / 15)

	# See if the character is a npc and has the value
	# Then it overrides any items
	if char.hastag(info[0]):
		return int(char.gettag(info[0]))

	for layer in range(LAYER_RIGHTHAND, LAYER_MOUNT):
		item = char.itemonlayer(layer)

		if item:
			value += fromitem(item, property)

	# Min + Max values for players
	if resistance:
		if value < minvalue:
			value = minvalue

		# Reactive Armor
		if char.propertyflags & 0x10000:
			if property == RESISTANCE_PHYSICAL:
				value += 15 + char.skill[INSCRIPTION] / 200
			else:
				value = max(0, value - 5)

		# Protection
		if char.propertyflags & 0x20000:
			if property == RESISTANCE_PHYSICAL:
				value = max(0, value - (15 - char.skill[INSCRIPTION] / 200))

		# Magic Reflect
		if char.propertyflags & 0x40000:
			if property == RESISTANCE_PHYSICAL:
				value = max(0, value - (25 - char.skill[INSCRIPTION] / 200))
			else:
				value += 10

		if value > 70:
			value = 70

	return value

#
# Computes the damage a certain character
# deals.
#
def getdamage(char):
	weapon = char.getweapon()

	# See if it's a npc with special min and maxdamage settings
	if char.npc:
		# If the npc has mindamage and maxdamage tags, they
		# override all other settings
		mindamage = char.mindamage
		maxdamage = char.maxdamage
		
		try:
			if char.hastag('mindamage'):
				mindamage = int(char.gettag('mindamage'))
			
			if char.hastag('maxdamage'):
				maxdamage = int(char.gettag('maxdamage'))
		except:
			pass

		# Special treatment for fists.
		if not weapon:
			# Use the basedef values.
			if maxdamage == 0:
				mindamage = char.strength / 28
				maxdamage = mindamage + 7
		else:
			mindamage = fromitem(weapon, MINDAMAGE)
			maxdamage = fromitem(weapon, MAXDAMAGE)
			
		return (mindamage, maxdamage)
	else:
		if not weapon and char.maxdamage != 0:
			return (char.mindamage, char.maxdamage)

		mindamage = fromitem(weapon, MINDAMAGE)
		maxdamage = fromitem(weapon, MAXDAMAGE)

	return (mindamage, maxdamage)

#
# See if the given item has a specific property
# This is used more as a type check.
# To see if an item is an armor, a bashing weapon, a ranged weapon. etc.
#
def itemcheck(item, check):
	if not item:
		return 0

	# No real check for this yet
	if check == ITEM_ARMOR:
		return item.type == 1009

	# Only type check yet.
	if check == ITEM_SHIELD:
		return item.type == 1008

	# Only type check yet.
	if check == ITEM_WEAPON:
		return item.type >= 1001 and item.type <= 1007

	# Only type check yet.
	if check == ITEM_MELEE:
		return item.type >= 1001 and item.type <= 1005

	# Only type check yet.
	if check == ITEM_RANGED:
		return item.type == 1006 or item.type == 1007

	# Check weapon combat skill : only type check
	if check == ITEM_SLASHING:
		return item.type == 1001 or item.type == 1002
	if check == ITEM_BASHING:
		return item.type == 1003 or item.type == 1004
	if check == ITEM_PIERCING:
		return item.type == 1005

	return 0

LUCKTABLE = []
# Generate lucktable
p = 1.0 / 1.8
for i in range(0, 1201):
	LUCKTABLE.append(int(pow(i, p) * 100))

# 0, 1, 2, 3, 4, 5 properties chances
CHANCES = [
	[3, 1, 0, 0, 0, 0], # 1 max property
	[6, 3, 1, 0, 0, 0], # 2 max properties
	[10, 6, 3, 1, 0, 0], # 3 max properties
	[16, 12, 6, 5, 1, 0], # 4 max properties
	[30, 25, 20, 15, 9, 1], # 5 max properties
]

#
# How many properties does the item actually have.
#
def bonusProps(maxprops, luckChance):
	if maxprops > 5:
		maxprops = 5
		
	if maxprops < 1:
		return 0
		
	maxprops -= 1
			
	chances = CHANCES[maxprops]
	chancesum = sum(chances)
	
	result = 0
	
	rnd = random.randrange(0, chancesum)
	for i in [5, 4, 3, 2, 1, 0]:
		if rnd < chances[i]:
			result = i
			break
		else:
			rnd -= chances[i]
			
	# Take luck into account
	if result < maxprops and luckChance > random.randint(0, 10000):
		result += 1

	return result

#
# Scale a property value based on luck and intensity and a custom scale factor
#
def scaleValue( minimum, maximum, propmin, propmax, scale, luckchance ):
	if scale != 1:
		propmin = propmin / float(scale)
		propmax = propmax / float(scale)
	
	# Although this is the "Worst system ever" (c) RunUO
	# I think the basic idea of having the chance increase slower than
	# using a normal rand function is good.	
	percent = 100 - sqrt(random.randint(0, 10000)) # Range of the property value
	
	if luckchance > random.randint(0, 10000):
		percent += 10
		
	# Normalize the percentage
	percent = min(maximum, max(minimum, percent))

	if scale != 1:
		return int(propmin + float(propmax - propmin) * (percent / 100.0)) * scale
	else:
		return int(propmin + float(propmax - propmin) * (percent / 100.0))

# List of allowed properties
ARMOR_PROPERTIES = {
	# PROPERT KEY, min value, max value, factor, accumulate
	LOWERREQS: [10, 100, 10, True],
	SELFREPAIR: [1, 5, 1, False],
	DURABILITYBONUS: [10, 100, 10, True],
	MAGEARMOR: [1, 1, 1, False],
	REGENHITPOINTS: [1, 2, 1, False],
	REGENSTAMINA: [1, 3, 1, False],
	REGENMANA: [1, 2, 1, False],
	BONUSHITPOINTS: [1, 5, 1, False],
	BONUSSTAMINA: [1, 8, 1, False],
	BONUSMANA: [1, 8, 1, False],
	LOWERMANACOST: [1, 8, 1, False],
	LOWERREAGENTCOST: [1, 20, 1, False],
	LUCK: [1, 100, 1, True],
	REFLECTPHYSICAL: [1, 15, 1, False],
	RESISTANCE_PHYSICAL: [1, 15, 1, True],
	RESISTANCE_FIRE: [1, 15, 1, True],
	RESISTANCE_COLD: [1, 15, 1, True],
	RESISTANCE_POISON: [1, 15, 1, True],
	RESISTANCE_ENERGY: [1, 15, 1, True],
}

#
#  Apply a random armor property
#
def applyArmorRandom(item, props, minintensity, maxintensity, luckchance):
	properties = ARMOR_PROPERTIES.keys()
	
	# Remove MageArmor from the list if it's already on the item
	magearmor = fromitem(item, MAGEARMOR)
	if magearmor:
		properties.remove(MAGEARMOR)

	# Select unique properties
	for i in range(0, props):
		property = random.choice(properties)
		properties.remove(property)
		
		if not PROPERTIES.has_key(property):
			continue

		# Scale the value for the property
		info = ARMOR_PROPERTIES[property]
		value = scaleValue(minintensity, maxintensity, info[0], info[1], info[2], luckchance)

		# Some special handling for special boni
		if property == DURABILITYBONUS:
			bonus = int(ceil(item.maxhealth * (value / 100.0)))
			item.maxhealth = max(1, item.maxhealth + bonus)
			item.health = item.maxhealth

		# Resistances are cummulative
		if info[3]:
			value += fromitem(item, property)		
		
		item.settag(PROPERTIES[property][0], value)	

	item.resendtooltip()

# List of allowed properties
SHIELD_PROPERTIES = {
	# PROPERT KEY, min value, max value, factor, accumulate
	SPELLCHANNELING: [1, 1, 1, False],
	DEFENSEBONUS: [1, 15, 1, True],
	HITBONUS: [1, 15, 1, True],
	CASTSPEEDBONUS: [1, 1, 1, True],
	LOWERREQS: [10, 100, 10, True],
	SELFREPAIR: [1, 5, 1, False],
	DURABILITYBONUS: [10, 100, 10, True],
}

#
#  Apply a random shield property
#
def applyShieldRandom(item, props, minintensity, maxintensity, luckchance):
	properties = SHIELD_PROPERTIES.keys()

	# Select unique properties
	for i in range(0, props):
		property = random.choice(properties)
		properties.remove(property)
		
		if not PROPERTIES.has_key(property):
			continue

		# Scale the value for the property
		info = SHIELD_PROPERTIES[property]
		value = scaleValue(minintensity, maxintensity, info[0], info[1], info[2], luckchance)

		# Some special handling for special boni
		if property == DURABILITYBONUS:
			bonus = int(ceil(item.maxhealth * (value / 100.0)))
			item.maxhealth = max(1, item.maxhealth + bonus)
			item.health = item.maxhealth

		# Set cast speed to -1
		elif property == SPELLCHANNELING:
			item.settag(PROPERTIES[CASTSPEEDBONUS][0], -1)

		# Resistances are cummulative
		if info[3]:
			value += fromitem(item, property)		
		
		item.settag(PROPERTIES[property][0], value)	

	item.resendtooltip()

# Those are no real properties
SKILLBONUS1 = 1000
SKILLBONUS2 = 1001
SKILLBONUS3 = 1002
SKILLBONUS4 = 1003
SKILLBONUS5 = 1004

# List of allowed properties
JUWEL_PROPERTIES = {
	# PROPERT KEY, min value, max value, factor, accumulate
	RESISTANCE_PHYSICAL: [1, 15, 1, False],
	RESISTANCE_FIRE: [1, 15, 1, False],
	RESISTANCE_COLD: [1, 15, 1, False],
	RESISTANCE_POISON: [1, 15, 1, False],
	RESISTANCE_ENERGY: [1, 15, 1, False],	
	DAMAGEBONUS: [1, 25, 1, False],
	DEFENSEBONUS: [1, 15, 1, False],
	HITBONUS: [1, 15, 1, False],
	BONUSSTRENGTH: [1, 8, 1, False],
	BONUSDEXTERITY: [1, 8, 1, False],
	BONUSINTELLIGENCE: [1, 8, 1, False],
	ENHANCEPOTIONS: [5, 25, 5, False],
	CASTSPEEDBONUS: [1, 1, 1, False],
	CASTRECOVERYBONUS: [1, 3, 1, False],
	LOWERMANACOST: [1, 8, 1, False],
	LOWERREAGENTCOST: [1, 20, 1, False],
	LUCK: [1, 100, 1, False],
	SPELLDAMAGEBONUS: [1, 12, 1, False],
	SKILLBONUS1: [1, 15, 1, False],
	SKILLBONUS2: [1, 15, 1, False],
	SKILLBONUS3: [1, 15, 1, False],
	SKILLBONUS4: [1, 15, 1, False],
	SKILLBONUS5: [1, 15, 1, False],
}

def applyJuwelRandom(item, props, minintensity, maxintensity, luckchance):
	properties = JUWEL_PROPERTIES.keys()
	
	# Possible bonus skills
	skills = [SWORDSMANSHIP, FENCING, MACEFIGHTING, ARCHERY, WRESTLING, PARRYING, TACTICS, ANATOMY, HEALING, MAGERY, MEDITATION, EVALUATINGINTEL,
				MAGICRESISTANCE, TAMING, ANIMALLORE, VETERINARY, MUSICIANSHIP, PROVOCATION, ENTICEMENT, PEACEMAKING, CHIVALRY, FOCUS, NECROMANCY,
				STEALING, STEALTH, SPIRITSPEAK]

	# Select unique properties
	for i in range(0, props):
		property = random.choice(properties)
		properties.remove(property)
		
		if property in [SKILLBONUS1, SKILLBONUS2, SKILLBONUS3, SKILLBONUS4, SKILLBONUS5]:
			# Apply a random skillbonus
			info = JUWEL_PROPERTIES[property]
			value = scaleValue(minintensity, maxintensity, info[0], info[1], info[2], luckchance)
			
			skill = random.choice(skills)
			skills.remove(skill)
			
			item.settag('skillbonus_%u' % (property - 1000), '%u,%u' % (skill, value * 10))
			continue
		
		if not PROPERTIES.has_key(property):
			continue

		# Scale the value for the property
		info = JUWEL_PROPERTIES[property]
		value = scaleValue(minintensity, maxintensity, info[0], info[1], info[2], luckchance)

		# Resistances are cummulative
		if info[3]:
			value += fromitem(item, property)		
		
		item.settag(PROPERTIES[property][0], value)

	item.resendtooltip()

# List of allowed properties
WEAPON_PROPERTIES = {
	# PROPERT KEY, min value, max value, factor, accumulate
	#SPLASH_PHYSICAL: [2, 50, 2, False],
	#SPLASH_FIRE: [2, 50, 2, False],
	#SPLASH_COLD: [2, 50, 2, False],
	#SPLASH_POISON: [2, 50, 2, False],
	#SPLASH_ENERGY: [2, 50, 2, False],
	
	#HIT_MAGICARROW: [2, 50, 2, False],
	#HIT_HARM: [2, 50, 2, False],
	#HIT_FIREBALL: [2, 50, 2, False],
	#HIT_LIGHTNING: [2, 50, 2, False],
	
	#HIT_DISPEL: [2, 50, 2, False],
	#HIT_LEECHHITS: [2, 50, 2, False],
	#HIT_LEECHSTAMINA: [2, 50, 2, False],
	#HIT_LEECHMANA: [2, 50, 2, False],
	#HIT_LOWERATTACK: [2, 50, 2, False],
	#HIT_LOWERDEFEND: [2, 50, 2, False],
	
	BESTSKILL: [1, 1, 1, False],
	#MAGEWEAPON: [29, 20, 1, False],

	RESISTANCE_PHYSICAL: [1, 15, 1, True],
	RESISTANCE_FIRE: [1, 15, 1, True],
	RESISTANCE_COLD: [1, 15, 1, True],
	RESISTANCE_POISON: [1, 15, 1, True],
	RESISTANCE_ENERGY: [1, 15, 1, True],	
	DAMAGEBONUS: [1, 25, 1, True],
	DEFENSEBONUS: [1, 15, 1, True],
	HITBONUS: [1, 15, 1, True],
	CASTSPEEDBONUS: [1, 1, 1, False],
	LUCK: [1, 100, 1, False],
	SPEEDBONUS: [5, 30, 5, False],
	SPELLCHANNELING: [1, 1, 1, False],
	SPELLDAMAGEBONUS: [1, 12, 1, False],
	DURABILITYBONUS: [10, 100, 10, True],
	LOWERREQS: [10, 100, 10, True],
}

def applyWeaponRandom(item, props, minintensity, maxintensity, luckchance):
	properties = WEAPON_PROPERTIES.keys()
	
	# No bestskill property for ranged weapons
	if itemcheck(item, ITEM_RANGED):
		properties.remove(BESTSKILL)

	# Select unique properties
	for i in range(0, props):
		property = random.choice(properties)
		properties.remove(property)
		
		if not PROPERTIES.has_key(property):
			continue

		# Scale the value for the property
		info = WEAPON_PROPERTIES[property]
		value = scaleValue(minintensity, maxintensity, info[0], info[1], info[2], luckchance)

		# Some special handling for special boni
		if property == DURABILITYBONUS:
			bonus = int(ceil(item.maxhealth * (value / 100.0)))
			item.maxhealth = max(1, item.maxhealth + bonus)
			item.health = item.maxhealth

		# Set cast speed to -1
		elif property == SPELLCHANNELING:
			item.settag(PROPERTIES[CASTSPEEDBONUS][0], -1)				
			
		# MageWeapon and BestSkill are exclusive
		elif property == BESTSKILL:
			if MAGEWEAPON in properties:
				properties.remove(MAGEWEAPON)
		elif property == MAGEWEAPON:
			if BESTSKILL in properties:
				properties.remove(BESTSKILL)

		# Resistances are cummulative
		if info[3]:
			value += fromitem(item, property)		
		
		item.settag(PROPERTIES[property][0], value)	

	item.resendtooltip()

#
# Apply random properties to the given items.
#
def applyRandom(item, maxprops, minintensity, maxintensity, luckchance=0 ):
	props = 1 + bonusProps(maxprops, luckchance)
	
	if itemcheck(item, ITEM_ARMOR):
		applyArmorRandom(item, props, minintensity, maxintensity, luckchance)
		
	elif itemcheck(item, ITEM_SHIELD):
		applyShieldRandom(item, props, minintensity, maxintensity, luckchance)

	elif itemcheck(item, ITEM_WEAPON):
		applyWeaponRandom(item, props, minintensity, maxintensity, luckchance)
		
	else:
		applyJuwelRandom(item, props, minintensity, maxintensity, luckchance)

#
# Get the luckchance for a char
#
def luckchance(char):
	luck = fromchar(char, LUCK)
	
	if luck < 0:
		luck = 0

	if luck > 1200:
		luck = 1200
		
	return LUCKTABLE[luck]
	