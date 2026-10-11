#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Antigravity                    #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Peerless Boss System Definitions & Data (ServUO Compatible)   #
#===============================================================#

import random
import wolfpack
from wolfpack.consts import *
from powerscroll import create_powerscroll

PEERLESS_CONFIG = {
	"dread_horn": {
		"name": "Dread Horn",
		"boss_id": "dread_horn",
		"altar_name": "Twisted Weald Altar",
		"altar_id": "0x207c",
		"altar_hue": 0,
		"keys": {
			"blighted_cotton": {"name": "Blighted Cotton", "itemid": "df9", "hue": 0x37},
			"gnaws_fang": {"name": "Gnaw's Fang", "itemid": "10e1", "hue": 0x481},
			"irks_brain": {"name": "Irk's Brain", "itemid": "1cf0", "hue": 0x21},
			"lissiths_silk": {"name": "Lissith's Silk", "itemid": "10ee", "hue": 0x35},
			"sabrixs_eye": {"name": "Sabrix's Eye", "itemid": "1f14", "hue": 0x494},
			"thorny_briar": {"name": "Thorny Briar", "itemid": "d15", "hue": 0x10},
		},
		"arena": (2137, 1247, -60, 2), # Twisted Weald lair
		"exit": (1448, 1537, -28, 2)
	},
	"chief_paroxysmus": {
		"name": "Chief Paroxysmus",
		"boss_id": "chief_paroxysmus",
		"altar_name": "Paroxysmus Altar",
		"altar_id": "0x207a",
		"altar_hue": 0x465,
		"keys": {
			"coagulated_legs": {"name": "Coagulated Legs", "itemid": "1ce2", "hue": 0x465},
			"digested_torso": {"name": "Partially Digested Torso", "itemid": "1d9f", "hue": 0x465},
			"gelatinous_skull": {"name": "Gelatinous Skull", "itemid": "1ae0", "hue": 0x465},
			"putrefier_spleen": {"name": "Spleen of the Putrefier", "itemid": "1cee", "hue": 0x465},
		},
		"arena": (6517, 357, 0, 0), # Paroxysmus lair
		"exit": (5623, 3038, 15, 0)
	}
}

def create_peerless_key(key_code):
	for p_type, cfg in PEERLESS_CONFIG.items():
		if key_code in cfg["keys"]:
			info = cfg["keys"][key_code]
			item = wolfpack.additem(info["itemid"])
			if item:
				item.name = info["name"]
				item.color = info["hue"]
				item.settag("peerless_key", key_code)
				item.settag("peerless_type", p_type)
			return item
	return None

def drop_peerless_loot(boss, corpse):
	"""
	Drops major Mondain's Legacy artifacts and treasures into boss corpse.
	"""
	if not corpse:
		return

	# 1. Guaranteed Gold Pile (5000 - 10000 gp)
	gold = wolfpack.additem("eed")
	if gold:
		gold.amount = random.randint(5000, 10000)
		corpse.additem(gold)

	# 2. Crimson Cincture (rare ~ 5% chance)
	if random.random() < 0.05:
		cincture = wolfpack.additem("153b") # Half apron
		if cincture:
			cincture.name = "Crimson Cincture"
			cincture.color = 0x485
			cincture.settag("hitpointincrease", 10)
			cincture.settag("defensechanceincrease", 20)
			cincture.settag("hp_regen", 2)
			corpse.additem(cincture)

	# 3. Fey Leggings (10% chance)
	if random.random() < 0.10:
		fey = wolfpack.additem("1411") # Plate legs
		if fey:
			fey.name = "Fey Leggings"
			fey.color = 0x481
			fey.settag("defensechanceincrease", 20)
			fey.settag("hitpointincrease", 6)
			corpse.additem(fey)

	# 4. Power Scroll (115 or 120)
	if random.random() < 0.35:
		ps = create_powerscroll(value=1200 if random.random() < 0.4 else 1150)
		if ps:
			corpse.additem(ps)

	# 5. Peerless Trophy / Carvable Item
	trophy = wolfpack.additem("10e1")
	if trophy:
		trophy.name = f"{boss.name}'s Trophy"
		trophy.color = 0x501
		corpse.additem(trophy)
