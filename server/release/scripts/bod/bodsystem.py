#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Antigravity                    #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Bulk Order System Core (Enhanced / OSI Compatible)             #
#===============================================================#

import random
import wolfpack
from wolfpack.consts import *
from powerscroll import create_powerscroll

#
# Materials Definitions
#
SMITH_MATERIALS = {
	0: {"name": "Iron", "resname": "iron", "color": 0, "pts": 0},
	1: {"name": "Dull Copper", "resname": "dullcopper", "color": 0x973, "pts": 200},
	2: {"name": "Shadow Iron", "resname": "shadowiron", "color": 0x966, "pts": 250},
	3: {"name": "Copper", "resname": "copper", "color": 0x96D, "pts": 300},
	4: {"name": "Bronze", "resname": "bronze", "color": 0x972, "pts": 350},
	5: {"name": "Gold", "resname": "gold", "color": 0x8A5, "pts": 400},
	6: {"name": "Agapite", "resname": "agapite", "color": 0x979, "pts": 450},
	7: {"name": "Verite", "resname": "verite", "color": 0x89F, "pts": 500},
	8: {"name": "Valorite", "resname": "valorite", "color": 0x8AB, "pts": 550},
}

TAILOR_MATERIALS = {
	0: {"name": "Cloth", "resname": "cloth", "color": 0, "pts": 0},
	1: {"name": "Plain Leather", "resname": "leather", "color": 0, "pts": 0},
	2: {"name": "Spined Leather", "resname": "spined", "color": 0x8AC, "pts": 100},
	3: {"name": "Horned Leather", "resname": "horned", "color": 0x845, "pts": 200},
	4: {"name": "Barbed Leather", "resname": "barbed", "color": 0x851, "pts": 300},
}

#
# Blacksmith Craft Items Registry
#
SMITH_ITEMS = {
	# Armors - Ringmail
	"ringmail_gloves": {"name": "ringmail gloves", "graphic": 0x13EB, "set": "ringmail"},
	"ringmail_tunic": {"name": "ringmail tunic", "graphic": 0x13EC, "set": "ringmail"},
	"ringmail_sleeves": {"name": "ringmail sleeves", "graphic": 0x13EE, "set": "ringmail"},
	"ringmail_leggings": {"name": "ringmail leggings", "graphic": 0x13F0, "set": "ringmail"},

	# Armors - Chainmail
	"chainmail_coif": {"name": "chainmail coif", "graphic": 0x13BB, "set": "chainmail"},
	"chainmail_leggings": {"name": "chainmail leggings", "graphic": 0x13BE, "set": "chainmail"},
	"chainmail_tunic": {"name": "chainmail tunic", "graphic": 0x13BF, "set": "chainmail"},

	# Armors - Platemail
	"platemail_arms": {"name": "platemail arms", "graphic": 0x1410, "set": "platemail"},
	"platemail_legs": {"name": "platemail legs", "graphic": 0x1411, "set": "platemail"},
	"platemail_helm": {"name": "platemail helm", "graphic": 0x1412, "set": "platemail"},
	"platemail_gorget": {"name": "platemail gorget", "graphic": 0x1413, "set": "platemail"},
	"platemail_gloves": {"name": "platemail gloves", "graphic": 0x1414, "set": "platemail"},
	"platemail_tunic": {"name": "platemail chest", "graphic": 0x1415, "set": "platemail"},

	# Weapons - Fencing
	"dagger": {"name": "dagger", "graphic": 0xF52, "set": "fencing"},
	"short_spear": {"name": "short spear", "graphic": 0x1403, "set": "fencing"},
	"spear": {"name": "spear", "graphic": 0xF62, "set": "fencing"},
	"war_fork": {"name": "war fork", "graphic": 0x1405, "set": "fencing"},
	"kryss": {"name": "kryss", "graphic": 0x1401, "set": "fencing"},

	# Weapons - Swords
	"broadsword": {"name": "broadsword", "graphic": 0xF5E, "set": "swords"},
	"cutlass": {"name": "cutlass", "graphic": 0x1441, "set": "swords"},
	"katana": {"name": "katana", "graphic": 0x13FF, "set": "swords"},
	"longsword": {"name": "longsword", "graphic": 0xF61, "set": "swords"},
	"scimitar": {"name": "scimitar", "graphic": 0x13B6, "set": "swords"},
	"viking_sword": {"name": "viking sword", "graphic": 0x13B9, "set": "swords"},

	# Weapons - Axes
	"axe": {"name": "axe", "graphic": 0xF49, "set": "axes"},
	"battle_axe": {"name": "battle axe", "graphic": 0xF47, "set": "axes"},
	"double_axe": {"name": "double axe", "graphic": 0xF4B, "set": "axes"},
	"executioners_axe": {"name": "executioner's axe", "graphic": 0xF45, "set": "axes"},
	"large_battle_axe": {"name": "large battle axe", "graphic": 0x13FB, "set": "axes"},
	"two_handed_axe": {"name": "two handed axe", "graphic": 0x1443, "set": "axes"},

	# Weapons - Maces / Hammers
	"war_axe": {"name": "war axe", "graphic": 0x13B0, "set": "maces"},
	"hammer_pick": {"name": "hammer pick", "graphic": 0x143D, "set": "maces"},
	"mace": {"name": "mace", "graphic": 0xF5C, "set": "maces"},
	"maul": {"name": "maul", "graphic": 0x143B, "set": "maces"},
	"war_hammer": {"name": "war hammer", "graphic": 0x1439, "set": "maces"},
	"war_mace": {"name": "war mace", "graphic": 0x1407, "set": "maces"},
}

