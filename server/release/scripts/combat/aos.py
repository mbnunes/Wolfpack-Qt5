
import wolfpack
from wolfpack.consts import *
import combat.properties
import combat.utilities
import random

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
  attackerSkill = combat.properties.weaponskill(attacker, attackerWeapon, 1)
  defenderSkill = combat.properties.weaponskill(defender, defenderWeapon)

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
# The character hit his target. Calculate and deal the damage
# and process other effects.
#
def hit(attacker, defender, weapon, time):
  if attacker.socket:
    attacker.socket.sysmessage('You hit ' + str(defender.name))

  if defender.socket:
    defender.socket.sysmessage('You are hit by ' + str(attacker.name))

  #defender.damage(DAMAGE_PHYSICAL, 1, attacker)

#
# The character missed his target. Show that he missed and and 
# play a sound effect.
#
def miss(attacker, defender, weapon, time):
  if attacker.socket:
    attacker.socket.sysmessage('You miss ' + str(defender.name))

  if defender.socket:
    defender.socket.sysmessage('You are missed by ' + str(attacker.name))
