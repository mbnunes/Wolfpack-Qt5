
import random
import wolfpack

# Type dependant animation lists
# The first element of the assigned arrays contains the single 
# handed animations while the second element contains the twohanded 
# animations
WEAPON_ANIMATIONS = {
		# 1001: Sword Weapons (Swordsmanship)
		# 1002: Axe Weapons (Swordsmanship + Lumberjacking)
		1001: [[0x09, 0x0A, 0x0D], [0x0C, 0x0D, 0x0E]],
		1002: [[0x0C, 0x0D], [0xC, 0xD]],

		# 1003: Macefighting (Staffs)
		# 1004: Macefighting (Maces/WarHammer)
		1003: [[0xC, 0xD, 0xE], [0xC, 0xD, 0xE]],
		1004: [[0x9, 0xD], [0xC, 0xD, 0xE]],

		# 1005: Fencing
    1005: [[0x09, 0x0A, 0x0D], [0x0D, 0x0E]],

		# 1006: Bows
		# 1007: Crossbows
		1006: [[0x12], [0x12]],
		1007: [[0x13], [0x12]]
}

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

    if weapon and WEAPON_ANIMATIONS.has_key(weapon.type):
      if weapon.twohanded:
        action = random.choice(WEAPON_ANIMATIONS[weapon.type][1])
      else:
        action = random.choice(WEAPON_ANIMATIONS[weapon.type][0])
      
    char.action(action)