SMITH_SETS = {
	"ringmail": ["ringmail_gloves", "ringmail_tunic", "ringmail_sleeves", "ringmail_leggings"],
	"chainmail": ["chainmail_coif", "chainmail_leggings", "chainmail_tunic"],
	"platemail": ["platemail_arms", "platemail_legs", "platemail_helm", "platemail_gorget", "platemail_gloves", "platemail_tunic"],
	"fencing": ["dagger", "short_spear", "spear", "war_fork", "kryss"],
	"swords": ["broadsword", "cutlass", "katana", "longsword", "scimitar", "viking_sword"],
	"axes": ["axe", "battle_axe", "double_axe", "executioners_axe", "large_battle_axe", "two_handed_axe"],
	"maces": ["war_axe", "hammer_pick", "mace", "maul", "war_hammer", "war_mace"],
}

#
# Tailor Craft Items Registry
#
TAILOR_ITEMS = {
	# Leather Armor
	"leather_cap": {"name": "leather cap", "graphic": 0x1DB9, "set": "leather_set"},
	"leather_tunic": {"name": "leather tunic", "graphic": 0x13CC, "set": "leather_set"},
	"leather_sleeves": {"name": "leather sleeves", "graphic": 0x13C5, "set": "leather_set"},
	"leather_leggings": {"name": "leather leggings", "graphic": 0x13CB, "set": "leather_set"},
	"leather_gorget": {"name": "leather gorget", "graphic": 0x13C7, "set": "leather_set"},
	"leather_gloves": {"name": "leather gloves", "graphic": 0x13C6, "set": "leather_set"},

	# Studded Armor
	"studded_tunic": {"name": "studded tunic", "graphic": 0x13E2, "set": "studded_set"},
	"studded_sleeves": {"name": "studded sleeves", "graphic": 0x13DC, "set": "studded_set"},
	"studded_leggings": {"name": "studded leggings", "graphic": 0x13DA, "set": "studded_set"},
	"studded_gorget": {"name": "studded gorget", "graphic": 0x13D6, "set": "studded_set"},
	"studded_gloves": {"name": "studded gloves", "graphic": 0x13D5, "set": "studded_set"},

	# Townfolk Clothing
	"fancy_shirt": {"name": "fancy shirt", "graphic": 0x1EFD, "set": "town_folk"},
	"long_pants": {"name": "long pants", "graphic": 0x1539, "set": "town_folk"},
	"skirt": {"name": "skirt", "graphic": 0x1516, "set": "town_folk"},
	"shoes": {"name": "shoes", "graphic": 0x170F, "set": "town_folk"},

	# Wizard Clothing
	"robe": {"name": "robe", "graphic": 0x1F03, "set": "wizard_set"},
	"wizards_hat": {"name": "wizard's hat", "graphic": 0x1718, "set": "wizard_set"},
	"cloak": {"name": "cloak", "graphic": 0x1515, "set": "wizard_set"},
	"boots": {"name": "boots", "graphic": 0x170B, "set": "wizard_set"},
}

TAILOR_SETS = {
	"leather_set": ["leather_cap", "leather_tunic", "leather_sleeves", "leather_leggings", "leather_gorget", "leather_gloves"],
	"studded_set": ["studded_tunic", "studded_sleeves", "studded_leggings", "studded_gorget", "studded_gloves"],
	"town_folk": ["fancy_shirt", "long_pants", "skirt", "shoes"],
	"wizard_set": ["robe", "wizards_hat", "cloak", "boots"],
}

