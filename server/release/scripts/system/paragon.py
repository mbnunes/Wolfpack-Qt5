#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Antigravity                    #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Paragon System & Minor Artifact Drops (ServUO Compatible)     #
#===============================================================#

import random
import math
import wolfpack
import wolfpack.utilities
from wolfpack.consts import *
from wolfpack import tr

PARAGON_HUE = 0x501

# Buff Multipliers
HITS_BUFF = 5.0
STR_BUFF = 1.05
INT_BUFF = 1.20
DEX_BUFF = 1.20
SKILLS_BUFF = 1.20
DAMAGE_BUFF = 5
FAME_BUFF = 1.40
KARMA_BUFF = 1.40
CHEST_CHANCE = 0.10

# All 31 Minor Artifacts from minor_artifacts.xml
PARAGON_ARTIFACTS = [
	"alchemists_bauble", "arctic_death_dealer", "blaze_death",
	"bow_of_the_juka_king", "burglars_bandana", "candelabra_souls",
	"captain_quacklebushs_cutlass", "cavorting_club", "cold_blood",
	"dread_pirate_hat", "enchanted_titan_leg_bone", "ghost_ship_anchor",
	"gloves_of_the_pugilist", "gold_bricks", "gwennos_harp",
	"heart_of_the_lion", "iolos_lute", "luna_lance",
	"nights_kiss", "nox_rangers_heavy_crossbow", "orcish_visage",
	"phillips_wooden_steed", "pixie_swatter", "polar_bear_mask",
	"seahorse_statuette", "shield_of_invulnerability",
	"ship_model_of_the_hms_cape", "staff_of_power",
	"admirals_hearty_rum", "violet_courage", "wrath_of_the_dryad"
]

CHEST_HUES = [
	0x0, 0x455, 0x47E, 0x501, 0x89F, 0x8A5, 0x8AB,
	0x966, 0x96D, 0x972, 0x973, 0x979
]

CHEST_IDS = ["9ab", "e40", "e41", "e7c"]

GEMS = ["f10", "f13", "f15", "f16", "f18", "f25", "f26", "f2d"]

def check_convert(bc):
	"""
	Determines if a spawned creature should become a Paragon.
	"""
	if not bc or not bc.npc:
		return False

	if bc.player or bc.summoned or bc.owner:
		return False

	if bc.hastag("is_paragon") or bc.hastag("champ_altar") or bc.hasscript("npc.vendor"):
		return False

	# Ilshenar is Map 2 in UO
	ilshenar_only = wolfpack.settings.getbool("Paragon", "Ilshenar Only", True)
	if ilshenar_only and bc.pos.map != 2:
		return False

	fame = min(32000, bc.fame)
	divisor = max(1.0, 20.0 - (fame / 3200.0))
	chance = 1.0 / round(divisor)

	if random.random() < chance:
		convert(bc)
		return True

	return False

def convert(bc):
	"""
	Buffs the creature to Paragon status.
	"""
	if bc.hastag("is_paragon"):
		return

	bc.settag("is_paragon", "1")
	bc.settag("orig_skin", bc.skin)
	bc.settag("orig_maxhits", bc.maxhitpoints)
	bc.settag("orig_str", bc.strength)
	bc.settag("orig_dex", bc.dexterity)
	bc.settag("orig_int", bc.intelligence)
	bc.settag("orig_fame", bc.fame)

	# Visual
	bc.skin = PARAGON_HUE
	if not "(Paragon)" in bc.name:
		bc.name = f"{bc.name} (Paragon)"

	# Buffs
	bc.maxhitpoints = int(bc.maxhitpoints * HITS_BUFF)
	bc.hitpoints = bc.maxhitpoints

	bc.strength = int(bc.strength * STR_BUFF)
	bc.dexterity = int(bc.dexterity * DEX_BUFF)
	bc.intelligence = int(bc.intelligence * INT_BUFF)
	bc.stamina = bc.dexterity
	bc.mana = bc.intelligence

	# Skills +20%
	for s in range(ALLSKILLS):
		val = bc.skill[s]
		if val > 0:
			bc.skill[s] = int(val * SKILLS_BUFF)

	# Damage +5
	bc.mindamage += DAMAGE_BUFF
	bc.maxdamage += DAMAGE_BUFF

	# Fame & Karma
	bc.fame = min(32000, int(bc.fame * FAME_BUFF))
	if bc.karma != 0:
		bc.karma = min(32000, max(-32000, int(bc.karma * KARMA_BUFF)))

	bc.addscript("system.paragon")

