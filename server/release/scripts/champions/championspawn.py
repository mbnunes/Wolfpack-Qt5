#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Antigravity                    #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Champion Spawn Engine (ServUO Compatible)                     #
#===============================================================#

import random
import time
import wolfpack
import wolfpack.gumps
import wolfpack.console
from wolfpack.consts import *
from wolfpack import tr

CHAMPION_TYPES = {
	"abyss": {
		"name": "Abyss",
		"boss": "semidar",
		"waves": [
			["greater_mongbat", "imp"],
			["gargoyle", "harpy"],
			["fire_gargoyle", "stone_gargoyle"],
			["daemon", "succubus"]
		]
	},
	"arachnid": {
		"name": "Arachnid",
		"boss": "mephitis",
		"waves": [
			["scorpion", "giant_spider"],
			["terathan_drone", "terathan_warrior"],
			["dread_spider", "terathan_matriarch"],
			["poison_elemental", "terathan_avenger"]
		]
	},
	"cold_blood": {
		"name": "Cold Blood",
		"boss": "rikktor",
		"waves": [
			["lizardman", "snake"],
			["lava_lizard", "ophidian_warrior"],
			["drake", "ophidian_archmage"],
			["dragon", "ophidian_knight"]
		]
	},
	"forest_lord": {
		"name": "Forest Lord",
		"boss": "lord_oaks",
		"waves": [
			["pixie", "shadow_wisp"],
			["kirin", "wisp"],
			["centaur", "unicorn"],
			["ethereal_warrior", "serpentine_dragon"]
		]
	},
	"vermin_horde": {
		"name": "Vermin Horde",
		"boss": "barracoon_the_piper",
		"waves": [
			["giant_rat", "slime"],
			["dire_wolf", "ratman"],
			["hell_hound", "ratman_mage"],
			["ratman_archer", "silver_serpent"]
		]
	},
	"unholy_terror": {
		"name": "Unholy Terror",
		"boss": "neira",
		"waves": [
			["ghoul", "shade", "spectre", "wraith", "bogle"],
			["bone_magi", "mummy", "skeletal_mage"],
			["bone_knight", "lich", "skeletal_knight"],
			["lich_lord", "rotting_corpse"]
		]
	}
}

# Pentagram tiles in order 3x3
PENTAGRAM_ITEMS = ["fe7", "fe6", "fe9", "fe8", "fea", "fec", "feb", "fee", "fed"]

# Kills required per rank (Rank 0: levels 0-4, Rank 1: levels 5-9, Rank 2: levels 10-13, Rank 3: levels 14-15)
RANK_MAX_KILLS = [128, 64, 32, 16]

def get_rank_for_level(level):
	if level < 5:
		return 0
	elif level < 10:
		return 1
	elif level < 14:
		return 2
	else:
		return 3

def get_max_kills_for_level(level):
	rank = get_rank_for_level(level)
	return RANK_MAX_KILLS[rank]

def get_red_skull_loc(cx, cy, cz, index):
	if index < 5:
		x = index - 2
		y = -2
	elif index < 9:
		x = 2
		y = index - 6
	elif index < 13:
		x = 10 - index
		y = 2
	else:
		x = -2
		y = 14 - index
	return (cx + x, cy + y, cz)

def get_white_skull_loc(cx, cy, cz, index):
	offsets = [(-1, -1), (1, -1), (1, 1), (-1, 1)]
	ox, oy = offsets[index % 4]
	return (cx + ox, cy + oy, cz)

def get_serials_from_tag(item, tag_name):
	if not item.hastag(tag_name):
		return []
	val = str(item.gettag(tag_name)).strip()
	if not val:
		return []
	res = []
	for s in val.split(','):
		s = s.strip()
		if s:
			try:
				res.append(int(s))
			except ValueError:
				pass
	return res

def set_serials_to_tag(item, tag_name, serials):
	if not serials:
		item.deltag(tag_name)
	else:
		item.settag(tag_name, ','.join(str(s) for s in serials))

