
import wolfpack
from wolfpack.consts import ANIM_FIDGET3, SOUND_DRINK1, MAGERY, ALCHEMY
from potions import *
from potions.utilities import consumePotion, canUsePotion

def potion( char, potion, refreshtype ):
	socket = char.socket

	if not canUsePotion( char, potion ):
		return False

	# refresh potion
	if refreshtype == 18:
		char.stamina += ( char.maxstamina / 4 )
		char.updatestamina()
	# total refresh potion
	elif refreshtype == 19:
		char.stamina = char.maxstamina
		char.updatestamina()
	else:
		return False

	if char.stamina > char.maxstamina:
		char.stamina = char.maxstamina
		char.updatestamina()
		return False

	char.action( ANIM_FIDGET3 )
	char.soundeffect( SOUND_DRINK1 )
	consumePotion( char, potion, POTIONS[ refreshtype ][ POT_RETURN_BOTTLE ] )
	return True
