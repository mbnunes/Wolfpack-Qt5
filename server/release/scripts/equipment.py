
import wolfpack
from wolfpack.utilities import *
from wolfpack.consts import *
import wolfpack.weaponinfo
import wolfpack.armorinfo
from wolfpack import properties, tr
from combat.utilities import weaponskill
from math import ceil
import system.slayer

#
# Show certain modifiers stored in tags.
#
def modifiers(object, tooltip):
	modifiers = {
		"remaining_uses": 1060584,
		"aos_boni_damage": 1060401,
	}

	for (tag, cliloc) in modifiers.items():
		if object.hastag(tag):
			tooltip.add(cliloc, str(object.gettag(tag)))

	speedbonus = properties.fromitem(object, SPEEDBONUS)
	if speedbonus != 0:
		tooltip.add(1060486, str(speedbonus))


	hitbonus = properties.fromitem(object, HITBONUS)
	if hitbonus != 0:
		tooltip.add(1060415, str(hitbonus))

	defensebonus = properties.fromitem(object, DEFENSEBONUS)
	if defensebonus != 0:
		tooltip.add(1060408, str(defensebonus))

	enhancepotions = properties.fromitem(object, ENHANCEPOTIONS)
	if enhancepotions != 0:
		tooltip.add(1060411, str(enhancepotions))

	reflectphysical = properties.fromitem(object, REFLECTPHYSICAL)

	if reflectphysical:
		tooltip.add(1060442, str(reflectphysical))

	luck = properties.fromitem(object, LUCK)
	if luck != 0:
		tooltip.add(1060436, str(luck))

	bonus = properties.fromitem(object, BONUSSTRENGTH)
	if bonus != 0:
		tooltip.add(1060485, str(bonus))

	bonus = properties.fromitem(object, BONUSDEXTERITY)
	if bonus != 0:
		tooltip.add(1060409, str(bonus))

	bonus = properties.fromitem(object, BONUSINTELLIGENCE)
	if bonus != 0:
		tooltip.add(1060432, str(bonus))

	bonus = properties.fromitem(object, BONUSHITPOINTS)
	if bonus != 0:
		tooltip.add(1060431, str(bonus))

	bonus = properties.fromitem(object, BONUSSTAMINA)
	if bonus != 0:
		tooltip.add(1060484, str(bonus))

	bonus = properties.fromitem(object, BONUSMANA)
	if bonus != 0:
		tooltip.add(1060439, str(bonus))

	regenhitpoints = properties.fromitem(object, REGENHITPOINTS)
	if regenhitpoints:
		tooltip.add(1060444, str(regenhitpoints))

	regenstamina = properties.fromitem(object, REGENSTAMINA)
	if regenstamina:
		tooltip.add(1060443, str(regenstamina))

	regenmana = properties.fromitem(object, REGENMANA)
	if regenmana:
		tooltip.add(1060440, str(regenmana))

	if properties.fromitem(object, BESTSKILL):
		tooltip.add(1060400, "")

	mageweapon = properties.fromitem(object, MAGEWEAPON)
	if mageweapon != 0:
		tooltip.add(1060438, str(mageweapon))

	if properties.fromitem(object, MAGEARMOR) and not object.allowmeditation:
		tooltip.add(1060437, "")

	selfrepair = properties.fromitem(object, SELFREPAIR)
	if selfrepair != 0:
		tooltip.add(1060450, str(selfrepair))

