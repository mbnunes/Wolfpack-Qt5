#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: codex                          #
#   )).-' {{ ;'`   # Revised by: Incanus                        #
#  ( (  ;._ \\ ctr # Last Modification:                         #
#################################################################
from speech.healer import offerResurrection

def onContextCheckVisible(char, target, tag):
	if( int( tag ) == 1 ):
		return char.dead

	return True

def onContextEntry( char, healer, tag ):
	if( int( tag ) == 1 ):
		if not char or not healer:
			return False

		if healer.dead:
			char.socket.clilocmessage( 501040  ) # The resurrecter must be alive
			return False

		offerResurrection(healer, char)
	return True

