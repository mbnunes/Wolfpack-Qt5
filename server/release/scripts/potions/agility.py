
import wolfpack
import time
import random
from wolfpack.consts import ANIM_FIDGET3, SOUND_DRINK1, SOUND_AGILITY_UP, \
	MAGERY, ALCHEMY, AGILITY_TIME
from potions.consts import *
from potions.utilities import consumePotion, canUsePotion

# Agility Potion
def potion( char, potion, agilitytype ):
	socket = char.socket
	bonus = 0

	if not canUsePotion( char, potion ):
		return False

	# Agility
	if agilitytype == 7:
		bonus = 10
	# Greater Agility
	elif agilitytype == 8:
		bonus = 20
	# Oops!
	else:
		return False

	if char.hastag( "dex_pot_timer" ):
		# Compare
		elapsed = int( char.gettag( "dex_pot_timer" ) )

		# Some bug occured
		if elapsed - time.time() > AGILITY_TIME:
			char.deltag('dex_pot_timer')
		elif elapsed > time.time():
				socket.clilocmessage(502173) # You are already under a similar effect.
				return False

	char.settag( 'dex_pot_timer', time.time() + AGILITY_TIME )

	if char.dexterity + bonus < 1:
		bonus = -(char.strength - 1)

	char.dexterity2 += bonus
	char.dexterity += bonus
	char.stamina = min( char.stamina, char.maxstamina )
	char.updatestamina()
	char.updatestats()

	char.addtimer( int( AGILITY_TIME * 1000 ), "magic.utilities.statmodifier_expire", [1, bonus], 1, 1, "magic_statmodifier_1", "magic.utilities.statmodifier_dispel" )

	char.action( ANIM_FIDGET3 )
	char.soundeffect( SOUND_DRINK1 )
	char.effect( 0x375a, 10, 15 )
	char.soundeffect( SOUND_AGILITY_UP )
	consumePotion( char, potion, POTIONS[ agilitytype ][ POT_RETURN_BOTTLE ] )

	return True
