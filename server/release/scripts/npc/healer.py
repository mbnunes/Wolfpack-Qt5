#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: codex                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
from wolfpack.consts import *

def onContextEntry( char, healer, tag ):
	if( int( tag ) == 1 ):
		healer.say("Hold still!")
		if not char or not healer:
			return False

		if healer.dead:
			char.socket.clilocmessage( 501040, "", YELLOW, NORMAL ) # The resurrecter must be alive
			return False

		if char.dead:
			char.socket.clilocmessage( 3002069, "", YELLOW, NORMAL ) # Resurrection
			char.soundeffect( 0x215 )
			char.resurrect()
			return True
		else:
			char.socket.clilocmessage( 1060197, "", YELLOW, NORMAL ) # You are not dead, and thus cannot be resurrected!
			return False

		return True
	else:
		char.say("A HEALER! RESURRECT ME!")
		return True
