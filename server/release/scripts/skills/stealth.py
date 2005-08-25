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
from math import ceil

STEALTH_DELAY = 5000
# the hiding skill before you can use the stealth skill
MIN_HIDING = 800

def onWalk(char, direction, sequence):
	if char.stealthedsteps <= 0:
		char.removescript('skills.stealth')
	elif direction & 0x80:
		if char.socket:
			char.socket.clilocmessage(500814, "", 0x3b2, 3) # You have been revealed!
		char.hidden = False
		char.stealthedsteps = 0
		char.removescript('skills.stealth')
		char.update()
	
	return False

def stealth( char, skill ):
	if char.socket.hastag( 'skill_delay' ):
		if wolfpack.time.currenttime() < char.socket.gettag( 'skill_delay' ):
			char.socket.clilocmessage( 500118, "", 0x3b2, 3 ) # You must wait a few moments to use another skill.
			return True
		else:
			char.socket.deltag( 'skill_delay' )

	# use hiding first
	if not char.hidden:
		char.socket.clilocmessage( 502725, "", 0x3b2, 3 ) # You must hide first
		return True

	# Clean invisibility spell timer
	char.dispel(None, 1, "invisibility_reveal")

	# you can use stealth only when your hiding skill is over 80.0
	# or if you got hidden by the invisibility spell
	if not( char.hasscript('skills.stealth') ):
		if char.skill[ HIDING ] < MIN_HIDING:
			char.socket.clilocmessage( 502726, "", 0x3b2, 3 ) # You are not hidden well enough.  Become better at hiding.
			return True

	# TODO :
	# too much armor
	# too many chars around
	# needed ? if riding an animal

	success = char.checkskill( STEALTH, 0, 1000 )

	if success:
		char.socket.clilocmessage( 502730, "", 0x3b2, 3 ) # You begin to move quietly.
		steps = int(ceil(char.skill[STEALTH] / 50.0))
		if steps < 0:
			steps = 1
		char.stealthedsteps = steps
		char.addscript('skills.stealth') # Unhide on run
	else:
		char.socket.clilocmessage( 502731, "", 0x3b2, 3 ) # You fail in your attempt to move unnoticed.
		char.hidden = False
		char.update()

	char.socket.settag( 'skill_delay', int( wolfpack.time.currenttime() + STEALTH_DELAY ) )

	return True

def onLoad():
	skills.register( STEALTH, stealth )
