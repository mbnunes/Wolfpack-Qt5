import wolfpack
from random import randint
from wolfpack.time import *
from wolfpack.consts import *
from wolfpack.utilities import *

HEAL_POT_DELAY = 10000 # 10 Seconds
AGILITY_TIME = 120000  # 2 minutes
STRENGTH_TIME = 120000  # 2 minutes
INTELLIGENCE_TIME = 120000 # 2 minutes

# potion [ return_bottle, aggressive, target ]
potions = \
{
	'nightsight':				[ 1, 0, 0 ],
	'lesser_heal':				[ 1, 0, 0 ],
	'heal':						[ 1, 0, 0 ],
	'greater_heal':			[ 1, 0, 0 ],
	'lesser_cure':				[ 1, 0, 0 ],
	'cure':						[ 1, 0, 0 ],
	'greater_cure':			[ 1, 0, 0 ],
	'agility':						[ 1, 0, 0 ],
	'greater_agility':			[ 1, 0, 0 ],
	'strength':					[ 1, 0, 0 ],
	'greater_strength':		[ 1, 0, 0 ],
	'lesser_explosion':		[ 0, 1, 1 ],
	'explosion':				[ 0, 1, 1 ],
	'greater_explosion':	[ 0, 1, 1 ],
	'lesser_poison':			[ 1, 0, 0 ],
	'poison':					[ 1, 0, 0 ],
	'greater_poison':		[ 1, 0, 0 ],
	'deadly_poison':			[ 1, 0, 0 ]
}

POT_RETURN_BOTTLE = 0
POT_AGGRESSIVE = 1
POT_TARGET = 2

# Use the potion
def onUse( char, item ):
	# Potions need to be on your body to use them, or in arms reach.
	if item.getoutmostchar() != char:
		char.message( "This potion is out of your reach..." )
		return OK

	if not item.hastag( 'potiontype' ):
		return OOPS
	else:
		potiontype = item.gettag( 'potiontype' )
		potioncheck( char, item, potiontype )

	return OK

# Check what kind of potion we use, drink or throw
def potioncheck( char, item, potiontype ):
	socket = char.socket
	
	# Lets get the type of potion
	if not potiontype:
		return OOPS
	
	# Do we throw this thing?
	if potions[ potiontype ][ POT_TARGET ] == TRUE:
		# Explosion Potion
		if potiontype == 'lesser_explosion' or potiontype == 'explosion' or potiontype == 'greater_explosion':
			socket.sysmessage( 'Please select a target...', RED )
		
		# Not Known
		else:
			socket.sysmessage( 'What am I throwing!?', RED )
	
	# We just drink this potion...
	else:
		# Heal Potions
		if potiontype == 'lesser_heal' or potiontype == 'heal' or potiontype == 'greater_heal':
			if canUsePotion( char, item ):
				if char.hitpoints >= char.maxhitpoints:
					socket.sysmessage( 'You are already at full health!' )
				else:
					healPotion( char, item, potiontype )
		
		# Cure Potions
		elif potiontype == 'lesser_cure' or potiontype == 'cure' or potiontype == 'greater_cure':
			if canUsePotion( char, item ):
				socket.sysmessage( 'Drinking a cure potion.' )
		
		# Agility Potions
		elif potiontype == 'agility' or potiontype == 'greater_agility':
			if canUsePotion( char, item ):
				if not char.hastag( 'agility_effect' ):
					agilityPotion( char, item, potiontype )
				else:
					char.socket.sysmessage( 'You are already under a similar effect!', GRAY )
		
		# Strength Potions
		elif potiontype == 'strength' or potiontype == 'greater_strength':
			if canUsePotion( char, item ):
				if not char.hastag( 'strength_effect' ):
					strengthPotion( char, item, potiontype )
				else:
					char.socket.sysmessage( 'You are already under a similar effect!', GRAY )
		
		# Poison Potions
		elif potiontype == 'lesser_poison' or potiontype == 'poison' or potiontype == 'greater_poison' or potiontype == 'deadly_poison':
			if canUsePotion( char, item ):
				poisonPotion( char, item, potiontype )
				#socket.sysmessage( 'Drinking a poison potion.' )
		
		# Nightsight Potions
		elif potiontype == 'nightsight':
			if canUsePotion( char, item ):
				nightsightPotion( char, potion )
		
		# Not Known
		else:
			socket.sysmessage( 'Unknown potion...' )
			return OOPS

	return OK

# You have to have one hand free for using a potion
# This is not valid for explosion potions
def canUsePotion( char, item ):
	firsthand = char.itemonlayer( 1 )
	secondhand = char.itemonlayer( 2 )

	if not firsthand and not secondhand:
		return OK

	if firsthand and not secondhand and not firsthand.twohanded:
		return OK
	
	if not firsthand and secondhand and not secondhand.twohanded:
		return OK

	char.socket.cilocmessage( 0x7A99C ) # You must have a free hand to drink a potion.
	return OOPS

# Display the drink action
def drinkAnim( char ):
	char.action( ANIM_FIDGET3 )
	char.soundeffect( SOUND_DRINK1 )

# Consume the potion
def consumePotion( char, potion, givebottle ):
	
	if potion.amount == 1:
		potion.delete()
	else:
		potion.amount -= 1
		potion.update()

	if givebottle == TRUE: # Lets add an empty bottle!
		bottle = wolfpack.additem( 'f0e' ) # Empty Bottle Definition
		if not wolfpack.utilities.tocontainer( bottle, char.getbackpack() ):
			bottle.update()

