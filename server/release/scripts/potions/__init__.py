
from wolfpack import tr
import wolfpack
import wolfpack.time
import math
import time
from random import randint, random
from wolfpack.utilities import hex2dec, throwobject, energydamage, checkLoS
from system import poison
from wolfpack.consts import *
from potions.consts import *
from potions.utilities import *
# Potions to enable
import potions.agility
import potions.cure
import potions.explosion
import potions.heal
import potions.mana
import potions.nightsight
import potions.poison
import potions.refresh
import potions.shrink
import potions.strength

# Use the potion
def onUse( char, item ):
	socket = char.socket
	# Potions need to be on your body to use them, or in arms reach.
	if not char.canreach(item, -1):
		char.message( "This potion is out of your reach..." )
		return False

	# Lets make sure the tag exists.
	if not item.hastag( 'potiontype' ):
		return False
	else:
		potiontype = item.gettag( 'potiontype' )
		# Make sure it's in the index
		if not potiontype in POTIONS:
			return False

		# Do we throw this thing?
		if POTIONS[ potiontype ][ POT_TARGET ] == True:
			# Explosion Potion
			if potiontype in [ 11, 12, 13 ] and not item.hastag('exploding'):
				# char, potion, counter value
				if item.amount == 1:
					potions.explosion.potion( char.serial, item.serial )
					socket.sysmessage( 'You should throw this now!', RED )
					socket.attachtarget( "potions.utilities.targetpotion", [ item.serial ] )
				else:
					socket.sysmessage( 'You can only throw one potion at a time!' )
			# Shrink Potion
			elif potiontype == 25:
				if item.amount == 1:
					socket.sysmessage( 'What do you want to shrink?' )
					potions.shrink.potion( char, item )
					socket.attachtarget( 'potions.shrink.potion', [ item.serial ] )
				else:
					socket.sysmessage( 'You can only throw one potion at a time!' )

		# We just drink this potion...
		else:
			# If it's a drinkable potion we check if there is a free hand.
			if not canUsePotion( char, item ):
				return True

			# Nightsight Potions
			if potiontype == 0:
				potions.nightsight.potion( char, item )
			# Heal Potions
			elif potiontype in [1,2,3]:
				potions.heal.potion(char, item, potiontype )
			# Cure Potions
			elif potiontype in [4,5,6]:
				potions.cure.potion( char, item, potiontype )
			# Agility Potions
			elif potiontype in [7,8]:
				potions.agility.potion( char, item, potiontype )
			# Strength Potions
			elif potiontype in [ 9, 10 ]:
				potions.strength.potion( char, item, potiontype )
			# Poison Potions
			elif potiontype in [ 14, 15, 16, 17 ]:
				potions.poison.potion( char, item, potiontype )
			# Refresh Potions
			elif potiontype in [ 18, 19 ]:
				potions.refresh.potion( char, item, potiontype )

		return True


