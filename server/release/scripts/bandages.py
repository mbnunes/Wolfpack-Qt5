
import wolfpack
from wolfpack import additem
from wolfpack.utilities import tobackpack
from wolfpack.consts import *
import random

def onUse( char, item ):
	# Bandages have to be in backpack
	if item.getoutmostchar() != char:
		char.message( 'You have to have these bandages in your belongings in order to use them.' )
		return 1
		
	# Already Bandaging ??
	if char.socket.hastag( 'using_bandages' ):
		char.message( 'You are already using bandages.' )
		return 1
		
	# Display Target
	if item.id == 0xe21 or item.id == 0xee9:
		char.message( 'What do you want to use these bandages on?' )
		char.socket.attachtarget( 'bandages.bandage_response', [ item.serial ]  )
		
	elif item.id == 0xe20 or item.id == 0xe22:
		char.message( 'Where do you want to wash these bandages?' )
		char.socket.attachtarget( 'bandages.wash_response', [ item.serial ] )
		
	return 1
		
def wash_response( char, args, target ):
	bandages = wolfpack.finditem( args[0] )
	
	if not bandages:
		return
		
	# Did we target something wet ?
	id = 0
	
	if target.item:
		id = target.item.id
	else:
		id = target.model
		
	tiledata = wolfpack.tiledata( id )

	if not tiledata[ 'flag1' ] & 0x80:
		char.message( 'You cannot wash your bandages here.' )
		return
		
	char.message( 'You wash your bandages and put the clean bandages into your backpack.' )
		
	if bandages.id == 0xe20:
		bandages.id = 0xe21
	elif bandages.id == 0xe22:
		bandages.id = 0xee9
	
	bandages.update()
		
def validCorpseTarget( char, target ):
	if not target:
		return 0

	if not char.canreach( target, 2 ):
		char.message( 'You can''t reach that.' )
		return 0
				
	if target.id != 0x2006:
		char.message( 'You have to target a living thing.' )
		return 0
		
	# Check Owner
	owner = wolfpack.findchar( target.owner )

	if not owner or not owner.dead:
		char.message( 'You can''t help them anymore.' )
		return 0
		
	return 1
		
def validCharTarget( char, target ):
	# Do we have a valid target? Corpse Healing not done yet.
	if not target:
		char.message( 'You have to target a living thing.' )
		return 0
		
	if not char.canreach( target, 2 ):
		char.message( 'You are too far away to apply bandages on %s' % target.name )
		return 0
	
	# No resurrection feature in yet
	if target.dead:
		char.message( 'You can''t heal a ghost.' )
		return 0
	
	# Already at full health	
	if target.health >= target.strength:
		if target.char == char:
			char.message( 'You are healthy.' )
		else:
			char.message( '%s does not require you to heal or cure them!' % target.name )
		return 0
	
	return 1
		
def bandage_response( char, args, target ):
	corpse = None
	
	if target.item and validCorpseTarget( char, target ):
		corpse = target.item
	elif not validCharTarget( char, target.char ):
		return
		
	if corpse and ( char.skill[ HEALING ] < 800 or char.skill[ ANATOMY ] < 800 ):
		char.message( 'You are not skilled enough to resurrect.' )
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
	if not corpse:
		success = char.checkskill( HEALING, 0, 800 )
	else:
		reschance = int( ( char.skill[ HEALING ] + char.skill[ ANATOMY ] ) * 0.17 )
		rescheck = random.randint( 1, 100 )

		if char.checkskill( HEALING, 800, 1000 ) and char.checkskill( ANATOMY, 800, 1000 ) and reschance < rescheck:
			success = 1

	char.action( 0x09 )
	
	if char == target.char:
		char.message( 'You start applying bandages on yourself' )
	else:
		char.message( 'You start applying bandages on %s' % target.char.name )
		char.turnto( target.char )
		
	if corpse:
		char.addtimer( random.randint( 2500, 5000 ), 'bandages.bandage_timer', [ 1, success, target.item.serial, baseid ] ) # It takes 5 seconds to bandage
	else:
		char.addtimer( random.randint( 1500, 2500 ), 'bandages.bandage_timer', [ 0, success, target.char.serial, baseid ] ) # It takes 5 seconds to bandage
		
	char.socket.settag( 'using_bandages', 1 )
	
def bandage_timer( char, args ):
	char.socket.deltag( 'using_bandages' )

	resurrect = args[0]
	success = args[1]
	target = wolfpack.findchar( args[2] )
	baseid = args[3]

	if not target:
		return

	if not success:
		if resurrect:
			char.message( 'You fail to resurrect the target.' )
		else:	
			if target != char:
				char.message( 'You fail applying bandages to %s.' % target.name )
			else:
				char.message( 'You fail applying bandages to yourself.' )
		return
	else:
		# Create bloody bandages	
		if baseid == 0xe21:
			item = additem( 'e20' )
			if not tobackpack( item, char ):
				item.update()
				
		elif baseid == 0xee9:
			item = additem( 'e22' )
			if not tobackpack( item, char ):
				item.update()

	if resurrecting:
		corpse_owner = wolfpack.findchar( target.owner )
		
		if corpse_owner:
			corpse_owner.moveto( target.pos )
			corpse_owner.update()
			corpse_owner.resurrect()
			char.message( 'You successfully resurrect ' + corpse_owner.name )
	
	else:					
		if not validCharTarget( char, target ):
			return
		
		# Human target ?
		is target.id == 0x190 or target.id == 0x191:
			firstskill = HEALING
			secondskill = ANATOMY:
		else:
			firstskill = VETERINARY
			secondskill = ANIMALLORE
			
		# Heal a bit		
		# I'd say between 5 and 15 points. This can be adjusted later
		healmin = int( char.skill[ firstskill ] / 5 ) + int( char.skill[ secondskill ] / 5 ) + 3
		healmax = int( char.skill[ firstskill ] / 5 ) + int( char.skill[ secondskill ] / 2 ) + 10	
		
		amount = random.randint( healmin, healmax )
	
		target.health = min( target.maxhitpoints, target.health + amount )
		target.updatehealth()
		
		if char == target:
			char.message( 'You successfully apply bandages on yourself.' )
		else:
			char.message( 'You successfully apply bandages on %s' % target.name )
