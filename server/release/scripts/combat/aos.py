
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
# Get the energy distribution values for a given character.
#
def damagetypes(char):
  fire = 0
  cold = 0
  poison = 0
  energy = 0

  weapon = char.getweapon()

  # See if the npc has specific energy distribution values.
  if char.npc and not weapon:
    if char.hastag('dmg_fire'):
      fire = int(char.gettag('dmg_fire'))
    if char.hastag('dmg_cold'):
      cold = int(char.gettag('dmg_cold'))
    if char.hastag('dmg_poison'):
      poison = int(char.gettag('dmg_poison'))
    if char.hastag('dmg_energy'):
      energy = int(char.gettag('dmg_energy'))
  elif weapon:
    fire = combat.properties.fromitem(weapon, DAMAGE_FIRE)
    cold = combat.properties.fromitem(weapon, DAMAGE_COLD)
    poison = combat.properties.fromitem(weapon, DAMAGE_POISON)
    energy = combat.properties.fromitem(weapon, DAMAGE_ENERGY)

  # See if the energy distribution is correct
  if fire + cold + poison + energy > 100:
    fire = 0
    cold = 0
    poison = 0
    energy = 0
    if weapon:
      char.log(LOG_WARNING, "Character is using broken weapon (0x%x) with wrong damage types.\n" % weapon.serial)
    else:
      char.log(LOG_WARNING, "NPC (0x%x) has wrong damage types.\n" % char.serial)

  physical = 100 - (fire + cold + poison + energy)

  return (physical, fire, cold, poison, energy)  

#
# Absorb damage done by an attacker.
# This damages equipped armor of the defender and 
# checks if the damage can be blocked using a shield.
# This returns the new damage value.
#
def absorbdamage(defender, damage):
  # I think RunUO does this in a good fashion. 
  # Determine the layer using a floating point chance.
  position = random.random()
  armor = None

  # 7% chance: Neck
  if position <= 0.07:
    armor = defender.itemonlayer(LAYER_NECK)

  # 7% chance: Gloves
  elif position <= 0.014:
    armor = defender.itemonlayer(LAYER_GLOVES)

  # 14% chance: Arms
  elif position <= 0.28:
    armor = defender.itemonlayer(LAYER_ARMS)

  # 15% chance: Head
  elif position <= 0.43:
    armor = defender.itemonlayer(LAYER_HELM)
  
  # 22% chance: Legs
  elif position <= 0.65:
    armor = defender.itemonlayer(LAYER_LEGS)

  # 35% chance: Chest
  else:
    armor = defender.itemonlayer(LAYER_CHEST)

  # See if it's really an armor
  if not combat.properties.itemcheck(armor, ITEM_ARMOR):
    armor = None    

  # If there is armor at the given position,
  # it should be damaged. This implementation is so crappy because
  # we lack the required properties for armors yet.
  if armor:
    armor.health -= 1
    if armor.health < 1:
      armor.delete()

  # Only players can parry using a shield or a weapon
  if defender.player:
    shield = defender.itemonlayer(LAYER_LEFTHAND)
    weapon = defender.getweapon()
    blocked = 0

    if not combat.properties.itemcheck(shield, ITEM_SHIELD):
      shield = None

    if not combat.properties.itemcheck(weapon, ITEM_MELEE):
      weapon = None

    # If we have a shield determine if we blocked the blow by 
    # really checking the parry skill
    if shield:
      # There is a 0.3% chance to block for each skill point
      chance = defender.skill[PARRYING] * 0.0003
      defender.checkskill(PARRYING, 0, 1200)
      blocked = chance >= random.random()

    # Otherwise just use the parry skill as a chance value
    # we can't gain it using a weapon as a shield.
    # Note: no ranged weapons
    elif weapon:
      # There is a 0.15% chance to block for each skill point
      chance = defender.skill[PARRYING] * 0.00015
      blocked = chance >= random.random()
    
    # If we blocked the blow. Show it, absorb the damage
    # and damage the shield if there is any
    if blocked:
      defender.effect(0x37B9, 10, 16)
      damage = 0

      # This is as lame as above
      if shield:
        shield.health -= 1
        if shield.health < 1:
          shield.delete()

  return damage

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
  
  # Get the damage distribution of the attackers weapon
  (physical, fire, cold, poison, energy) = damagetypes(attacker)
  
  # Give the defender a chance to absorb damage
  damage = absorbdamage(defender, damage)

  defender.damage(DAMAGE_PHYSICAL, damage, attacker)

#
# The character missed his target. Show that he missed and and 
# play a sound effect.
#
def miss(attacker, defender, weapon, time):
  combat.utilities.playmisssound(attacker, defender)
