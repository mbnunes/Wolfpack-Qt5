#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
import wolfpack
from math import floor

# Register as a global script
def onLoad():
	wolfpack.registerglobal( HOOK_CHAR, EVENT_SKILLUSE, "skills.anatomy" )

# Button for Anatomy pressed on skill gump
def onSkillUse( char, skill ):
	# We only handle anatomy
	if skill != ANATOMY:
		return 0

	socket = char.socket

	# Assign the target request
	socket.clilocmessage( 0x7A261 ) # Whom shall I examine?
	socket.attachtarget( "skills.anatomy.response" )

	return 1

def response( char, args, target ):

	socket = char.socket

	# Check for a valid target
	if not target.char:
		socket.clilocmessage( 0x7A263, "", 0x3b2, 3, char ) # Only living things have anatomies
		return

	if target.char == char:
		socket.clilocmessage( 0x7A264, "", 0x3b2, 3, char ) # You know yourself quite well enough already.
		return

	# You can't reach that (too far away, no los, wrong map)
	if not char.canreach( target.char, 16 ):
		socket.clilocmessage( 0x7A265, "", 0x3b2, 3, target.char ) # I am too far away to do that.
		return

	# Turn toward the char we want to look at
	char.turnto( target.char )
	
	# Invulnerable Characters cannot be examined
	if target.char.priv & 0x04:
		socket.clilocmessage( 0x7A266, "", 0x3b2, 3, target.char ) # That cannot be inspected.
		return

	# Make a skillcheck and display the fail or success message above the targets head
	if not char.checkskill( ANATOMY, 0, 1000 ):
		socket.clilocmessage( 0xFE8EA, "", 0x3b2, 3, target.char ) # You cannot quite get a sense of their physical characteristics.
		return

	# Anatomy Messages start: 0xFD6DD
	strId = min( 10, floor( target.char.strength / 10 ) )
	dexId = min( 10, floor( target.char.dexterity / 10 ) )
	msgId = int( 0xFD6DD + strId * 11 + dexId )

	socket.clilocmessage( msgId, "", 0x3b2, 3, target.char )
