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

STEALTH_DELAY = 5000
# the hiding skill before you can use the stealth skill
MIN_HIDING = 800

def stealth( char, skill ):
	if skill != STEALTH:
		return 0

	if char.socket.hastag( 'skill_delay' ):
		cur_time = servertime()
		if cur_time < char.socket.gettag( 'skill_delay' ):
			char.socket.clilocmessage( 500118, "", 0x3b2, 3 )
			return 1
		else:
			char.socket.deltag( 'skill_delay' )

	# use hiding first
	if not char.hidden:
		char.socket.clilocmessage( 502725, "", 0x3b2, 3 )
		return 1

	# you can use stealth only when your hiding skill is over 80.0
	if char.skill[ HIDING ] < MIN_HIDING:
		char.socket.clilocmessage( 502726, "", 0x3b2, 3 )
		return 1

	# TODO :
	# too much armor
	# too many chars around
	# needed ? if riding an animal

	success = char.checkskill( STEALTH, 0, 1000 )

	if success:
		char.socket.clilocmessage( 502730, "", 0x3b2, 3 )
		char.stealth = 0
	else:
		char.socket.clilocmessage( 502731, "", 0x3b2, 3 )
		char.hidden = 0
	
	cur_time = servertime()
	char.socket.settag( 'skill_delay', ( cur_time + STEALTH_DELAY ) )

	return 1

def onLoad():
	skills.register( STEALTH, stealth )
