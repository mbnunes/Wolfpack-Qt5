#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
from wolfpack.utilities import *
import wolfpack

HIDING_DELAY = 5000

def onLoad():
	wolfpack.registerglobal( HOOK_CHAR, EVENT_SKILLUSE, "skills.hiding" )

def onSkillUse( char, skill ):
	if skill != HIDING:
		return 0

	success = char.checkskill( HIDING, 0, 1000 )

	if success:
		char.socket.clilocmessage( 501240, "", 0x3b2, 3 )
		char.hidden = 1
		char.update()
	else:
		char.socket.clilocmessage( 501237, "", 0x3b2, 4, char )
	
	char.settag( 'skill_delay', 1 )
	char.addtimer( HIDING_DELAY, "skills.hiding.release", [ ] )

	return 1

def release( char, args ):
	if not char or not char.hastag( 'skill_delay' ):
		return
	char.deltag( 'skill_delay' )
