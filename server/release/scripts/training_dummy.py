#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
import random
from combat.utilities import weaponskill
from wolfpack.consts import FENCING, MACEFIGHTING, SWORDSMANSHIP, WRESTLING
from wolfpack import tr

# 0x1070 Facing South/North (Swinging: 0x1071)
# 0x1074 Facing East/West   (Swinging: 0x1075)

def onUse( char, item ):
	# Either the dummy is swinging or we aren't assigned to a dummy
	if( item.id != 0x1070 and item.id != 0x1074  ):
		char.socket.clilocmessage( 501815 ) # You have to wait until it stops swinging.
		return True

	# Distance & Direction checks
	if( char.distanceto( item ) > 1 ):
		char.message( 500295, '' ) # You are too far away to do that.
		return True

	if char.itemonlayer( 21 ):
		char.socket.clilocmessage( 501829 ) # You can't practice on this while on a mount.
		return True

	# Calculates the direction we'll have to look
	# to focus the dummy
	direction = char.directionto( item )

	# For a n/s dummy we need to either face north or south
	if( item.id == 0x1070 and direction != 0 and direction != 4 ):
		char.message( tr('You must be standing in front of or behind the dummy to use it.') )
		return True

	# For a e/w dummy we need to either face eath or west
	elif( item.id == 0x1074 and direction != 2 and direction != 6 ):
		char.message( tr('You must be standing in front of or behind the dummy to use it.') )
		return True

	# Turn to the correct direction
	char.turnto( item )

 	# Determine the combat skill used by the character
	skill = weaponskill(char, char.getweapon())

	# We can only train FENCING+MACEFIGHTING+SWORDSMANSHIP+WRESTLING
	if( skill != FENCING and skill != MACEFIGHTING and skill != SWORDSMANSHIP and skill != WRESTLING ):
		char.message( 501822, '' ) # You can't practice ranged weapons on this.
		return True

	# If we've already learned all we can > cancel.
	if( char.skill[ skill ] >= 300 ):
		char.message( 501828, '' ) # Your skill cannot improve any further by simply practicing with a dummy.
		return True

	# We'll get revealed if successfully punching the dummy
	char.reveal()

	# This increases the users skill
	char.checkskill( skill, 0, 1000 )

	# Display the char-action
	# (combat swing 1handed)
	char.action( 0x09 )

	if( item.id == 0x1070 or item.id == 0x1074 ):
		item.id += 1

	# Resend the item to surrounding clients after
	# changing the id and play the soundeffect
	# originating from the dummy
	item.update()
	item.soundeffect( 0x33 )

	# Add a timer to reset the id (serializable!)
	wolfpack.addtimer( random.randint( 2000, 3000 ), resetid, [ item.serial ], 1 )

	return True

# Reset the id of a swinging dummy
def resetid( object, args ):
	item = wolfpack.finditem( args[0] )

	if( item and ( item.id == 0x1071 or item.id == 0x1075 ) ):
		item.id -= 1
		item.update()
