
import wolfpack.time
from random import randint
from wolfpack.consts import *

# POTIONS SCRIPT
# This script in general has all functions for potions
# more1 defines the type of the potion
# more2 the strength

# You have to have one hand free for using a potion
# This is not valid for explosion potions
def canUsePotion( char ):
	firsthand = char.itemonlayer( 1 )
	secondhand = char.itemonlayer( 2 )

	if not firsthand and not secondhand:
		return 1

	if firsthand and not secondhand and not firsthand.twohanded:
		return 1
	
	if not firsthand and secondhand and not secondhand.twohanded:
		return 1

	char.socket.cilocmessage( 0x7A99C ) # You must have a free hand to drink a potion.
	return 0

# 10 Seconds Timeout
def checkPotionTimer( char ):
	socket = char.socket

	if not socket:
		return 0

	if not socket.hastag( "heal_timer" ):
		socket.settag( "heal_timer", wolfpack.time.servertime() + 10000 )
		return 1

	# Compare 
	elapsed = int( socket.gettag( "heal_timer" ) )
	if elapsed > wolfpack.time.servertime():
		char.clilocmessage( 0x7A20B ) # You must wait 10 seconds before using another healing potion.
		return 0

	socket.settag( "heal_timer", wolfpack.time.servertime() + 10000 )
	return 1

# Display the drink action
def drinkAnim( char ):
	char.action( 0x21 )
	char.soundeffect( 0x50 )
	
# Consume the potion
def consumePotion( potion ):
	if potion.amount == 1:
		potion.delete()
	else:
		potion.amount -= 1

# Nightsight potion
def nightsightPotion( char, potion ):
	char.message( "This potion will help you to see in the dark" )

# Heal potions
def healPotion( char, potion ):
	if not canUsePotion( char ) or not checkPotionTimer( char ):
		return

	amount = 0
	pType = potion.more1
	
	# Lesser Heal
	if pType == 1:
		amount = randint( POTION_LESSERHEAL_RANGE[0], POTION_LESSERHEAL_RANGE[1] )

	# Heal
	elif pType == 2:
		amount = randint( POTION_HEAL_RANGE[0], POTION_HEAL_RANGE[1] )

	# Greater Heal
	elif pType == 3:
		amount = randint( POTION_GREATERHEAL_RANGE[0], POTION_GREATERHEAL_RANGE[1] )

	char.health = min( char.health + amount, char.strength ) # We don't heal over our maximum health

	# Resend Health
	char.updatehealth()

	drinkAnim( char )
	consumePotion( potion )

# Poison Potions
def poisonPotion( char, potion ):
	char.socket.sysmessage( "You better don't drink that." )

# Agility Potion
def agilityPotion( char, potion ):
	if not canUsePotion( char ):
		return

	amount = 0
	pType = potion.more1
	
	# Agility
	if pType == 9:
#		amount = randint( POTION_LESSERHEAL_RANGE[0], POTION_LESSERHEAL_RANGE[1] )
		pass

	# Greater Agility
	elif pType == 10:
		pass
#		amount = randint( POTION_HEAL_RANGE[0], POTION_HEAL_RANGE[1] )

#	char.health = min( char.health + amount, char.strength ) # We don't heal over our maximum health
	# Resend Stamina
	#char.updatestamina()
	# NOTE: TEMPEFFECT!	

	drinkAnim( char )
	consumePotion( potion )

# Strength Potion
def strengthPotion( char, potion ):
	if not canUsePotion( char ):
		return

	amount = 0
	pType = potion.more1
	
	# Strength
	if pType == 11:
		pass
#		amount = randint( POTION_LESSERHEAL_RANGE[0], POTION_LESSERHEAL_RANGE[1] )

	# Greater Strength
	elif pType == 12:
		pass
#		amount = randint( POTION_HEAL_RANGE[0], POTION_HEAL_RANGE[1] )

#	char.health = min( char.health + amount, char.strength ) # We don't heal over our maximum health
	# Resend Health
	#char.updatestamina()
	# NOTE: TEMPEFFECT!	

	drinkAnim( char )
	consumePotion( potion )

potions = {
	1: nightsightPotion,

	2: healPotion, 		# Lesser Heal
	3: healPotion, 		# Heal
	4: healPotion, 		# Greater Heal

	5: poisonPotion, 	# Lesser Poison
	6: poisonPotion, 	# Poison
	7: poisonPotion,	# Greater Poison
	8: poisonPotion, 	# Deadly Poison

	9: agilityPotion,	# Agility
	10: agilityPotion,	# Greater Agility

	11: strengthPotion,  # Strength
	12: strengthPotion  # Greater Strength
}

def onUse( char, item ):
	# Potions need to be on your body to use them.
	if item.getoutmostchar() != char:
		char.message( "This potion needs to be in your belongings to use it." )
		return 1

	pType = item.more1

	if not potions.has_key( pType ):
		char.socket.sysmessage( "This potion has an unknown type: %u" % pType )
		return 1
	
	potions[ pType ]( char, item )
	return 1

# INVIS POTION
# ID: 0x7A9A3 (0)
# Your skin becomes extremely sensitive to light, changing to mirror the colors of things around you.
