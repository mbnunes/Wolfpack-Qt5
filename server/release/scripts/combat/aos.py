
import wolfpack
from wolfpack.consts import *
from wolfpack import properties
from wolfpack.utilities import consumeresources, tobackpack, energydamage
import combat.utilities
import random
from math import floor, ceil, sqrt
from system.debugging import DEBUG_COMBAT_INFO
from skills import poisoning
import system.slayer
from combat.specialmoves import getability
import combat.hiteffects

#
# Check if the given weapon can slay the given
# NPC.
#
def checkSlaying(weapon, defender):
	slayer = properties.fromitem(weapon, SLAYER)
	if slayer == '':
		return False
				
	slayer = system.slayer.findEntry(slayer)
	
	if not slayer:
		return False
		
	return slayer.slays(defender)

#
# Check if a certain chance can be met using the skill
#
def checkskill(char, skill, chance):
	# Normalize
	chance = min(1.0, max(0.02, chance))
	minskill = min(char.skill[skill], (1.0 - chance) * 1200)
	maxskill = 1200
	char.checkskill(skill, minskill, maxskill)
	return chance >= random.random()

#
# Fire a ranged weapon and consume the
# ammunition. If the ammunition is not available
# return false.
#
def fireweapon(attacker, defender, weapon):
	ammo = properties.fromitem(weapon, AMMUNITION)

	# Only consume ammo if this weapon requires it
	if len(ammo) != 0:
		if not consumeresources(attacker.getbackpack(), ammo, 1):
			if attacker.socket:
				attacker.socket.sysmessage('You are out of ammo.')
			if attacker.npc:
				tobackpack(weapon, attacker)
			return False

		if random.random() >= 0.50:
			if defender.player:
				item = wolfpack.additem(ammo)
				if not tobackpack(item, defender):
					item.update()
			else:
				# Search for items at the same position
				items = wolfpack.items(defender.pos.x, defender.pos.y, defender.pos.map)
				handled = 0

				for item in items:
					if item.baseid == ammo:
						item.amount += 1
						item.update()
						handled = 1
						break

				if not handled:
					item = wolfpack.additem(ammo)
					item.moveto(defender.pos)
					item.update()

	projectile = properties.fromitem(weapon, PROJECTILE)

	# Fire a projectile if applicable.
	if projectile:
		hue = properties.fromitem(weapon, PROJECTILEHUE)
		attacker.movingeffect(projectile, defender, 0, 0, 14, hue)

	return True

#
# Checks if the character hits his target or misses instead
# Returns 1 if the character hits and 0 if the character misses.
#
def checkhit(attacker, defender, time):
	# Get the skills used by the attacker and defender
	attackerWeapon = attacker.getweapon()
	defenderWeapon = defender.getweapon()
	attackerSkill = combat.utilities.weaponskill(attacker, attackerWeapon, True)
	defenderSkill = combat.utilities.weaponskill(defender, defenderWeapon, True)

	combat.utilities.playswinganimation(attacker, defender, attackerWeapon)

	# Retrieve the skill values
	attackerValue = attacker.skill[attackerSkill] / 10
	defenderValue = defender.skill[defenderSkill] / 10
	
	# Calculate the hit chance
	bonus = 0 # Get the weapon "accuracy" status
	bonus += properties.fromchar(attacker, HITBONUS) # Get the attackers AttackChance bonus
	attackChance = (attackerValue + 20.0) * (100 + bonus)

	# Calculate the defense chance
	bonus = properties.fromchar(defender, DEFENSEBONUS) # Get the defenders defend chance
	defendChance = (defenderValue + 20.0) * (100 + bonus)

	# Give a minimum chance of 2%
	chance = max(0.02, attackChance / (defendChance * 2.0))

	# Scale the chance using the ability
	ability = getability(attacker)
	if ability:
		chance = ability.scalehitchance(attacker, defender, chance)

	return checkskill(attacker, attackerSkill, chance)

