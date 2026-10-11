
import wolfpack
from wolfpack.consts import *
from wolfpack import properties, tr
from wolfpack.utilities import consumeresources, tobackpack, energydamage, mayAreaHarm
import combat.utilities
import random
from math import floor, ceil, sqrt
from system.debugging import DEBUG_COMBAT_INFO
import system.slayer
from combat.specialmoves import getability, clearability
import combat.hiteffects
import magic.chivalry
import magic.necromancy
import magic.bushido
import magic.ninjitsu
import system.poison

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
	minskill = min(char.skill[skill], int((1.0 - chance) * 1200) )
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
				attacker.socket.sysmessage(tr('You are out of ammo.'))
			if attacker.npc:
				tobackpack(weapon, attacker)
			return False

		# The ammo is created and packed into the defenders backpack/put on the ground
		createammo(defender, ammo)

	projectile = properties.fromitem(weapon, PROJECTILE)

	# Fire a projectile if applicable.
	if projectile:
		hue = properties.fromitem(weapon, PROJECTILEHUE)
		attacker.movingeffect(projectile, defender, 0, 0, 14, hue)

	return True

def createammo(defender, ammo):
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

def GetPackInstinctBonus( attacker, defender ):
	if attacker.player or defender.player:
		return 0

	packinstinct = attacker.getstrproperty('packinstinct', '')
	if not packinstinct or (not attacker.owner and not attacker.summoned):
		return 0

	master = attacker.owner
	if not master:
		return 0

	inPack = 1

	chars = wolfpack.chars(defender.pos.x, defender.pos.y, defender.pos.map, 1)
	for char in chars:
		if char != attacker:
			if packinstinct == char.getstrproperty('packinstinct', '') or char.player or (not char.owner and not char.summoned):
				continue

			theirMaster = char.owner
			if master == theirMaster and defender in char.getopponents():
				inPack += 1

	if inPack >= 5:
		return 100
	elif inPack >= 4:
		return 75
	elif inPack >=3:
		return 50
	elif inPack >= 2:
		return 25
	return 0

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

	# attacker gets 10% bonus when they're under divine fury
	if attacker.hasscript('magic.divinefury'):
		bonus += 10

	if attacker.hastag('hitlowerattack'):
		bonus -= 25 # Under Hit Lower Attack effect -> 25% malus

	# Bushido: Lightning Strike gives +50% hit chance
	if attacker.hastag('lightning_strike'):
		bonus += 50

	attackChance = (attackerValue + 20.0) * (100 + bonus)

	# Calculate the defense chance
	bonus = properties.fromchar(defender, DEFENSEBONUS) # Get the defenders defend chance
	# defender loses 20% bonus when they're under divine fury
	if defender.hasscript('magic.divinefury'):
		bonus -= 20

	if defender.hastag('hitlowerdefense'):
		bonus -= 25 # Under Hit Lower Defense effect -> 25% malus

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

	if char.hasscript('magic.horrificbeast'):
		bonus += 25

	if char.hasscript('magic.divinefury'):
		bonus += 10

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

	damage = damage + damage * (bonus / 100.0)

	return max(1, floor(damage))

def consecratedweapon( defender ):
	physical = properties.fromchar(defender, RESISTANCE_PHYSICAL)
	fire = properties.fromchar(defender, RESISTANCE_FIRE)
	cold = properties.fromchar(defender, RESISTANCE_COLD)
	poison = properties.fromchar(defender, RESISTANCE_POISON)
	energy = properties.fromchar(defender, RESISTANCE_ENERGY)

	low = physical
	type = 0

	if fire < low:
		low = fire
		type = 1
	if cold < low:
		low = cold
		type = 2
	if poison < low:
		low = poison
		type = 3
	if energy < low:
		low = energy
		type = 4
	physical = fire = cold = poison = energy = 0

	if type == 0:
		physical = 100
	elif type == 1:
		fire = 100
	elif type == 2:
		cold = 100
	elif type == 3:
		poison = 100
	elif type == 4:
		energy = 100

	return (physical, fire, cold, poison, energy)

