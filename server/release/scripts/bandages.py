
import wolfpack
import random
import system.poison
import wolfpack.utilities
from wolfpack.consts import HEALING, ANATOMY, VETERINARY, ANIMALLORE


def onUse( char, item ):
	# Bandages have to be in backpack
	if item.getoutmostchar() != char:
		char.socket.clilocmessage(500294)
		# you cannot use that
		return 1

	# Already Bandaging ??
	if char.socket.hastag( 'using_bandages' ):
		char.socket.sysmessage( 'You are already using bandages.' )
		return 1

	# Display Target
	if item.id == 0xe21 or item.id == 0xee9:
		char.socket.clilocmessage(500948)
		#who will you use on
		char.socket.attachtarget('bandages.bandage_response', [item.serial])

	elif item.id == 0xe20 or item.id == 0xe22:
		char.socket.sysmessage( 'Where do you want to wash these bandages?' )
		char.socket.attachtarget( 'bandages.wash_response', [ item.serial ] )

	return 1

def wash_response( char, args, target ):
	bandages = wolfpack.finditem( args[0] )

	if not bandages:
		return

	if (target.item and target.item.getoutmostchar() and target.item.getoutmostchar() != char) or not char.canreach(target.pos, 5):
		char.socket.clilocmessage(500312)
		return

	# Did we target something wet?
	id = 0

	if target.item:
		id = target.item.id
	else:
		id = target.model

	# Wash Basins are allowed.
	if not target.item or target.item.id != 0x1008:
		tiledata = wolfpack.tiledata(id)
		if not tiledata[ 'flag1' ] & 0x80:
			char.socket.sysmessage('You cannot wash your bandages here.')
			return

	char.socket.sysmessage('You wash your bandages and put the clean bandages into your backpack.')

	if bandages.id == 0xe20:
		bandages.id = 0xe21
		bandages.baseid = 'e21'
	elif bandages.id == 0xe22:
		bandages.id = 0xee9
		bandages.baseid = 'ee9'

	bandages.update()

def validCorpseTarget( char, target ):
	if not target:
		return 0

	if not char.gm and not char.canreach( target, 2 ):
		char.socket.clilocmessage(500312)
		# cannot reach
		return 0

	if target.id != 0x2006:
		char.socket.clilocmessage(500970)
		#bandages cannot be used on that
		return 0

	# Check Owner
	if not target.owner or not target.owner.dead:
		char.socket.clilocmessage(500970)
		#bandages cannot be used on that
		return 0

	return 1

def validCharTarget( char, target ):
	# Do we have a valid target
	if not target:
		char.socket.sysmessage( 'You have to target a living thing.' )
		return 0

	if not char.canreach( target, 2 ):
		char.socket.sysmessage( 'You are too far away to apply bandages on %s' % target.name )
		return 0

	# Already at full health
	if not target.poison > -1 and target.health >= target.maxhitpoints:
		if target == char:
			char.socket.clilocmessage(1061288)
			#You do not require healing.
		else:
			char.socket.clilocmessage(500955)
			#that being is not damaged
		return 0

	return 1