#
# Scale damage using AOS calculations
# If checkskills is true, the skills for
# the character will actually have the chance to gain
#
def scaledamage(char, damage, checkskills = True, checkability = False):
	if checkskills:
		char.checkskill(TACTICS, 0, 1200) # Tactics gain
		char.checkskill(ANATOMY, 0, 1200) # Anatomy gain

	weapon = char.getweapon()

	# Get the total damage bonus this character gets
	bonus = properties.fromchar(char, DAMAGEBONUS)

	# For axes a lumberjacking skill check is made to gain
	# in that skill
	if weapon and weapon.type == 1002:
		if checkskills:
			char.checkskill(LUMBERJACKING, 0, 1000)
		bonus += char.skill[LUMBERJACKING] * 0.02

		# If above grandmaster grant an additional 10% bonus for axe weapon users
		if char.skill[LUMBERJACKING] >= 1000:
			bonus += 10

	# Only players get strength boni (or human npcs!)
	if not char.npc or weapon:
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
		fire = char.getintproperty( 'dmg_fire', 0 )
		if char.hastag('dmg_fire'):
			fire = int(char.gettag('dmg_fire'))

		cold = char.getintproperty( 'dmg_cold', 0 )
		if char.hastag('dmg_cold'):
			cold = int(char.gettag('dmg_cold'))

		poison = char.getintproperty( 'dmg_poison', 0 )
		if char.hastag('dmg_poison'):
			poison = int(char.gettag('dmg_poison'))

		energy = char.getintproperty( 'dmg_energy', 0 )
		if char.hastag('dmg_energy'):
			energy = int(char.gettag('dmg_energy'))
	elif weapon:
		# Physical Damage Distribution
		fire = weapon.getintproperty( 'dmg_fire', 0 )
		if weapon.hastag( 'dmg_fire' ):
			fire = int( weapon.gettag( 'dmg_fire' ) )

		cold = weapon.getintproperty( 'dmg_cold', 0 )
		if weapon.hastag( 'dmg_cold' ):
			cold = int( weapon.gettag( 'dmg_cold' ) )

		poison = weapon.getintproperty( 'dmg_poison', 0 )
		if weapon.hastag( 'dmg_poison' ):
			poison = int( weapon.gettag( 'dmg_poison' ) )

		energy = weapon.getintproperty( 'dmg_energy', 0 )
		if weapon.hastag( 'dmg_energy' ):
			energy = int( weapon.gettag( 'dmg_energy' ) )

	# See if the energy distribution is correct
	if fire + cold + poison + energy > 100:
		fire = 0
		cold = 0
		poison = 0
		energy = 0
		if weapon:
			if DEBUG_COMBAT_INFO == 1:
				char.log(LOG_WARNING, "Character is using broken weapon (0x%x) with wrong damage types.\n" % weapon.serial)
		else:
			if DEBUG_COMBAT_INFO == 1:
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
	elif position <= 0.14:
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
	if not properties.itemcheck(armor, ITEM_ARMOR):
		armor = None

	# If there is armor at the given position,
	# it should be damaged. This implementation is so crappy because
	# we lack the required properties for armors yet.
	if armor and armor.health > 0:
		# 4% chance for losing one hitpoint
		if 0.04 >= random.random():
			# If it's a self repairing item, grant health instead of reducing it
			selfrepair = properties.fromitem(armor, SELFREPAIR)
			if selfrepair > 0 and armor.health < armor.maxhealth - 1:
				if selfrepair > random.randint(0, 9):
					armor.health += 2
					armor.resendtooltip()
			else:
				armor.health -= 1
				armor.resendtooltip()

	# Unequip the armor
	if armor and armor.health <= 1:
		tobackpack(armor, defender)
		armor.update()
		if defender.socket:
			defender.socket.clilocmessage(500645)
		armor = None

	# Only players can parry using a shield or a weapon
	if defender.player or defender.skill[PARRYING] > 0:
		shield = defender.itemonlayer(LAYER_LEFTHAND)
		weapon = defender.getweapon()
		blocked = 0

		if not properties.itemcheck(shield, ITEM_SHIELD):
			shield = None

		if not properties.itemcheck(weapon, ITEM_MELEE):
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
		elif weapon and weapon.twohanded:
			# There is a 0.15% chance to block for each skill point
			chance = defender.skill[PARRYING] * 0.00015
			blocked = chance >= random.random()

		# If we blocked the blow. Show it, absorb the damage
		# and damage the shield if there is any
		if blocked:
			defender.effect(0x37B9, 10, 16)
			damage = 0

			# This is as lame as above
			if shield and shield.health > 0:
				# 4% chance for losing one hitpoint				
				if 0.04 >= random.random():
					selfrepair = properties.fromitem(shield, SELFREPAIR)
					if selfrepair > 0 and shield.health < shield.maxhealth - 1:
						if selfrepair > random.randint(0, 9):
							shield.health += 2
							shield.resendtooltip()
					else:							
						shield.health -= 1
						shield.resendtooltip()

			if shield and shield.health <= 0:
				tobackpack(shield, defender)
				shield.update()
				if defender.socket:
					defender.socket.clilocmessage(500645)

	return damage

