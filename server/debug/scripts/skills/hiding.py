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

	# If any opponent is within 10 tiles. Deny hiding.
	opponents = char.getopponents()

	for enemy in opponents:
		# Forget about old fights
		if enemy.attacktarget != char and char.attacktarget != enemy:
			continue

		if enemy.distanceto(char) < 10:
			char.socket.clilocmessage(501238)
			return False

	# Remove the ability to use stealth, even if hiding < 80.0
	# do this only if he isn't using stealth yet
	if not( char.hidden ):
		char.removescript( 'skills.stealth' )
		char.stealthedsteps = 0

	success = char.checkskill( HIDING, 0, 1000 )

	if success:
		char.socket.clilocmessage(501240, "", 0x3b2, 3)
		char.removefromview()
		char.hidden = True
		char.update()
		char.dispel( None, 1, "invisibility_reveal" )
	else:
		# Unhide Safe Guard
		if char.hidden:
			char.removefromview()
			char.hidden = False
			char.update()
		char.socket.clilocmessage( 501237, "", 0x3b2, 4, char )

	char.socket.settag( 'skill_delay', int( wolfpack.time.currenttime() + HIDING_DELAY ) )

	return True

def onLoad():
	skills.register( HIDING, hiding )
