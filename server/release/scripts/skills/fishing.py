#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
import wolfpack

# This is the rather complex fishing script

def onUse( char, item ):
	# Check if he used a fishing pole
	if item.id != 0xdbf and item.id != 0xdc0:
		return 0

	socket = char.socket

	# Can't fish on horses
	if char.itemonlayer( LAYER_MOUNT ):
		socket.clilocmessage( 0x7A4EB, "", 0x3b2, 3 ) # You can't fish while riding!
		return 1

	# Are we already fishing?
	if socket.hastag( 'is_fishing' ):
		socket.clilocmessage( 0x7A4EC, "", 0x3b2, 3 ) # You are already fishing.
		return 1

	# Assign the target request
	socket.clilocmessage( 0x7A4EE, "", 0x3b2, 3 ) # What water do you want to fish in?
	socket.attachtarget( "skills.fishing.response" )

	return 1

def response( char, args, target ):
	# First: Check Distance (easiest)

	# Second: Check Map/Static/Dynamic Water and eventual blocking stuff above it
	
	# Third: Show Animation/Sound + Show Effect (delayed) + Get Fish (delayed)
	char.action( 0x0c )

	# Soundeffect together with static effect (delayed 2 seconds)
	# char.soundeffect( 0x364 )
	# -- static effect, id: 0x352d, speed: 0x4, duration: 0x10

	# SkillFail: 
	# SkillSuccess:
	# Too Far away to do that: ID: 0x7A4F0 (0) / You need to be closer to the water to fish!
	# There are impassables with z >= water.z and z <= water.z + 13: 0x7A4F5 You can't reach the water there.
	# No water there: 0x7A4F2 You need water to fish in!

	pass
