#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
from wolfpack.consts import STEALING
from wolfpack import tr

# Pickpocket dips
# Version 1: 0x1EC0 (Animated: 0x1EC0), Facing E/W
# Version 2: 0x1EC3 (Animated: 0x1EC4), Facing E/W

def onUse( char, item ):
	# If we've already learned all we can > cancel.
	if( char.skill[ STEALING ] >= 600 ):
		char.message( 501830 ) # Your ability to steal cannot improve any further by simply practicing on a dummy.
		return True

	# Either the dummy is swinging or we aren't assigned to a dummy
	if( item.id != 0x1ec0 and item.id != 0x1ec3 ):
		return True

	# Distance & Direction checks
	if( char.distanceto( item ) > 1 ):
		char.message( tr('You must be standing in front of or behind the dummy to use it.') )
		return True

	# Calculates the direction we'll have to look
	# to focus the dummy
	direction = char.directionto( item )

	# We only have E/W facing Pickpocket dips
	if( direction != 2 and direction != 6 ):
		char.message( tr('You must be standing in front of or behind the dummy to use it.') )
		return True

	# Turn to the correct direction if not already
	if( char.direction != direction ):
		char.direction = direction
		char.update()

	# Only swing the dip if the check failed
	if( char.checkskill( STEALING, 0, 1000 ) ):
		char.message( tr("You succeed in your try to pickpocket the dip.") )
		return True

	if( item.id == 0x1ec0 ):
		item.id = 0x1ec1
	elif( item.id == 0x1ec3 ):
		item.id = 0x1ec4

	# Resend the item to surrounding clients after
	# changing the id and play the soundeffect
	# originating from the dummy
	item.update()
	item.soundeffect( 0x41 )

	# Display the char-action
	# (combat swing 1handed)
	char.action( 0x09 )

	# Add a timer to reset the id
	item.addtimer( 3000, resetid, [item.serial] )

	return True

# Reset the id of a swinging dummy
def resetid( char, args ):
	item = wolfpack.finditem( args[0] )

	if item :
		if( item.id == 0x1ec1 or item.id == 0x1ec4 ):
			item.id -= 1

		# Resend the item to surrounding clients after
		# changing the id
		item.update()