#
# Deal a splashdamage attack
#
def splashdamage(attacker, effect):
	(physical, cold, fire, poison, energy) = (0, 0, 0, 0, 0)
	
	if effect == SPLASHPHYSICAL:
		sound = 0x10e
		hue = 50
		physical = 100
	elif effect == SPLASHFIRE:
		sound = 0x11d
		hue = 1160
		fire = 100
	elif effect == SPLASHCOLD:
		sound = 0xfc
		hue = 2100
		cold = 100
	elif effect == SPLASHPOISON:
		sound = 0x205
		hue = 1166
		poison = 100
	elif effect == SPLASHENERGY:
		sound = 0x1f1
		hue = 120
		energy = 100
	else:
		raise RuntimeError, "Invalid effect passed to splashdamage: %s" % effect
		
	guild = attacker.guild # Cache the guild
	party = attacker.guild # Cache the party
	didsound = False # Did we play a soundeffect yet?
	(mindamage, maxdamage) = properties.getdamage(attacker) # Cache the min+maxdamage
	
	pos = attacker.pos
	chariterator = wolfpack.charregion(pos.x - 10, pos.y - 10, pos.x + 10, pos.y + 10, pos.map)
	target = chariterator.first
	while target:
		if attacker != target and (not party or party != target.party) and (not guild or guild != target.guild):
			if not target.dead and not target.invulnerable and not target.invisible and not target.hidden:
				tpos = target.pos

				# Calculate the real distance between the two characters				
				distance = sqrt((pos.x - tpos.x) * (pos.x - tpos.x) + (pos.y - tpos.y) * (pos.y - tpos.y))
				factor = min(1.0, (11 - distance) / 10)
				if factor > 0.0:
					damage = int(random.randint(mindamage, maxdamage) * factor)
					
					if damage > 0:
						if not didsound:
							attacker.soundeffect(sound)
							didsound = True
						target.effect(0x3779, 1, 15, hue)
						energydamage(target, attacker, damage, physical, fire, cold, poison, energy, 0, DAMAGE_MAGICAL)

		target = chariterator.next

