#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Viper                          #
#   )).-" {{ ;"`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
from wolfpack.utilities import isarmor
from wolfpack.utilities import isclothing
from wolfpack.utilities import isweapon
from wolfpack.consts import *
import re

def onShowToolTip( sender, target, tooltip ):

	tooltip.add( 1006018, "" )
	tooltip.add( 1038021, "" )
	tooltip.send( sender )
	return 1


def onUse( char, item ):

	char.socket.clilocmessage( 0x7A31A, "", 0x3b2, 3 )
	char.socket.attachtarget( "deeds.item_bless_deed.response", [item.serial] )

	return 1

def response( char, args, target ):

	if not target.item:
		char.socket.clilocmessage( 0x7A31B, "", 0x3b2, 3 )
		return 0

	if not target.item.getoutmostchar() == char:
		char.socket.clilocmessage( 0x7A31C, "", 0x3b2, 3 )
		return 0

	if not isarmor( target.item ) or isweapon( target.item ):
		if not isclothing( target.item ) or ishat( target.item ) or isshield( target.item):
			char.socket.clilocmessage( 0x7A31E, "", 0x3b2, 3 )
			return 0

	if target.item.hastag( "blessed" ):
		char.socket.clilocmessage( 0x7A31F, "", 0x3b2, 3 )
		return 0

	item = wolfpack.finditem(args[0])
	backpack = char.getbackpack()

	target.item.settag( "blessed", 1 )
	target.item.newbie = 1
	item.delete()
	char.socket.sendcontainer( backpack )

	return 1
