
import wolfpack
from wolfpack.utilities import *
from wolfpack.consts import *
from combat.properties import itemcheck, fromitem, fromchar
from combat.utilities import weaponskill
from combat import armorinfo, weaponinfo
from math import ceil

#
# Show certain modifiers stored in tags.
#
def modifiers(object, tooltip):
	modifiers = {
		"boni_dex": 1060409,
		"boni_int": 1060432,
		"boni_str": 1060485,
		"remaining_uses": 1060584,
		"aos_boni_damage": 1060401,
		"aos_boni_durability": 1060410,
		"regenhitpoints": 1060444,
		"regenstamina": 1060443,
		"regenmana": 1060440,
	}
	
	for (tag, cliloc) in modifiers.items():
		if object.hastag(tag):
			tooltip.add(cliloc, str(object.gettag(tag)))
			
	if object.hastag('magearmor'):
		tooltip.add(1060437)

#
# Equipment has a lot of additional effects.
# These are shown to the user in form of tooltips.
#
def onShowTooltip(viewer, object, tooltip):
	armor = itemcheck(object, ITEM_ARMOR)
	weapon = itemcheck(object, ITEM_WEAPON)
	shield = itemcheck(object, ITEM_SHIELD)

	if (armor or weapon or shield) and object.amount == 1:
		# Reinsert the name if we need an ore prefix
		prefix1 = None
		if object.hastag('resname'):		
			resname = str(object.gettag('resname'))
			if armor and armorinfo.ARMOR_RESNAME_BONI.has_key(resname):
				resinfo = armorinfo.ARMOR_RESNAME_BONI[resname]
				if resinfo.has_key(MATERIALPREFIX):
					prefix1 = resinfo[MATERIALPREFIX]				
			if weapon and weaponinfo.WEAPON_RESNAME_BONI.has_key(resname):
				resinfo = weaponinfo.WEAPON_RESNAME_BONI[resname]
				if resinfo.has_key(MATERIALPREFIX):
					prefix1 = resinfo[MATERIALPREFIX]
		
		prefix2 = None
		if object.hastag('resname2'):
			resname2 = str(object.gettag('resname2'))
			if armor and armorinfo.ARMOR_RESNAME_BONI.has_key(resname2):
				resinfo = armorinfo.ARMOR_RESNAME_BONI[resname2]
				if resinfo.has_key(MATERIALPREFIX):
					prefix2 = resinfo[MATERIALPREFIX]
			if weapon and weaponinfo.WEAPON_RESNAME_BONI.has_key(resname2):
				resinfo = weaponinfo.WEAPON_RESNAME_BONI[resname2]
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
	
	# Weapon specific properties
	if weapon:
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

		# Special weapon range
		if object.hastag('range'):
			weaponrange = int(object.gettag('range'))
			if weaponrange > 1:
				tooltip.add(1061169, str(weaponrange))

		# Max-Mindamage
		mindamage = fromitem(object, MINDAMAGE)
		maxdamage = fromitem(object, MAXDAMAGE)
		tooltip.add(1061168, "%u\t%u" % (mindamage, maxdamage))		

		# Speed
		speed = fromitem(object, SPEED)
		tooltip.add(1061167, str(speed))

		# Physical Damage Distribution		
		fire = fromitem(object, DAMAGE_FIRE)
		cold = fromitem(object, DAMAGE_COLD)
		poison = fromitem(object, DAMAGE_POISON)
		energy = fromitem(object, DAMAGE_ENERGY)
		physical = 100 - (fire + cold + poison + energy)

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

	fire = fromitem(object, RESISTANCE_FIRE)
	cold = fromitem(object, RESISTANCE_COLD)
	poison = fromitem(object, RESISTANCE_POISON)
	energy = fromitem(object, RESISTANCE_ENERGY)
	physical = fromitem(object, RESISTANCE_PHYSICAL)

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
	
	lower = fromitem(object, LOWERREQS)
	if lower:
		tooltip.add(1060435, str(lower))
	lower /= 100.0

	req_str = fromitem(object, REQSTR)
	if lower:
		req_str = int(ceil(req_str) * (1.0 - lower))
	if req_str:
		tooltip.add(1061170, str(req_str))