def unconvert(bc):
	"""
	Reverts a creature from Paragon status.
	"""
	if not bc.hastag("is_paragon"):
		return

	try:
		bc.skin = int(bc.gettag("orig_skin", 0))
		bc.maxhitpoints = int(bc.gettag("orig_maxhits", bc.maxhitpoints))
		bc.hitpoints = min(bc.hitpoints, bc.maxhitpoints)
		bc.strength = int(bc.gettag("orig_str", bc.strength))
		bc.dexterity = int(bc.gettag("orig_dex", bc.dexterity))
		bc.intelligence = int(bc.gettag("orig_int", bc.intelligence))
		bc.fame = int(bc.gettag("orig_fame", bc.fame))
	except Exception:
		pass

	bc.name = bc.name.replace(" (Paragon)", "")
	bc.deltag("is_paragon")
	bc.removescript("system.paragon")

def onDeath(char, killer, corpse):
	"""
	Handles Paragon Chest drop and Minor Artifact reward upon Paragon death.
	"""
	if not char.hastag("is_paragon"):
		return False

	player = None
	if killer:
		if killer.player:
			player = killer
		elif hasattr(killer, "owner") and killer.owner and killer.owner.player:
			player = killer.owner

	# 1. Paragon Chest Drop in corpse (10% chance)
	if random.random() < CHEST_CHANCE and corpse:
		chest = create_paragon_chest(char)
		if chest:
			corpse.additem(chest)

	# 2. Minor Artifact Reward to killer
	if player:
		if check_artifact_chance(player, char):
			give_artifact_to(player)

	return False

def check_artifact_chance(player, bc):
	fame = min(32000, max(1, bc.fame))
	luck = 0
	try:
		if hasattr(player, "luck"):
			luck = player.luck
	except Exception:
		luck = 0

	try:
		log_val = math.log10((fame / 6000.0) + 0.001)
		denominator = max(10.0, 100.0 * (0.83 - round(log_val, 3)))
		luck_factor = max(1.0, (100.0 - math.sqrt(max(0, luck))) / 100.0)
		chance = 1.0 / (denominator * luck_factor)
	except Exception:
		chance = 0.02

	return random.random() < chance

def give_artifact_to(player):
	art_id = random.choice(PARAGON_ARTIFACTS)
	item = wolfpack.additem(art_id)
	if not item:
		return

	pack = player.getbackpack()
	if not (pack and wolfpack.utilities.tocontainer(item, pack)):
		item.moveto(player.pos)

	player.socket.sysmessage("As a reward for slaying the mighty paragon, an artifact has been placed in your backpack.")
	player.pos.soundeffect(0x243)
	player.effect(0x375A, 35, 90)

def create_paragon_chest(monster):
	"""
	Generates a locked and trapped Paragon Chest with loot.
	"""
	chest = wolfpack.additem(random.choice(CHEST_IDS))
	if not chest:
		return None

	chest.color = random.choice(CHEST_HUES)
	clean_name = monster.name.replace(" (Paragon)", "")
	chest.name = f"a paragon chest (recovered from a {clean_name})"

	# Determine level 1 to 5 from monster fame
	fame = monster.fame
	if fame < 5000:
		lvl = 1
		lock_diff = 360
	elif fame < 10000:
		lvl = 2
		lock_diff = 760
	elif fame < 15000:
		lvl = 3
		lock_diff = 840
	elif fame < 20000:
		lvl = 4
		lock_diff = 920
	else:
		lvl = 5
		lock_diff = 1000

	# Lock properties
	chest.addscript("lock")
	chest.settag("lock", "lockpick")
	chest.settag("locked", "1")
	chest.settag("lockpick_difficult", lock_diff)

	# Explosion Trap
	chest.addscript("magic.trap")
	chest.settag("trap_type", "explosion")
	chest.settag("trap_damage", lvl * 15)

	# Fill Chest with Gold and Gems
	gold = wolfpack.additem("eed")
	if gold:
		gold.amount = random.randint(lvl * 500, lvl * 1200)
		chest.additem(gold)

	for _ in range(random.randint(1, 3)):
		gem = wolfpack.additem(random.choice(GEMS))
		if gem:
			gem.amount = random.randint(lvl * 2, lvl * 6)
			chest.additem(gem)

	return chest