def bandage_response( char, args, target ):
	# char is healer
	# target is pointer to healing target

	corpse = None

	if target.item:
		if validCorpseTarget( char, target.item ):
			corpse = target.item
		else:
			return

	elif target.char and not validCharTarget( char, target.char ):
		return

	elif not target.char:
		char.socket.sysmessage( 'You have to target either a corpse or a creature.' )
		return

	if corpse and ( char.skill[ HEALING ] < 800 or char.skill[ ANATOMY ] < 800 ):
		char.socket.sysmessage("You are not skilled enough to heal the dead")
		return

	if target.char and target.char.dead and ( char.skill[ HEALING ] < 800 or char.skill[ ANATOMY ] < 800 ):
		char.socket.sysmessage("You are not skilled enough to heal the dead.")
		return

	if target.char and target.char.poison > -1 and ( char.skill[ HEALING ] < 600 or char.skill[ ANATOMY ] < 600 ):
		char.socket.sysmessage("You are not skilled enough to cure poisons.")
		return

	# Consume Bandages
	bandages = wolfpack.finditem( args[0] )

	if not bandages:
		return

	baseid = bandages.id
	container = bandages.container

	if bandages.amount == 1:
		bandages.delete()
	else:
		bandages.amount -= 1
		bandages.update()

	success = 0

	# SkillCheck (0% to 80%)
	if not corpse and not target.char.dead and not target.char.poison > -1:
		success = char.checkskill( HEALING, 0, 800 )
	elif corpse or target.char.dead:
		reschance = int( ( char.skill[ HEALING ] + char.skill[ ANATOMY ] ) * 0.17 )
		rescheck = random.randint( 1, 100 )
		if char.checkskill( HEALING, 800, 1000 ) and char.checkskill( ANATOMY, 800, 1000 ) and reschance > rescheck:
			success = 1
	else: # must be poisoned
		reschance = int( ( char.skill[ HEALING ] + char.skill[ ANATOMY ] ) * 0.27 )
		rescheck = random.randint( 1, 100 )
		if char.checkskill( HEALING, 600, 1000 ) and char.checkskill( ANATOMY, 600, 1000 ) and reschance > rescheck:
			success = 1


	#this is non osi, but cool!
	#char.action( 0x09 )

	if corpse:
		char.addtimer( random.randint( 2500, 5000 ), 'bandages.bandage_timer', [ 1, success, target.item.serial, baseid ] ) # It takes 5 seconds to bandage
	else:
		if target.char.dead:
			target.char.socket.sysmessage( char.name + ' begins applying a bandage to you.' )
			char.addtimer( random.randint( 5000, 10000 ), 'bandages.bandage_timer', [ 2, success, target.char.serial, baseid ] ) # It takes 5 seconds to bandage
		elif char == target.char:
			char.socket.sysmessage( 'You start applying bandages on yourself' )
			if target.char.poison > -1:
				char.addtimer( random.randint( 4000, 7000 ), 'bandages.bandage_timer', [ 0, success, target.char.serial, baseid ] ) # It takes 5 seconds to bandage
			else:
				char.addtimer( random.randint( 3000, 6000 ), 'bandages.bandage_timer', [ 0, success, target.char.serial, baseid ] ) # It takes 5 seconds to bandage
		else:
			char.socket.sysmessage( 'You start applying bandages on %s' % target.char.name )
			if target.char.player:
				target.char.socket.sysmessage( char.name + ' begins applying a bandage to you.' )
			char.turnto( target.char )
			if target.char.poison > -1:
				char.addtimer( random.randint( 3000, 4000 ), 'bandages.bandage_timer', [ 0, success, target.char.serial, baseid ] ) # It takes 5 seconds to bandage
			else:
				char.addtimer( random.randint( 1500, 3000 ), 'bandages.bandage_timer', [ 0, success, target.char.serial, baseid ] ) # It takes 5 seconds to bandage

	char.socket.settag( 'using_bandages', 1 )

def bandage_timer( char, args ):
	char.socket.deltag( 'using_bandages' )

	resurrect = args[0]
	success = args[1]
	baseid = args[3]


	if resurrect == 1:
		# Corpse Target		
		target = wolfpack.finditem( args[2] )

		owner = target.owner

		if not validCorpseTarget( char, target ):
			return

		if not success:
			char.socket.clilocmessage(500966)
			return

		if target.owner:
			target.owner.moveto( target.pos )
			target.owner.update()
			target.owner.resurrect( char )

			# Move all the belongings from the corpse to the character
			backpack = target.owner.getbackpack()

			for item in target.content:
				# Random Position (for now, maybe storing the original position in a tag would be good)
				# Handle Weight but no Auto Stacking
				backpack.wolfpack.additem( item, 1, 1, 0 )
				item.update()

			target.delete()

			char.socket.clilocmessage(500965)
		else:
			char.socket.sysmessage( 'You can''t help them anymore' )

	# Character Target
	else:
		target = wolfpack.findchar( args[2] )


		if not validCharTarget( char, target ):
			return
		
		if target.dead:
			if not success:
				char.socket.clilocmessage(500966)
				return

			target.resurrect( char )
			target.update()

			char.socket.clilocmessage(500965)
		elif target.poison > -1:
			if not success:
				#char.socket.sysmessage( 'You fail to cure the target.' )
				char.socket.clilocmessage(1010060)
				return

			system.poison.cure(target)

			if target <> char:
				target.socket.clilocmessage(1010059)
				target.soundeffect( 0x57,0 )
			char.socket.clilocmessage(1010058)
			char.soundeffect( 0x57,0 )
		else:
			if not success:
				char.socket.clilocmessage(500968)
				return

			# Human target ? (players always human)
			if target.player or target.id == 0x190 or target.id == 0x191:
				firstskill = HEALING
				secondskill = ANATOMY
			else:
				firstskill = VETERINARY
				secondskill = ANIMALLORE

			# Heal a bit
			healmin = int( char.skill[ firstskill ] / 50.0 ) + int( char.skill[ secondskill ] / 50.0 ) + 3
			healmax = int( char.skill[ firstskill ] / 50.0 ) + int( char.skill[ secondskill ] / 20.0 ) + 10

			amount = random.randint( healmin, healmax )

			target.health = min( target.maxhitpoints, target.health + amount )
			target.updatehealth()
			char.soundeffect( 0x57 )
			if target <> char:
				target.soundeffect( 0x57 )
			char.socket.clilocmessage(500969)

	# Create bloody bandages
	# This is target independent
	#if baseid == 0xe21:
	#	item = wolfpack.additem( 'e20' )
	#	if not wolfpack.utilities.tobackpack( item, char ):
	#		item.update()
	#
	#elif baseid == 0xee9:
	#	item = wolfpack.additem( 'e22' )
	#	if not wolfpack.utilities.tobackpack( item, char ):
	#		item.update()
