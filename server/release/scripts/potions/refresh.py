
import wolfpack
from wolfpack import properties
from wolfpack.consts import *
from potions import *
from potions.utilities import consumePotion, canUsePotion

def potion( char, potion, refreshtype ):
	socket = char.socket

	if not canUsePotion( char, potion ):
		return False

	# refresh potion
	if refreshtype == 18:
		amount = char.maxstamina / 4
		
		# Apply Enhancepotions Bonus
		enhancepotions = properties.fromchar(char, ENHANCEPOTIONS)
		if enhancepotions > 0:
			amount += (enhancepotions * amount) / 100
							
		char.stamina = min(char.maxstamina, char.stamina + amount)
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
