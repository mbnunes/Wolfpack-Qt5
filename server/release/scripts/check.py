#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Scripts for checks created by bankers.                        #
#===============================================================#

#ID: 0xFE8F1 (0)
#Into your bank box I have placed a check in the amount of:

import wolfpack

def onCreate( item, definition ):
	item.settag( 'value', 0 )

def onShowToolTip( sender, target, tooltip ):

	tooltip.add( 1041361, "" ) # A Bank Check
	tooltip.add( 1038021, "" ) # Blessed
	tooltip.add( 1060738, str( target.gettag( "value") ) )
	tooltip.send ( sender )
	return True

def onUse( char, item ):
	#D: 0xFE8F0 (0)
	#Gold was deposited in your account:
	bankbox = char.getbankbox()
	value = item.gettag( "value" )
	if bankbox:
		while value > 0:
			gold = wolfpack.additem( "eed" )
			gold.amount = min( [ value, 60000 ] )
			gold.container = bankbox
			value -= min( [ value, 60000 ] )

		amount = str( value )
		char.soundeffect( 0x37, 0 )
		char.socket.sendcontainer( bankbox )
		item.delete()
	return True