#
# The character hit his target. Calculate and deal the damage
# and process other effects.
#
def hit(attacker, defender, weapon, time):
	combat.utilities.playhitsound(attacker, defender)

	(mindamage, maxdamage) = properties.getdamage(attacker)

	damage = random.randint(mindamage, maxdamage)
	damage = scaledamage(attacker, damage, checkability = True)

	# Slaying? (only against NPCs)
	if weapon and defender.npc and checkSlaying(weapon, defender):
		defender.effect(0x37B9, 5, 10)
		damage *= 2

	# Get the ability used by the attacker
	ability = getability(attacker)
	
	# Scale Damage using a weapons ability
	if ability:
		damage = ability.scaledamage(attacker, defender, damage)

	# Give the defender a chance to absorb damage
	damage = absorbdamage(defender, damage)
	
	# If the attack was parried, the ability was wasted
	if damage == 0 and ability:
		ability.use(attacker)
		if attacker.socket:
			attacker.socket.clilocmessage(1061140) # Your attack was parried
		ability = None # Reset ability

	ignorephysical = False
	if ability:
		ignorephysical = ability.ignorephysical

	# Get the damage distribution of the attackers weapon
	(physical, fire, cold, poison, energy) = damagetypes(attacker)
	damagedone = energydamage(defender, attacker, damage, physical, fire, cold, poison, energy, 0, DAMAGE_PHYSICAL, ignorephysical=ignorephysical)

	# Wear out the weapon
	if weapon:
		# Leeching
		leech = properties.fromitem(weapon, LIFELEECH)
		if leech and leech > random.randint(0, 99) and attacker.maxhitpoints > attacker.hitpoints:
			amount = (damagedone * 30) / 100 # Leech 30% Health
			if amount > 0:
				attacker.hitpoints = min(attacker.maxhitpoints, attacker.hitpoints + amount)
				attacker.updatehealth()
			
		leech = properties.fromitem(weapon, STAMINALEECH)
		if leech and leech > random.randint(0, 99) and attacker.maxhitpoints > attacker.stamina:
			amount = (damagedone * 100) / 100 # Leech 100% Stamina
			if amount > 0:
				attacker.stamina = min(attacker.maxstamina, attacker.stamina + amount)
				attacker.updatehealth()

		leech = properties.fromitem(weapon, MANALEECH)
		if leech and leech > random.randint(0, 99) and attacker.maxmana > attacker.mana:
			amount = (damagedone * 40) / 100 # Leech 40% Mana
			if amount > 0:
				attacker.mana = min(attacker.maxmana, attacker.mana + amount)
				attacker.updatemana()

		# Splash Damage
		for effectid in [SPLASHPHYSICAL, SPLASHFIRE, SPLASHCOLD, SPLASHPOISON, SPLASHENERGY]:
			effect = properties.fromitem(weapon, effectid)
			if effect and effect > random.randint(0, 99):
				splashdamage(attacker, effectid)
				
		# Hit Spell effects
		for (effectid, callback) in combat.hiteffects.EFFECTS.items():
			effect = properties.fromitem(weapon, effectid)
			if effect and effect > random.randint(0, 99):
				callback(attacker, defender)

		# poisoning doesn't work that way anymore
		#if weapon.hastag( 'poisoning_uses' ):
		#	poisoning.hitEffect( defender, weapon )
		
		# 4% chance for losing one hitpoint
		if 0.04 >= random.random():
			# If it's a self repairing item, grant health instead of reducing it
			selfrepair = properties.fromitem(weapon, SELFREPAIR)
			if selfrepair > 0 and weapon.health < weapon.maxhealth - 1:
				if selfrepair > random.randint(0, 9):
					weapon.health += 2
					weapon.resendtooltip()
			elif weapon.health > 0:
				weapon.health -= 1
				weapon.resendtooltip()
		if weapon.health <= 0:
			tobackpack(weapon, attacker)
			weapon.update()
			if attacker.socket:
				attacker.socket.clilocmessage(500645)
	
	# Notify the weapon ability
	if ability:
		ability.hit(attacker, defender, damage)

#
# The character missed his target. Show that he missed and and
# play a sound effect.
#
def miss(attacker, defender, weapon, time):
	combat.utilities.playmisssound(attacker, defender)

	# Notify the weapon ability
	ability = getability(attacker)
	if ability:
		ability.miss(attacker, defender)
