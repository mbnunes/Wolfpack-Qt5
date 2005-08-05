import wolfpack
from wolfpack import tr, properties
import time
from random import randint
from wolfpack.consts import *
from potions.consts import *
from potions.utilities import consumePotion, canUsePotion

MANA_POT_DELAY = 10.0
POTION_LESSERMANA_RANGE = [ 3, 10]
POTION_MANA_RANGE = [ 6, 20 ]
POTION_GREATERMANA_RANGE = [ 9, 30 ]

# Mana Potions
def potion( char, potion, manatype ):
	socket = char.socket
	if not canUsePotion( char, potion ):
		return False

	if char.mana >= char.maxmana:
		socket.sysmessage( tr('You are already at full mana.') )
		if char.mana > char.maxmana:
			char.mana = char.maxmana
			char.updatemana()
		return False

	# Compare
	if socket.hastag('mana_pot_timer'):
		elapsed = int( socket.gettag( 'mana_pot_timer' ) )
		if elapsed > time.time():
			# Broken Timer
			if time.time() - elapsed > MANA_POT_DELAY:
				socket.deltag('mana_pot_timer')
			else:
				socket.sysmessage( tr('You must wait a few seconds before using another mana potion.') ) 
				return False

	socket.settag( 'mana_pot_timer', time.time() + MANA_POT_DELAY )
	amount = 0

	# Lesser Mana
	if manatype == 22:
		amount = randint( POTION_LESSERMANA_RANGE[0], POTION_LESSERMANA_RANGE[1] )
	# Mana
	elif manatype == 23:
		amount = randint( POTION_MANA_RANGE[0], POTION_MANA_RANGE[1] )
	# Greater Mana
	elif manatype == 24:
		amount = randint( POTION_GREATERMANA_RANGE[0], POTION_GREATERMANA_RANGE[1] )

	# Apply Enhancepotions Bonus
	enhancepotions = properties.fromchar(char, ENHANCEPOTIONS)
	if enhancepotions > 0:
		amount += (enhancepotions * amount) / 100

	char.mana = min( char.mana + amount, char.maxmana ) # We don't add mana over our maximum mana

	# Resend Manath
	char.updatemana()

	char.action( ANIM_FIDGET3 )
	char.soundeffect( SOUND_DRINK1 )
	consumePotion( char, potion, POTIONS[ manatype ][ POT_RETURN_BOTTLE ] )

	return True
