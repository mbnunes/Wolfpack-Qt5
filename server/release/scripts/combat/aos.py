
import wolfpack
from wolfpack.consts import *
import combat.properties
import combat.utilities
import random
from math import floor

#
# Check if a certain chance can be met using the skill
#
def checkskill(char, skill, chance):
  # Normalize
  chance = min(1.0, max(0.02, chance))
  minskill = (1.0 - chance) * 1200
  maxskill = 1200
  char.checkskill(skill, minskill, maxskill)
  return chance >= random.random()

#
# Checks if the character hits his target or misses instead
# Returns 1 if the character hits and 0 if the character misses.
#
def checkhit(attacker, defender, time):
  # Get the skills used by the attacker and defender
  attackerWeapon = attacker.getweapon()
  defenderWeapon = defender.getweapon()
  attackerSkill = combat.utilities.weaponskill(attacker, attackerWeapon, 1)
  defenderSkill = combat.utilities.weaponskill(defender, defenderWeapon)

  # Retrieve the skill values
  attackerValue = attacker.skill[attackerSkill] / 10
  defenderValue = defender.skill[defenderSkill] / 10

  # Calculate the hit chance
  bonus = 0 # Get the weapon "accuracy" status
  bonus += 0 # Get the attackers AttackChance bonus
  attackChance = (attackerValue + 20.0) * (100 + bonus)
  
  # Calculate the defense chance
  bonus = 0 # Get the defenders defend chance
  defendChance = (defenderValue + 20.0) * (100 + bonus)

  # Give a minimum chance of 2%
  chance = max(0.02, attackChance / (defendChance * 2.0))

  return checkskill(attacker, attackerSkill, chance)

#
# Scale damage using AOS calculations
# If checkskills is true, the skills for
# the character will actually have the chance to gain
#
def scaledamage(char, damage, checkskills = 1):
  if checkskills:
    char.checkskill(TACTICS, 0, 1200) # Tactics gain
    char.checkskill(ANATOMY, 0, 1200) # Anatomy gain
  
  weapon = char.getweapon()

  # Get the total damage bonus this character gets
  bonus = combat.properties.fromchar(char, DAMAGEBONUS)

  # For axes a lumberjacking skill check is made to gain
  # in that skill
  if weapon and weapon.type == 1002:
    if checkskills:
      char.checkskill(LUMBERJACKING, 0, 1000)
    bonus += char.skill[LUMBERJACKING] * 0.02

    # If above grandmaster grant an additional 10% bonus for axe weapon users
    if char.skill[LUMBERJACKING] >= 1000:
      bonus += 10

  # Strength bonus
  bonus += char.strength * 0.3

  # If strength is above 100, grant an extra 5 percent bonus
  if char.strength >= 100:
    bonus += 5

  # Anatomy bonus
  bonus += char.skill[ANATOMY] * 0.05

  # Grant another 5 percent for anatomy grandmasters
  if char.skill[ANATOMY] >= 1000:
    bonus += 5

  # Tactics bonus
  bonus += char.skill[TACTICS] * 0.0625

  # Add another 6.25 percent for grandmasters
  if char.skill[TACTICS] >= 1000:   
    bonus += 6.25

  damage = damage + damage * bonus / 100.0
  return max(1, floor(damage))

#
# The character hit his target. Calculate and deal the damage
# and process other effects.
#
def hit(attacker, defender, weapon, time):
  combat.utilities.playhitsound(attacker, defender)
  combat.utilities.playhurtanimation(defender)

  (mindamage, maxdamage) = combat.properties.getdamage(attacker)

  damage = random.randint(mindamage, maxdamage)
  damage = scaledamage(attacker, damage)

  # Scale Damage based on boni, skills and other effects

  if attacker.socket:
    attacker.socket.sysmessage('DEALT: %u,%u,%u' % (mindamage, maxdamage, damage))

  if defender.socket:
    defender.socket.sysmessage('TAKEN: %u,%u,%u' % (mindamage, maxdamage, damage))

  defender.damage(DAMAGE_PHYSICAL, damage, attacker)

#
# The character missed his target. Show that he missed and and 
# play a sound effect.
#
def miss(attacker, defender, weapon, time):
  combat.utilities.playmisssound(attacker, defender)