#
# Points Computation (Standard)
#
def compute_points(bod_type, quantity, exceptional, material_idx, is_large=False, set_name=None):
	points = 0
	if quantity == 10:
		points += 10
	elif quantity == 15:
		points += 25
	elif quantity == 20:
		points += 50

	if exceptional:
		points += 200

	if bod_type == "smith":
		mat_info = SMITH_MATERIALS.get(material_idx, SMITH_MATERIALS[0])
		points += mat_info["pts"]
		if is_large:
			points += 300 # Large BOD completion bonus
	else:
		mat_info = TAILOR_MATERIALS.get(material_idx, TAILOR_MATERIALS[0])
		points += mat_info["pts"]
		if is_large:
			points += 250

	return points

#
# Reward Delivery
#
def give_smith_reward(player, points):
	# Base Gold Reward
	gold_amount = max(250, points * 15)
	gold = wolfpack.additem('eed')
	if gold:
		gold.amount = gold_amount
		pack = player.getbackpack()
		if not (pack and wolfpack.utilities.tocontainer(gold, pack)):
			gold.moveto(player.pos)

	# Items Tiers
	reward_item = None
	if points >= 900: # Valorite Runic or 120 PowerScroll
		if random.random() < 0.5:
			reward_item = create_powerscroll(SWORDSMANSHIP if random.random() < 0.5 else BLACKSMITHING, 1200)
		else:
			reward_item = wolfpack.additem("13e3") # Hammer
			if reward_item:
				reward_item.name = "valorite runic hammer"
				reward_item.color = SMITH_MATERIALS[8]["color"]
				reward_item.settag("resname", "valorite")
				reward_item.settag("uses", 50)
	elif points >= 750: # Verite / 115 PS
		if random.random() < 0.5:
			reward_item = create_powerscroll(BLACKSMITHING, 1150)
		else:
			reward_item = wolfpack.additem("13e3")
			if reward_item:
				reward_item.name = "verite runic hammer"
				reward_item.color = SMITH_MATERIALS[7]["color"]
				reward_item.settag("resname", "verite")
				reward_item.settag("uses", 45)
	elif points >= 600: # Agapite / Gold Runic / 110 PS
		if random.random() < 0.4:
			reward_item = create_powerscroll(BLACKSMITHING, 1100)
		else:
			reward_item = wolfpack.additem("13e3")
			if reward_item:
				reward_item.name = "gold runic hammer"
				reward_item.color = SMITH_MATERIALS[5]["color"]
				reward_item.settag("resname", "gold")
				reward_item.settag("uses", 40)
	elif points >= 450: # Bronze / Copper Runic / Colored Anvil
		if random.random() < 0.3:
			reward_item = wolfpack.additem("faf") # Anvil
			if reward_item:
				reward_item.name = "colored anvil"
				reward_item.color = random.choice([0x973, 0x966, 0x96D, 0x972, 0x8A5])
		else:
			reward_item = wolfpack.additem("13e3")
			if reward_item:
				reward_item.name = "copper runic hammer"
				reward_item.color = SMITH_MATERIALS[3]["color"]
				reward_item.settag("resname", "copper")
				reward_item.settag("uses", 35)
	elif points >= 300: # Shadow / Dull Copper Runic / Powder of Temperament
		reward_item = wolfpack.additem("13e3")
		if reward_item:
			reward_item.name = "dull copper runic hammer"
			reward_item.color = SMITH_MATERIALS[1]["color"]
			reward_item.settag("resname", "dullcopper")
			reward_item.settag("uses", 30)
	elif points >= 150: # Prospector's Tool or Gargoyle Pickaxe
		reward_item = wolfpack.additem("fb4") # Prospector Tool
		if reward_item:
			reward_item.name = "prospector's tool"
			reward_item.settag("uses", 50)
	else: # Sturdy Shovel or Pickaxe
		reward_item = wolfpack.additem("f39") # Shovel
		if reward_item:
			reward_item.name = "sturdy shovel"
			reward_item.color = 0x973
			reward_item.settag("uses", 180)

	if reward_item:
		pack = player.getbackpack()
		if not (pack and wolfpack.utilities.tocontainer(reward_item, pack)):
			reward_item.moveto(player.pos)

	return gold_amount, reward_item