#
# Equipment has a lot of additional effects.
# These are shown to the user in form of tooltips.
#
def onShowTooltip(viewer, object, tooltip):
	armor = properties.itemcheck(object, ITEM_ARMOR)
	weapon = properties.itemcheck(object, ITEM_WEAPON)
	shield = properties.itemcheck(object, ITEM_SHIELD)

	if (armor or weapon or shield) and object.amount == 1:
		# Reinsert the name if we need an ore prefix
		prefix1 = None
		if object.hastag('resname'):
			resname = str(object.gettag('resname'))
			if (armor or shield) and wolfpack.armorinfo.ARMOR_RESNAME_BONI.has_key(resname):
				resinfo = wolfpack.armorinfo.ARMOR_RESNAME_BONI[resname]
				if resinfo.has_key(MATERIALPREFIX):
					prefix1 = resinfo[MATERIALPREFIX]
			if weapon and wolfpack.weaponinfo.WEAPON_RESNAME_BONI.has_key(resname):
				resinfo = wolfpack.weaponinfo.WEAPON_RESNAME_BONI[resname]
				if resinfo.has_key(MATERIALPREFIX):
					prefix1 = resinfo[MATERIALPREFIX]
		elif object.hasstrproperty( 'resname' ):
			resname = str( object.getstrproperty( 'resname' ) )
			if (armor or shield) and wolfpack.armorinfo.ARMOR_RESNAME_BONI.has_key(resname):
				resinfo = wolfpack.armorinfo.ARMOR_RESNAME_BONI[resname]
				if resinfo.has_key(MATERIALPREFIX):
					prefix1 = resinfo[MATERIALPREFIX]
			if weapon and wolfpack.weaponinfo.WEAPON_RESNAME_BONI.has_key(resname):
				resinfo = wolfpack.weaponinfo.WEAPON_RESNAME_BONI[resname]
				if resinfo.has_key(MATERIALPREFIX):
					prefix1 = resinfo[MATERIALPREFIX]

		prefix2 = None
		if object.hastag('resname2'):
			resname2 = str(object.gettag('resname2'))
			if (armor or shield) and wolfpack.armorinfo.ARMOR_RESNAME_BONI.has_key(resname2):
				resinfo = wolfpack.armorinfo.ARMOR_RESNAME_BONI[resname2]
				if resinfo.has_key(MATERIALPREFIX):
					prefix2 = resinfo[MATERIALPREFIX]
			if weapon and wolfpack.weaponinfo.WEAPON_RESNAME_BONI.has_key(resname2):
				resinfo = wolfpack.weaponinfo.WEAPON_RESNAME_BONI[resname2]
				if resinfo.has_key(MATERIALPREFIX):
					prefix2 = resinfo[MATERIALPREFIX]
		elif object.hasstrproperty( 'resname2' ):
			resname2 = str( object.getstrproperty( 'resname2' ) )
			if (armor or shield) and wolfpack.armorinfo.ARMOR_RESNAME_BONI.has_key(resname2):
				resinfo = wolfpack.armorinfo.ARMOR_RESNAME_BONI[resname2]
				if resinfo.has_key(MATERIALPREFIX):
					prefix2 = resinfo[MATERIALPREFIX]
			if weapon and wolfpack.weaponinfo.WEAPON_RESNAME_BONI.has_key(resname2):
				resinfo = wolfpack.weaponinfo.WEAPON_RESNAME_BONI[resname2]
				if resinfo.has_key(MATERIALPREFIX):
					prefix2 = resinfo[MATERIALPREFIX]

		if len(object.name) == 0:
			itemname = '#' + str(1020000 + object.id)
		else:
			itemname = object.name

		if prefix1 and prefix2:
			tooltip.reset()
			tooltip.add(1053099, "%s %s\t%s" % (prefix1, prefix2, itemname))
		elif prefix1 and not prefix2:
			tooltip.reset()
			tooltip.add(1053099, "%s\t%s" % (prefix1, itemname))
		elif not prefix1 and prefix2:
			tooltip.reset()
			tooltip.add(1053099, "%s\t%s" % (prefix2, itemname))

	# Exceptional item?
	if object.hastag('exceptional'):
		tooltip.add(1060636, '')

		# 1050043: Crafted by ~param~
		serial = int(object.gettag('exceptional'))
		crafter = wolfpack.findchar(serial)
		if crafter:
			tooltip.add(1050043, crafter.name)

	# Only Armors and Weapons have durability
	if weapon or armor or shield:
		tooltip.add(1060639, "%u\t%u" % (object.health, object.maxhealth))

		durabilitybonus = properties.fromitem(object, DURABILITYBONUS)
		if durabilitybonus:
			if durabilitybonus > 0:
				tooltip.add(1060410, '+' + str(durabilitybonus))
			else:
				tooltip.add(1060410, str(durabilitybonus))

	# Weapon specific properties
	if weapon:
		# Leeching
		leech = properties.fromitem(object, LIFELEECH)
		if leech:
			tooltip.add(1060422, str(leech))
		leech = properties.fromitem(object, STAMINALEECH)
		if leech:
			tooltip.add(1060430, str(leech))
		leech = properties.fromitem(object, MANALEECH)
		if leech:
			tooltip.add(1060427, str(leech))						
	
		# Splash 
		for (effect, cliloc) in {SPLASHPHYSICAL: 1060428, SPLASHFIRE: 1060419, SPLASHCOLD: 1060416, SPLASHPOISON: 1060429, SPLASHENERGY: 1060418}.items():
			effect = properties.fromitem(object, effect)
			if effect > 0:
				tooltip.add(cliloc, str(effect))
	
		# Hit Effects
		for (effect, cliloc) in {HITMAGICARROW: 1060426, HITHARM: 1060421, HITFIREBALL: 1060420, HITLIGHTNING: 1060423, HITDISPEL: 1060417}.items():
			effect = properties.fromitem(object, effect)
			if effect > 0:
				tooltip.add(cliloc, str(effect))

		# Slayer
		slayer = properties.fromitem(object, SLAYER)
		if slayer != '':
			slayer = system.slayer.findEntry(slayer)
			if slayer:
				tooltip.add(slayer.name, '')

		# One or twohanded weapon
		if object.twohanded:
			tooltip.add(1061171, '')
		else:
			tooltip.add(1061824, '')

		# Used weaponskill
		skill = weaponskill(viewer, object)
		if skill == SWORDSMANSHIP:
			tooltip.add(1061172, '')
		elif skill == MACEFIGHTING:
			tooltip.add(1061173, '')
		elif skill == FENCING:
			tooltip.add(1061174, '')
		elif skill == ARCHERY:
			tooltip.add(1061175, '')
		#elif skill == WRESTLING:
		#	tooltip.add(1061172, '')

		# Special weapon range
		if object.hasintproperty( 'range' ) or object.hastag( 'range' ):
			if object.hastag( 'range' ):
				weaponrange = int( object.gettag( 'range' ) )
			else:
				weaponrange = int( object.getintproperty( 'range', 12 ) )
			if weaponrange > 1:
				tooltip.add( 1061169, str(weaponrange) )

		# Max-Mindamage
		mindamage = object.getintproperty( 'mindamage', 1 )
		if object.hastag( 'mindamage' ):
			mindamage = int( object.gettag( 'mindamage' ) )
		maxdamage = object.getintproperty( 'maxdamage', 2 )
		if object.hastag( 'maxdamage' ):
			maxdamage = int( object.gettag( 'maxdamage' ) )
		tooltip.add( 1061168, "%u\t%u" % ( mindamage, maxdamage ) )

		# Speed
		speed = object.getintproperty( 'speed', 10 )
		if object.hastag( 'speed' ):
			speed = int( object.gettag( 'speed' ) )
		tooltip.add(1061167, str(speed))

		# Physical Damage Distribution
		fire = properties.fromitem(object, DAMAGE_FIRE)
		cold = properties.fromitem(object, DAMAGE_COLD)
		poison = properties.fromitem(object, DAMAGE_POISON)
		energy = properties.fromitem(object, DAMAGE_ENERGY)

		# This must always total 100
		physical = 100 - (fire + cold + poison + energy)
		if (physical + fire + cold + poison + energy) != 100:
			physical = 100
			fire = 0
			cold = 0
			poison = 0
			energy = 0

		if physical:
			tooltip.add(1060403, str(physical))

		if fire:
			tooltip.add(1060405, str(fire))

		if cold:
			tooltip.add(1060404, str(cold))

		if poison:
			tooltip.add(1060406, str(poison))

		if energy:
			tooltip.add(1060407, str(energy))

	if weapon or shield:
		# Spell Channeling
		spellchanneling = properties.fromitem(object, SPELLCHANNELING)
		if spellchanneling:
			tooltip.add(1060482, "")
	else:
		spellchanneling = False

	# Those are only relevant if its not a shield/weapon or for spellchanneling items
	if (not weapon and not shield) or spellchanneling:
		castrecovery = properties.fromitem(object, CASTRECOVERYBONUS)

		if castrecovery:
			tooltip.add(1060412, str(castrecovery))

		castspeed = properties.fromitem(object, CASTSPEEDBONUS)

		if castspeed:
			tooltip.add(1060413, str(castspeed))

		spelldamagebonus = properties.fromitem(object, SPELLDAMAGEBONUS)

		if spelldamagebonus:
			tooltip.add(1060483, str(spelldamagebonus))

	physical = properties.fromitem(object, RESISTANCE_PHYSICAL)
	fire = properties.fromitem(object, RESISTANCE_FIRE)
	cold = properties.fromitem(object, RESISTANCE_COLD)
	poison = properties.fromitem(object, RESISTANCE_POISON)
	energy = properties.fromitem(object, RESISTANCE_ENERGY)

	if physical:
		tooltip.add(1060448, str(physical))

	if fire:
		tooltip.add(1060447, str(fire))

	if cold:
		tooltip.add(1060445, str(cold))

	if poison:
		tooltip.add(1060449, str(poison))

	if energy:
		tooltip.add(1060446, str(energy))

	modifiers(object, tooltip)

	lowermana = properties.fromitem(object, LOWERMANACOST)
	if lowermana:
		tooltip.add(1060433, str(lowermana))

	lowerreags = properties.fromitem(object, LOWERREAGENTCOST)
	if lowerreags:
		tooltip.add(1060434, str(lowerreags))

	lower = properties.fromitem(object, LOWERREQS)
	if lower:
		tooltip.add(1060435, str(lower))
	lower /= 100.0

	# Tag will override.
	req_str = object.getintproperty( 'req_strength', 0 )
	if object.hastag( 'req_strength' ):
		req_str = int( object.gettag( 'req_strength' ) )

	if lower:
		req_str = int(ceil(req_str) * (1.0 - lower))
	if req_str:
		tooltip.add(1061170, str(req_str))

	# Skill Boni (1-5)
	for i in range(0, 5):
		if object.hastag('skillbonus_%u' % i):
			try:
				(skill, bonus) = object.gettag('skillbonus_%u' % i).split(',')
				(skill, bonus) = (int(skill), int(bonus))

				if bonus == 0 or skill < 0 or skill >= ALLSKILLS:
					continue

				# Add a Bonus for the skill
				tooltip.add(1060451 + i, "#%u\t%u" % (1044060 + skill, int(bonus / 10)))
			except:
				object.deltag('skillbonus_%u' % i)
				continue
				
	return False

