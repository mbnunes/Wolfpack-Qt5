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
import wolfpack.utilities

def onShowTooltip( sender, target, tooltip ):
	tooltip.reset()
	tooltip.add( 1041361, "" ) # A Bank Check
	tooltip.add( 1038021, "" ) # Blessed
	if target.hastag('value'):
		tooltip.add( 1060738, str( target.gettag( "value") ) )

def onUse( char, item ):
	#D: 0xFE8F0 (0)	
	#Gold was deposited in your account:
	bankbox = char.getbankbox()
	value = int(item.gettag("value"))
	if bankbox:
		amount = value
		while value > 0:
			gold = wolfpack.additem( "eed" )
			gold.amount = min(value, 60000)			
			value -= gold.amount
			
			if not wolfpack.utilities.tocontainer(gold, bankbox):
				gold.update()
		
		item.delete()
		char.socket.resendstatus()
		char.socket.clilocmessage(1042672, "", 0x3b2, 3, None, " %s" % amount)
		char.soundeffect( 0x37, 0 )
	return True
