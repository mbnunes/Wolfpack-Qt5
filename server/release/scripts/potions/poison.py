
import wolfpack
from wolfpack.consts import ANIM_FIDGET3, SOUND_DRINK1, MAGERY, ALCHEMY
from potions import *
from potions.utilities import consumePotion, canUsePotion

# Poison Potions
def potion( char, potion, poisontype ):
	socket = char.socket
	if not canUsePotion( char, potion ):
		return False

	# Weakest to strongest
	levels = [14, 15, 16, 17]
	if poisontype == levels[0]:
		poison.poison( char, 0 )
	elif poisontype == levels[1]:
		poison.poison( char, 1 )
	elif poisontype == levels[2]:
		poison.poison( char, 2 )
	elif poisontype == levels[3]:
		poison.poison( char, 3 )
	else:
		return False

	char.action( ANIM_FIDGET3 )
	char.soundeffect( SOUND_DRINK1 )
	consumePotion( char, potion, POTIONS[ poisontype ][ POT_RETURN_BOTTLE ] )
	return True
