
from wolfpack import tr
import wolfpack
import wolfpack.time
import math
import time
from random import randint, random
from wolfpack.utilities import hex2dec, throwobject, energydamage, checkLoS
from system import poison
from wolfpack.consts import *

POT_RETURN_BOTTLE = 0
POT_AGGRESSIVE = 1
POT_TARGET = 2
POT_NAME = 3
KEG_NAME = 4
POT_DEF = 5

explosions = [ 0x36b0, 0x36bd, 0x36cb ]
explodables = [ 'potion_greaterexplosion', 'potion_explosion', 'potion_lesserexplosion', 'f0d' ]

POTIONS = {
	# Nightsight
	0:	[ True, 0, 0, '#1044542', '#1041620', 'potion_nightsight' ],
	# Healing
	1:	[ True, 0, 0, '#1044543', '#1041634', 'potion_lesserheal' ],
	2:	[ True, 0, 0, '#1044544', '#1041635', 'potion_heal' ],
	3:	[ True, 0, 0, '#1044545', '#1041636', 'potion_greaterheal' ],
	# Cure
	4:	[ True, 0, 0, '#1044552', '#1041621', 'potion_lessercure' ],
	5:	[ True, 0, 0, '#1044553', '#1041622', 'potion_cure' ],
	6:	[ True, 0, 0, '#1044554', '#1041623', 'potion_greatercure' ],
	# Agility
	7:	[ True, 0, 0, '#1044540', '#1041624', 'potion_agility' ],
	8:	[ True, 0, 0, '#1044541', '#1041625', 'potion_greateragility' ],
	# Strength
	9:	[ True, 0, 0, '#1044546', '#1041626', 'potion_strength' ],
	10:	[ True, 0, 0, '#1044547', '#1041627', 'potion_greaterstrength' ],
	# Explosions
	11:	[ False, 1, 1, '#1044555', '#1041637', 'potion_lesserexplosion' ],
	12:	[ False, 1, 1, '#1044556', '#1041638', 'potion_explosion' ],
	13:	[ False, 1, 1, '#1044557', '#1041639', 'potion_greaterexplosion' ],
	# Poisons
	14:	[ True, 0, 0, '#1044548', '#1041628', 'potion_lesserpoison' ],
	15:	[ True, 0, 0, '#1044549', '#1041629', 'potion_poison' ],
	16:	[ True, 0, 0, '#1044550', '#1041630', 'potion_greaterpoison' ],
	17:	[ True, 0, 0, '#1044551', '#1041631', 'potion_deadlypoison' ],
	# Stamina
	18:	[ True, 0, 0, '#1044538', '#1041632', 'potion_refresh' ],
	19:	[ True, 0, 0, '#1044539', '#1041633', 'potion_totalrefresh' ],
	# Intelligence
	20:	[ True, 0, 0, 'Intellegence', 'A keg of Intelligence potions', 'potion_intelligence' ],
	21:	[ True, 0, 0, 'Greater Intellegence', 'A keg of Greater Intelligence potions', 'potion_greaterintelligence' ],
	# Mana
	22:	[ 1, 0, 0, 'Lesser Mana', 'A keg of Lesser Mana potions', 'potion_lessermana' ],
	23:	[ 1, 0, 0, 'Mana', 'A keg of Mana potions', 'potion_mana' ],
	24:	[ 1, 0, 0, 'Greater Mana', 'A keg of Greater Mana potions', 'potion_greatermana' ],
	# Shrink
	25:	[ True, False, True, 'Shrink', 'A keg of Shrink potions', 'potion_shrink' ],
}

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
					socket.attachtarget( "potions.targetexplosionpotion", [ item ] )
				else:
					socket.sysmessage( 'You can only throw one potion at a time!' )
			# Shrink Potion
			elif potiontype == 25:
				socket.sysmessage( 'What do you want to shrink?' )
				potions.shrink.potion( char, item )
				socket.attachtarget( 'potions.shrink.potion', [ item.serial ] )

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


