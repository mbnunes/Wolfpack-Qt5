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
	wolfpack.registerglobal( HOOK_CHAR, EVENT_SKILLUSE, "skills.evaluateintelligence" )

# Button for Anatomy pressed on skill gump
def onSkillUse( char, skill ):
	# We only handle anatomy
	if skill != EVALUATEINTELLIGENCE:
		return 0

	socket = char.socket

	# Assign the target request
	socket.clilocmessage( 0x7A4AA ) # What would you like to evaluate?
	socket.attachtarget( "skills.evaluateintelligence.response" )

	return 1

def response( char, args, target ):

	socket = char.socket

	# Check for a valid target
	if not target.char:
		socket.clilocmessage( 0x7A4AC, "", 0x3b2, 3, char ) # It looks smarter than a rock, but dumber than a piece of wood.
		return

	if target.char == char:
		socket.clilocmessage( 0x7A4AE, "", 0x3b2, 3, char ) # Hmm, that person looks really silly.
		return

	# You can't reach that (too far away, no los, wrong map)
	if not char.canreach( target.char, 16 ):
		socket.clilocmessage( 0x7A265, "", 0x3b2, 3, target.char ) # I am too far away to do that.
		return

	# Turn toward the char we want to look at
	char.turnto( target.char )
	
	# Vendors: 0x7A4AD That person could probably calculate the cost of what you buy from them.
	# Town Criers: 0x7A4AB He looks smart enough to remember the news.  Ask him about it.

	# Invulnerable Characters cannot be examined
	if target.char.priv & 0x04 or target.char.dead:
		socket.clilocmessage( 0x7A266, "", 0x3b2, 3, target.char ) # That cannot be inspected.
		return

	# MALE = Fail( 0xFD756 ), Base( 0xFD759 )
	if target.char.id == 0x190:
		failId = 0xfd756
		msgBase = 0xfd759

	# FEMALE = Fail( 0xFD757 ), Base( 0xFD764 )
	elif target.char.id == 0x191:
		failId = 0xfd757
		msgBase = 0xfd764

	# THING = Fail( 0xFD758 ), Base( 0xFD76F )
	else:
		failId = 0xfd758
		msgBase = 0xfd76f

	# Make a skillcheck and display the fail or success message above the targets head
	if not char.checkskill( EVALUATEINTELLIGENCE, 0, 1000 ):
		socket.clilocmessage( failId, "", 0x3b2, 3, target.char )
		return

	intId = min( 10, floor( target.char.intelligence / 10 ) )
	msgId = int( msgBase + intId )

	socket.clilocmessage( msgId, "", 0x3b2, 3, target.char )