#
# Check for certain equipment requirements
#
def onWearItem(player, wearer, item, layer):
	if wearer.socket and wearer.socket.hastag('block_equip'):
		expire = int(wearer.socket.gettag('block_equip'))
		if expire < wolfpack.currenttime():
			wearer.socket.deltag('block_equip')
		else:
			if player == wearer:
				player.socket.sysmessage(tr('You cannot equip another so soon after being disarmed.'))
			else:
				player.socket.sysmessage(tr('They cannot equip another so soon after being disarmed.'))
			return True
	
	lower = properties.fromitem(item, LOWERREQS) / 100.0

	req_str = item.getintproperty( 'req_strength', 0 )
	if item.hastag( 'req_strength' ):
		req_str = int( item.gettag( 'req_strength' ) )
	if lower:
		req_str = int(ceil(req_str) * (1.0 - lower))

	req_dex = item.getintproperty( 'req_dexterity', 0 )
	if item.hastag( 'req_dexterity' ):
		req_dex = int( item.gettag( 'req_dexterity' ) )
	if lower:
		req_dex = int(ceil(req_dex) * (1.0 - lower))

	req_int = item.getintproperty( 'req_intelligence', 0 )
	if item.hastag( 'req_intelligence' ):
		req_int = int( item.gettag( 'req_intelligence' ) )
	if lower:
		req_int = int(ceil(req_int) * (1.0 - lower))

	if wearer.strength < req_str:
		if player != wearer:
			player.socket.sysmessage('This person can\'t wear that item, seems not strong enough.')
		else:
			player.socket.clilocmessage(500213)
		return True

	if wearer.dexterity < req_dex:
		if player != wearer:
			player.socket.sysmessage('This person can\'t wear that item, seems not agile enough.')
		else:
			player.socket.clilocmessage(502077)
		return True

	if wearer.intelligence < req_int:
		if player != wearer:
			player.socket.sysmessage('This person can\'t wear that item, seems not smart enough.')
		else:
			player.socket.sysmessage('You are not ingellgent enough to equip this item.')
		return True

	# Reject equipping an item with durability 1 or less
	# if it's an armor, shield or weapon
	armor = properties.itemcheck(item, ITEM_ARMOR)
	weapon = properties.itemcheck(item, ITEM_WEAPON)
	shield = properties.itemcheck(item, ITEM_SHIELD)

	if (armor or weapon or shield) and item.health < 1:
		player.socket.sysmessage('You need to repair this before using it again.')
		return True

	return False

