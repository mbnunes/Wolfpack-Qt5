#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Antigravity                    #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Bulk Order Commands (Player & GM)                             #
#===============================================================#

r"""
	\command bod
	\description Manage or create Bulk Order Deeds.
	Usage:
	  'getbod [smith|tailor]' - Request a bulk order deed from nearby vendor
	  'bod create small <smith|tailor> [item] [amount] [exceptional 0/1] [material 0-8]'
	  'bod create large <smith|tailor> [set] [amount] [exceptional 0/1] [material 0-8]'
	  'bod reset' - Reset your BOD waiting timer
"""

import time
import wolfpack
from wolfpack.consts import *
from bod.bodsystem import generate_bod_for_player

def commandGetBod(socket, cmd, args):
	player = socket.player
	if not player:
		return True

	bod_type = "smith"
	args_str = args.strip().lower()
	if "tailor" in args_str:
		bod_type = "tailor"

	now = int(time.time())
	tag_name = f"next_bod_{bod_type}"
	next_time = int(player.gettag(tag_name, 0))

	# Staff bypasses timer
	is_staff = (player.account and player.account.rank > 0)

	if now < next_time and not is_staff:
		mins_left = int((next_time - now) / 60) + 1
		socket.sysmessage(f"An offer may only be given once every hour. You must wait {mins_left} more minutes.")
		return True

	deed = generate_bod_for_player(player, bod_type)
	if deed:
		pack = player.getbackpack()
		if not (pack and wolfpack.utilities.tocontainer(deed, pack)):
			deed.moveto(player.pos)

		# Set 1 hour cooldown (3600 seconds)
		player.settag(tag_name, now + 3600)
		player.pos.soundeffect(0x249)
		socket.sysmessage(f"A bulk order deed for {bod_type.capitalize()}ing has been placed in your backpack.")
	else:
		socket.sysmessage("Failed to generate bulk order deed.")
	return True

def commandBod(socket, cmd, args):
	player = socket.player
	if not player:
		return True

	tokens = args.strip().lower().split()
	if not tokens:
		return commandGetBod(socket, "getbod", "")

	sub = tokens[0]

	if sub == "reset":
		player.deltag("next_bod_smith")
		player.deltag("next_bod_tailor")
		socket.sysmessage("Your bulk order timer has been reset.")
		return True

	if sub == "create":
		if len(tokens) < 3:
			socket.sysmessage("Usage: bod create <small|large> <smith|tailor>")
			return True

		b_size = tokens[1]
		b_type = tokens[2] if tokens[2] in ["smith", "tailor"] else "smith"

		deed = generate_bod_for_player(player, b_type)
		if deed:
			pack = player.getbackpack()
			if not (pack and wolfpack.utilities.tocontainer(deed, pack)):
				deed.moveto(player.pos)
			socket.sysmessage("Bulk order deed created.")
		return True

	return commandGetBod(socket, "getbod", args)

def onLoad():
	wolfpack.registercommand("bod", commandBod)
	wolfpack.registercommand("getbod", commandGetBod)
