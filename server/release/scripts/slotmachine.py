#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
import random
from wolfpack import tr

# Define prizes here
# Format: <message>, <item-def>
prizes = ( ( tr("Single bars, you are a winner!"),     "slot_prize_1" ),
			( tr("Double bars, you are a winner!"),     "slot_prize_2" ),
			( tr("Triple bars, you are a winner!"),     "slot_prize_3" ),
			( tr("Any three 7`s, you are a winner!"),   "slot_prize_4" ),
			( tr("Three blue 7`s, you are a winner!"),  "slot_prize_5" ),
			( tr("Three white 7`s, you are a winner!"), "slot_prize_6" ),
			( tr("Three red 7`s, you are a winner!"),   "slot_prize_7" ),
			( tr("Jackpot, you are a winner!"),         "slot_prize_8" ) )

def onUse( char, item ):
	if( char.dead ):
		char.message( tr("Your ghostly hand passes trough this object.") )
		return 1

	# Calc the distance to this object and check if he can use it
	if( char.pos.distance( item.pos ) > 1 ):
		char.message( 500295, "" ) # You are too far away to do that.
		return 1

	# Consume a given amount of gold
	if( char.countresource( 0xEED ) < 5 ):
		char.message( tr("You don't have enough gold to play.") )
		return 1

	char.useresource( 5, 0xEED ) # Lets use our gold

	# Spinsound (If it sounds awful it's rippers fault!)
	item.soundeffect( 0x57 )

	# 5% Win chance
	chance = random.randrange( 1, 100 )

	# We won!
	if( chance <= 5 ):
		# Select a price randomly
		prize = random.choice( prizes )
		char.message( prize[0] )

		item = wolfpack.additem( prize[1] )
		item.container = char.getbackpack()
		item.update()

		# Play a soundeffect
		char.soundeffect( 0x38 )

	# We Lost
	else:
		char.message( tr("You lost! Insert coins to try again.") )

	return 1
