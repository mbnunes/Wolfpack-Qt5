
import random
import wolfpack
import combat.properties
from wolfpack.utilities import hex2dec
from wolfpack.consts import *

ONEHANDED_SWING = 1
TWOHANDED_SWING = 2
SKILL = 3

#
# Static information for certain types of weapons.
#
WEAPON_INFORMATION = {
	# 1001: Sword Weapons (Swordsmanship)
	1001: {
		ONEHANDED_SWING: [0x09, 0x0A, 0x0D],
		TWOHANDED_SWING: [0x0C, 0x0D, 0x0E],
		SKILL: SWORDSMANSHIP,
	},

	# 1002: Axe Weapons (Swordsmanship + Lumberjacking)
	1002: {
		ONEHANDED_SWING: [0xC, 0xD],
		TWOHANDED_SWING: [0xC, 0xD],
		SKILL: SWORDSMANSHIP,
	},

	# 1003: Macefighting (Staffs)
	1003: {
		ONEHANDED_SWING: [0xC, 0xD, 0xE],
		TWOHANDED_SWING: [0xC, 0xD, 0xE],
		SKILL: MACEFIGHTING,
	},

	# 1004: Macefighting (Maces/WarHammer)
	1004: {
		ONEHANDED_SWING: [0x9, 0xD],
		TWOHANDED_SWING: [0xC, 0xD, 0xE],
		SKILL: MACEFIGHTING,
	},

	# 1005: Fencing
	1005: {
		ONEHANDED_SWING: [0x09, 0x0A, 0x0D],
		TWOHANDED_SWING: [0x0D, 0x0E],
		SKILL: FENCING,
	},

	# 1006: Bows
	1006: {
		ONEHANDED_SWING: [0x12],
		TWOHANDED_SWING: [0x12],
		SKILL: ARCHERY,
	},

	# 1007: Crossbows
	1007: {
		ONEHANDED_SWING: [0x13],
		TWOHANDED_SWING: [0x12],
		SKILL: ARCHERY,
	},
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
    if not WEAPON_INFORMATION.has_key(weapon.type):
      return WRESTLING
    else:
      return WEAPON_INFORMATION[weapon.type][SKILL]

#
# This function plays the swing animation for the
# given character.
#
def playswinganimation(char, target, weapon):
  char.turnto(target)

  # Monsters
  if char.id < 0x190:
    action = random.randint(4, 6)
    basedef = wolfpack.charbase(char.id)

    # Anti Blink Bit
    if basedef and basedef['flags'] & 0x02:
      action += 1

    char.action(action)

  # Humans
  else:
    action = random.choice([0x0a, 0x09, 0x1f])

    if weapon and WEAPON_INFORMATION.has_key(weapon.type):
      if weapon.twohanded:
        action = random.choice(WEAPON_INFORMATION[weapon.type][TWOHANDED_SWING])
      else:
        action = random.choice(WEAPON_INFORMATION[weapon.type][ONEHANDED_SWING])

    char.action(action)

#
# Play hurt animation for the defender
#
def playhurtanimation(char):
  # The default action for humans
  if char.id >= 0x190:
    char.action(20)
  else:
    # Note: We lack support for correct animations
    # processing here. animals have different hit anims
    # for instance.
    char.action(10)

#
# Plays the miss sound for a given attacker
# and defender.
#
def playmisssound(attacker, defender):
  weapon = attacker.getweapon()
  sounds = combat.properties.fromitem(weapon, MISSSOUND)

  # No sounds
  if len(sounds) == 0:
    return

  attacker.soundeffect(random.choice(sounds))

#
# Play hurt sound
#
def playhurtsound(defender):
  if defender.id == 0x190:
    # Play a random soundeffect for a human male defender
    sounds = wolfpack.list('SOUNDS_COMBAT_HIT_HUMAN_MALE')

    if len(sounds) > 0:
      sound = hex2dec(random.choice(sounds))
      defender.soundeffect(sound)
    else:
      defender.soundeffect(0x156)

  elif defender.id == 0x191:
    # Play a random soundeffect for a human female defender
    sounds = wolfpack.list('SOUNDS_COMBAT_HIT_HUMAN_FEMALE')

    if len(sounds) > 0:
      sound = hex2dec(random.choice(sounds))
      defender.soundeffect(sound)
    else:
      defender.soundeffect(0x14b)
  else:
    # A standard monster defend sound
    defender.sound(SND_DEFEND)

#
# Plays the hit sound for a given attacker and
# defender
#
def playhitsound(attacker, defender):
  weapon = attacker.getweapon()

  # Play a special sound for monsters
  if not weapon and attacker.id < 0x190:
    attacker.sound(SND_ATTACK)
  else:
    sounds = combat.properties.fromitem(weapon, HITSOUND)

    # Only play a sound if there are any
    if len(sounds) != 0:
      attacker.soundeffect(random.choice(sounds))
