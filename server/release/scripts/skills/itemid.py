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
	wolfpack.registerglobal( HOOK_CHAR, EVENT_SKILLUSE, "skills.itemid" )

def onSkillUse( char, skill ):
	if skill != ITEMID:
		return 0

	char.socket.clilocmessage( 0x7A277, "", 0x3b2, 3 )
	char.socket.attachtarget( "skills.itemid.response" )
	return 1

def response( char, args, target ):

	if target.item:
		if not target.item.getoutmostchar() == char:
			if not char.canreach( target, 4 ):
				char.socket.clilocmessage( 0x7A278, "", 0x3b2, 3 )
				return 0

			if not char.distanceto( target ) < 5:
				char.socket.clilocmessage( 0x7A27E, "", 0x3b2, 3 )
				return 0
 
		if not char.checkskill( ITEMID, 0, 1000 ):
			char.socket.clilocmessage( 0xFE3C8, "", 0x3b2, 3, char )
			return 0

		name = str( target.item.getname() )
		value = int( target.item.price )
		char.socket.clilocmessage( 0xFE3C5, "", 0x3b2, 3, char, " %s" %name ) # It appears to be:
		char.socket.clilocmessage( 0xFE3C7, "", 0x3b2, 3, char, " %i gold coins" %value )# You guess the value of that item...
		return 1

	else:
		if not char.canreach( target.char, 4 ):
			char.socket.clilocmessage( 0x7A278, "", 0x3b2, 3 )
			return 0

		if not char.distanceto( target.char ) < 5:
			char.socket.clilocmessage( 0x7A27E, "", 0x3b2, 3 )
			return 0

		name = str( target.char.name )
		char.socket.clilocmessage( 0xFE3C5, "", 0x3b2, 3, char, " %s" %name )
		return 1