#
# Get the energy distribution values for a given character.
#
def damagetypes(char, defender):
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
		if magic.chivalry.isConsecrated(weapon):
			(physical, fire, cold, poison, energy) = consecratedweapon( defender )

		else:
			fire = properties.fromitem(weapon, DAMAGE_FIRE)

			cold = properties.fromitem(weapon, DAMAGE_COLD)

			poison = properties.fromitem(weapon, DAMAGE_POISON)

			energy = properties.fromitem(weapon, DAMAGE_ENERGY)

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
		armor = defender.itemonlayer(LAYER_PANTS)

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
		# 25% chance for losing one hitpoint
		if 0.25 >= random.random():
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
		damage = blockdamage(defender, damage)

	return damage

#
# Checks if the damage can be blocked using a shield.
# Called by absorbdamage
# This returns the new damage value.
#
def blockdamage(defender, damage):
	shield = defender.itemonlayer(LAYER_LEFTHAND)
	weapon = defender.getweapon()
	blocked = False

	if not properties.itemcheck(shield, ITEM_SHIELD):
		shield = None

	if not properties.itemcheck(weapon, ITEM_MELEE):
		weapon = None

	chance = 0.0
	bushido = defender.skill[BUSHIDO] / 10.0
	parrying = defender.skill[PARRYING] / 10.0

	if shield:
		chance = defender.skill[PARRYING] * 0.0003
		defender.checkskill(PARRYING, 0, 1200)
	elif weapon:
		# Weapon parrying (with Bushido or Parrying)
		if bushido > 0 or parrying > 0:
			if weapon.twohanded:
				chance = (parrying * bushido) / 48000.0 if bushido > 0 else (parrying * 0.0002)
			else:
				chance = (parrying * bushido) / 41140.0 if bushido > 0 else (parrying * 0.00015)
			# Normalize minimal parry chance if weapon equipped
			chance = max(0.05, min(0.40, chance))
			if parrying > 0:
				defender.checkskill(PARRYING, 0, 1200)
			if bushido > 0:
				defender.checkskill(BUSHIDO, 0, 1200)

	# Bushido: Evasion scalar
	if defender.hastag('evasion_active') and (shield or weapon):
		evasion_bonus = 0.16 + max(0.0, (bushido - 60.0) * 0.004)
		if defender.skill[ANATOMY] >= 1000 and defender.skill[TACTICS] >= 1000 and defender.skill[BUSHIDO] > 1000:
			evasion_bonus += 0.10
		chance *= (1.0 + evasion_bonus)

	if chance > 0.0:
		blocked = chance >= random.random()

	# If we blocked the blow. Show it, absorb the damage
	# and damage the shield if there is any
	if blocked:
		defender.effect(0x37B9, 10, 16)
		damage = 0

		# Bushido: Confidence heal on successful parry
		if defender.hastag('confidence_active'):
			heal_hp = random.randint(1, max(1, int(defender.skill[BUSHIDO] // 120)))
			stam_refresh = random.randint(1, max(1, int(defender.skill[BUSHIDO] // 50)))
			if defender.hitpoints < defender.maxhitpoints:
				defender.hitpoints = min(defender.maxhitpoints, defender.hitpoints + heal_hp)
				defender.updatehealth()
			if defender.stamina < defender.maxstamina:
				defender.stamina = min(defender.maxstamina, defender.stamina + stam_refresh)
				defender.updatehealth()

		# Bushido: Counter Attack immediate stance retaliatory response
		if defender.hastag('counterattack_active') and defender.combatant and not defender.dead:
			magic.bushido.endCounterAttack(defender)
			counter_target = defender.combatant
			if not counter_target.dead and defender.canreach(counter_target, 2):
				if defender.socket:
					defender.socket.clilocmessage(1063117) # You counterattack your opponent!
				hit(defender, counter_target, defender.getweapon(), wolfpack.time.currenttime())

		# This is as lame as above
		if shield and shield.health > 0:
			if random.randint(0, 2) == 0:
				wear = random.randint(0, 1)
				if wear > 0 and shield.maxhealth > 0:
					if shield.health >= wear:
						shield.health -= wear
						wear = 0
					else:
						wear -= shield.hitpoints
						shield.hitpoints = 0

					if wear > 0:
						if shield.maxhitpoints > wear:
							shield.maxhitpoints -= wear
							defender.message(1061121, '') # Your equipment is severely damaged.
						else:
							shield.delete()
					if shield:
						shield.resendtooltip()

			# 4% chance for losing one hitpoint
			#if 0.04 >= random.random():
			#	selfrepair = properties.fromitem(shield, SELFREPAIR)
			#	if selfrepair > 0 and shield.health < shield.maxhealth - 1:
			#		if selfrepair > random.randint(0, 9):
			#			shield.health += 2
			#			shield.resendtooltip()
			#	else:							
			#		shield.health -= 1
			#		shield.resendtooltip()

		#if shield and shield.health <= 0:
		#	tobackpack(shield, defender)
		#	shield.update()
		#	if defender.socket:
		#		defender.socket.clilocmessage(500645)

	return damage
#
# Deal a splashdamage attack
#
def splashdamage(attacker, effect, excludechar = None):
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
		raise RuntimeError( "Invalid effect passed to splashdamage: %s" % effect )

	guild = attacker.guild # Cache the guild
	party = attacker.guild # Cache the party
	didsound = False # Did we play a soundeffect yet?
	(mindamage, maxdamage) = properties.getdamage(attacker) # Cache the min+maxdamage

	pos = attacker.pos
	chariterator = wolfpack.charregion(pos.x - 3, pos.y - 3, pos.x + 3, pos.y + 3, pos.map)
	target = chariterator.first
	while target:
		if attacker != target and excludechar != target and mayAreaHarm(attacker, target):
			tpos = target.pos

			# Calculate the real distance between the two characters
			distance = sqrt((pos.x - tpos.x) * (pos.x - tpos.x) + (pos.y - tpos.y) * (pos.y - tpos.y))
			factor = min(1.0, (4 - distance) / 3)
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

	# Enemy of One (chivalry)
	if attacker.npc:
		if defender.player:
			if defender.hastag( "enemyofonetype" ) and defender.gettag( "enemyofonetype" ) != attacker.id:
				damage *= 2
	if defender.npc: # only NPC
		if attacker.player:
			if attacker.hastag( "waitingforenemy" ):
				attacker.settag( "enemyofonetype", defender.id )
				attacker.deltag( "waitingforenemy" )
			if attacker.hastag( "enemyofonetype" ) and attacker.gettag( "enemyofonetype" ) == defender.id:
				defender.effect( 0x37B9, 10, 5 )
				damage += scaledamage(attacker, 50 )

	packInstinctBonus = GetPackInstinctBonus( attacker, defender )
	if packInstinctBonus:
		damage += scaledamage(attacker, packInstinctBonus)

	slayer = properties.fromitem(weapon, SLAYER)
	if slayer and slayer == "silver" and magic.necromancy.transformed(defender) and not defender.hasscript("magic.horrificbeast"):
		damage += scaledamage(attacker, 25 ) # Every necromancer transformation other than horrific beast takes an additional 25% damage

	# Bushido: Honorable Execution damage bonus
	if attacker.hastag('honorable_execution'):
		bushido = attacker.skill[BUSHIDO] / 10.0
		damage = int(damage * (1.0 + (bushido * 20.0) / 10000.0))

	# Ninjitsu: Focus Attack damage bonus
	if attacker.hastag('focus_attack'):
		ninjitsu = attacker.skill[NINJITSU] / 10.0
		damage = int(damage * (1.0 + (ninjitsu * ninjitsu) / 43636.0))

	# Ninjitsu: Death Strike half direct damage on initial hit
	if attacker.hastag('death_strike'):
		damage = max(1, int(damage * 0.5))

	# Ninjitsu: Ki Attack distance damage bonus
	if attacker.hastag('ki_attack') and attacker.hastag('ki_start_x') and attacker.hastag('ki_start_y'):
		sx = int(attacker.gettag('ki_start_x'))
		sy = int(attacker.gettag('ki_start_y'))
		dx = attacker.pos.x - sx
		dy = attacker.pos.y - sy
		dist = min(20.0, math.sqrt(dx * dx + dy * dy))
		damage = int(damage * (1.0 + dist / 10.0))

	# Ninjitsu: Backstab damage bonus
	if attacker.hastag('backstab'):
		ninjitsu = attacker.skill[NINJITSU] / 10.0
		damage = int(damage * (1.0 + (ninjitsu / 360.0)))

	# Give the defender a chance to absorb damage
	damage = absorbdamage(defender, damage)
	blocked = damage <= 0

	# Bushido: Disruption of Confidence on defender
	if defender.hastag('confidence_regen_left'):
		defender.deltag('confidence_regen_left')
		defender.deltag('confidence_regen_tick')
		defender.dispel(defender, True, 'CONFIDENCE_REGEN')

	# If the attack was parried, the ability was wasted
	if AGEOFSHADOWS and blocked:
		if attacker.socket:
			attacker.socket.clilocmessage(1061140) # Your attack was parried
		clearability(attacker)

	ignorephysical = False
	if ability:
		ignorephysical = ability.ignorephysical

	# Bushido: Lightning Strike critical armor ignore check
	if attacker.hastag('lightning_strike'):
		bushido = attacker.skill[BUSHIDO] / 10.0
		crit_chance = (bushido * bushido) / 72000.0
		if crit_chance >= random.random():
			ignorephysical = True

	# Get the damage distribution of the attackers weapon
	(physical, fire, cold, poison, energy) = damagetypes(attacker, defender)
	damagedone = energydamage(defender, attacker, damage, physical, fire, cold, poison, energy, 0, DAMAGE_PHYSICAL, ignorephysical=ignorephysical)

	# Bushido: Honorable Execution result
	if attacker.hastag('honorable_execution'):
		magic.bushido.clearMove(attacker, 'honorable_execution', 401)
		attacker.checkskill(BUSHIDO, 0, 1200)
		if defender.dead or defender.hitpoints <= 0:
			# Success: heal & swing speed bonus
			attacker.effect(0x373A, 1, 17)
			bushido = attacker.skill[BUSHIDO] / 10.0
			heal = 20 + int((bushido * bushido) / 480.0)
			if attacker.hitpoints < attacker.maxhitpoints:
				attacker.hitpoints = min(attacker.maxhitpoints, attacker.hitpoints + heal)
				attacker.updatehealth()
			attacker.settag('honorable_swing_bonus', 1)
			attacker.dispel(attacker, True, 'HONORABLE_SWING_BONUS')
			attacker.addtimer(20000, magic.bushido.honorable_swing_bonus_expire, [], True, False, 'HONORABLE_SWING_BONUS')
		else:
			# Failure: penalty -40 all resistances for 7s
			attacker.settag('honorable_penalty', 1)
			attacker.dispel(attacker, True, 'HONORABLE_PENALTY')
			attacker.addtimer(7000, magic.bushido.honorable_penalty_expire, [], True, False, 'HONORABLE_PENALTY')

	# Bushido: Lightning Strike trigger
	if attacker.hastag('lightning_strike'):
		magic.bushido.clearMove(attacker, 'lightning_strike', 405)
		attacker.checkskill(BUSHIDO, 0, 1200)
		if attacker.socket:
			attacker.socket.clilocmessage(1063168) # You attack with lightning precision!
		if defender.socket:
			defender.socket.clilocmessage(1063169) # Your opponent's quick strike causes extra damage!
		defender.effect(0x3818, 1, 11)
		defender.soundeffect(0x51D)

	# Bushido: Momentum Strike trigger
	if attacker.hastag('momentum_strike'):
		magic.bushido.clearMove(attacker, 'momentum_strike', 406)
		attacker.checkskill(BUSHIDO, 0, 1200)
		# Find another hostile in weapon range
		weap_range = 1
		if weapon:
			weap_range = weapon.getintproperty('range', 1)
		center = attacker.pos
		sec_targets = []
		chariter = wolfpack.charregion(center.x - weap_range, center.y - weap_range, center.x + weap_range, center.y + weap_range, center.map)
		sec_char = chariter.first
		while sec_char:
			if sec_char != attacker and sec_char != defender and not sec_char.dead and not sec_char.invulnerable and not sec_char.region.safe:
				if mayAreaHarm(attacker, sec_char) and attacker.canreach(sec_char, weap_range):
					sec_targets.append(sec_char)
			sec_char = chariter.next

		if len(sec_targets) > 0:
			sec_target = random.choice(sec_targets)
			if attacker.socket:
				attacker.socket.clilocmessage(1063171) # You transfer the momentum of your weapon into another enemy!
			if sec_target.socket:
				sec_target.socket.clilocmessage(1063172) # You were hit by the momentum of a Samurai's weapon!
			sec_target.effect(0x37B9, 1, 4)
			attacker.soundeffect(0x510)
			# Bonus damage: bushido / 100 (or 1.5x if primary defender died)
			bonus_mult = (attacker.skill[BUSHIDO] / 1000.0)
			if defender.dead or defender.hitpoints <= 0:
				bonus_mult *= 1.5
			sec_damage = max(1, int(damagedone * (1.0 + bonus_mult)))
			energydamage(sec_target, attacker, sec_damage, physical, fire, cold, poison, energy, 0, DAMAGE_PHYSICAL)
		else:
			if attacker.socket:
				attacker.socket.clilocmessage(1063123) # There are no valid targets to attack!

	# Ninjitsu: Focus Attack onHit
	if attacker.hastag('focus_attack'):
		magic.ninjitsu.clearNinjaMove(attacker, 'focus_attack', 501)
		attacker.checkskill(NINJITSU, 0, 1200)
		if attacker.socket:
			attacker.socket.clilocmessage(1063098) # You focus all of your abilities and strike with deadly force!
		attacker.soundeffect(0x510)

	# Ninjitsu: Death Strike onHit
	if attacker.hastag('death_strike'):
		magic.ninjitsu.clearNinjaMove(attacker, 'death_strike', 502)
		attacker.checkskill(NINJITSU, 0, 1200)
		ninjitsu = attacker.skill[NINJITSU] / 10.0
		chance = 30 + (ninjitsu - 85) * 2.2 if ninjitsu < 100 else 63 + (ninjitsu - 100) * 1.1
		if (chance / 100.0) >= random.random():
			if attacker.socket:
				attacker.socket.clilocmessage(1063094) # You inflict a Death Strike upon your opponent!
			if defender.socket:
				defender.socket.clilocmessage(1063093) # You have been hit by a Death Strike! Move with caution!
			defender.effect(0x374A, 1, 17)
			attacker.soundeffect(0x50D if attacker.gender else 0x50E)
			defender.settag('deathstrike_victim', 1)
			defender.settag('deathstrike_attacker', attacker.serial)
			defender.settag('deathstrike_steps', 0)
			defender.dispel(defender, True, 'DEATHSTRIKE_TIMER')
			defender.addtimer(5000, magic.ninjitsu.deathstrike_process, [], True, False, 'DEATHSTRIKE_TIMER')
		else:
			if attacker.socket:
				attacker.socket.clilocmessage(1070779) # You missed your opponent with a Death Strike.

	# Ninjitsu: Ki Attack onHit
	if attacker.hastag('ki_attack'):
		magic.ninjitsu.clearNinjaMove(attacker, 'ki_attack', 504)
		attacker.deltag('ki_start_x')
		attacker.deltag('ki_start_y')
		attacker.dispel(attacker, True, 'KI_ATTACK_TIMER')
		attacker.checkskill(NINJITSU, 0, 1200)
		attacker.effect(0x37BE, 1, 5)
		defender.effect(0x37BE, 1, 5)
		attacker.soundeffect(0x510)
		if attacker.socket:
			attacker.socket.clilocmessage(1063100) # Your quick flight to your target causes extra damage as you strike!

	# Ninjitsu: Surprise Attack onHit
	if attacker.hastag('surprise_attack'):
		magic.ninjitsu.clearNinjaMove(attacker, 'surprise_attack', 505)
		attacker.checkskill(NINJITSU, 0, 1200)
		if attacker.socket:
			attacker.socket.clilocmessage(1063129) # You catch your opponent off guard with your Surprise Attack!
		if defender.socket:
			defender.socket.clilocmessage(1063130) # Your defenses are lowered as your opponent surprises you!
		defender.effect(0x37B9, 1, 5)
		attacker.reveal()
		defender.settag('surprise_attack_malus', 1)
		defender.dispel(defender, True, 'SURPRISE_ATTACK_MALUS')
		defender.addtimer(8000, magic.ninjitsu.surprise_attack_expire, [], True, False, 'SURPRISE_ATTACK_MALUS')

	# Ninjitsu: Backstab onHit
	if attacker.hastag('backstab'):
		magic.ninjitsu.clearNinjaMove(attacker, 'backstab', 506)
		attacker.checkskill(NINJITSU, 0, 1200)
		if attacker.socket:
			attacker.socket.clilocmessage(1063090) # You quickly stab your opponent as you come out of hiding!
		defender.effect(0x37B9, 1, 5)
		attacker.reveal()

	# Wear out the weapon
	if weapon:
		# Leeching
		if not blocked:
			# Making default Leechs to prevent errors

			lifeleech = 0
			staminaleech = 0
			manaleech = 0

			leech = properties.fromitem(weapon, LIFELEECH)
			if leech and leech > random.randint(0, 99) and attacker.maxhitpoints > attacker.hitpoints:
				lifeleech = (damagedone * 30) / 100 # Leech 30% Health

			leech = properties.fromitem(weapon, STAMINALEECH)
			if leech and leech > random.randint(0, 99) and attacker.maxhitpoints > attacker.stamina:
				staminaleech = (damagedone * 100) / 100 # Leech 100% Stamina

			leech = properties.fromitem(weapon, MANALEECH)
			if leech and leech > random.randint(0, 99) and attacker.maxmana > attacker.mana:
				manaleech = (damagedone * 40) / 100 # Leech 40% Mana

			# Now leech life, stamina and mana
			if lifeleech > 0:
				attacker.hitpoints = min(attacker.maxhitpoints, attacker.hitpoints + lifeleech)
				attacker.updatehealth()

			if staminaleech > 0:
				attacker.stamina = min(attacker.maxstamina, attacker.stamina + staminaleech)
				attacker.updatehealth()

			if manaleech > 0:
				attacker.mana = min(attacker.maxmana, attacker.mana + manaleech)
				attacker.updatemana()

			# Poisoning (50% chance)
			if weapon.hastag('poisoning_uses'):
				poisoning_uses = int(weapon.gettag('poisoning_uses'))
				if poisoning_uses <= 0:
					weapon.deltag('poisoning_uses')
					weapon.resendtooltip()
				else:
					poisoning_uses -= 1
					if poisoning_uses <= 0:
						weapon.deltag('poisoning_uses')
						weapon.resendtooltip()
					else:
						weapon.settag('poisoning_uses', poisoning_uses)

					poisoning_strength = 0 # Assume lesser unless the tag tells so otherwise
					if weapon.hastag('poisoning_strength'):
						poisoning_strength = int(weapon.gettag('poisoning_strength'))
					if defender.hasscript("magic.evilomen") and poisoning_strength < 4:
						poisoning_strength += 1
					if random.random() >= 0.50:
						if system.poison.poison(defender, poisoning_strength):
							if attacker.socket:
								attacker.socket.clilocmessage(1008096, "", 0x3b2, 3, None, defender.name, False, False)
							if defender.socket:
								attacker.socket.clilocmessage(1008097, "", 0x3b2, 3, None, attacker.name, False, True)

			# Splash Damage
			for effectid in [SPLASHPHYSICAL, SPLASHFIRE, SPLASHCOLD, SPLASHPOISON, SPLASHENERGY]:
				effect = properties.fromitem(weapon, effectid)
				if effect and effect > random.randint(0, 99):
					splashdamage(attacker, effectid, excludechar = defender)

			# Hit Spell effects
			for (effectid, callback) in combat.hiteffects.EFFECTS.items():
				effect = properties.fromitem(weapon, effectid)
				if effect and effect > random.randint(0, 99):
					callback(attacker, defender)

		# Slime or Toxic Elemental, 4% chance for losing one hitpoint
		if weapon.maxhealth > 0 and ( (weapon.getintproperty( 'range', 1 ) <= 1 and (defender.id == 51 or defender.id == 158)) or 0.04 >= random.random() ):
			if (weapon.getintproperty( 'range', 1 ) <= 1 and (defender.id == 51 or defender.id == 158)):
				attacker.message( 500263, '' ) # *Acid blood scars your weapon!*

			# If it's a self repairing item, grant health instead of reducing it
			selfrepair = properties.fromitem(weapon, SELFREPAIR)
			if AGEOFSHADOWS and selfrepair > 0:
				if selfrepair > random.randint(0, 9):
					weapon.health += 2
					weapon.resendtooltip()
			else:
				if weapon.health > 0:
					weapon.health -= 1
				elif weapon.maxhealth > 1:
					weapon.maxhealth -= 1
					attacker.message( 1061121, '' ) # Your equipment is severely damaged.
				else:
					weapon.delete()
				if weapon:
					weapon.resendtooltip()

		#if weapon.health <= 0:
		#	tobackpack(weapon, attacker)
		#	weapon.update()
		#	if attacker.socket:
		#		attacker.socket.clilocmessage(500645)

	# Notify the weapon ability
	if not blocked and ability:
		ability.hit(attacker, defender, damage)

#
# The character missed his target. Show that he missed and and
# play a sound effect.
#
def miss(attacker, defender, weapon, time):
	combat.utilities.playmisssound(attacker, defender)

	# Clear active moves on miss
	if attacker.hastag('honorable_execution'):
		magic.bushido.clearMove(attacker, 'honorable_execution', 401)
	if attacker.hastag('lightning_strike'):
		magic.bushido.clearMove(attacker, 'lightning_strike', 405)
	if attacker.hastag('momentum_strike'):
		magic.bushido.clearMove(attacker, 'momentum_strike', 406)
	if attacker.hastag('focus_attack'):
		magic.ninjitsu.clearNinjaMove(attacker, 'focus_attack', 501)
	if attacker.hastag('death_strike'):
		magic.ninjitsu.clearNinjaMove(attacker, 'death_strike', 502)
	if attacker.hastag('ki_attack'):
		magic.ninjitsu.clearNinjaMove(attacker, 'ki_attack', 504)
		attacker.deltag('ki_start_x')
		attacker.deltag('ki_start_y')
	if attacker.hastag('surprise_attack'):
		magic.ninjitsu.clearNinjaMove(attacker, 'surprise_attack', 505)
		if attacker.socket:
			attacker.socket.clilocmessage(1063161) # You failed to properly use the element of surprise.
		attacker.reveal()
	if attacker.hastag('backstab'):
		magic.ninjitsu.clearNinjaMove(attacker, 'backstab', 506)
		if attacker.socket:
			attacker.socket.clilocmessage(1063161) # You failed to properly use the element of surprise.
		attacker.reveal()

	# Notify the weapon ability
	ability = getability(attacker)
	if ability:
		ability.miss(attacker, defender)
