
import wolfpack
from wolfpack.consts import *
from wolfpack import console, utilities
import random
from system.lootlists import *

# Indices
PACK_CHANCE = 0
PACK_ITEM = 1
PACK_AMOUNT = 2
PACK_STACKABLE = 3

# Lootpacks
# [Chance (of 1.0), Item/Item List, Amount, Stackable]
# Make sure you don't accidently mix stackable items with non stackable items if you
# use a list of item ids instead of just a single id.
#
PACKS = {
	# Gold Packs
	'verypoor': [
		[0.9, 'eed', '1d10', 1] # 1 - 10
	],
	'poor': [
		[1.0, 'eed', '1d10+10', 1] # 11 - 20
	],
	'meager': [
		[1.0, 'eed', '3d10+20', 1] # 23 - 50
	],
	'average': [
		[1.0, 'eed', '5d10+50', 1] # 55 - 100
	],
	'rich': [
		[1.0, 'eed', '10d10+150', 1] # 160 - 250
	],
	'filthyrich': [
		[1.0, 'eed', '2d100+200', 1] # 202 - 400
	],
	'ultrarich': [
		[1.0, 'eed', '5d100+500', 1] # 505 - 1000
	],
	'superboss': [
		[1.0, 'eed', '10d100+1000', 1] # 1010 - 2000
	],
	# Gem Packs
	'gems_poor': [
		[1.0, DEF_BASEGEMS, '1d2', 1] # 100%, 1 - 2
	],
	'gems_low': [
		[1.0, DEF_BASEGEMS, '1d4+1', 1], # 100%, 2 - 5
		[0.5, DEF_BASEGEMS, '1d4', 1] # 50%, 1 - 4
	],
	'gems_medium': [
		[1.0, DEF_BASEGEMS, '1d6+2', 1], # 100%, 3 - 8
		[0.5, DEF_BASEGEMS, '1d2', 1] # 50%, 1 - 2
	],
	'gems_high': [
		[1.0, DEF_BASEGEMS, '1d8+5', 1], # 100%, 6 - 13
		[0.66, DEF_BASEGEMS, '1d6+5', 1], # 66%, 6 - 11
		[0.33, DEF_BASEGEMS, '1d4+5', 1] # 33%, 6 - 9
	],
	'gems_veryhigh': [
		[1.0, DEF_BASEGEMS, '1d10+5', 1], # 100%, 6 - 15
		[0.66, DEF_BASEGEMS, '1d10+8', 1], # 66%, 9 - 18
		[0.33, DEF_BASEGEMS, '1d10+10', 1] # 33%, 11 - 20
	],
	# Instrument Packs
	'instrument_spoor': [
		[0.25, DEF_INSTRUMENTS, 1, 0]
	],
	'instrument_average': [
		[0.5, DEF_INSTRUMENTS, 1, 0]
	],
	'instruments_high': [
		[0.75, DEF_INSTRUMENTS, 1, 0]
	],
	'instruments_always': [
		[1.0, DEF_INSTRUMENTS, 1, 0]
	],
	# Weapons Packs
	'weapons_all': [
		[1.0, DEF_ALLWEAPONS, 1, 0]
	],
	'weapons_swords': [
		[1.0, DEF_WEAPONS_SWORDS, 1, 0]
	],
	'weapons_axes': [
		[1.0, DEF_WEAPONS_AXES, 1, 0]
	],
	'weapons_swordsmanship': [
		[1.0, DEF_WEAPONS_SWORDSMANSHIP, 1, 0]
	],
	'weapons_maces': [
		[1.0, DEF_WEAPONS_MACES, 1, 0]
	],
	'weapons_fencing': [
		[1.0, DEF_WEAPONS_FENCING, 1, 0]
	],
	'weapons_bows': [
		[1.0, DEF_WEAPONS_BOWS, 1, 0]
	],
	'weapons_high_all': [
		[0.75, DEF_ALLWEAPONS, 1, 0]
	],
	'weapons_high_swords': [
		[0.75, DEF_WEAPONS_SWORDS, 1, 0]
	],
	'weapons_high_axes': [
		[0.75, DEF_WEAPONS_AXES, 1, 0]
	],
	'weapons_high_swordsmanship': [
		[0.75, DEF_WEAPONS_SWORDSMANSHIP, 1, 0]
	],
	'weapons_high_maces': [
		[0.75, DEF_WEAPONS_MACES, 1, 0]
	],
	'weapons_high_fencing': [
		[0.75, DEF_WEAPONS_FENCING, 1, 0]
	],
	'weapons_high_bows': [
		[0.75, DEF_WEAPONS_BOWS, 1, 0]
	],
	'weapons_med_all': [
		[0.5, DEF_ALLWEAPONS, 1, 0]
	],
	'weapons_med_swords': [
		[0.5, DEF_WEAPONS_SWORDS, 1, 0]
	],
	'weapons_med_axes': [
		[0.5, DEF_WEAPONS_AXES, 1, 0]
	],
	'weapons_med_swordsmanship': [
		[0.5, DEF_WEAPONS_SWORDSMANSHIP, 1, 0]
	],
	'weapons_med_maces': [
		[0.5, DEF_WEAPONS_MACES, 1, 0]
	],
	'weapons_med_fencing': [
		[0.5, DEF_WEAPONS_FENCING, 1, 0]
	],
	'weapons_med_bows': [
		[0.5, DEF_WEAPONS_BOWS, 1, 0]
	],
	'weapons_low_all': [
		[0.25, DEF_ALLWEAPONS, 1, 0]
	],
	'weapons_low_swords': [
		[0.25, DEF_WEAPONS_SWORDS, 1, 0]
	],
	'weapons_low_axes': [
		[0.25, DEF_WEAPONS_AXES, 1, 0]
	],
	'weapons_low_swordsmanship': [
		[0.25, DEF_WEAPONS_SWORDSMANSHIP, 1, 0]
	],
	'weapons_low_maces': [
		[0.25, DEF_WEAPONS_MACES, 1, 0]
	],
	'weapons_low_fencing': [
		[0.25, DEF_WEAPONS_FENCING, 1, 0]
	],
	'weapons_low_bows': [
		[0.25, DEF_WEAPONS_BOWS, 1, 0]
	],
	# Armor Packs
	'armor_all_low': [
		[0.33, DEF_ALLARMOR, 1, 0]
	],
	'armor_all_med': [
		[0.66, DEF_ALLARMOR, 1, 0]
	],
	'armor_all_high': [
		[0.99, DEF_ALLARMOR, 1, 0]
	],
	'armor_platemail_low': [
		[0.33, DEF_PLATEMAIL, 1, 0]
	],
	'armor_platemail_med': [
		[0.66, DEF_PLATEMAIL, 1, 0]
	],
	'armor_platemail_high': [
		[0.99, DEF_PLATEMAIL, 1, 0]
	],
	'armor_ringmail_low': [
		[0.33, DEF_RINGMAIL, 1, 0]
	],
	'armor_ringmail_med': [
		[0.66, DEF_RINGMAIL, 1, 0]
	],
	'armor_ringmail_high': [
		[0.99, DEF_RINGMAIL, 1, 0]
	],
	'armor_chainmail_low': [
		[0.33, DEF_CHAINMAIL, 1, 0]
	],
	'armor_chainmail_med': [
		[0.66, DEF_CHAINMAIL, 1, 0]
	],
	'armor_chianmail_high': [
		[0.99, DEF_CHAINMAIL, 1, 0]
	],
	'armor_bonemail_low': [
		[0.33, DEF_BONEMAIL, 1, 0]
	],
	'armor_bonemail_med': [
		[0.66, DEF_BONEMAIL, 1, 0]
	],
	'armor_bonemail_high': [
		[0.99, DEF_BONEMAIL, 1, 0]
	],
	'armor_shields_low': [
		[0.33, DEF_SHIELDS, 1, 0]
	],
	'armor_shields_med': [
		[0.66, DEF_SHIELDS, 1, 0]
	],
	'armor_shields_high': [
		[0.99, DEF_SHIELDS, 1, 0]
	],
	'armor_helms_low': [
		[0.33, DEF_HELMS, 1, 0]
	],
	'armor_helms_med': [
		[0.66, DEF_HELMS, 1, 0]
	],
	'armor_helms_high': [
		[0.99, DEF_HELMS, 1, 0]
	],
	'armor_leather_low': [
		[0.33, DEF_LEATHER, 1, 0]
	],
	'armor_leather_med': [
		[0.66, DEF_LEATHER, 1, 0]
	],
	'armor_leather_high': [
		[0.99, DEF_LEATHER, 1, 0]
	],
	'armor_studded_low': [
		[0.33, DEF_STUDDED, 1, 0]
	],
	'armor_studded_med': [
		[0.66, DEF_STUDDED, 1, 0]
	],
	'armor_studded_high': [
		[0.99, DEF_STUDDED, 1, 0]
	],
	'armor_hats_low': [
		[0.33, DEF_HATS, 1, 0]
	],
	'armor_hats_med': [
		[0.66, DEF_HATS, 1, 0]
	],
	'armor_hats_high': [
		[0.99, DEF_HATS, 1, 0]
	],
	'armor_masks_low': [
		[0.33, DEF_ALLMASKS, 1, 0]
	],
	'armor_masks_med': [
		[0.66, DEF_ALLMASKS, 1, 0]
	],
	'armor_masks_high': [
		[0.99, DEF_ALLMASKS, 1, 0]
	],
	'armor_orcmasks_low': [
		[0.33, DEF_ORCMASKS, 1, 0]
	],
	'armor_orcmasks_med': [
		[0.66, DEF_ORCMASKS, 1, 0]
	],
	'armor_orcmasks_high': [
		[0.99, DEF_ORCMASKS, 1, 0]
	],
	'armor_tribalmasks_low': [
		[0.33, DEF_TRIBALMASKS, 1, 0]
	],
	'armor_tribalmasks_med': [
		[0.66, DEF_TRIBALMASKS, 1, 0]
	],
	'armor_tribalmasks_high': [
		[0.99, DEF_TRIBALMASKS, 1, 0]
	],
	'armor_orchelms_low': [
		[0.33, DEF_ORCHELMS, 1, 0]
	],
	'armor_orchelms_med': [
		[0.66, DEF_ORCHELMS, 1, 0]
	],
	'armor_orchelms_high': [
		[0.99, DEF_ORCHELMS, 1, 0]
	],
	# Reagent Packs

	# Monster Packs
	'orcpack_low': [
		[0.6, DEF_FOOD_MEATS + DEF_FOOD_GREENS, 1, 0],
		[0.52, DEF_DRINKS_ALCOHOL, 1, 0],
		[0.15, DEF_WEAPONS_SWORDSMANSHIP, 1, 0],
		[0.23, DEF_ORCHEADGEAR, 1, 0]
	],
	'orcpack_med': [
		[0.8, DEF_FOOD_MEATS + DEF_FOOD_GREENS, 1, 0],
		[0.62, DEF_DRINKS_ALCOHOL, 1, 0],
		[0.48, DEF_WEAPONS_SWORDSMANSHIP, 1, 0],
		[0.5, DEF_ORCHEADGEAR, 1, 0]
	],
	'orcpack_high': [
		[0.95, DEF_FOOD_MEATS + DEF_FOOD_GREENS, 1, 0],
		[0.85, DEF_DRINKS_ALCOHOL, 1, 0],
		[0.65, DEF_WEAPONS_SWORDSMANSHIP, 1, 0],
		[0.75, DEF_ORCHEADGEAR, 1, 0]
	],
	# Food Packs
	'foodpack_low': [
		[0.33, DEF_FOOD_MEATS + DEF_FOOD_GREENS, 1, 0]
	],
	'foodpack_med': [
		[0.66, DEF_FOOD_MEATS + DEF_FOOD_GREENS, 1, 0]
	],
	'foodpack_high': [
		[0.99, DEF_FOOD_MEATS + DEF_FOOD_GREENS, 1, 0]
	],
	# Alcohol Drink Packs
	'alcoholpack_low': [
		[0.33, DEF_DRINKS_ALCOHOL, 1, 0]
	],
	'alcoholpack_med': [
		[0.66, DEF_DRINKS_ALCOHOL, 1, 0]
	],
	'alcoholpack_high': [
		[0.99, DEF_DRINKS_ALCOHOL, 1, 0]
	]
	#
}

