#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: codex                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
import wolfpack

def onContextEntry( char, healer, tag  ):
    if( tag == 1 ):
	if not char or not healer:
		return OOPS

	if healer.dead:
		char.socket.clilocmessage( 501040, "", YELLOW, NORMAL ) # The resurrecter must be alive
		return OOPS

	if char.dead:
		char.socket.clilocmessage( 3002069, "", YELLOW, NORMAL ) # Resurrection
		char.soundeffect( 0x215 )
		char.resurrect()
		return OK
	else:
		char.socket.clilocmessage( 1060197, "", YELLOW, NORMAL ) # You are not dead, and thus cannot be resurrected!
		return OOPS
		
	return OK

