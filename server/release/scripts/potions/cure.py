
import wolfpack
from wolfpack.consts import ANIM_FIDGET3, SOUND_DRINK1, MAGERY, ALCHEMY
from potions import *
from potions.utilities import consumePotion, canUsePotion

# Cure Potions
def potion( char, potion, curetype ):
	socket = char.socket
	if not canUsePotion( char, potion ):
		return False

	if char.poison == -1:
		# You are not poisoned.
		socket.clilocmessage( 1042000 )
		return False

	if curetype == 4:
		curelevel = 0
	elif curetype == 5:
		curelevel = 1
	elif curetype == 6:
		curelevel = 2

	if curelevel >= char.poison:
		poison.cure( char )
		char.effect( 0x373a, 10, 15 )
		char.soundeffect( 0x1e0 )
	# curelevel now must be lower than char.poison
	else:
		if ( char.poison - curelevel ) == 1:
			chance = 0.5
		elif ( char.poison - curelevel ) == 2:
			chance = 0.25
		elif ( char.poison - curelevel ) == 3:
			chance = 0.1

		if chance > random():
			poison.cure(char)
			char.effect( 0x373a, 10, 15 )
			char.soundeffect( 0x1e0 )

	# Drinking and consume
	char.action( ANIM_FIDGET3 )
	char.soundeffect( SOUND_DRINK1 )
	consumePotion( char, potion, POTIONS[ curetype ][ POT_RETURN_BOTTLE ] )
	# If we succeeded, special effects
	if char.poison == -1:
		char.effect( 0x373a, 10, 15 )
		char.soundeffect( 0x1e0 )
		socket.clilocmessage( 500231 ) # You feel cured of poison!
	else:
		socket.clilocmessage( 500232 ) # That potion was not strong enough to cure your ailment!
	return True
