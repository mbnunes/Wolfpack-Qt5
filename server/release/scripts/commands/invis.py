#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: Dreoth
#   )).-' {{ ;'`   | Revised by:
#  ( (  ;._ \\ ctr | Last Modification: Created
#===============================================================#
"""
	\command invis
	\description Toggle invisibility.
"""

import wolfpack
from wolfpack.utilities import booleantoggle

def onLoad():
	wolfpack.registercommand( "invis", commandInvis )
	return

def commandInvis( socket, cmd, args ):
	char = socket.player
	char.invisible = booleantoggle( char.invisible )
	char.removefromview()
	char.update()
	socket.sysmessage( "Invisible is now '%i'." % char.invisible )
	return True