#
# Platform & Component Creation
#
def build_platform(altar):
	pos = altar.pos
	components = []

	# Base 5x5 sandstone lower layer at z - 5
	for x in range(pos.x - 2, pos.x + 3):
		for y in range(pos.y - 2, pos.y + 3):
			tile = wolfpack.additem("3ee")
			if tile:
				tile.moveto(wolfpack.coord(x, y, pos.z - 5, pos.map))
				tile.color = 0x452
				tile.movable = 0
				components.append(tile.serial)

	# Central 3x3 platform layer at z
	for x in range(pos.x - 1, pos.x + 2):
		for y in range(pos.y - 1, pos.y + 2):
			tile = wolfpack.additem("3ee")
			if tile:
				tile.moveto(wolfpack.coord(x, y, pos.z, pos.map))
				tile.color = 0x452
				tile.movable = 0
				components.append(tile.serial)

	# Edges & Corners at z
	edge_defs = [
		(0x3F7, -2, -2), (0x3F8, 2, 2), (0x3F9, -2, 2), (0x3FA, 2, -2), # Corners
	]
	for id_val, ox, oy in edge_defs:
		tile = wolfpack.additem(hex(id_val)[2:])
		if tile:
			tile.moveto(wolfpack.coord(pos.x + ox, pos.y + oy, pos.z, pos.map))
			tile.color = 0x452
			tile.movable = 0
			components.append(tile.serial)

	for i in [-1, 0, 1]:
		# (i, 2) -> 0x3EF, (2, i) -> 0x3F0, (i, -2) -> 0x3F1, (-2, i) -> 0x3F2
		for id_val, ox, oy in [(0x3EF, i, 2), (0x3F0, 2, i), (0x3F1, i, -2), (0x3F2, -2, i)]:
			tile = wolfpack.additem(hex(id_val)[2:])
			if tile:
				tile.moveto(wolfpack.coord(pos.x + ox, pos.y + oy, pos.z, pos.map))
				tile.color = 0x452
				tile.movable = 0
				components.append(tile.serial)

	# Pentagram 3x3
	idx = 0
	for x in range(pos.x - 1, pos.x + 2):
		for y in range(pos.y - 1, pos.y + 2):
			tile = wolfpack.additem(PENTAGRAM_ITEMS[idx])
			if tile:
				tile.moveto(wolfpack.coord(x, y, pos.z, pos.map))
				tile.color = 0x455 # Inactive hue
				tile.movable = 0
				components.append(tile.serial)
			idx += 1

	set_serials_to_tag(altar, "components", components)

def set_pentagram_active(altar, active):
	components = get_serials_from_tag(altar, "components")
	color = 0 if active else 0x455
	for s in components:
		item = wolfpack.finditem(s)
		if item and item.id in [0xFE6, 0xFE7, 0xFE8, 0xFE9, 0xFEA, 0xFEB, 0xFEC, 0xFED, 0xFEE]:
			item.color = color

def remove_components(altar):
	components = get_serials_from_tag(altar, "components")
	for s in components:
		item = wolfpack.finditem(s)
		if item:
			item.delete()
	altar.deltag("components")

def remove_all_skulls(altar):
	# Red skulls
	for s in get_serials_from_tag(altar, "red_skulls"):
		item = wolfpack.finditem(s)
		if item:
			item.delete()
	altar.deltag("red_skulls")

	# White skulls
	for s in get_serials_from_tag(altar, "white_skulls"):
		item = wolfpack.finditem(s)
		if item:
			item.delete()
	altar.deltag("white_skulls")

def set_red_skull_count(altar, count):
	pos = altar.pos
	current = get_serials_from_tag(altar, "red_skulls")

	# Delete extra skulls
	while len(current) > count:
		s = current.pop()
		item = wolfpack.finditem(s)
		if item:
			item.delete()

	# Add needed skulls
	while len(current) < count:
		idx = len(current)
		sx, sy, sz = get_red_skull_loc(pos.x, pos.y, pos.z, idx)
		skull = wolfpack.additem("1854")
		if skull:
			skull.moveto(wolfpack.coord(sx, sy, sz, pos.map))
			skull.color = 0x26
			skull.movable = 0
			current.append(skull.serial)

			pos.soundeffect(0x29)
			wolfpack.effect(0x3728, skull.pos, 10, 10)

	set_serials_to_tag(altar, "red_skulls", current)

def set_white_skull_count(altar, count):
	pos = altar.pos
	current = get_serials_from_tag(altar, "white_skulls")

	while len(current) > count:
		s = current.pop()
		item = wolfpack.finditem(s)
		if item:
			item.delete()

	while len(current) < count:
		idx = len(current)
		sx, sy, sz = get_white_skull_loc(pos.x, pos.y, pos.z, idx)
		skull = wolfpack.additem("1854")
		if skull:
			skull.moveto(wolfpack.coord(sx, sy, sz, pos.map))
			skull.color = 0
			skull.movable = 0
			current.append(skull.serial)

			pos.soundeffect(0x29)
			wolfpack.effect(0x3728, skull.pos, 10, 10)

	set_serials_to_tag(altar, "white_skulls", current)

