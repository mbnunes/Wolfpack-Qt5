import wolfpack
from random import randint
from wolfpack.time import *
from wolfpack.consts import *
from wolfpack.utilities import hex2dec, throwobject, energydamage

HEAL_POT_DELAY = 10000 # 10 Seconds
AGILITY_TIME = 120000  # 2 minutes
STRENGTH_TIME = 120000  # 2 minutes
INTELLIGENCE_TIME = 120000 # 2 minutes

# potion [ return_bottle, aggressive, target ]
potions = \
{
	0:		[ 1, 0, 0 ], # nightsight
	1:		[ 1, 0, 0 ], # lesser heal
	2:		[ 1, 0, 0 ], # heal
	3:		[ 1, 0, 0 ], # greater heal
	4:		[ 1, 0, 0 ], # lesser cure
	5:		[ 1, 0, 0 ], # cure
	6:		[ 1, 0, 0 ], # greater cure
	7:		[ 1, 0, 0 ], # agility
	8:		[ 1, 0, 0 ], # greater agility
	9:		[ 1, 0, 0 ], # strength
	10:	[ 1, 0, 0 ], # greater strength
	11:	[ 0, 1, 1 ], # lesser explosion
	12:	[ 0, 1, 1 ], # explosion
	13:	[ 0, 1, 1 ], # greater explosion
	14:	[ 1, 0, 0 ], # lesser poison
	15:	[ 1, 0, 0 ], # poison
	16:	[ 1, 0, 0 ], # greater poison
	17:	[ 1, 0, 0 ], # deadly poison
	18:	[ 1, 0, 0 ], # refresh
	19:	[ 1, 0, 0 ] # total refresh
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

def targetexplosionpotion(char, args, target):
	potion = args[0]
	throwobject( char, potion, target, 1, 3, 5 )
	# char, potion, counter value
	potionexplosion( [ char, potion, 3 ] )
	potion.settag('exploding', 'true')
	return

def potionexplosion(args):
	# item.say() or similar feature is missing
	# item.effect() is missing
	char = args[0]
	potion = args[1]
	counter = args[2]
	if counter > 0:
		wolfpack.addtimer(1000, "potions.potioncountdown", [char, potion, counter] )
		return
	else:
		potion.soundeffect(0x307) # Boom!
		potion.effect(0x36BD, 20, 10)
		potionregion( [char, potion] )
		potion.delete()
		return

def potioncountdown( time, args ):
	char = args[0]
	potion = args[1]
	counter = args[2]
	if counter > 0:
		potion.say("%s" % str(counter) )
		potionexplosion([ char, potion, int(counter - 1) ])
	return

def potionregion( args ):
	char = args[0]
	potion = args[1]
	if potion.container:
		return OOPS
	if potion.gettag('potiontype') == 11:
		outradius = 1
	elif potion.gettag('potiontype') == 12:
		outradius = 2
	elif potion.gettag('potiontype') == 13:
		outradius = 3
	else:
		outradius = 1
	x1 = int(potion.pos.x - outradius)
	y1 = int(potion.pos.y - outradius)
	x2 = int(potion.pos.x + outradius)
	y2 = int(potion.pos.y + outradius)
	# Character Bombing
	damageregion = wolfpack.charregion( x1, y1, x2, y2, potion.pos.map )
	target = damageregion.first
	while target:
		target.effect(0x36BD, 20, 10)
		potiondamage(char, target, potion)
		target = damageregion.next

	# Chain Reaction Bombing
	chainregion = wolfpack.itemregion( x1, y1, x2, y2, potion.pos.map )
	chainbomb= chainregion.first
	while chainbomb:
		if chainbomb.baseid in [ 'potion_greaterexplosion', 'potion_explosion', 'potion_lesserexplosion', 'f0d' ]:
			if not chainbomb.hastag('exploding'):
				chainbomb.settag('exploding', 'true')
				wolfpack.addtimer(randint(1000, 2250), "potions.potioncountdown", [char, chainbomb, 1] )
				chainbomb = chainregion.next
			else:
				chainbomb = chainregion.next
		else:
			chainbomb = chainregion.next

	return

def potiondamage( char, target, potion ):
	target.say("Ouch!")
	if potion.gettag('potiontype') == 11:
		damage = randint(1, 5)
	elif potion.gettag('potiontype') == 12:
		damage = randint(6, 10)
	elif potion.gettag('potiontype') == 13:
		damage = randint(11, 20)
	else:
		damage = randint(1, 20)
	energydamage(target, char, damage, fire=100)
	return

# Check what kind of potion we use, drink or throw
def potioncheck( char, item, potiontype ):
	socket = char.socket

	# Lets get the type of potion
	if not potiontype:
		return OOPS

	# Do we throw this thing?
	if potions[ potiontype ][ POT_TARGET ] == TRUE:
		# Explosion Potion
		if potiontype in [ 11, 12, 13 ]:
			socket.sysmessage( 'Please select a target...', RED )
			socket.attachtarget( "potions.targetexplosionpotion", [ item ] )

		# Not Known
		else:
			socket.sysmessage( 'What am I throwing!?', RED )

	# We just drink this potion...
	else:
		# Heal Potions
		if potiontype in [ 1, 2, 3 ]:
			if canUsePotion(char, item):
				if char.hastag('poisoned'):
					# You can not heal yourself in your current state.
					char.socket.clilocmessage(1005000)
					return OOPS
				if char.hitpoints >= char.maxhitpoints:
					# You decide against drinking this potion, as you are already at full health.
					char.socket.clilocmessage(1049547)
				else:
					healPotion(char, item, potiontype)

		# Cure Potions
		elif potiontype in [ 4, 5, 6 ]:
			if canUsePotion(char, item):
				socket.sysmessage('Drinking a cure potion.')

		# Agility Potions
		elif potiontype in [ 7, 8 ]:
			if canUsePotion(char, item):
				if not char.hastag('agility_effect'):
					agilityPotion(char, item, potiontype)
				else:
					# You are already under a similar effect.
					char.socket.clilocmessage(502173)

		# Strength Potions
		elif potiontype in [ 9, 10 ]:
			if canUsePotion( char, item ):
				if not char.hastag( 'strength_effect' ):
					strengthPotion( char, item, potiontype )
				else:
					# You are already under a similar effect.
					char.socket.clilocmessage( 502173, '', GRAY )

		# Poison Potions
		elif potiontype in [ 14, 15, 16, 17 ]:
			if canUsePotion( char, item ):
				poisonPotion( char, item, potiontype )

		# Nightsight Potions
		elif potiontype in [ 0 ]:
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

	char.socket.clilocmessage( 0x7A99C ) # You must have a free hand to drink a potion.
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
		char.socket.clilocmessage( 500235, '', GRAY ) # You must wait 10 seconds before using another healing potion.
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
	#char.socket.clilocmessage( 1060203, str(amount) , GRAY, NORMAL ) # broken
	char.socket.sysmessage( 'You have had ' + str( amount ) + ' hit points of damage healed.', GRAY )

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
	wolfpack.addtimer( AGILITY_TIME, "potions.effectdextimer", [ char, agilitytype, bonus ], 1 )
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
	wolfpack.addtimer( STRENGTH_TIME, "potions.effectstrtimer", [ char, strengthtype, bonus ], 1 )
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
