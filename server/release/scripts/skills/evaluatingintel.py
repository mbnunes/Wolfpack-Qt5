#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Viper                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
from wolfpack.consts import *
from math import floor

def onLoad():
	wolfpack.registerglobal( HOOK_CHAR, EVENT_SKILLUSE, "skills.evaluatingintel" )

def onSkillUse( char, skill ):
	#Only Handle Evalint
	if skill != EVALUATINGINTEL:
		return 0

	if char.hastag( 'skill_delay' ):
		char.socket.clilocmessage( 500118, "", 0x3b2, 3 )
		return 1

	char.socket.clilocmessage( 0x7A4AA, "", 0x3b2, 3 ) # What would you like to evaluate
	char.socket.attachtarget( "skills.evaluatingintel.response" )
	return 1

def response( char, args, target ):

	if target.item:
		# It looks smarter than a rock, but dumber than a piece of wood
		char.socket.clilocmessage( 0x7A4AC, "", 0x3b2, 3, target.item )
		return 0

	if not char.canreach( target.char, 8 ):
		return 0

	#if target.char == char:
		# Hmm, that person looks really silly.
		#char.socket.clilocmessage( 0x7A4AE, "", 0x3b2, 3, target.char )
		#return 0

	if not char.checkskill( EVALUATINGINTEL, 0, 1000 ):
		char.socket.clilocmessage( 0xFD756, "", 0x3b2, 3, target.char )
		return 0

	IntRatio = float( 100.0 / target.char.intelligence )
	ManaId = floor( ( target.char.mana * IntRatio ) / 10 )
	IntId = min( 10, floor( target.char.intelligence / 10 ) )
	
	if target.char.id == 0x190:
		msgId = int( 0xFD759 + IntId )
	elif target.char.id == 0x191:
		msgId = int( 0xFD764 + IntId )
	else:
		msgId = int( 0xFD76F + IntId )
		
	msgId2 = int( 0xFD77A + ManaId )

	char.socket.clilocmessage( msgId, "", 0x3b2, 3, target.char )
	char.socket.clilocmessage( msgId2, "", 0x3b2, 3, target.char )
	return 1
