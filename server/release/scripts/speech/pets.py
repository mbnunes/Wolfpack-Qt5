#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Speech Script for Pets                                        #
#===============================================================#

from wolfpack.speech import addKeyword,setCatchAll
from wolfpack.consts import *
import wolfpack

def onLoad():
	setCatchAll( 'speech_pets', 0 ) # We don't want to catch all speech but only the pet commands

	# Register some keywords
	addKeyword( 'speech_pets', 0x155 ) # "*come"
	addKeyword( 'speech_pets', 0x156 ) # "*drop"
	#addKeyword( 'speech_pets', 0x157 ) # "*fetch"
	#addKeyword( 'speech_pets', 0x158 ) # "*get"
	#addKeyword( 'speech_pets', 0x159 ) # "*bring"
	addKeyword( 'speech_pets', 0x15A ) # "*follow"
	addKeyword( 'speech_pets', 0x15B ) # "*friend"
	addKeyword( 'speech_pets', 0x15C ) # "*guard"
	addKeyword( 'speech_pets', 0x15D ) # "*kill"
	addKeyword( 'speech_pets', 0x15E ) # "*attack"
	#addKeyword( 'speech_pets', 0x15F ) # "*patrol"
	#addKeyword( 'speech_pets', 0x160 ) # "*report"
	addKeyword( 'speech_pets', 0x161 ) # "*stop"
	#addKeyword( 'speech_pets', 0x162 ) # "*hire"
	addKeyword( 'speech_pets', 0x163 ) # "*follow me"
	addKeyword( 'speech_pets', 0x164 ) # "all come"
	addKeyword( 'speech_pets', 0x165 ) # "all follow"
	addKeyword( 'speech_pets', 0x166 ) # "all guard"
	addKeyword( 'speech_pets', 0x167 ) # "all stop"
	addKeyword( 'speech_pets', 0x168 ) # "all kill"
	addKeyword( 'speech_pets', 0x169 ) # "all attack"
	#addKeyword( 'speech_pets', 0x16A ) # "all report"
	addKeyword( 'speech_pets', 0x16B ) # "all guard me"
	addKeyword( 'speech_pets', 0x16C ) # "all follow me"
	addKeyword( 'speech_pets', 0x16D ) # "*release"
	addKeyword( 'speech_pets', 0x16E ) # "*transfer"
	addKeyword( 'speech_pets', 0x16F ) # "*stay"
	addKeyword( 'speech_pets', 0x170 ) # "all stay"

def onSpeech( pet, char, text, keywords ):
	if not char.socket:
		return 0

	# Check for keywords
	checkname = 1

	# for all* commands we dont need to check for the name
	for keyword in keywords:
		if ( keyword >= 0x164 and keyword <= 0x16C ) or keyword == 0x170:
			checkname = 0

	# Check if we are mean't by this command
	if checkname:
		if not text.lower().startswith( pet.name.lower() + " " ):
			return 0
	
	# Check which command was issued:
	for keyword in keywords:

		# "come"
		if keyword == 0x155:
			char.socket.showspeech( pet, pet.name + " comes to your position." )

			# Check if all pets should come (0x0164 = all come)
			if 0x164 in keywords:
				for pet in char.followers:
					if pet.pos.distance( char.pos ) < 16:
						pet.goto( char.pos )
						pet.sound( SND_ATTACK )
			else:
				pet.goto( char.pos )
				pet.sound( SND_ATTACK )

			break

		# "attack"/"kill"
		elif keyword == 0x15D or keyword == 0x15E:
			char.socket.sysmessage( "Select the target to attack." )

			# 0x0168 "all kill"
			# 0x0169 "all attack"
			if ( keyword == 0x15D and 0x168 in keywords ) or ( keyword == 0x15E and 0x169 in keywords ):
				char.socket.attachtarget( "speech.pets.kill_target", [ "all" ] )
			else:
				char.socket.attachtarget( "speech.pets.kill_target", [ "single", pet.serial ] )
			break

		# "guard"
		elif keyword == 0x15C:
			char.socket.sysmessage( "Who should be guarded?" )

			# all guard?
			if 0x166 in keywords:				
				char.socket.attachtarget( "speech.pets.guard_target", [ "all" ] )
			else:
				char.socket.attachtarget( "speech.pets.guard_target", [ "single", pet.serial ] )

		# "transfer"
		elif keyword == 0x16E:
			char.socket.sysmessage( "Who do you want to transfer your pet to?" )
			char.socket.attachtarget( "speech.pets.transfer_target", [ pet.serial ] )

		# "drop"
		elif keyword == 0x156:
			char.socket.sysmessage( pet.name + " drops all his belongings to ground." )

			# This drops all items in the backpack of the pet to the ground
			backpack = pet.getbackpack()
			pos = pet.pos

			for item in backpack.content:
				item.container = None
				item.moveto( pos.x, pos.y, pos.z, pos.map )
				item.update()
			
			# Play the confirmation
			pet.sound( SND_ATTACK )

		# "stay"
		elif keyword == 0x16F:
			# Check if all followers should stay at their current position and stop fighting
			if 0x170 in keywords:
				for pet in char.followers:
					if pet.pos.distance( char.pos ) < 16:
						pet.target = None
						pet.following = None
						pet.guarding = None
						pet.war = 0
						pet.npcwander = 0
						pet.updateflags()
						pet.sound( SND_ATTACK )
			else:
				pet.target = None
				pet.following = None
				pet.guarding = None
				pet.war = 0
				pet.npcwander = 0
				pet.updateflags()
				pet.sound( SND_ATTACK )

		# "follow"
	
		# "release"

	return 1

# Target for selecting a target to transfer a pet to
def transfer_target( char, args, target ):
	if not target.char or not target.char.socket:
		char.socket.sysmessage( "You can only transfer your pets to other players." )

	pet = wolfpack.findchar( args[0] )

	if pet:
		pet.owner = target.char
		pet.sound( SND_ATTACK )

# Target for selecting a target to guard
def guard_target( char, args, target ):
	if not target.char:
		char.socket.sysmessage( "You have to target a character." )
		return

	# Check who should guard
	if args[0] == "single":
		pet = wolfpack.findchar( args[1] )
		if pet:
			pet.follow( target.char )
			pet.guarding = target.char
			pet.sound( SND_ATTACK )

	elif args[0] == "all":
		# Let all followers in sight guard the target
		for pet in char.followers:
			if( pet.distanceto( char ) < 16 ):
				pet.follow( target.char )
				pet.guarding = target.char
				pet.sound( SND_ATTACK )

# Target for selecting a target to kill
def kill_target( char, args, target ):
	if not target.char:
		char.socket.sysmessage( "You have to target a character." )
		return

	# Check who should attack
	if args[0] == "single":
		pet = wolfpack.findchar( args[1] )
		if pet:
			pet.attack( target.char )

	elif args[0] == "all":
		# Let all followers in sight attack the target
		for pet in char.followers:			
			if( pet.distanceto( char ) < 16 ):
				pet.attack( target.char )
