#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
import random
from wolfpack.consts import ARCHERY, LAYER_MOUNT, ANIM_ATTACKBOW, \
	ANIM_ATTACKXBOX
from combat.utilities import weaponskill
from random import choice

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
		return True

	if( arrowCount ):
		if( arrowCount > 1 ):
			arrow = wolfpack.additem( "f3f" )
			arrow.container = char.getbackpack()
			arrow.amount = arrowCount
			arrow.update()

	if( boltCount ):
		if( boltCount > 1 ):
			bolt = wolfpack.additem( "1bfb" )
			bolt.container = char.getbackpack()
			bolt.amount = boltCount
			bolt.update()

	#You gather the arrows and bolts.
	char.socket.clilocmessage(500592)

	# Reset the counters
	item.settag( "arrow_count", 0 )
	item.settag( "bolt_count", 0 )

	return 1

def checkDirection(char, item):
	if( ( item.id == 0x100b and ( char.pos.x != item.pos.x or char.pos.y <= item.pos.y ) ) or ( item.id == 0x100a and ( char.pos.y != item.pos.y or char.pos.x <= item.pos.x ) ) ):
		return False
	else:
		return True

def onUse( char, item ):
	# Init the butte
	if( not item.hastag( "arrow_count" ) or not item.hastag( "bolt_count" ) ):
		item.settag( "arrow_count", 0 )
		item.settag( "bolt_count", 0 )

	# Retrieve Arrows + Bolts when standing near
	# to the butte
	if( char.distanceto( item ) <= 1 ):
		return giveAmmo( char, item )

	if( char.distanceto( item ) > 10):
		#You are too far away from the archery butte to get an accurate shot.
		char.socket.clilocmessage(500598)
		return True
	elif( char.distanceto( item ) < 5 ):
		#You are too close to the target.
		char.socket.clilocmessage(500599)
		return True

	#riding?
	if( char.itemonlayer( LAYER_MOUNT ) ):
		#You can't practice on this while on a mount.
		char.socket.clilocmessage(501829)
		return True

	# Sanity checks for the line-of-fire
	if not checkDirection(char, item):
		#You aren't properly lined up with the archery butte to get an accurate shot.
		char.socket.clilocmessage(500597)
		return True

	#Line of Sight Check
	if not( char.canreach( item, 10 ) ):
		#You cannot reach ..
		char.socket.clilocmessage( 500312 )
		return True

	# Calculates the direction we'll have to look
	# to focus the dummy
	char.turnto( item )

	# Only Bows or Crossbows (Check ammo-type too)
	ammo = ammoType( char )

	if( ( weaponskill(char, char.getweapon()) != ARCHERY ) or ( ammo == -1 ) ):
		#You must practice with ranged weapons on this.
		char.socket.clilocmessage(500593)
		return True

	# If we've already learned all we can > cancel.
	if( char.skill[ ARCHERY ] >= 300 ):
		#Your skill cannot improve any further by simply practicing with a dummy.
		char.socket.clilocmessage(501828)
		return True

	if( ammo == 0xf3f ):
		ammoname = "arrow"
		movingeff = 0xf42
		if( not char.useresource( 1, ammo ) ):
			#You do not have any arrows with which to practice.
			char.socket.clilocmessage(500594)
			return True
	else:
		ammoname = "bolt"
		movingeff = 0x1bfe
		# Use ammo (if 0 was used = no ammo)
		if( not char.useresource( 1, ammo ) ):
			#You do not have any crossbow bolts with which to practice.
			char.socket.clilocmessage(500595)
			return True

	#reveal the char if an arrow is fired
	char.reveal()

	char.movingeffect( movingeff, item, 1, 1, 1, 0, 1 )

	# Display the char-action
	# (archery shot)
	# Soundeffect
	char.soundeffect( random.choice( [0x224, 0x234] ) )
	if( ammo == 0xf3f ):
		char.action( ANIM_ATTACKBOW )
	else:
		char.action( ANIM_ATTACKXBOX )

	# This increases the users skill
	# 10% of destroying the ammo on failure
	if( ( not char.checkskill( ARCHERY, 0, 1000 ) ) ):
		if( not random.randrange( 0, 9 ) ):
			char.emote( "You see " + char.name + "'s poor shot destroys the " + ammoname )
		else:
			char.emote( "You see " + char.name + "'s " + random.choice( failureText ) )
	else:
		char.emote( "You see " + char.name + "'s " + random.choice( successText ) )

		# Increase the ammo we have in the butte
		if( ammo == 0xf3f ):
			item.settag( "arrow_count", item.gettag( "arrow_count" ) + 1 )
		else:
			item.settag( "bolt_count", item.gettag( "bolt_count" ) + 1 )

	return True

def ammoType( char ):
	# Bows & Crossbows are on layer 1
	# Yumi, Composite Bow and Repeating Crossbow on layer 2
	item = char.getweapon()

	if( item == None ):
		return -1
	# Bow
	if( item.id == 0x13B1 or item.id == 0x13B2 ):
		return 0xf3f

	# Composite Bow
	if( item.id == 0x26cc or item.id == 0x26c2 ):
		return 0xf3f

	# Light Crossbow
	if( item.id == 0xF4F or item.id == 0xF50 ):
		return 0x1bfb

	# Heavy Crossbow
	if( item.id == 0x13FC or item.id == 0x13FD ):
		return 0x1bfb

	# Repeating Crossbow
	if( item.id == 0x26c3 or item.id == 0x26cd ):
		return 0x1bfb

	# Yumi
	if( item.id == 0x27a5 or item.id == 0x27f0 ):
		return 0xf3f

	# Unknown Weapon
	return -1
