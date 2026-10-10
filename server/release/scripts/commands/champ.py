#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Antigravity                    #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Champion Spawn Command for GMs/Staff                          #
#===============================================================#

r"""
	\command champ
	\description Manage Champion Spawns.
	Usage:
	  'champ' - Target a location to create or an altar to manage
	  'champ create [type]' - Create a champion spawn at your position
	  'champ start' - Start nearest champion spawn
	  'champ stop' - Stop nearest champion spawn
	  'champ boss' - Force spawn boss at nearest champion spawn
"""

import wolfpack
from wolfpack.consts import *
from champions.championspawn import (
	CHAMPION_TYPES,
	send_champ_gump,
	start_champion_spawn,
	stop_champion_spawn,
	spawn_boss
)

def find_nearest_altar(char, max_dist=15):
	nearest = None
	min_dist = max_dist + 1
	for item in wolfpack.items(char.pos.x, char.pos.y, char.pos.map, max_dist):
		if item and item.hastag('champ_type'):
			dist = char.distanceto(item)
			if dist < min_dist:
				min_dist = dist
				nearest = item
	return nearest

def commandChamp(socket, cmd, args):
	char = socket.player
	if not char:
		return True

	args_str = args.strip().lower()
	tokens = args_str.split()

	if not tokens:
		altar = find_nearest_altar(char)
		if altar:
			send_champ_gump(char, altar)
		else:
			socket.sysmessage("Select location to create a Champion Spawn, or target an existing Altar:")
			socket.attachtarget("commands.champ.target_callback", [])
		return True

	sub = tokens[0]

	if sub == "create":
		champ_type = "vermin_horde"
		if len(tokens) > 1 and tokens[1] in CHAMPION_TYPES:
			champ_type = tokens[1]

		altar = wolfpack.additem("champ_altar")
		if altar:
			altar.moveto(char.pos)
			altar.settag("champ_type", champ_type)
			socket.sysmessage(f"Champion Altar created at your location with type '{champ_type}'.")
			send_champ_gump(char, altar)
		else:
			socket.sysmessage("Failed to create Champion Altar.")
		return True

	elif sub == "start":
		altar = find_nearest_altar(char)
		if altar:
			start_champion_spawn(altar)
			socket.sysmessage("Champion spawn started.")
		else:
			socket.sysmessage("No Champion Altar found nearby.")
		return True

	elif sub == "stop":
		altar = find_nearest_altar(char)
		if altar:
			stop_champion_spawn(altar)
			socket.sysmessage("Champion spawn stopped.")
		else:
			socket.sysmessage("No Champion Altar found nearby.")
		return True

	elif sub == "boss":
		altar = find_nearest_altar(char)
		if altar:
			spawn_boss(altar)
			socket.sysmessage("Champion Boss spawned!")
		else:
			socket.sysmessage("No Champion Altar found nearby.")
		return True

	elif sub == "list":
		types_list = ", ".join(CHAMPION_TYPES.keys())
		socket.sysmessage(f"Available Champion Types: {types_list}")
		return True

	else:
		socket.sysmessage("Usage: champ [create <type> | start | stop | boss | list]")
		return True

def target_callback(char, args, target):
	if target.item and target.item.hastag('champ_type'):
		send_champ_gump(char, target.item)
		return True

	pos = target.pos
	altar = wolfpack.additem("champ_altar")
	if altar:
		altar.moveto(pos)
		char.socket.sysmessage("Champion Altar created.")
		send_champ_gump(char, altar)
	else:
		char.socket.sysmessage("Could not create Champion Altar.")
	return True

def onLoad():
	wolfpack.registercommand("champ", commandChamp)