#
# Grant certain stat or skill boni.
#
def onEquip(char, item, layer):
	changed = 0

	# Bonus Str
	bonus = properties.fromitem(item, BONUSSTRENGTH)
	if char.strength + bonus < 1:
		bonus = 1 - char.strength
	if char.strength + bonus > char.strengthcap:
		bonus = max(0, char.strengthcap - char.strength)
	if bonus != 0:
		char.strength += bonus
		char.strength2 += bonus
		item.settag('real_strength_bonus', bonus)
		changed = True
	else:
		item.deltag('real_strength_bonus')

	# Bonus Dex
	bonus = properties.fromitem(item, BONUSDEXTERITY)
	if char.dexterity + bonus < 1:
		bonus = 1 - char.dexterity
	if char.dexterity + bonus > char.dexteritycap:
		bonus = max(0, char.dexteritycap - char.dexterity)
	if bonus != 0:
		char.dexterity += bonus
		char.dexterity2 += bonus
		item.settag('real_dexterity_bonus', bonus)
		changed = True
	else:
		item.deltag('real_dexterity_bonus')

	# Bonus Int
	bonus = properties.fromitem(item, BONUSINTELLIGENCE)
	if char.intelligence + bonus < 1:
		bonus = 1 - char.intelligence
	if char.intelligence + bonus > char.intelligencecap:
		bonus = max(0, char.intelligencecap - char.intelligence)
	if bonus != 0:
		char.intelligence += bonus
		char.intelligence2 += bonus
		item.settag('real_intelligence_bonus', bonus)
		changed = True
	else:
		item.deltag('real_intelligence_bonus')

	# Skill Boni (1-5)
	for i in range(0, 5):
		if item.hastag('skillbonus_%u' % i):
			try:
				(skill, bonus) = item.gettag('skillbonus_%u' % i).split(',')
				(skill, bonus) = (int(skill), int(bonus))

				if bonus == 0 or skill < 0 or skill >= ALLSKILLS:
					item.deltag('real_skillbonus_%u' % i)
					continue

				# Add a Bonus for the skill
				if char.skill[skill] + bonus < 0:
					bonus = - char.skill[skill]

				item.settag('real_skillbonus_%u' % i, '%u,%u' % (skill, bonus))
				char.skill[skill] += bonus
			except:
				item.deltag('skillbonus_%u' % i)
				continue

			# Update the bonus tag for the character
			tagname = 'skillbonus_%u' % skill
			if char.hastag(tagname):
				value = int(char.gettag(tagname)) + bonus
			else:
				value = bonus
			char.settag(tagname, value)
		else:
			item.deltag('real_skillbonus_%u' % i)

	# Bonus Hitpoints
	bonushitpoints = properties.fromitem(item, BONUSHITPOINTS)
	if bonushitpoints != 0:
		char.hitpointsbonus += bonushitpoints
		changed = True

	# Bonus Stamina
	bonusstamina = properties.fromitem(item, BONUSSTAMINA)
	if bonusstamina != 0:
		char.staminabonus += bonusstamina
		changed = True

	# Bonus Mana
	bonusmana = properties.fromitem(item, BONUSMANA)
	if bonusmana != 0:
		char.manabonus += bonusmana
		changed = True

	# Add hitpoint regeneration rate bonus
	regenhitpoints = properties.fromitem(item, REGENHITPOINTS)
	if regenhitpoints:
		if char.hastag('regenhitpoints'):
			regenhitpoints += int(char.gettag('regenhitpoints'))

		char.settag('regenhitpoints', regenhitpoints)

	# Add stamina regeneration rate bonus
	regenstamina = properties.fromitem(item, REGENSTAMINA)
	if regenstamina:
		if char.hastag('regenstamina'):
			regenstamina += int(char.gettag('regenstamina'))

		char.settag('regenstamina', regenstamina)

	# Add mana regeneration rate bonus
	regenmana = properties.fromitem(item, REGENMANA)
	if regenmana:
		if char.hastag('regenmana'):
			regenmana += int(char.gettag('regenmana'))

		char.settag('regenmana', regenmana)

	# Update Stats
	if changed:
		char.updatestats()

