#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
from wolfpack.utilities import *
import wolfpack
import wolfpack.time
import skills

HIDING_DELAY = 5000

def hiding( char, skill ):
	if skill != HIDING:
		return False

	if char.socket.hastag( 'skill_delay' ):
		if wolfpack.time.currenttime() < char.socket.gettag( 'skill_delay' ):
			char.socket.clilocmessage( 500118, "", 0x3b2, 3 )
			return True
		else:
			char.socket.deltag( 'skill_delay' )

	success = char.checkskill( HIDING, 0, 1000 )

	if success:
		char.socket.clilocmessage(501240, "", 0x3b2, 3)
		char.removefromview()
		char.hidden = 1
		char.update()
		char.dispel( None, 1, "invisibility_reveal" )
	else:
		char.socket.clilocmessage( 501237, "", 0x3b2, 4, char )

	char.socket.settag( 'skill_delay', int( wolfpack.time.currenttime() + HIDING_DELAY ) )

	return True

def onLoad():
	skills.register( HIDING, hiding )
