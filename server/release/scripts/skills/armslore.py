#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Viper                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
from wolfpack.utilities import *
import wolfpack

def onLoad():
	wolfpack.registerglobal( HOOK_CHAR, EVENT_SKILLUSE, "skills.armslore" )

def onSkillUse( char, skill ):
	# only handle armslore
	if skill != ARMSLORE:
		return 0

	if char.hastag( 'skill_delay' ):
		char.socket.clilocmessage( 500118, "", 0x3b2, 3 )
		return 1

	char.socket.clilocmessage( 0x7A27D, "", 0x3b2, 3 )
	char.socket.attachtarget( "skills.armslore.response" )

	return 1

def response( char, args, target ):

	backpack = char.getbackpack()
	item = target.item

	if item:
		if not item.getoutmostchar() == char:
			if not char.canreach( item, 4 ):
				char.socket.clilocmessage( 0x7A27F, "", 0x3b2, 3 )
				return 0

			if not char.distanceto( item ) < 5:
				char.socket.clilocmessage( 0x7A27E, "", 0x3b2, 3 )
				return 0

		if isweapon( item ) or isarmor( item ) or isshield( item ):	
			if not char.checkskill( ARMSLORE, 0, 1000 ):
				char.socket.clilocmessage( 0x7A281, "", 0x3b2, 3 )
				return 0

			char.socket.clilocmessage( 0x103319, "", 0x3b2, 3 )
			return 1
		else:
			char.socket.clilocmessage( 0x7A280, "", 0x3b2, 3 )
			return 0

	else:
		if not char.canreach( target.char, 4 ):
			char.socket.clilocmessage( 0x7A27F, "", 0x3b2, 3 )
			return 0

		if not char.distanceto ( target.char ) < 5:
			char.socket.clilocmessage( 0x7A27E, "", 0x3b2, 3 )
			return 0

		char.socket.clilocmessage( 0x7A280, "", 0x3b2, 3 )
		return 0


	