#
# Remove certain stat or skill boni.
#
def onUnequip(char, item, layer):
	changed = 0

	# Bonus Str
	if item.hastag('real_strength_bonus'):
		value = int(item.gettag('real_strength_bonus'))
		char.strength = max(1, char.strength - value)
		char.strength2 -= value
		changed = True
		item.deltag('real_strength_bonus')

	# Bonus Dex
	if item.hastag('real_dexterity_bonus'):
		value = int(item.gettag('real_dexterity_bonus'))
		char.dexterity = max(1, char.dexterity - value)
		char.dexterity2 -= value
		changed = True
		item.deltag('real_dexterity_bonus')

	# Bonus Int
	if item.hastag('real_intelligence_bonus'):
		value = int(item.gettag('real_intelligence_bonus'))
		char.intelligence = max(1, char.intelligence - value)
		char.intelligence2 -= value
		changed = True
		item.deltag('real_intelligence_bonus')

	# Skill Boni (1-5)
	for i in range(0, 5):
		if item.hastag('real_skillbonus_%u' % i):
			try:
				(skill, bonus) = item.gettag('real_skillbonus_%u' % i).split(',')
				(skill, bonus) = (int(skill), int(bonus))

				if bonus == 0 or skill < 0 or skill >= ALLSKILLS:
					item.deltag('real_skillbonus_%u' % i)
					continue

				# If the bonus would add over the skill limit,
				# make sure it doesnt
				if char.skill[skill] - bonus < 0:
					bonus = char.skill[skill]

				item.deltag('real_skillbonus_%u' % i, bonus)
				char.skill[skill] -= bonus

				# Update the bonus tag for the character
				tagname = 'skillbonus_%u' % skill
				if char.hastag(tagname):
					value = int(char.gettag(tagname)) - bonus

					if value != 0:
						char.settag(tagname, value)
					else:
						char.deltag(tagname)
			except:
				item.deltag('real_skillbonus_%u' % i)
				continue

	# Bonus Hitpoints
	bonushitpoints = properties.fromitem(item, BONUSHITPOINTS)
	if bonushitpoints != 0:
		char.hitpointsbonus -= bonushitpoints
		changed = True

	# Bonus Stamina
	bonusstamina = properties.fromitem(item, BONUSSTAMINA)
	if bonusstamina != 0:
		char.staminabonus -= bonusstamina
		changed = True

	# Bonus Mana
	bonusmana = properties.fromitem(item, BONUSMANA)
	if bonusmana != 0:
		char.manabonus -= bonusmana
		changed = True

	# Remove hitpoint regeneration rate bonus
	regenhitpoints = properties.fromitem(item, REGENHITPOINTS)
	if regenhitpoints and char.hastag('regenhitpoints'):
		value = int(char.gettag('regenhitpoints')) - regenhitpoints

		if value <= 0:
			char.deltag('regenhitpoints')
		else:
			char.settag('regenhitpoints', value)

	# Remove stamina regeneration rate bonus
	regenstamina = properties.fromitem(item, REGENSTAMINA)
	if regenstamina and char.hastag('regenstamina'):
		value = int(char.gettag('regenstamina')) - regenstamina

		if value <= 0:
			char.deltag('regenstamina')
		else:
			char.settag('regenstamina', value)

	# Remove mana regeneration rate bonus
	regenmana = properties.fromitem(item, REGENMANA)
	if regenmana and char.hastag('regenmana'):
		value = int(char.gettag('regenmana')) - regenmana

		if value <= 0:
			char.deltag('regenmana')
		else:
			char.settag('regenmana', value)

	# Update Stats
	if changed:
		char.updatestats()

