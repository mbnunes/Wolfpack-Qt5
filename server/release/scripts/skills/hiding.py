#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
from wolfpack.utilities import *
from wolfpack.time import *
import wolfpack
import skills

HIDING_DELAY = 5000

def hiding( char, skill ):
	if skill != HIDING:
		return 0

	if char.hastag( 'skill_delay' ):
		cur_time = servertime()
		if cur_time < char.gettag( 'skill_delay' ):
			char.socket.clilocmessage( 500118, "", 0x3b2, 3 )
			return 1
		else:
			char.deltag( 'skill_delay' )

	success = char.checkskill( HIDING, 0, 1000 )

	if success:
		char.socket.clilocmessage( 501240, "", 0x3b2, 3 )
		char.hidden = 1
		char.update()
	else:
		char.socket.clilocmessage( 501237, "", 0x3b2, 4, char )
	
	cur_time = servertime()
	char.settag( 'skill_delay', cur_time + HIDING_DELAY )

	return 1

def onLoad():
	skills.register( HIDING, hiding )
