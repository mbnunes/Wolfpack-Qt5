
from wolfpack.consts import *
from math import floor
from combat.weaponinfo import WEAPONINFO, WEAPON_RESNAME_BONI
from combat.armorinfo import ARMORINFO, ARMOR_RESNAME_BONI

#
# Get the delay for the next swing from this attacker and his weapon.
# The return value is in miliseconds.
#
def getdelay(attacker, weapon):
	speed = fromitem(weapon, SPEED)
	value = max(1, (attacker.stamina + 100) * speed)

	# Scale value according to bonus
	# value += bonus * value / 100

	return floor(40000.0 / value) * 500

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

	# % Boni
	DAMAGEBONUS: ['aos_boni_damage', 0, 1],
	SPEEDBONUS: ['aos_boni_speed', 0, 1],
	HITBONUS: ['aos_boni_hit_chance', 0, 1],
	DEFENSEBONUS: ['aos_defense_chance', 0, 1],
	SPELLDAMAGEBONUS: ['spelldamagebonus', 0, 1],

	# Regular Combat Properties
	MINDAMAGE: ['mindamage', 1, 0],
	MAXDAMAGE: ['maxdamage', 4, 0],
	SPEED: ['speed', 50, 0],
	HITSOUND: ['hitsound', [0x135, 0x137, 0x13b], 0],
	MISSSOUND: ['misssound', [0x238, 0x23a], 0],
	AMMUNITION: ['ammunition', '', 0],
	PROJECTILE: ['projectile', 0, 0],
	PROJECTILEHUE: ['projectilehue', 0, 0],
}

#
# Get a certain property for an item.
#
def fromitem(item, property):
	if not PROPERTIES.has_key(property):
		raise Exception, "Unknown property value %u" % property

	info = PROPERTIES[property]

	# Fists are no items.
	if not item:
		return info[1]

	# Tag overrides properties
	if item.hastag(info[0]):
		# HITSOUND and MISSSOUND need special treatment beacuse of
		# the list character.
		if property == HITSOUND or property == MISSSOUND:
			return str(item.gettag(info[0])).split(',')
		else:
			return int(item.gettag(info[0]))
	else:
		# See if our weapon info has anything for the
		# requested item. Otherwise return the default value.
		if itemcheck(item, ITEM_WEAPON):
			if WEAPONINFO.has_key(item.baseid):
				weaponinfo = WEAPONINFO[item.baseid]

				if weaponinfo.has_key(property):
					value = weaponinfo[property]
				else:
					value = info[1]

				# Resource dependant boni
				if item.hastag('resname'):
					resname = str(item.gettag('resname'))
					if WEAPON_RESNAME_BONI.has_key(resname):
						if WEAPON_RESNAME_BONI[resname].has_key(property):
							value += WEAPON_RESNAME_BONI[resname][property]

				if item.hastag('resname2'):
					resname = str(item.gettag('resname2'))
					if WEAPON_RESNAME_BONI.has_key(resname):
						if WEAPON_RESNAME_BONI[resname].has_key(property):
							value += WEAPON_RESNAME_BONI[resname][property]

				return value
		elif itemcheck(item, ITEM_ARMOR) or itemcheck(item, ITEM_SHIELD):
			if ARMORINFO.has_key(item.baseid):
				armorinfo = ARMORINFO[item.baseid]

				if armorinfo.has_key(property):
					value = armorinfo[property]
				else:
					value = info[1]

				# Resource dependant boni
				if item.hastag('resname'):
					resname = str(item.gettag('resname'))
					if ARMOR_RESNAME_BONI.has_key(resname):
						if ARMOR_RESNAME_BONI[resname].has_key(property):
							value += ARMOR_RESNAME_BONI[resname][property]

				if item.hastag('resname2'):
					resname = str(item.gettag('resname2'))
					if ARMOR_RESNAME_BONI.has_key(resname):
						if ARMOR_RESNAME_BONI[resname].has_key(property):
							value += ARMOR_RESNAME_BONI[resname][property]

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
		if char.hastag('mindamage') and char.hastag('maxdamage'):
			mindamage = int(char.gettag('mindamage'))
			maxdamage = int(char.gettag('maxdamage'))
			return (mindamage, maxdamage)

		# Special treatment for fists.
		if not weapon:
			mindamage = char.strength / 28
			maxdamage = mindamage + 7
		else:
			mindamage = fromitem(weapon, MINDAMAGE)
			maxdamage = fromitem(weapon, MAXDAMAGE)
	else:
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

	return 0
