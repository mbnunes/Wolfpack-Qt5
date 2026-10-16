#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Antigravity                    #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Champion Boss Script (Damage Tracking, Gold Shower, Scrolls) #
#===============================================================#

import random
import wolfpack
import wolfpack.utilities
from wolfpack.consts import *
from powerscroll import create_powerscroll

# Storage for damage dealt to champion bosses: { boss_serial: { player_serial: total_damage } }
boss_damage = {}

def get_player_source(source):
	if not source:
		return None
	if source.player:
		return source
	if hasattr(source, 'owner') and source.owner and source.owner.player:
		return source.owner
	return None

def onDamage(char, type, amount, source):
	player = get_player_source(source)
	if player:
		b_serial = char.serial
		if b_serial not in boss_damage:
			boss_damage[b_serial] = {}
		p_serial = player.serial
		boss_damage[b_serial][p_serial] = boss_damage[b_serial].get(p_serial, 0) + int(amount)
	return False

def onDeath(char, source, corpse):
	boss_serial = char.serial
	boss_pos = char.pos

	# 1. Distribute Power Scrolls to top damagers
	distribute_powerscrolls(char)

	# 2. Trigger Gold Shower around boss corpse/altar
	do_gold_shower(boss_pos)

	# 3. Notify Champion Altar
	if char.hastag('champ_altar'):
		try:
			altar_serial = int(char.gettag('champ_altar'))
			altar = wolfpack.finditem(altar_serial)
			if altar:
				from champions.championspawn import on_champion_defeated
				on_champion_defeated(altar)
		except Exception as e:
			wolfpack.console.log(LOG_ERROR, f"Error notifying champ altar: {e}")

	if boss_serial in boss_damage:
		del boss_damage[boss_serial]
	return False

def onDelete(char):
	boss_serial = char.serial
	if boss_serial in boss_damage:
		del boss_damage[boss_serial]
	if char.hastag('champ_altar'):
		try:
			altar_serial = int(char.gettag('champ_altar'))
			altar = wolfpack.finditem(altar_serial)
			if altar:
				from champions.championspawn import on_champion_defeated
				on_champion_defeated(altar)
		except Exception:
			pass
	return False

def distribute_powerscrolls(boss):
	boss_serial = boss.serial
	entries = boss_damage.get(boss_serial, {})

	candidates = []
	for p_serial, dmg in sorted(entries.items(), key=lambda kv: kv[1], reverse=True):
		player = wolfpack.findchar(p_serial)
		if player and player.player and player.pos.map == boss.pos.map:
			if player.distanceto(boss) <= 100:
				candidates.append(player)

	# If no players in damage list, check players nearby within 35 tiles
	if not candidates:
		for nearby in wolfpack.chars(boss.pos.x, boss.pos.y, boss.pos.map, 35):
			if nearby and nearby.player:
				candidates.append(nearby)

	if not candidates:
		return

	# Default: 6 power scrolls
	total_scrolls = 6
	for i in range(total_scrolls):
		lucky_player = candidates[i % len(candidates)]
		ps = create_powerscroll()
		if ps:
			pack = lucky_player.getbackpack()
			if not (pack and wolfpack.utilities.tocontainer(ps, pack)):
				ps.moveto(lucky_player.pos)

			# Cliloc: You have received a scroll of power!
			lucky_player.socket.clilocmessage(1049524)
			lucky_player.pos.soundeffect(0x243)
			lucky_player.effect(0x375A, 35, 90)

def do_gold_shower(center_pos, piles=50, min_amount=4000, max_amount=5500):
	"""
	Drops gold piles in an 8-tile radius around the altar/boss with fire & explosion effects.
	"""
	cx = center_pos.x
	cy = center_pos.y
	cz = center_pos.z
	cmap = center_pos.map

	for _ in range(piles):
		dx = random.randint(-8, 8)
		dy = random.randint(-8, 8)
		if dx * dx + dy * dy > 64:
			continue

		target_pos = wolfpack.coord(cx + dx, cy + dy, cz, cmap)
		gold = wolfpack.additem('eed')
		if gold:
			gold.amount = random.randint(min_amount, max_amount)
			gold.moveto(target_pos)

			# Visual explosion/fire effect
			eff = random.choice([0, 1, 2])
			if eff == 0:
				target_pos.soundeffect(0x208)
				wolfpack.effect(0x3709, target_pos, 10, 30)
			elif eff == 1:
				target_pos.soundeffect(0x307)
				wolfpack.effect(0x36BD, 20, 10)
			else:
				wolfpack.effect(0x36FE, target_pos, 10, 10)