#
# Remove boni
#
def onDelete(item):
	if not item.container or not item.container.ischar():
		return

	char = item.container
	onUnequip(char, item, item.layer)

# Try to equip an item after calling onWearItem for it
def onUse(player, item):
	if not player.gm and item.movable == 3:
		player.objectdelay = 0
		return True
		
	if item.container == player:
		return False

	tile = wolfpack.tiledata(item.id)

	if not tile.has_key('layer'):
		return False

	layer = tile['layer']

	if layer == 0 or not (tile['flag3'] & 0x40):
		return False

	previous = player.itemonlayer(layer)
	if previous:
		tobackpack(previous, player)
		previous.soundeffect(0x57)
		previous.update()

	if layer == LAYER_RIGHTHAND or layer == LAYER_LEFTHAND:
		# Check if we're equipping on one of the standard layers
		righthand = player.itemonlayer(LAYER_RIGHTHAND)
		lefthand = player.itemonlayer(LAYER_LEFTHAND)

		if righthand and (righthand.twohanded or (layer == 2 and item.twohanded)):
			tobackpack(righthand, player)
			righthand.update()
			righthand.soundeffect(0x57)

		if lefthand and (lefthand.twohanded or (layer == 1 and item.twohanded)):
			tobackpack(lefthand, player)
			lefthand.update()
			lefthand.soundeffect(0x57)

	# Check if there is another dclick handler
	# in the eventchain somewhere. if not,
	# return True to handle the equip event.
	scripts = item.scripts

	for script in scripts:
		if wolfpack.hasevent(script, EVENT_WEARITEM):
			result = wolfpack.callevent(script, EVENT_WEARITEM, (player, player, item, layer))
			if result:
				return True

	player.additem(layer, item)
	item.update()
	item.soundeffect(0x57)
	player.updatestats()

	# Remove the use delay, equipping should be for free...
	player.objectdelay = 0

	for script in scripts[scripts.index("equipment")+1:]:
		if wolfpack.hasevent(script, EVENT_USE):
			return False

	return True
