#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
from wolfpack.consts import *
from math import floor
import skills
import wolfpack.time

EVALINTDELAY = 1000

def evaluatingintel( char, skill ):
	#Only Handle Evalint
	if skill != EVALUATINGINTEL:
		return False

	if char.socket.hastag( 'skill_delay' ):
		if wolfpack.time.currenttime() < char.socket.gettag( 'skill_delay' ):
			char.socket.clilocmessage( 500118, "", 0x3b2, 3 )
			return True
		else:
			char.socket.deltag( 'skill_delay' )

	char.socket.clilocmessage( 0x7A4AA, "", 0x3b2, 3 ) # What would you like to evaluate
	char.socket.attachtarget( "skills.evaluatingintel.response" )
	return True

def response( char, args, target ):

	if target.item:
		# It looks smarter than a rock, but dumber than a piece of wood
		char.socket.clilocmessage( 0x7A4AC, "", 0x3b2, 3, target.item )
		return False
		
	if not target.char:
		return False

	if not char.canreach( target.char, 8 ):
		# No Cliloc when failing LoS or Distance Check
		return False

	# Vendors: 0x7A4AD That person could probably calculate the cost of what you buy from them.
	# Town Criers: 0x7A4AB He looks smart enough to remember the news.  Ask him about it.

	if target.char == char:
		# Hmm, that person looks really silly.
		char.socket.clilocmessage( 0x7A4AE, "", 0x3b2, 3, target.char )
		return False

	char.socket.settag( 'skill_delay', int( wolfpack.time.currenttime() + EVALINTDELAY ) )

	if not char.checkskill( EVALUATINGINTEL, 0, 1200 ):
		char.socket.clilocmessage( 0xFD756, "", 0x3b2, 3, target.char )
		return False

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

	char.socket.clilocmessage( msgId, "", 0x3b2, 3, target.char, "", True )
	
	if char.skill[EVALUATINGINTEL] >= 760:
		char.socket.clilocmessage( msgId2, "", 0x3b2, 3, target.char, "", True )

	return True

def onLoad():
	skills.register(EVALUATINGINTEL, evaluatingintel)
