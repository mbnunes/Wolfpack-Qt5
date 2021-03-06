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

def commandInvis( socket, cmd, args ):
	char = socket.player
	char.removefromview()
	char.invisible = booleantoggle( char.invisible )
	char.update()
	socket.sysmessage( "Invisible is now '%i'." % char.invisible )
	return True

def onLoad():
	wolfpack.registercommand( "invis", commandInvis )
	return
