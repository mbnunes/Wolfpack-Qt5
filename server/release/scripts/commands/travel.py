#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: codex                          #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# travel Command  						#
#===============================================================#

"""
	\command travel
	\description Shows the moongate gump.
"""

import wolfpack
from moongate import sendGump

def travel( socket, command, args ):
	char = socket.player
	sendGump( char, 0 )
	return True

def onLoad():
	wolfpack.registercommand( "travel", travel )
