#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Antigravity                    #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Paragon Management Commands                                   #
#===============================================================#

r"""
	\command paragon
	\description Manage Paragon status, spawn Paragon Chests or test Minor Artifacts.
	Usage:
	  'paragon' - Target a creature to convert to Paragon
	  'paragon unconvert' - Target a creature to revert from Paragon
	  'paragon chest [level 1-5]' - Spawn a Paragon Chest in your backpack
	  'paragon artifact' - Receive a random Minor Artifact
"""

import wolfpack
from system.paragon import (
	convert, unconvert,
	create_paragon_chest, give_artifact_to
)

def commandParagon(socket, cmd, args):
	player = socket.player
	if not player:
		return True

	tokens = args.strip().lower().split()
	sub = tokens[0] if tokens else "convert"

	if sub == "unconvert":
		socket.sysmessage("Target the creature you wish to unconvert from Paragon:")
		socket.attachtarget("commands.paragon.unconvert_target", [])
		return True

	elif sub == "chest":
		lvl = 3
		if len(tokens) > 1:
			try:
				lvl = max(1, min(5, int(tokens[1])))
			except ValueError:
				lvl = 3

		# Mock monster for naming/fame
		class FakeMonster:
			def __init__(self, lvl):
				self.name = "Daemon"
				self.fame = lvl * 4500

		chest = create_paragon_chest(FakeMonster(lvl))
		if chest:
			pack = player.getbackpack()
			if not (pack and wolfpack.utilities.tocontainer(chest, pack)):
				chest.moveto(player.pos)
			socket.sysmessage(f"Paragon Chest (Level {lvl}) created.")
		else:
			socket.sysmessage("Failed to create Paragon Chest.")
		return True

	elif sub == "artifact":
		give_artifact_to(player)
		socket.sysmessage("Minor Artifact given.")
		return True

	else: # Default: convert
		socket.sysmessage("Target the creature you wish to convert to Paragon:")
		socket.attachtarget("commands.paragon.convert_target", [])
		return True

def convert_target(char, args, target):
	mob = target.char
	if not mob or not mob.npc:
		char.socket.sysmessage("You must target an NPC creature.")
		return False

	if mob.player:
		char.socket.sysmessage("Players cannot be Paragons.")
		return False

	convert(mob)
	char.socket.sysmessage(f"{mob.name} is now a Paragon!")
	char.pos.soundeffect(0x29)
	wolfpack.effect(0x3728, mob.pos, 10, 10)
	return True

def unconvert_target(char, args, target):
	mob = target.char
	if not mob or not mob.npc:
		char.socket.sysmessage("You must target an NPC creature.")
		return False

	unconvert(mob)
	char.socket.sysmessage(f"{mob.name} is no longer a Paragon.")
	return True

def onLoad():
	wolfpack.registercommand("paragon", commandParagon)
