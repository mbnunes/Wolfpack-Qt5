
import wolfpack
from wolfpack.consts import ANIM_FIDGET3, MAGERY, ALCHEMY
from potions import *
from potions.utilities import consumePotion, canUsePotion

def potion( char, potion ):
	socket = char.socket
	if not canUsePotion( char, potion ):
		return False

	# Remove an old bonus
	if char.hastag( 'nightsight' ):
		bonus = char.gettag( 'nightsight' )
		char.lightbonus = max( 0, char.lightbonus - bonus )

	# With 100% magery you gain a 18 light level bonus (min. 5)
	bonus = max( 8, min( 18, math.floor( 18 * ( char.skill[MAGERY] / 1000.0 ) ) ) )

	char.addscript( 'magic.nightsight' )
	char.settag( 'nightsight', bonus )
	char.settag( 'nightsight_start', wolfpack.time.minutes() )
	char.lightbonus += bonus

	if char.socket:
		socket.updatelightlevel()

	char.action( ANIM_FIDGET3 )
	char.soundeffect( 0x1e3 )
	char.effect( 0x376a, 9, 32 )
	consumePotion( char, potion, POTIONS[ potion.gettag('potiontype') ][ POT_RETURN_BOTTLE ] )
	return True
