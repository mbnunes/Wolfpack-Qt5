
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
import potionkeg
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
		char.message( tr("This potion is out of your reach...") )
		return False

	# Lets make sure the tag exists.
	if not item.hasintproperty( 'potiontype' ):
		return False

	potiontype = item.getintproperty( 'potiontype' )
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
				socket.sysmessage( tr('You should throw this now!'), RED )
				socket.attachtarget( "potions.utilities.targetpotion", [ item.serial ] )
			else:
				socket.sysmessage( tr('You can only throw one potion at a time!') )
		# Shrink Potion
		elif potiontype == 25:
			if item.amount == 1:
				socket.sysmessage( tr('What do you want to shrink?') )
				potions.shrink.potion( char, item )
				socket.attachtarget( 'potions.shrink.potion', [ item.serial ] )
			else:
				socket.sysmessage( tr('You can only throw one potion at a time!') )

	# We just drink this potion...
	else:
		# If it's a drinkable potion we check if there is a free hand.
		if not canUsePotion( char, item ):
			return True

		# Nightsight Potions
		if potiontype == 0:
			potions.nightsight.potion( char, item, potiontype )
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


# Droping a potion on a potion keg
def onDropOnItem( keg, potion ):
	char = potion.container
	socket = char.socket
	potiontype = None
	kegtype = None

	if not char or not socket:
		return False

	socket.sysmessage( "Potion: %s" % potion.baseid )
	socket.sysmessage( "Keg: %s" % keg.baseid )
	if keg.baseid in [ 'potion_keg' ]:

		if not potion.hasscript('potions') and not keg.hasscript('potionkeg'):
			return False

		if potion.hasscript( 'potionkeg' ) or keg.hasscript('potions'):
			return False

		socket.sysmessage( "Passed script test!" )
		if not keg.hastag( 'kegfill' ):
			kegfill = 0
			keg.settag( 'kegfill', kegfill )
		else:
			kegfill = int( keg.gettag( 'kegfill' ) )
			if kegfill < 0: # Safeguard against negative fills
				kegfill = 0

		if potion.hasintproperty( 'potiontype' ):
			potiontype = potion.getintproperty( 'potiontype' )
		if potion.hastag( 'potiontype' ):
			potiontype = int( potion.gettag( 'potiontype' ) )

		if keg.hasintproperty( 'potiontype' ):
			kegtype = keg.getintproperty( 'potiontype' )
		if keg.hastag( 'potiontype' ):
			kegtype = int( keg.gettag( 'potiontype' ) )

		if not potiontype:
			socket.sysmessage( tr( "Only potions may be added to a potion keg!" ) )
			return True

		if kegfill >= 100:
			socket.clilocmessage( 502247 )
			return True

		socket.sysmessage( "Keg ID: %s" % ( hex(keg.id) ) )
		socket.sysmessage( "Potion Type: %u" % ( potiontype ) )
		if keg.baseid in [ 'potion_keg' ]:
			socket.sysmessage( "Testing 2" )
			if kegtype:
				socket.sysmessage( "Keg Type: %u" % ( kegtype ) )
				socket.sysmessage( "Testing 3" )
				if potiontype == kegtype:
					if kegfill < 100 and kegfill >= 0:
						kegfill += 1
						keg.settag( 'kegfill', kegfill )
						char.soundeffect( 0x240 )
						potions.utilities.consumePotion( char, potion )
						keg.update()
						potionkeg.kegfillmessage( char, kegfill )
						socket.clilocmessage( 502239 )
						return True
					else:
						# The keg will not hold any more!
						socket.clilocmessage( 502233 )
						return True
				else:
					# You decide that it would be a bad idea to mix different types of otions.
					socket.clilocmessage( 502236 )
					return True
			else:
				if potion.hastag( 'potiontype' ):
					kegtype = potion.gettag( 'potiontype' )
				elif potion.hasintproperty( 'potiontype' ):
					kegtype = potion.getintproperty( 'potiontype' )
				keg.settag( 'potiontype', kegtype )
				keg.settag( 'kegfill', 1 )
				keg.name = POTIONS[ kegtype ][ KEG_NAME ]
				char.soundeffect( 0x240 )
				consumePotion( char, potion )
				keg.update()
				return True
	else:
		return False

	return False

