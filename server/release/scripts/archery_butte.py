#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
from wolfpack.consts import *
from random import choice, randrange

# failure texts
failureText = ( 
        "shot is well off target.",
        "shot is wide of the mark.",
        "shot misses terribly.",
        "shot nearly misses the archery butte."
)

# success texts
successText = (
        "hit the outer ring.",
        "hit the middle ring.",
        "hit the inner ring.",
        "hit the bullseye!."
)

def giveAmmo( char, item ):
	# morex: Arrow count
	# morey: Bolt count
	boltCount = item.gettag( "bolt_count" )
	arrowCount = item.gettag( "arrow_count" )
	
	if( not boltCount and not arrowCount ):
		char.message( "The butte is empty." )
		return 1

	if( arrowCount ):
		if( arrowCount > 1 ):
			message = "You retrieve " + str( arrowCount ) + " arrows."
		else:
			message = "You retrieve an arrow."
	
		char.message( arrowCount )
		arrow = wolfpack.additem( "f3f" )
		arrow.container = char.getbackpack()
		arrow.amount = arrowCount
		arrow.update()

	if( boltCount ):
		if( boltCount > 1 ):
			message = "You retrieve " + str( boltCount ) + " bolts."
		else:
			message = "You retrieve a bolt."
		
		char.message( message )			
		bolt = wolfpack.additem( "1bfb" )
		bolt.container = char.getbackpack()
		bolt.amount = boltCount
		bolt.update()		

	# Reset the counters
	item.settag( "arrow_count", 0 )
	item.settag( "bolt_count", 0 )
	
	return 1

def onUse( char, item ):
	# Init the butte
	if( not item.hastag( "arrow_count" ) or not item.hastag( "bolt_count" ) ):
		item.settag( "arrow_count", 0 )
		item.settag( "bolt_count", 0 )
	
	# Retrieve Arrows + Bolts when standing near
	# to the butte
	if( char.distanceto( item ) <= 1 ):
		return giveAmmo( char, item )

	if( char.distanceto( item ) > 10 or char.distanceto( item ) < 5 ):
		char.message( "You are either too near or far to shoot." )
		return 1
	
	# Sanity checks for the line-of-fire
	if( ( item.id == 0x100b and ( char.pos.x != item.pos.x or char.pos.y <= item.pos.y ) ) or ( item.id == 0x100a and ( char.pos.y != item.pos.y or char.pos.x <= item.pos.x ) ) ):
		char.message( "You can't shoot from here." )
		return 1
	
	# TODO: Check line of sight
	
	# Calculates the direction we'll have to look
	# to focus the dummy
	direction = char.directionto( item )

	if( char.direction != direction ):
		char.direction = direction
		char.update()

	# Only Bows or Crossbows (Check ammo-type too)
	ammo = ammoType( char )
	
	if( ( char.combatskill() != ARCHERY ) or ( ammo == -1 ) ):
		char.message( "You only can use crossbows and bows on this butte." )
		return 1
	
	# If we've already learned all we can > cancel.
	if( char.skill[ ARCHERY ] >= 300 ):
		char.message( "You can learn much from a dummy but you have already learned it all." )
		return 1

	# Use ammo (if 0 was used = no ammo)
	if( not char.useresource( 1, ammo ) ):
		char.message( "You are out of ammunition." )
		return 1

	# Display the char-action
	# (archery shot)
	# Soundeffect
	char.soundeffect( 0x224 )
	char.action( 0x09 )
	
	if( ammo == 0xf3f ):
		ammoname = "arrow"
		movingeff = 0xf42
	else:
		ammoname = "bolt"
		movingeff = 0x1bfe


	char.movingeffect( movingeff, item, 1, 1, 1, 0, 1 )
	
	
	# This increases the users skill
	# 10% of destroying the ammo on failure
	if( ( not char.checkskill( ARCHERY, 0, 1000 ) ) ):
		if( not randrange( 0, 9 ) ):
			char.emote( "You see " + char.name + "'s poor shot destroys the " + ammoname )
		else:
			char.emote( "You see " + char.name + "'s " + choice( failureText ) )
	else:
		char.emote( "You see " + char.name + "'s " + choice( successText ) )
		
		# Increase the ammo we have in the butte
		if( ammo == 0xf3f ):
			item.settag( "arrow_count", item.gettag( "arrow_count" ) + 1 )
		else:
			item.settag( "bolt_count", item.gettag( "bolt_count" ) + 1 )

	return 1

def ammoType( char ):
	# Bows & Crossbows are on layer 1
	item = char.itemonlayer( 1 )
	
	if( item == None ):
		return -1

	# Bow
	if( item.id == 0x13B1 or item.id == 0x13B2 ):
		return 0xf3f
		
	# Light Crossbow
	if( item.id == 0xF4F or item.id == 0xF50 ):
		return 0x1bfb
		
	# Heavy Crossbow
	if( item.id == 0x13FC or item.id == 0x13FD ):
		return 0x1bfb
	
	# Unknown Weapon
	return -1
