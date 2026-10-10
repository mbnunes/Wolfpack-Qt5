#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Antigravity                    #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Peerless Boss Script (Loot Drop & Altar Notification)         #
#===============================================================#

import wolfpack
from wolfpack.consts import LOG_ERROR
from peerless.peerless_system import drop_peerless_loot

def onDeath(char, killer, corpse):
	# Drop major artifacts & peerless loot
	drop_peerless_loot(char, corpse)

	# Sound & thunder
	char.pos.soundeffect(0x211)
	wolfpack.effect(0x3709, char.pos, 10, 30)

	# Notify Altar
	if char.hastag("peerless_altar"):
		try:
			altar_serial = int(char.gettag("peerless_altar"))
			altar = wolfpack.finditem(altar_serial)
			if altar:
				from peerless.altar import on_boss_defeated
				on_boss_defeated(altar)
		except Exception as e:
			wolfpack.console.log(LOG_ERROR, f"Error notifying peerless altar: {e}")

	# Broadcast to nearby fighters
	for nearby in wolfpack.chars(char.pos.x, char.pos.y, char.pos.map, 35):
		if nearby and nearby.player:
			nearby.socket.sysmessage(f"The mighty {char.name} has been vanquished! You have 15 minutes to gather your spoils!")
	return False

def onDelete(char):
	if char.hastag("peerless_altar"):
		try:
			altar_serial = int(char.gettag("peerless_altar"))
			altar = wolfpack.finditem(altar_serial)
			if altar:
				from peerless.altar import on_boss_defeated
				on_boss_defeated(altar)
		except Exception:
			pass
	return False