#
# Check for certain equipment requirements
#
def onWearItem(player, wearer, item, layer):
	lower = fromitem(item, LOWERREQS) / 100.0

	req_str = fromitem(item, REQSTR)
	if lower:
		req_str = int(ceil(req_str) * (1.0 - lower))
		
	req_dex = fromitem(item, REQDEX)
	if lower:
		req_dex = int(ceil(req_dex) * (1.0 - lower))
		
	req_int = fromitem(item, REQINT)
	if lower:
		req_int = int(ceil(req_int) * (1.0 - lower))

	if wearer.strength < req_str:
		if player != wearer:
			player.socket.sysmessage('This person can\'t wear that item, seems not strong enough.')
		else:
			player.socket.clilocmessage(500213)
		return 1
			
	if wearer.dexterity < req_dex:
		if player != wearer:
			player.socket.sysmessage('This person can\'t wear that item, seems not agile enough.')
		else:
			player.socket.clilocmessage(502077)
		return 1

	if wearer.intelligence < req_int:
		if player != wearer:
			player.socket.sysmessage('This person can\'t wear that item, seems not smart enough.')
		else:
			player.socket.sysmessage('You are not ingellgent enough to equip this item.')
		return 1

	# Reject equipping an item with durability 1 or less
	# if it's an armor, shield or weapon
	armor = itemcheck(item, ITEM_ARMOR)
	weapon = itemcheck(item, ITEM_WEAPON)
	shield = itemcheck(item, ITEM_SHIELD)

	if (armor or weapon or shield) and item.health < 1:
		player.socket.sysmessage('You need to repair this before using it again.')
		return 1		

	return 0

#
# Grant certain stat or skill boni.
#
def onEquip(char, item, layer):
	changed = 0

	# Bonus Strength
	if item.hastag('boni_str'):
		char.strength = char.strength + int(item.gettag('boni_str'))
		changed = 1

	# Bonus Dex
	if item.hastag('boni_dex'):
		char.dexterity = char.dexterity + int(item.gettag('boni_dex'))
		changed = 1

	# Bonus Int
	if item.hastag('boni_int'):
		char.intelligence = char.intelligence + int(item.gettag('boni_int'))
		changed = 1

	# Add hitpoint regeneration rate bonus
	if item.hastag('regenhitpoints'):
		if char.hastag('regenhitpoints'):
			regenhitpoints = int(char.gettag('regenhitpoints')) + int(item.gettag('regenhitpoints'))
		else:
			regenhitpoints = int(item.gettag('regenhitpoints'))

		char.settag('regenhitpoints', regenhitpoints)
		
	# Add stamina regeneration rate bonus
	if item.hastag('regenstamina'):
		if char.hastag('regenstamina'):
			regenstamina = int(char.gettag('regenstamina')) + int(item.gettag('regenstamina'))
		else:
			regenstamina = int(item.gettag('regenstamina'))

		char.settag('regenstamina', regenstamina)		

	# Update Stats
	if changed:
		char.updatestats()

#
# Remove certain stat or skill boni.
#
def onUnequip(char, item, layer):
	changed = 0

	# Bonus Str
	if item.hastag('boni_str'):
		char.strength = char.strength - int(item.gettag('boni_str'))
		changed = 1

	# Bonus Dex
	if item.hastag('boni_dex'):
		char.dexterity = char.dexterity - int(item.gettag('boni_dex'))
		changed = 1

	# Bonus Int
	if item.hastag('boni_int'):
		char.intelligence = char.intelligence - int(item.gettag('boni_int'))
		changed = 1

	# Remove the hitpoint regeneration rate bonus
	if item.hastag('regenhitpoints') and char.hastag('regenhitpoints'):
		regenhitpoints = int(char.gettag('regenhitpoints'))
		regenhitpoints -= int(item.gettag('regenhitpoints'))
		if regenhitpoints <= 0:
			char.deltag('regenhitpoints')
		else:
			char.settag('regenhitpoints', regenhitpoints)
			
	# Remove the stamina regeneration rate bonus
	if item.hastag('regenstamina') and char.hastag('regenstamina'):
		regenstamina = int(char.gettag('regenstamina'))
		regenstamina -= int(item.gettag('regenstamina'))
		if regenstamina <= 0:
			char.deltag('regenstamina')
		else:
			char.settag('regenstamina', regenstamina)			

	# Update Stats
	if changed:
		char.updatestats()
