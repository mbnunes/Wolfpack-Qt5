#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Viper                          #
#   )).-" {{ ;"`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
import wolfpack.utilities
from wolfpack.consts import *
import re

def onShowToolTip( sender, target, tooltip ):

	if target.gettag( 'filled' ) == 1:
		filled( target, tooltip )
	else:
		unfilled( target, tooltip)

	tooltip.send ( sender )
	return 1

def filled( target, tooltip ):

	tooltip.add( 1047016, "" ) # Commodity Deed
	tooltip.add( 1038021, "" ) # Blessed
	tooltip.add( 1060747, "" ) # filled
	tooltip.add( 1060659, str ( target.gettag("cname") )  + "\t" + str( target.gettag( "amount") ) )

def unfilled( target, tooltip ):

	tooltip.add( 1047016, "" ) # Commodity Deed
	tooltip.add( 1038021, "" ) # Blessed
	tooltip.add( 1060748, "" ) # unfilled

def onUse ( char, item ):

	bank = char.getbankbox()

	if not item.getoutmostitem() == bank:
		char.socket.clilocmessage( 0xFF9F2, "", 0x3b2, 3 ) # That must be in your bankbox to use it.
		return 0

	if not item.hastag ( "filled" ):
		char.socket.clilocmessage( 0xFF9F5, "", 0x3b2, 3 )
		char.socket.attachtarget( "deeds.commoditydeed.response", [item] )
		return 1

	amount = item.gettag( "amount" )
	commodity = item.gettag( "commodity" )
	color = item.gettag( "color" )
	cname = item.gettag( "cname" )

	redeemitem = wolfpack.additem( "%s" %commodity )
	redeemitem.name = item.gettag( "cname" )
	redeemitem.amount = item.gettag( "amount" )
	redeemitem.color = item.gettag( "color" )
	redeemitem.container = bank
	item.delete()
	char.socket.sendcontainer( bank )
	char.socket.clilocmessage( 0xFF9F7, "", 0x3b2, 3 )
	return 1

def response ( char, args, target ):

	bank = char.getbankbox()
	deed = args[0]

	# "0x1761", "0x1762", "0x1763", "0x1764" possible?
	commodities = [ 0x1bf2, 0x1bfb, 0x1bd7, 0x1766, 0x1768, 0x1081, 0x1078, \
			0x1079, 0xf3f, 0x1bd4, 0x1be0, 0xf7a, 0xf7b, 0x784, 0x785, \
			0x786, 0x788, 0x78c, 0x78d ]

	commodityid = [ "1bf2", "1bfb", "1bd7", "1766", "1768", "1081", "1078", \
			"1079", "f3f", "1bd4", "1be0", "f7a", "f7b", "f84", "f85", \
			"f86", "f88", "f8c", "f8d" ]

	if not target.item.getoutmostitem() == bank:
		char.socket.clilocmessage( 0xFF9F2, "", 0x3b2, 3 ) # That must be in your bankbox to use it.
		return 0

	if not target.item.id in commodities:
		char.socket.clilocmessage( 0xFF9F3, "", 0x3b2, 3 ) # That is not a commodity the bankers will fill a commodity deed with.
		return 0

	num = commodities.index( target.item.id )
	amount = target.item.amount
	color = target.item.color
	cname = target.item.getname()
	commodity = commodityid[num]

	deed.settag( "filled", 1 )
	deed.settag( "cname", cname )
	deed.settag( "commodity", commodity )
	deed.settag( "color", color )
	deed.settag( "amount", amount )
	deed.settag( "filled", 1 )
	target.item.delete()
	char.socket.clilocmessage( 0xFF9F0, "", 0x3b2, 3 )
	deed.update()

	return 1