# Nightsight potion
def nightsightPotion( char, potion ):
	char.message( "This potion will help you to see in the dark" )

# 10 Second Delay
def checkHealTimer( char ):
	
	if not char.hastag( "heal_timer" ):
		char.settag( "heal_timer", wolfpack.time.servertime() + HEAL_POT_DELAY )
		return OK

	# Compare 
	elapsed = int( char.gettag( "heal_timer" ) )
	if elapsed > wolfpack.time.servertime():
		char.socket.clilocmessage( 0x7A20B, GRAY ) # You must wait 10 seconds before using another healing potion.
		return OOPS
	else:
		char.settag( "heal_timer", wolfpack.time.servertime() + HEAL_POT_DELAY )

	return OK

# Heal Potions
def healPotion( char, potion, healtype ):
	if not canUsePotion( char, potion ) or not checkHealTimer( char ):
		return OOPS

	amount = 0

	# Lesser Heal
	if healtype == "lesser_heal":
		amount = randint( POTION_LESSERHEAL_RANGE[0], POTION_LESSERHEAL_RANGE[1] )

	# Heal
	elif healtype == "heal":
		amount = randint( POTION_HEAL_RANGE[0], POTION_HEAL_RANGE[1] )

	# Greater Heal
	elif healtype == "greater_heal":
		amount = randint( POTION_GREATERHEAL_RANGE[0], POTION_GREATERHEAL_RANGE[1] )

	char.hitpoints = min( char.hitpoints + amount, char.maxhitpoints ) # We don't heal over our maximum health

	# Resend Health
	char.updatehealth()
	char.socket.sysmessage( 'You healed ' + str( amount ) + ' hitpoints.', GRAY )

	drinkAnim( char )
	consumePotion( char, potion, potions[ healtype ][ POT_RETURN_BOTTLE ] )
	
	return OK

# Dexterity Effect Timer
def effectdextimer( time, args ):
	char = args[0]
	effecttype = args[1]
	bonus = args[2]
	
	if effecttype == 'agility' or effecttype == 'greater_agility':
		if not char.hastag( 'agility_effect' ):
			return OOPS
		char.dexterity = char.dexterity - bonus
		char.maxstamina = char.maxstamina - bonus
		char.deltag( 'agility_effect' )
		char.updatestamina()
		char.updatestats()
	return OK

# Strength Effect Timer
def effectstrtimer( time, args ):
	char = args[0]
	effecttype = args[1]
	bonus = args[2]
	
	if effecttype == 'strength' or effecttype == 'greater_strength':
		if not char.hastag( 'strength_effect' ):
			return OOPS
		char.strength = char.strength - bonus
		char.maxhitpoints = char.maxhitpoints - bonus
		char.deltag( 'strength_effect' )
		char.updatehealth()
		char.updatestats()
	return OK

# Intelligence Effect Timer
def effectinttimer( time, args ):
	char = args[0]
	effecttype = args[1]
	bonus = args[2]
	if effecttype == 'intelligence' or effecttype == 'greater_intelligence':
		if not char.hastag( 'intelligence_effect' ):
			return OOPS
		char.intelligence = char.intelligence - bonus
		char.maxmana = char.maxmana - bonus
		char.deltag( 'intelligence_effect' )
		char.updatemana()
		char.updatestats()
	return OK

# Agility Potion
def agilityPotion( char, potion, agilitytype ):
	if not canUsePotion( char, potion ):
		return OOPS

	bonus = 0

	# Agility
	if agilitytype == 'agility':
		bonus = 10
	
	# Greater Agility
	elif agilitytype == 'greater_agility':
		bonus = 20
	
	else:
		return OOPS
		
	char.dexterity = char.dexterity + bonus
	char.maxstamina = char.maxstamina + bonus
	wolfpack.addtimer( AGILITY_TIME, "potions.effectdextimer", [ char, agilitytype, bonus ] )
	char.settag( 'agility_effect', 'true' )
	char.updatestamina()
	char.updatestats()

	drinkAnim( char )
	char.effect( 0x375a, 10, 15 )
	char.soundeffect( SOUND_AGILITY_UP )
	consumePotion( char, potion, potions[ agilitytype ][ POT_RETURN_BOTTLE ] )
	
	return OK

# Strength Potion
def strengthPotion( char, potion, strengthtype ):
	if not canUsePotion( char, potion ):
		return OOPS

	bonus = 0

	# Agility
	if strengthtype == 'strength':
		bonus = 10
	
	# Greater Agility
	elif strengthtype == 'greater_strength':
		bonus = 20
	
	else:
		return OOPS
		
	char.strength = char.strength + bonus
	char.maxhitpoints = char.maxhitpoints + bonus
	wolfpack.addtimer( STRENGTH_TIME, "potions.effectstrtimer", [ char, strengthtype, bonus ] )
	char.settag( 'strength_effect', 'true' )
	char.updatehealth()
	char.updatestats()

	drinkAnim( char )
	char.effect( 0x375a, 10, 15 )
	char.soundeffect( SOUND_STRENGTH_UP )
	consumePotion( char, potion, potions[ strengthtype ][ POT_RETURN_BOTTLE ] )
	
	return OK

# Poison Potions
def poisonPotion( char, potion, poisontype ):
	char.socket.sysmessage( "You shouldn't drink that..." )
	return OK
	
# INVIS POTION
# ID: 0x7A9A3 (0)
# Your skin becomes extremely sensitive to light, changing to mirror the colors of things around you.
