#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
import wolfpack

# Register as a global script
def onLoad():
	wolfpack.registerglobal( HOOK_CHAR, EVENT_SKILLUSE, "skills.animallore" )

def onSkillUse( char, skill ):
	if skill != ANIMALLORE:
		return 0

	socket = char.socket

	# Assign the target request
	socket.clilocmessage( 0x7A268 ) # What animal should i look at?
	socket.attachtarget( "skills.animallore.response" )

	return 1

def response( char, args, target ):
	# SkillFail: 0x7A26E
	# Too Far away to do that: 0x7A26C
	# target dead: 0x7A26B (over own head)
	# Target invalid: 0x7A269 (over own head)
	# ID: 0x10044A (0)
	# At your skill level, you can only lore tamed creatures.
	# -----------------------------------------
	# ID: 0x10044B (0)
	# At your skill level, you can only lore tamed or tameable creatures.

	pass
