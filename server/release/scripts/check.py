#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Scripts for checks created by bankers.                        #
#===============================================================#

import wolfpack

def onCreate( item, definition ):
	item.settag( 'value', 0 )

def onSingleClick( char, item ):

def onUse( char, item ):
	char.socket.showspeech( item, "Drop this on a banker in order to deposit it." )
	return 1

# Check if the character has the banker speech event
# If yes, deposit the check
def onDropOnChar( char, item ):
	if not "speech_banker" in char.events:
		return 0

	if not item.hastag( 'value' ):
		return 0

	dropper = item.container # If its dropped it HAS to be on a char (dragging layer)
	value = int( item.gettag( 'value' ) )

	if value < 1:	
		char.say( "This check is worthless!" )
		return 1 # Auto Bounce

	# Perfect, we found a banker to deposit this check at.
	char.say( "I deposited %i gold on your bank in return for the check." % value )
	item.delete()

	bankbox = dropper.getbankbox()

	if bankbox:
		while value > 0:
			item = wolfpack.additem( "eed" )
			item.amount = min( [ value, 65535 ] )
			item.container = bankbox

			value -= min( [ value, 65535 ] )				

		dropper.soundeffect( 0x37, 0 )
	
	return 1
