
import wolfpack
import time
import random
from wolfpack.consts import ANIM_FIDGET3, MAGERY, ALCHEMY, HEAL_POT_DELAY, \
	POTION_LESSERHEAL_RANGE, POTION_HEAL_RANGE, POTION_GREATERHEAL_RANGE, \
	SOUND_DRINK1
from potions.consts import *
from potions.utilities import consumePotion, canUsePotion

# Heal Potions
def potion( char, potion, healtype ):
	socket = char.socket
	if not canUsePotion( char, potion ):
		return False

	if char.poison > -1:
		# You can not heal yourself in your current state.
		socket.clilocmessage(1005000)
		return False
	if char.hitpoints >= char.maxhitpoints:
		socket.clilocmessage(1049547)
		if char.hitpoints > char.maxhitpoints:
			char.hitpoints = char.maxhitpoints
			char.updatehealth()
		return False

	# Compare
	if socket.hastag('heal_pot_timer'):
		elapsed = int( socket.gettag( "heal_pot_timer" ) )
		if elapsed > time.time():
			# Broken Timer
			if time.time() - elapsed > HEAL_POT_DELAY:
				socket.deltag('heal_pot_timer')
			else:
				socket.clilocmessage( 500235 ) # You must wait 10 seconds before using another healing potion.
				return False

	socket.settag( "heal_pot_timer", time.time() + HEAL_POT_DELAY)
	amount = 0

	# Lesser Heal
	if healtype == 1:
		amount = random.randint( POTION_LESSERHEAL_RANGE[0], POTION_LESSERHEAL_RANGE[1] )
	# Heal
	elif healtype == 2:
		amount = random.randint( POTION_HEAL_RANGE[0], POTION_HEAL_RANGE[1] )
	# Greater Heal
	elif healtype == 3:
		amount = random.randint( POTION_GREATERHEAL_RANGE[0], POTION_GREATERHEAL_RANGE[1] )

	char.hitpoints = min( char.hitpoints + amount, char.maxhitpoints ) # We don't heal over our maximum health

	# Resend Health
	char.updatehealth()
	#char.socket.clilocmessage( 1060203, str(amount) ) # broken
	socket.sysmessage( 'You have had ' + str( amount ) + ' hit points of damage healed.' )

	char.action( ANIM_FIDGET3 )
	char.soundeffect( SOUND_DRINK1 )
	consumePotion( char, potion, POTIONS[ healtype ][ POT_RETURN_BOTTLE ] )

	return True