#
# Move item to corpse or to ground (and stack if neccesary)
#
def dropitem(item, char, container):
	if container:
		if not utilities.tocontainer(item, container):
			item.update()
	else:
		items = wolfpack.items(char.pos.x, char.pos.y, char.pos.map, 0)
		for stackable in items:
			if stackable.pos.z == item.pos.z:
				if item.canstack(stackable):
					# We can completely stack the item
					if stackable.amount + item.amount <= 65535:
						stackable.amount += item.amount
						stackable.update()
						item.delete()
						return

		item.moveto(char.pos)
		item.update()

#
# Create a specific loot pack
#
def createpack(char, killer, corpse, pack):
	# A pack is actually a list of lists
	for item in pack:
		if item[PACK_CHANCE] >= random.random():
			if type(item[PACK_AMOUNT]) == str:
				amount = utilities.rolldice(item[PACK_AMOUNT])
			else:
				amount = int(item[PACK_AMOUNT])

			if item[PACK_STACKABLE]:
				if type(item[PACK_ITEM]) == list:
					itemid = random.choice(item[PACK_ITEM])
				else:
					itemid = str(item[PACK_ITEM])

				item = wolfpack.additem(itemid)
				item.amount = amount
				dropitem(item, char, corpse)
			else:
				for i in range(0, amount):
					if type(item[PACK_ITEM]) == list:
						itemid = random.choice(item[PACK_ITEM])
					else:
						itemid = str(item[PACK_ITEM])

					item = wolfpack.additem(itemid)
					dropitem(item, char, corpse)

#
# Create Loot
#
def onDeath(char, killer, corpse):
	if char.player:
		return

	lootlist = char.lootlist

	if len(lootlist) == 0:
		return

	packs = lootlist.split(';')

	# Create the items for each pack
	for pack in packs:
		if not PACKS.has_key(pack):
			console.log(LOG_ERROR, "Trying to create an unknown loot pack %s.\n" % pack)
			continue

		createpack(char, killer, corpse, PACKS[pack])

#
# Register global onDeath event
#
def onLoad():
	wolfpack.registerglobal(EVENT_DEATH, "system.loot")
