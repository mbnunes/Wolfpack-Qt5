#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack import *

# Pickpocket dips
# Version 1: 0x1EC0 (Animated: 0x1EC0), Facing E/W
# Version 2: 0x1EC3 (Animated: 0x1EC4), Facing E/W

def onUse( char, item ):
	# If we've already learned all we can > cancel.
	if( char.skill[ STEALING ] >= 600 ):
		char.message( "Go search for real people." )
		return 1

	# Either the dummy is swinging or we aren't assigned to a dummy
	if( item.id != 0x1EC0 and item.id != 0x1EC3 ):
		return 1

	# Distance & Direction checks
	if( char.distanceto( item ) > 1 ):
		char.message( 'You must be standing in front of or behind the dummy to use it.' )
		return 1

	# Calculates the direction we'll have to look
	# to focus the dummy
	direction = char.directionto( item )

	# We only have E/W facing Pickpocket dips
	if( direction != 2 and direction != 6 ):
		char.message( 'You must be standing in front of or behind the dummy to use it.' )
		return 1

	# Turn to the correct direction if not already
	if( char.direction != direction ):
		char.direction = direction
		char.update()

	# Only swing the dip if the check failed
	if( char.checkskill( STEALING, 0, 1000 ) ):
		char.message( "You succeed in your try to pickpocket the dip" )
		return 1

	if( item.id == 0x1EC0 ):
		item.id = 0x1EC1
	elif( item.id == 0x1EC3 ):
		item.id = 0x1EC4

	# Resend the item to surrounding clients after
	# changing the id and play the soundeffect
	# originating from the dummy
	item.update()
	item.soundeffect( 0x41 )

	# Display the char-action
	# (combat swing 1handed)
	char.action( 0x09 )

	# Add a timer to reset the id
	addtimer( "pickpocket_dip.resetid", 3000, (item.serial,) )

	return 1

# Reset the id of a swinging dummy
def resetid( iSerial ):
	item = finditem( iSerial )

	if( item ):
		if( item.id == 0x1EC1 or item.id == 0x1EC4 ):
			item.id -= 1

		# Resend the item to surrounding clients after
		# changing the id
		item.update()