def give_tailor_reward(player, points):
	gold_amount = max(200, points * 12)
	gold = wolfpack.additem('eed')
	if gold:
		gold.amount = gold_amount
		pack = player.getbackpack()
		if not (pack and wolfpack.utilities.tocontainer(gold, pack)):
			gold.moveto(player.pos)

	reward_item = None
	if points >= 700: # Barbed Runic Kit or 120 PowerScroll
		if random.random() < 0.5:
			reward_item = create_powerscroll(TAILORING, 1200)
		else:
			reward_item = wolfpack.additem("f9d") # Sewing kit
			if reward_item:
				reward_item.name = "barbed leather runic sewing kit"
				reward_item.color = TAILOR_MATERIALS[4]["color"]
				reward_item.settag("resname", "barbed")
				reward_item.settag("uses", 45)
	elif points >= 500: # Horned Runic Kit or 115 PowerScroll
		if random.random() < 0.4:
			reward_item = create_powerscroll(TAILORING, 1150)
		else:
			reward_item = wolfpack.additem("f9d")
			if reward_item:
				reward_item.name = "horned leather runic sewing kit"
				reward_item.color = TAILOR_MATERIALS[3]["color"]
				reward_item.settag("resname", "horned")
				reward_item.settag("uses", 40)
	elif points >= 350: # Spined Runic Kit or 110 PowerScroll
		if random.random() < 0.3:
			reward_item = create_powerscroll(TAILORING, 1100)
		else:
			reward_item = wolfpack.additem("f9d")
			if reward_item:
				reward_item.name = "spined leather runic sewing kit"
				reward_item.color = TAILOR_MATERIALS[2]["color"]
				reward_item.settag("resname", "spined")
				reward_item.settag("uses", 35)
	else: # Special Colored Cloth
		reward_item = wolfpack.additem("175d") # Folded cloth
		if reward_item:
			reward_item.amount = 100
			reward_item.name = "special cloth"
			reward_item.color = random.choice([0x483, 0x488, 0x48E, 0x495, 0x499])

	if reward_item:
		pack = player.getbackpack()
		if not (pack and wolfpack.utilities.tocontainer(reward_item, pack)):
			reward_item.moveto(player.pos)

	return gold_amount, reward_item

#
# BOD Generation (Small & Large)
#
def generate_bod_for_player(player, bod_type):
	"""
	Generates a suitable Small or Large BOD based on player's skill.
	"""
	skill_val = player.skill[BLACKSMITHING] if bod_type == "smith" else player.skill[TAILORING]

	# Quantity: 10 (45%), 15 (35%), 20 (20%)
	q_roll = random.random()
	if q_roll < 0.45:
		amount_max = 10
	elif q_roll < 0.80:
		amount_max = 15
	else:
		amount_max = 20

	# Exceptional: depends on skill
	req_exceptional = 0
	if skill_val >= 700:
		req_exceptional = 1 if random.random() < 0.60 else 0
	elif skill_val >= 500:
		req_exceptional = 1 if random.random() < 0.35 else 0

	# Material Selection
	if bod_type == "smith":
		if skill_val >= 950:
			mat_idx = random.randint(0, 8)
		elif skill_val >= 850:
			mat_idx = random.randint(0, 6)
		elif skill_val >= 700:
			mat_idx = random.randint(0, 4)
		else:
			mat_idx = 0
	else:
		if skill_val >= 950:
			mat_idx = random.randint(0, 4)
		elif skill_val >= 800:
			mat_idx = random.randint(0, 3)
		elif skill_val >= 600:
			mat_idx = random.randint(0, 2)
		else:
			mat_idx = 0

	# Large BOD Chance: 25% if skill >= 70.0
	is_large = False
	if skill_val >= 700 and random.random() < 0.25:
		is_large = True

	if is_large:
		deed = wolfpack.newitem("large_bod")
		if not deed:
			return None
		sets_dict = SMITH_SETS if bod_type == "smith" else TAILOR_SETS
		set_name = random.choice(list(sets_dict.keys()))
		deed.settag("bod_type", bod_type)
		deed.settag("set_name", set_name)
		deed.settag("amount_max", amount_max)
		deed.settag("exceptional", req_exceptional)
		deed.settag("material", mat_idx)
		deed.hue = 0x44E if bod_type == "smith" else 0x483
		deed.resendtooltip()
		return deed
	else:
		deed = wolfpack.newitem("small_bod")
		if not deed:
			return None
		items_dict = SMITH_ITEMS if bod_type == "smith" else TAILOR_ITEMS
		item_key = random.choice(list(items_dict.keys()))
		deed.settag("bod_type", bod_type)
		deed.settag("item_def", item_key)
		deed.settag("item_name", items_dict[item_key]["name"])
		deed.settag("graphic", items_dict[item_key]["graphic"])
		deed.settag("amount_max", amount_max)
		deed.settag("amount_cur", 0)
		deed.settag("exceptional", req_exceptional)
		deed.settag("material", mat_idx)
		deed.hue = 0x44E if bod_type == "smith" else 0x483
		deed.resendtooltip()
		return deed
