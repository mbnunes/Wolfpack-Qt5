#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Viper                          #
#   )).-" {{ ;"`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
from wolfpack.utilities import *
from wolfpack.consts import *
import re

def onShowToolTip( sender, target, tooltip ):

	tooltip.add( 1041008, "" )
	tooltip.add( 1038021, "" )
	tooltip.send( sender )
	return 1
	

def onUse( char, item ):

	#char.socket.clilocmessage( 0x7A31A, "", 0x3b2, 3 )
	char.socket.clilocmessage( 0xF55DA, "", 0x3b2, 3 )
	char.socket.attachtarget( "clothing_bless_deed.response", [item] )

	return 1

def response( char, args, target ):

	if not isclothing( target.item ) or ishat( target.item ):
		char.socket.clilocmessage( 0xF55DB, "", 0x3b2, 3 )
		return 0

	if target.item.hastag( "blessed " ):
		char.socket.clilocmessage( 0xF55D9, "", 0x3b2, 3 )
		return 0

	item = args[0]
	backpack = char.getbackpack()

	char.socket.clilocmessage( 0xF6969, "", 0x3b2, 3 )
	target.item.settag( "blessed", 1 )
	target.item.newbie = 1
	item.delete()
	
	return 1

	
