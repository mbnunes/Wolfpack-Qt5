
from wolfpack.consts import *
from math import floor

# Weapon Skills
WEAPON_SKILLS = {
		# 1001: Sword Weapons (Swordsmanship)
		# 1002: Axe Weapons (Swordsmanship + Lumberjacking)
		1001: SWORDSMANSHIP,
		1002: SWORDSMANSHIP,

		# 1003: Macefighting (Staffs)
		# 1004: Macefighting (Maces/WarHammer)
		1003: MACEFIGHTING,
		1004: MACEFIGHTING,

		# 1005: Fencing
    1005: FENCING,

		# 1006: Bows
		# 1007: Crossbows
		1006: ARCHERY,
		1007: ARCHERY
}

#
# Returns the weapon skill used by the given character.
# If bestskill is 1 and the weapon has the given property.
# The best skill is used instead.
#
def weaponskill(char, weapon, bestskill = 0):
  if not weapon:
    return WRESTLING
  else:
    if not WEAPON_SKILLS.has_key(weapon.type):
      return WRESTLING
    else:
      return WEAPON_SKILLS[weapon.type]

#
# Get the delay for the next swing from this attacker and his weapon.
# The return value is in miliseconds.
#
def getdelay(attacker, weapon):
  if not weapon or not weapon.hastag('speed'):
    speed = 50
  else:
    speed = int(weapon.gettag('speed'))

  attacker.log(LOG_PYTHON, "Weapon Speed: " + str(speed) + "\n")

  value = max(1, (attacker.stamina + 100) * speed)

  # Scale value according to bonus
  # value += bonus * value / 100

  return floor(40000.0 / value) * 500

#
# This method calculates all required properties for a
# character status gump and returns them as a dictionary.
# Used keys:
# RESISTANCE_PHYSICAL
# RESISTANCE_ENERGY
# RESISTANCE_COLD
# RESISTANCE_POISON
# RESISTANCE_FIRE
# DAMAGEBONUS
# SPEEDBONUS
# HITBONUS
#
def fromchar(char):
  properties = {
    RESISTANCE_PHYSICAL: 0,
    RESISTANCE_ENERGY: 0,
    RESISTANCE_COLD: 0,
    RESISTANCE_POISON: 0,
    RESISTANCE_FIRE: 0,
    DAMAGEBONUS: 100,
    SPEEDBONUS: 100,
    HITBONUS: 100
  }

  # Now get all required properties from equipped items first
  for layer in range(LAYER_RIGHTHAND, LAYER_LEGS+1):
    item = char.itemonlayer(layer)

    if item:
      itemprops = fromitem(item)
      properties[RESISTANCE_PHYSICAL] += itemprops[RESISTANCE_PHYSICAL]
      properties[RESISTANCE_ENERGY] += itemprops[RESISTANCE_ENERGY]
      properties[RESISTANCE_COLD] += itemprops[RESISTANCE_COLD]
      properties[RESISTANCE_POISON] += itemprops[RESISTANCE_POISON]
      properties[RESISTANCE_FIRE] += itemprops[RESISTANCE_FIRE]
      properties[DAMAGEBONUS] += itemprops[DAMAGEBONUS]
      properties[SPEEDBONUS] += itemprops[SPEEDBONUS]
      properties[HITBONUS] += itemprops[HITBONUS]

  return properties

#
# This function returns the properties of the given item
# in a dictionary.
#
def fromitem(item):
  properties = {}

  # Physical Resistance
  if item and item.hastag('res_physical'):
    properties[RESISTANCE_PHYSICAL] = int(item.gettag('res_physical'))
  else:
    properties[RESISTANCE_PHYSICAL] = 0

  # Fire Resistance
  if item and item.hastag('res_fire'):
    properties[RESISTANCE_FIRE] = int(item.gettag('res_fire'))
  else:
    properties[RESISTANCE_FIRE] = 0

  # Cold Resistance
  if item and item.hastag('res_cold'):
    properties[RESISTANCE_COLD] = int(item.gettag('res_cold'))
  else:
    properties[RESISTANCE_COLD] = 0

  # Poison Resistance
  if item and item.hastag('res_poison'):
    properties[RESISTANCE_POISON] = int(item.gettag('res_poison'))
  else:
    properties[RESISTANCE_POISON] = 0

  # Energy Resistance
  if item and item.hastag('res_energy'):
    properties[RESISTANCE_ENERGY] = int(item.gettag('res_energy'))
  else:
    properties[RESISTANCE_ENERGY] = 0

  # Physical Damage
  if item and item.hastag('dmg_physical'):
    properties[DAMAGE_PHYSICAL] = int(item.gettag('dmg_physical'))
  else:
    properties[DAMAGE_PHYSICAL] = 0

  # Fire Damage
  if item and item.hastag('dmg_fire'):
    properties[DAMAGE_FIRE] = int(item.gettag('dmg_fire'))
  else:
    properties[DAMAGE_FIRE] = 0

  # Cold Damage
  if item and item.hastag('dmg_cold'):
    properties[DAMAGE_COLD] = int(item.gettag('dmg_cold'))
  else:
    properties[DAMAGE_COLD] = 0

  # Poison Damage
  if item and item.hastag('dmg_poison'):
    properties[DAMAGE_POISON] = int(item.gettag('dmg_poison'))
  else:
    properties[DAMAGE_POISON] = 0

  # Energy Damage
  if item and item.hastag('dmg_energy'):
    properties[DAMAGE_ENERGY] = int(item.gettag('dmg_energy'))
  else:
    properties[DAMAGE_ENERGY] = 0
   
  # Extra % Damage
  if item and item.hastag('aos_boni_damage'):
    properties[DAMAGEBONUS] = int(item.gettag('aos_boni_damage'))
  else:
    properties[DAMAGEBONUS] = 0

  # Extra % Speed
  if item and item.hastag('aos_boni_swing_speed'):
    properties[SPEEDBONUS] = int(item.gettag('aos_boni_speed'))
  else:
    properties[SPEEDBONUS] = 0

  # Extra % Hit Chance
  if item and item.hastag('aos_boni_hit_chance'):
    properties[HITBONUS] = int(item.gettag('aos_boni_hit_chance'))
  else:
    properties[HITBONUS] = 0

  if item and item.hastag('mindamage'):
    properties[MINDAMAGE] = int(item.gettag('mindamage'))
  else:
    properties[MINDAMAGE] = 1 # Wrestling Mindamage

  if item and item.hastag('maxdamage'):
    properties[MAXDAMAGE] = int(item.gettag('maxdamage'))
  else:
    properties[MAXDAMAGE] = 4 # Wrestling Maxdamage

  if item and item.hastag('speed'):
    properties[SPEED] = int(item.gettag('speed'))
  else:
    properties[SPEED] = 50 # Wrestling Speed

  return properties
