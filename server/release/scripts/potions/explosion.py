
import wolfpack
from wolfpack.consts import *
from wolfpack.utilities import *
from potions import *
from potions.utilities import *

# Explosion Potion Function
def targetexplosionpotion( char, args, target ):
	socket = char.socket
	potion = args[0]
	check = 10
	if not potion:
		return False
	if target.char:
		# i dont think we need some of these...
		if target.char.invulnerable:
			return False
		if target.char.dead:
			return False
		if not char.cansee(target.char):
			return False
		if char.distanceto(target.char) > check:
			return False
		if not char.canreach(target, check):
			return False
		pos = target.char.pos
	elif target.item:
		if not char.cansee(target.item):
			return False
		if char.distanceto(target.item) > check:
			return False
		if not char.canreach(target, check):
			return False
		item = target.item.getoutmostitem()
		if item.container:
			pos = item.container.pos
		else:
			pos = item.pos
	else:
		if not char.canreach(target, check):
			return False
		pos = target.pos
	# Distance Checking
	if char.distanceto( pos ) > 15:
		socket.clilocmessage( 1005539 )
		return False
	#verify the potion still exists to be thrown...
	if potion:
		if potion.amount == 1:
			throwobject( char, potion, pos, 1, 3, 5 )
		else:
			socket.sysmessage( 'Stackable potions is not yet complete.' )
			return False

	return True

# Explosion Potion Function
def potion( cserial, pserial, clicked=False, counter=4, bonus=0 ):
	amount = potion.amount
	potion = wolfpack.finditem( pserial )
	if not potion:
		return False

	if clicked == False:
		if not potion.hastag( 'exploding' ):
			potion.settag( 'exploding', cserial )
		potion( char, potion, 1, counter, bonus, exploding )
		return
	elif clicked == True:
		if counter > 0:
			potion.addtimer( 1000, "potions.potioncountdown", [cserial, counter, bonus] )
		else:
			potion.soundeffect( 0x307 ) # Boom!
			potion.effect( explosions[randint( 0, 2 )], 20, 10 )
			potionregion( cserial, pserial, bonus )
			potion.delete()
		return

# Explosion Potion Function
def potioncountdown( pserial, args ):
	cserial = args[0]
	counter = args[1]
	bonus = args[2]
	potion = wolfpack.finditem( pserial )
	if potion and potion.hastag( 'exploding' ):
		if counter >= 0:
			if counter > 0:
				if ( counter - 1 ) > 0:
					potion.say( "%u" % ( counter - 1 ) )
				counter -= 1
			potion( cserial, pserial, 1, counter, bonus )
	else:
		potion( cserial, pserial, 0, counter, bonus )
	return

# Explosion Potion Function
def potionregion( cserial, pserial, bonus=0 ):
	char = wolfpack.findchar( cserial )
	potion = wolfpack.finditem( pserial )
	if not potion or not char:
		return False
	ppos = potion.pos
	# Defaults
	outradius = 0
	potiontype = 11
	kegfill = 0
	iskeg = False

	if potion.hastag( 'potiontype' ):
		potiontype = potion.gettag( 'potiontype' )
		if not potiontype in [ 11, 12, 13 ]:
			potiontype = 11

	if potion.hastag( 'kegfill' ):
		iskeg = True
		kegfill = potion.gettag( 'kegfill' )

	if potiontype == 11:
		outradius = 1
	elif potiontype == 12:
		outradius = 2
	elif potiontype == 13:
		outradius = randint( 2, 3 )
	else:
		outradius = 1

	# Potion Keg Radius Override!
	if iskeg == True:
		if potiontype in [11, 12, 13] and kegfill >= 1:
			if kegfill == 100:
				outradius = 13
			elif kegfill >= 90:
				outradius = 12
			elif kegfill >= 80:
				outradius = 11
			elif kegfill >= 70:
				outradius = 10
			elif kegfill >= 60:
				outradius = 9
			elif kegfill >= 50:
				outradius = 8
			elif kegfill >= 40:
				outradius = 7
			elif kegfill >= 30:
				outradius = 6
			else:
				outradius = 5

	outradius = max( 1, outradius )

	# Potion is thrown on the ground
	if not potion.container:
		x1 = min( int( ppos.x - outradius ), int( ppos.x + outradius ) )
		x2 = max( int( ppos.x - outradius ), int( ppos.x + outradius ) )
		y1 = min( int( ppos.y - outradius ), int( ppos.y + outradius ) )
		y2 = max( int( ppos.y - outradius ), int( ppos.y + outradius ) )
		# Character Bombing
		damageregion = wolfpack.charregion( x1, y1, x2, y2, pos.map )
		target = damageregion.first
		while target:
			if not target.ischar:
				target = damageregion.next
			if checkLoS( target, potion, outradius ):
				potiondamage( cserial, target, pserial, bonus )
				target = damageregion.next
			else:
				target = damageregion.next

		# Chain Reaction Bombing
		chainregion = wolfpack.itemregion( x1, y1, x2, y2, potion.pos.map )
		chainbomb = chainregion.first
		# Scan the region, build a list of explosives
		while chainbomb:
			chainpotiontimer( cserial, pserial, chainbomb.serial, outradius )
			chainbomb = chainregion.next
		return
	# Potion is in a container
	else:
		x1 = min( int(char.pos.x - outradius), int(char.pos.x + outradius) )
		x2 = max( int(char.pos.x - outradius), int(char.pos.x + outradius) )
		y1 = min( int(char.pos.y - outradius), int(char.pos.y + outradius) )
		y2 = max( int(char.pos.y - outradius), int(char.pos.y + outradius) )
		# Area Bombing
		damageregion = wolfpack.charregion( x1, y1, x2, y2, potion.pos.map )
		target = damageregion.first
		while target:
			if not target.ischar:
				target = damageregion.next
			if checkLoS( char, target, outradius ):
				potiondamage( cserial, target, pserial, bonus )
				target = damageregion.next
			else:
				target = damageregion.next

		potion_chainlist = []
		# Chain Reaction Bombing
		chainregion = wolfpack.itemregion( x1, y1, x2, y2, char.pos.map )
		chainbomb = chainregion.first
		# Scan the region, build a list of explosives
		while chainbomb:
			chainpotiontimer( cserial, pserial, chainbomb.serial, outradius )
			chainbomb = chainregion.next
		return True