#
# Spawn Lifecycle & State
#
def onCreate(item, definition):
	item.settag("champ_type", "vermin_horde")
	item.settag("champ_active", "0")
	item.settag("champ_level", 0)
	item.settag("champ_kills", 0)
	item.settag("spawn_radius", 35)
	item.settag("max_mobs", 40)
	item.settag("auto_restart", "1")
	item.movable = 0
	item.visible = 0 # Altar marker itself is invisible, platform and pentagram are visible
	build_platform(item)

def onDelete(item):
	# Despawn creatures
	for s in get_serials_from_tag(item, "creatures"):
		char = wolfpack.findchar(s)
		if char:
			char.delete()

	# Despawn boss
	if item.hastag("boss_serial"):
		try:
			boss = wolfpack.findchar(int(item.gettag("boss_serial")))
			if boss:
				boss.delete()
		except Exception:
			pass

	remove_all_skulls(item)
	remove_components(item)

def start_champion_spawn(altar):
	altar.settag("champ_active", "1")
	altar.settag("champ_level", 0)
	altar.settag("champ_kills", 0)
	altar.settag("expire_time", int(time.time()) + 600) # 10 min
	set_pentagram_active(altar, True)
	set_red_skull_count(altar, 0)
	set_white_skull_count(altar, 0)

	# Sound & flame effect at center
	altar.pos.soundeffect(0x29)
	wolfpack.effect(0x3728, altar.pos, 10, 10)

	schedule_slice(altar)

def stop_champion_spawn(altar):
	altar.settag("champ_active", "0")
	set_pentagram_active(altar, False)
	remove_all_skulls(altar)

	# Despawn all active creatures
	for s in get_serials_from_tag(altar, "creatures"):
		char = wolfpack.findchar(s)
		if char:
			char.delete()
	altar.deltag("creatures")

	# Despawn boss
	if altar.hastag("boss_serial"):
		try:
			boss = wolfpack.findchar(int(altar.gettag("boss_serial")))
			if boss:
				boss.delete()
		except Exception:
			pass
		altar.deltag("boss_serial")

def schedule_slice(altar):
	wolfpack.addtimer(altar, 3000, "champions.championspawn.slice_timer", [altar.serial])

def slice_timer(args):
	if not args:
		return
	altar = wolfpack.finditem(args[0])
	if not altar:
		return

	if altar.gettag("champ_active") != "1":
		return

	now = int(time.time())

	# 1. Check if Boss is active
	if altar.hastag("boss_serial"):
		boss_serial = int(altar.gettag("boss_serial"))
		boss = wolfpack.findchar(boss_serial)
		if not boss:
			# Boss died or was removed
			on_champion_defeated(altar)
			return
		# Boss is still fighting, reschedule
		schedule_slice(altar)
		return

	# 2. Check if level expired (10 min timer)
	level = int(altar.gettag("champ_level"))
	expire_time = int(altar.gettag("expire_time", now + 600))

	if now >= expire_time:
		white_skulls = get_serials_from_tag(altar, "white_skulls")
		if len(white_skulls) == 0:
			# Lost a level!
			if level > 0:
				level -= 1
				altar.settag("champ_level", level)
				set_red_skull_count(altar, level)
		else:
			# Reset white skulls
			set_white_skull_count(altar, 0)

		altar.settag("champ_kills", 0)
		altar.settag("expire_time", now + 600)

	# 3. Clean up dead creature serials
	creatures = get_serials_from_tag(altar, "creatures")
	valid_creatures = []
	for s in creatures:
		char = wolfpack.findchar(s)
		if char:
			valid_creatures.append(s)
	creatures = valid_creatures
	set_serials_to_tag(altar, "creatures", creatures)

	# 4. Respawn wave creatures up to max_mobs
	max_mobs = int(altar.gettag("max_mobs", 40))
	spawn_radius = int(altar.gettag("spawn_radius", 35))
	champ_type = str(altar.gettag("champ_type", "vermin_horde"))

	if champ_type not in CHAMPION_TYPES:
		champ_type = "vermin_horde"

	info = CHAMPION_TYPES[champ_type]
	rank = get_rank_for_level(level)
	wave_mobs = info["waves"][rank]

	while len(creatures) < max_mobs:
		mob_id = random.choice(wave_mobs)
		dx = random.randint(-spawn_radius, spawn_radius)
		dy = random.randint(-spawn_radius, spawn_radius)
		spawn_pos = wolfpack.coord(altar.pos.x + dx, altar.pos.y + dy, altar.pos.z, altar.pos.map)

		char = wolfpack.addnpc(mob_id, spawn_pos)
		if char:
			char.settag("champ_altar", altar.serial)
			char.addscript("champions.champcreature")
			creatures.append(char.serial)
		else:
			break

	set_serials_to_tag(altar, "creatures", creatures)

	# Re-schedule next slice
	schedule_slice(altar)

