
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
	]
	'poor': [
		[1.0, 'eed', '1d10+10', 1]
	],
	'meager': [
		[1.0, 'eed', '3d10+20', 1]
	],
	'average': [
		[1.0, 'eed', '5d10+50', 1]
	],
	'rich': [
		[1.0, 'eed', '10d10+150', 1]
	],
	'filthyrich': [
		[1.0, 'eed', '2d100+200', 1]
	],
	'ultrarich': [
		[1.0, 'eed', '5d100+500', 1]
	],
	'superboss': [
		[1.0, 'eed', '5d100+500', 1]
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

	# Reagent Packs
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
