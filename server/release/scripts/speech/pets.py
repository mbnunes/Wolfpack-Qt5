#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Speech Script for Pets                                        #
#===============================================================#

from wolfpack.consts import *
import wolfpack

def onSpeech( pet, char, text, keywords ):
	if not char.socket:
		return 0

	# Check if the pet is owned by char
	if ( not pet.owner or pet.owner != char ) and not char.gm:
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
			# Check if all pets should come (0x0164 = all come)
			if 0x164 in keywords:
				for pet in char.followers:
					if pet.pos.distance( char.pos ) < 16:
						# If the pet is fighting, stop fighting
						if pet.war and pet.target:
							pet.target = None
							pet.war = 0
							pet.updateflags()

						pet.following = None
						pet.goto( char.pos )
						pet.sound( SND_ATTACK )
			else:
				# If the pet is fighting, stop fighting
				if pet.war and pet.target:
					pet.target = None
					pet.war = 0
					pet.updateflags()

				pet.following = None

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
						if pet.war:
							pet.war = 0
							pet.updateflags()
						pet.npcwander = 0
						pet.sound( SND_ATTACK )
			else:
				pet.target = None
				pet.following = None
				pet.guarding = None
				if pet.war:
					pet.war = 0
					pet.updateflags()
				pet.npcwander = 0
				pet.sound( SND_ATTACK )

		# "follow me"
		elif keyword == 0x163:
			# Check for all follow me
			if 0x16C in keywords:
				for pet in char.followers:
					if pet.war and pet.target:
						pet.target = None
						pet.war = 0
						pet.updateflags()

					pet.follow( char )
					pet.sound( SND_ATTACK )
			else:
				if pet.war and pet.target:
					pet.target = None
					pet.war = 0
					pet.updateflags()

				pet.follow( char )
				pet.sound( SND_ATTACK )

		# "follow"
		elif keyword == 0x15a:
			char.socket.sysmessage( "Who do you want your pet to follow?" )
			
			# all follow?
			if 0x165 in keywords:
				char.socket.attachtarget( "speech.pets.follow_target", [ "all" ] )
			else:
				char.socket.attachtarget( "speech.pets.follow_target", [ "single", pet.serial ] )
	
		# "release"
		elif keyword == 0x16d:
			pet.tamed = 0
			pet.owner = None
			pet.sound( SND_ATTACK )

	return 1

# Target for selecting a follow target
def follow_target( char, args, target ):
	if not target.char:
		char.socket.sysmessage( "You have to target a character." )
		return

	# Check who should guard
	if args[0] == "single":
		pet = wolfpack.findchar( args[1] )
		if pet:
			if pet.war and pet.target:
				pet.target = None
				pet.war = 0
				pet.updateflags()

			pet.follow( char )
			pet.sound( SND_ATTACK )

	elif args[0] == "all":
		# Let all followers in sight guard the target
		for pet in char.followers:
			if( pet.distanceto( char ) < 16 ):
				if pet.war and pet.target:
					pet.target = None
					pet.war = 0
					pet.updateflags()

				pet.follow( char )
				pet.sound( SND_ATTACK )

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
