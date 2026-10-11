#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Antigravity                    #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Champion Wave Creature Script                                 #
#===============================================================#

import wolfpack
from wolfpack.consts import LOG_ERROR

def onDeath(char, source, corpse):
	if char.hastag('champ_altar'):
		try:
			altar_serial = int(char.gettag('champ_altar'))
			altar = wolfpack.finditem(altar_serial)
			if altar:
				from champions.championspawn import on_creature_killed
				on_creature_killed(altar, char)
		except Exception as e:
			wolfpack.console.log(LOG_ERROR, f"Error notifying champ creature death: {e}")
	return False

def onDelete(char):
	if char.hastag('champ_altar'):
		try:
			altar_serial = int(char.gettag('champ_altar'))
			altar = wolfpack.finditem(altar_serial)
			if altar:
				from champions.championspawn import on_creature_deleted
				on_creature_deleted(altar, char)
		except Exception:
			pass
	return False