#
# Creature Kill Progression
#
def on_creature_killed(altar, creature):
	if altar.gettag("champ_active") != "1":
		return
	if altar.hastag("boss_serial"):
		return

	level = int(altar.gettag("champ_level", 0))
	kills = int(altar.gettag("champ_kills", 0)) + 1
	max_kills = get_max_kills_for_level(level)

	altar.settag("champ_kills", kills)

	# Check progress %
	progress = float(kills) / float(max_kills)

	if progress >= 0.90:
		# Advance level!
		advance_level(altar)
	else:
		# Update white skulls (0 to 4 skulls based on 20% intervals)
		white_count = int(progress / 0.20)
		white_count = min(4, max(0, white_count))
		set_white_skull_count(altar, white_count)

def on_creature_deleted(altar, creature):
	creatures = get_serials_from_tag(altar, "creatures")
	if creature.serial in creatures:
		creatures.remove(creature.serial)
		set_serials_to_tag(altar, "creatures", creatures)

def advance_level(altar):
	level = int(altar.gettag("champ_level", 0)) + 1
	altar.settag("champ_kills", 0)
	altar.settag("expire_time", int(time.time()) + 600)
	set_white_skull_count(altar, 0)

	if level >= 16:
		# Champion Spawn completed! Spawn Boss!
		spawn_boss(altar)
	else:
		altar.settag("champ_level", level)
		set_red_skull_count(altar, level)
		altar.pos.soundeffect(0x29)
		wolfpack.effect(0x3728, altar.pos, 10, 10)

def spawn_boss(altar):
	altar.settag("champ_level", 16)
	remove_all_skulls(altar)

	champ_type = str(altar.gettag("champ_type", "vermin_horde"))
	info = CHAMPION_TYPES.get(champ_type, CHAMPION_TYPES["vermin_horde"])
	boss_id = info["boss"]

	boss = wolfpack.addnpc(boss_id, altar.pos)
	if boss:
		boss.settag("champ_altar", altar.serial)
		boss.addscript("champions.champboss")
		altar.settag("boss_serial", boss.serial)

		# Epic lighting and effects
		altar.pos.soundeffect(0x211)
		wolfpack.effect(0x3709, altar.pos, 10, 30)

def on_champion_defeated(altar):
	altar.deltag("boss_serial")
	altar.settag("champ_level", 0)
	altar.settag("champ_kills", 0)
	set_pentagram_active(altar, False)

	# Despawn remaining minions
	for s in get_serials_from_tag(altar, "creatures"):
		char = wolfpack.findchar(s)
		if char:
			char.delete()
	altar.deltag("creatures")

	# If auto-restart is enabled, schedule restart in 10 minutes (600s)
	if altar.gettag("auto_restart", "1") == "1":
		wolfpack.addtimer(altar, 600000, "champions.championspawn.restart_timer", [altar.serial])
	else:
		altar.settag("champ_active", "0")

def restart_timer(args):
	if not args:
		return
	altar = wolfpack.finditem(args[0])
	if altar and altar.gettag("auto_restart", "1") == "1":
		start_champion_spawn(altar)

#
# GM Gump & Inspection
#
def onUse(char, item):
	if not char.player:
		return True
	send_champ_gump(char, item)
	return True

def onShowTooltip(char, item, tooltip):
	tooltip.reset()
	champ_type = str(item.gettag("champ_type", "vermin_horde"))
	active = (item.gettag("champ_active") == "1")
	level = int(item.gettag("champ_level", 0))
	kills = int(item.gettag("champ_kills", 0))
	max_kills = get_max_kills_for_level(level)

	type_name = CHAMPION_TYPES.get(champ_type, {}).get("name", champ_type.capitalize())
	status = "Active" if active else "Inactive"

	tooltip.add(1042971, tr("Champion Altar: %s (%s)" % (type_name, status)))
	if active:
		if item.hastag("boss_serial"):
			tooltip.add(1042971, tr("Champion Boss is summoned!"))
		else:
			tooltip.add(1042971, tr("Level: %d/16 (Kills: %d/%d)" % (level, kills, max_kills)))
	return True