def chainpotiontimer( cserial, pserial, bserial, outradius ):
	potion = wolfpack.finditem( pserial )
	bomb = wolfpack.finditem( bserial )

	if not potion or not bomb:
		return False

	# Doing error checks first, makes it faster
	if not checkLoS( potion, bomb, outradius ):
		return
	if not bomb.hastag('potiontype'):
		return
	if not int( bomb.gettag('potiontype') ) in [ 11, 12, 13 ]:
		return
	if bomb.hastag( 'exploding' ):
		return

	bomb.settag( 'exploding', cserial )

	if bomb.hastag( 'kegfill' ) and int( bomb.gettag( 'kegfill' ) ) >= 1:
		bomb.addtimer( randint( 1000, 2250 ), "potions.potioncountdown", [ char.serial, 0, int( bomb.gettag( 'kegfill' ) ) ] )
	else:
		bomb.addtimer( randint( 1000, 2250 ), "potions.potioncountdown", [ char.serial, 0, bomb.amount ] )
	return

# Explosion Potion Function
def potiondamage( cserial, target, pserial, dmgbonus ):
	char = wolfpack.findchar( cserial )
	potion = wolfpack.finditem( pserial )

	if not potion or not char:
		return False

	# Damage Range
	if potion.gettag( 'potiontype' ) == 11:
		damage = randint( POTION_LESSEREXPLOSION_RANGE[0], POTION_LESSEREXPLOSION_RANGE[1] )
	elif potion.gettag( 'potiontype' ) == 12:
		damage = randint( POTION_EXPLOSION_RANGE[0], POTION_EXPLOSION_RANGE[1] )
	elif potion.gettag( 'potiontype' ) == 13:
		damage = randint( POTION_GREATEREXPLOSION_RANGE[0], POTION_GREATEREXPLOSION_RANGE[1] )
	else:
		damage = randint( POTION_LESSEREXPLOSION_RANGE[0], POTION_GREATEREXPLOSION_RANGE[1] )
	# Bonuses
	if char.skill[ALCHEMY] == 1200:
		bonus = 10
	elif char.skill[ALCHEMY] >= 1100:
		bonus = randint(8,9)
	elif char.skill[ALCHEMY] >= 1000:
		bonus = randint(6,7)
	else:
		bonus = randint(0,5)
	if potion.amount > 1:
		damage = damage * potion.amount
	if dmgbonus > 1:
		damage *= dmgbonus
	damage += bonus

	if not potion.container:
		if char.distanceto(potion) > 1:
			damage = (damage / char.distanceto(potion))
	# Flamestrike effect
	if damage >= (target.maxhitpoints / 2):
		target.effect(0x3709, 10, 30)
	target.effect( explosions[randint(0,2)], 20, 10)
	energydamage(target, char, damage, fire=100 )
	return
