
import wolfpack
import time
import random
from wolfpack.consts import ANIM_FIDGET3, SOUND_DRINK1, SOUND_STRENGTH_UP, \
	MAGERY, ALCHEMY, STRENGTH_TIME
from potions.consts import *
from potions.utilities import consumePotion, canUsePotion

# Strength Potion
def potion( char, potion, strengthtype ):
	socket = char.socket
	if not canUsePotion( char, potion ):
		return False

	bonus = 0

	# Agility
	if strengthtype == 9:
		bonus = 10
	# Greater Agility
	elif strengthtype == 10:
		bonus = 20
	# Oops!
	else:
		return False

	if char.hastag( "str_pot_timer" ):
		# Compare
		elapsed = int( char.gettag( "str_pot_timer" ) )

		if elapsed - time.time() > STRENGTH_TIME:
			char.deltag('str_pot_timer')
		elif elapsed > time.time():
			socket.clilocmessage(502173) # You are already  under a similar effect
			return False

	char.settag( "str_pot_timer", time.time() + STRENGTH_TIME )

	if char.strength + bonus < 1:
		bonus = -(char.strength - 1)
	char.strength2 += bonus
	char.strength += bonus
	char.hitpoints = min(char.hitpoints, char.maxhitpoints)
	char.updatehealth()
	char.updatestats()

	char.addtimer( int(STRENGTH_TIME * 1000.0), "magic.utilities.statmodifier_expire", [0, bonus], 1, 1, "magic_statmodifier_0", "magic.utilities.statmodifier_dispel" )

	char.action( ANIM_FIDGET3 )
	char.soundeffect( SOUND_DRINK1 )
	char.effect( 0x375a, 10, 15 )
	char.soundeffect( SOUND_STRENGTH_UP )
	consumePotion( char, potion, POTIONS[ strengthtype ][ POT_RETURN_BOTTLE ] )

	return True