def send_champ_gump(char, altar):
	champ_type = str(altar.gettag("champ_type", "vermin_horde"))
	active = (altar.gettag("champ_active") == "1")
	level = int(altar.gettag("champ_level", 0))
	kills = int(altar.gettag("champ_kills", 0))
	max_kills = get_max_kills_for_level(level)
	has_boss = altar.hastag("boss_serial")
	creature_count = len(get_serials_from_tag(altar, "creatures"))

	gump = wolfpack.gumps.cGump()
	gump.setCallback(gump_callback)
	gump.setArgs([altar.serial])

	gump.addResizeGump(50, 50, 5054, 380, 360)
	gump.addTiledGump(58, 58, 364, 344, 2624)
	gump.addCheckerTrans(58, 58, 364, 344)

	title = tr("<basefont color=#FFFF00><big><center>Champion Spawn Control</center></big></basefont>")
	gump.addHtmlGump(60, 65, 360, 25, title, 0, 0)

	type_name = CHAMPION_TYPES.get(champ_type, {}).get("name", champ_type.capitalize())
	status_text = "<basefont color=#00FF00>Active</basefont>" if active else "<basefont color=#FF0000>Inactive</basefont>"
	gump.addHtmlGump(70, 95, 340, 20, tr("<basefont color=#FFFFFF>Type: <b>%s</b></basefont>" % type_name), 0, 0)
	gump.addHtmlGump(70, 115, 340, 20, tr("<basefont color=#FFFFFF>Status: %s</basefont>" % status_text), 0, 0)
	gump.addHtmlGump(70, 135, 340, 20, tr("<basefont color=#FFFFFF>Level: %d/16 | Kills: %d/%d</basefont>" % (level, kills, max_kills)), 0, 0)
	gump.addHtmlGump(70, 155, 340, 20, tr("<basefont color=#FFFFFF>Active Minions: %d | Boss: %s</basefont>" % (creature_count, "ALIVE" if has_boss else "None")), 0, 0)

	# Control Buttons
	# Button 1: Start / Stop
	gump.addButton(70, 190, 4005, 4007, 1)
	btn1_text = "Stop Spawn" if active else "Start Spawn"
	gump.addHtmlGump(105, 190, 200, 20, tr("<basefont color=#FFFFFF>%s</basefont>" % btn1_text), 0, 0)

	# Button 2: Next Type
	gump.addButton(70, 220, 4005, 4007, 2)
	gump.addHtmlGump(105, 220, 200, 20, tr("<basefont color=#FFFFFF>Change Type</basefont>"), 0, 0)

	# Button 3: Force Next Level
	gump.addButton(70, 250, 4005, 4007, 3)
	gump.addHtmlGump(105, 250, 200, 20, tr("<basefont color=#FFFFFF>Advance Level (+1)</basefont>"), 0, 0)

	# Button 4: Force Spawn Boss
	gump.addButton(70, 280, 4005, 4007, 4)
	gump.addHtmlGump(105, 280, 200, 20, tr("<basefont color=#FFFFFF>Spawn Champion Boss</basefont>"), 0, 0)

	# Button 5: Delete Spawn
	gump.addButton(70, 310, 4005, 4007, 5)
	gump.addHtmlGump(105, 310, 200, 20, tr("<basefont color=#FF5555>Delete Champion Spawn</basefont>"), 0, 0)

	# Close button
	gump.addButton(320, 310, 4017, 4019, 0)

	gump.send(char.socket)

def gump_callback(char, args, response):
	if response.button == 0 or not args:
		return False

	altar = wolfpack.finditem(args[0])
	if not altar:
		return False

	btn = response.button
	if btn == 1: # Start / Stop
		if altar.gettag("champ_active") == "1":
			stop_champion_spawn(altar)
		else:
			start_champion_spawn(altar)
		send_champ_gump(char, altar)

	elif btn == 2: # Change Type
		type_keys = list(CHAMPION_TYPES.keys())
		curr = str(altar.gettag("champ_type", "vermin_horde"))
		if curr in type_keys:
			next_idx = (type_keys.index(curr) + 1) % len(type_keys)
		else:
			next_idx = 0
		altar.settag("champ_type", type_keys[next_idx])
		send_champ_gump(char, altar)

	elif btn == 3: # Advance Level
		advance_level(altar)
		send_champ_gump(char, altar)

	elif btn == 4: # Spawn Boss
		spawn_boss(altar)
		send_champ_gump(char, altar)

	elif btn == 5: # Delete
		altar.delete()
		char.socket.sysmessage("Champion spawn deleted.")
		return True

	return True
