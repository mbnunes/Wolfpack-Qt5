
import wolfpack
from wolfpack import tr
from wolfpack.consts import *
from wolfpack.utilities import *
from potions.consts import *
from potions.utilities import *
from potionkeg import isPotionkeg

explosions = [ 0x36b0, 0x36bd, 0x36cb ]
explodables = [ 'potion_greaterexplosion', 'potion_explosion', 'potion_lesserexplosion', 'f0d', 'potion_keg' ]

# Explosion Potion Function
def potion( cserial, iserial, clicked=False, counter=4, bonus=0 ):
	item = wolfpack.finditem( iserial )
	amount = item.amount
	if not item:
		return False

	if not item.baseid in explodables:
		return False

	if not clicked:
		if not item.hastag( 'exploding' ):
			item.settag( 'exploding', cserial )
		potion( cserial, iserial, True, counter, bonus )
		return True
	# Triggered
	if clicked:
		if counter > 0:
			item.addtimer( 1000, "potions.explosion.potioncountdown", [cserial, counter, bonus] )
		else:
			item.soundeffect( 0x307 ) # Boom!
			item.effect( explosions[randint( 0, 2 )], 20, 10 )
			potionregion( cserial, iserial, bonus )
			item.delete()
		return True

# Explosion Potion Function
def potioncountdown( item, args ):
	cserial = args[0]
	counter = args[1]
	bonus = args[2]
	if item and item.hastag( 'exploding' ):
		if counter >= 0:
			if counter > 0:
				if ( counter - 1 ) > 0:
					item.say( unicode( counter - 1 ) )
				counter -= 1
			potion( cserial, item.serial, True, counter, bonus )
	else:
		potion( cserial, item.serial, False, counter, bonus )
	return

# Explosion Potion Function
def potionregion( cserial, iserial, bonus=0 ):
	char = wolfpack.findchar( cserial )
	item = wolfpack.finditem( iserial )
	if not item or not char:
		return False
	ppos = item.pos
	# Defaults
	outradius = 0
	potiontype = 11
	kegfill = 0
	iskeg = False

	potiontype = getPotionType( item )

	if not potiontype in [ 11, 12, 13 ]:
		potiontype = 11

	if potiontype == 11:
		outradius = 1
	elif potiontype == 12:
		outradius = 2
	elif potiontype == 13:
		outradius = randint( 2, 3 )
	else:
		outradius = 1

	# Potion Keg Radius Override!
	if isPotionkeg( item ):
		kegfill = int( item.gettag( 'kegfill' ) )
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
	if not item.container:
		x1 = min( int( ppos.x - outradius ), int( ppos.x + outradius ) )
		x2 = max( int( ppos.x - outradius ), int( ppos.x + outradius ) )
		y1 = min( int( ppos.y - outradius ), int( ppos.y + outradius ) )
		y2 = max( int( ppos.y - outradius ), int( ppos.y + outradius ) )
		# Character Bombing
		damageregion = wolfpack.charregion( x1, y1, x2, y2, ppos.map )
		target = damageregion.first
		while target:
			if not target.ischar:
				target = damageregion.next
			if checkLoS( target, item, outradius ):
				potiondamage( cserial, target, iserial, bonus, potiontype )
				target = damageregion.next
			else:
				target = damageregion.next

		# Chain Reaction Bombing
		chainregion = wolfpack.itemregion( x1, y1, x2, y2, ppos.map )
		chainbomb = chainregion.first
		# Scan the region, build a list of explosives
		while chainbomb:
			chainpotiontimer( cserial, iserial, chainbomb.serial, outradius )
			chainbomb = chainregion.next
		return
	# Potion is in a container
	else:
		x1 = min( int(char.pos.x - outradius), int(char.pos.x + outradius) )
		x2 = max( int(char.pos.x - outradius), int(char.pos.x + outradius) )
		y1 = min( int(char.pos.y - outradius), int(char.pos.y + outradius) )
		y2 = max( int(char.pos.y - outradius), int(char.pos.y + outradius) )
		# Area Bombing
		damageregion = wolfpack.charregion( x1, y1, x2, y2, char.pos.map )
		target = damageregion.first
		while target:
			if not target.ischar:
				target = damageregion.next
			if checkLoS( char, target, outradius ):
				potiondamage( cserial, target, iserial, bonus, potiontype )
				target = damageregion.next
			else:
				target = damageregion.next

		potion_chainlist = []
		# Chain Reaction Bombing
		chainregion = wolfpack.itemregion( x1, y1, x2, y2, char.pos.map )
		chainbomb = chainregion.first
		# Scan the region, build a list of explosives
		while chainbomb:
			chainpotiontimer( cserial, iserial, chainbomb.serial, outradius )
			chainbomb = chainregion.next
		return True

def chainpotiontimer( cserial, iserial, bserial, outradius ):
	char = wolfpack.findchar( cserial )
	item = wolfpack.finditem( iserial )
	bomb = wolfpack.finditem( bserial )

	if not item or not bomb:
		return False

	if bomb.hastag( 'exploding' ):
		return False

	# Doing error checks first, makes it faster
	if not checkLoS( item, bomb, outradius ):
		return False

	potiontype = getPotionType( bomb )

	if not potiontype in [11, 12, 13]:
		return False

	bomb.settag( 'exploding', cserial )

	if isPotionkeg( bomb ) and int( bomb.gettag( 'kegfill' ) ) >= 1:
		bomb.addtimer( randint( 1000, 2250 ), "potions.explosion.potioncountdown", [ char.serial, 10, int( bomb.gettag( 'kegfill' ) ) ] )
	else:
		bomb.addtimer( randint( 1000, 2250 ), "potions.explosion.potioncountdown", [ char.serial, 0, bomb.amount ] )
	return True

# Explosion Potion Function
def potiondamage( cserial, target, iserial, dmgbonus, potiontype ):
	char = wolfpack.findchar( cserial )
	item = wolfpack.finditem( iserial )

	if not item or not char:
		return False

	# Damage Range
	if potiontype == 11:
		damage = randint( POTION_LESSEREXPLOSION_RANGE[0], POTION_LESSEREXPLOSION_RANGE[1] )
	elif potiontype == 12:
		damage = randint( POTION_EXPLOSION_RANGE[0], POTION_EXPLOSION_RANGE[1] )
	elif potiontype == 13:
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
	if item.amount > 1:
		damage = damage * item.amount
	if dmgbonus > 1:
		damage *= dmgbonus
	damage += bonus

	if not item.container:
		if char.distanceto( item ) > 1:
			damage = ( damage / char.distanceto( item ) )
	# Flamestrike effect
	if damage >= ( target.maxhitpoints / 2 ):
		target.effect(0x3709, 10, 30)
	target.effect( explosions[randint(0,2)], 20, 10)
	energydamage(target, char, damage, fire=100 )
	return
