#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Antigravity                    #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Peerless Boss Management Commands                             #
#===============================================================#

r"""
	\command peerless
	\description Manage Peerless Bosses, Altars, and Keys.
	Usage:
	  'peerless create <dread_horn|chief_paroxysmus>' - Create a peerless altar
	  'peerless keys <dread_horn|chief_paroxysmus>' - Give all keys for testing
	  'peerless boss <dread_horn|chief_paroxysmus>' - Spawn boss directly
"""

import wolfpack
from peerless.peerless_system import PEERLESS_CONFIG, create_peerless_key

def commandPeerless(socket, cmd, args):
	player = socket.player
	if not player:
		return True

	tokens = args.strip().lower().split()
	sub = tokens[0] if tokens else "help"

	if sub == "create":
		p_type = tokens[1] if len(tokens) > 1 and tokens[1] in PEERLESS_CONFIG else "dread_horn"
		cfg = PEERLESS_CONFIG[p_type]

		altar = wolfpack.additem("peerless_altar")
		if altar:
			altar.name = cfg["altar_name"]
			altar.id = int(cfg["altar_id"], 16) if isinstance(cfg["altar_id"], str) else cfg["altar_id"]
			altar.color = cfg["altar_hue"]
			altar.settag("peerless_type", p_type)
			altar.moveto(player.pos)
			socket.sysmessage(f"{cfg['altar_name']} created at your position.")
		else:
			socket.sysmessage("Failed to create Peerless Altar.")
		return True

	elif sub == "keys":
		p_type = tokens[1] if len(tokens) > 1 and tokens[1] in PEERLESS_CONFIG else "dread_horn"
		cfg = PEERLESS_CONFIG[p_type]

		pack = player.getbackpack()
		count = 0
		for k_code in cfg["keys"].keys():
			key_item = create_peerless_key(k_code)
			if key_item:
				if not (pack and wolfpack.utilities.tocontainer(key_item, pack)):
					key_item.moveto(player.pos)
				count += 1
		socket.sysmessage(f"Placed {count} keys for {cfg['name']} in your backpack.")
		return True

	elif sub == "boss":
		p_type = tokens[1] if len(tokens) > 1 and tokens[1] in PEERLESS_CONFIG else "dread_horn"
		cfg = PEERLESS_CONFIG[p_type]
		boss = wolfpack.addnpc(cfg["boss_id"], player.pos)
		if boss:
			boss.addscript("peerless.boss")
			socket.sysmessage(f"{cfg['name']} spawned at your position.")
		return True

	elif sub == "list":
		types_str = ", ".join(PEERLESS_CONFIG.keys())
		socket.sysmessage(f"Available Peerless Bosses: {types_str}")
		return True

	else:
		socket.sysmessage("Usage: peerless [create <type> | keys <type> | boss <type> | list]")
		return True

def onLoad():
	wolfpack.registercommand("peerless", commandPeerless)
