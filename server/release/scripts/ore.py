
# Fear the wrath of the miners!
# ~ Dreoth

from wolfpack.consts import *
import whrandom
import wolfpack
import skills
from wolfpack.time import *
from wolfpack.utilities import *
from random import randrange, randint

# 0x19b7, 0x19b8, 0x19ba give 1 ingot.
# 0x19b9 gives 2 ingots.
oreids = [ 0x19b7, 0x19b8, 0x19ba, 0x19b9 ]
oredefsufx = [ '_ore_1', '_ore_2', '_ore_3', '_ore_4']

forgeids = [ 0xfb1, 0x197a, 0x197b, 0x197c, 0x197d, 0x197e, 0x197f, 0x1980,
				0x1981, 0x1982, 0x1983, 0x1984, 0x1985, 0x1986, 0x1987, 0x1988,
				0x1989, 0x198a, 0x198b, 0x198c, 0x198d, 0x198e, 0x198f, 0x1990,
				0x1991, 0x1992, 0x1993, 0x1994, 0x1995, 0x1996, 0x1997, 0x1998,
				0x1999, 0x199a, 0x199b, 0x199c, 0x199d, 0x199e, 0x199f, 0x19a0,
				0x19a1, 0x19a2, 0x19a3, 0x19a4, 0x19a5, 0x19a6, 0x19a7, 0x19a8,
				0x19a9 ]

# Name, reqSkill, minSkill, color, item name
ingottable = \
{
	'iron':			[ 0, -250, 0x0, 'iron ingot' ],
	'dullcopper':	[ 650, 325, 0x973, 'dull copper ingot' ],
	'shadowiron':	[ 700, 350, 0x966, 'shadow iron ingot' ],
	'copper':		[ 750, 375, 0x96d, 'copper ingot' ],
	'bronze':		[ 800, 400, 0x972, 'bronze ingot' ],
	'gold':			[ 850, 425, 0x8a5, 'gold ingot' ],
	'agapite':		[ 900, 450, 0x979, 'agapite ingot' ],
	'verite':			[ 950, 475, 0x89f, 'verite ingot' ],
	'valorite':		[ 990, 495, 0x8ab, 'valorite ingot' ]
}

REQSKILL = 0
MINSKILL = 1
COLORID = 2
NAME = 3

def onUse( char, ore ):
	if ore.id in oreids:
		if ore.getoutmostchar() != char:
			if char.pos.distance( ore.pos ) > 2:
				char.socket.clilocmessage( 501976, '', GRAY ) # You can't reach...
				return OK
			else:
				if not ore.hastag( 'resname' ):
					char.socket.clilocmessage( 501986, '', GRAY ) # Strange ore.
					return OK
				else:
					# Where do you want to smelt the ore?
					char.socket.clilocmessage( 501971, '', GRAY )
					char.socket.attachtarget( "ore.response", [ ore.serial ] )
					return OK
		else:
			if not ore.hastag( 'resname' ):
				char.socket.clilocmessage( 501986, '', GRAY )
				return OK
			else:
				# Where do you want to smelt the ore?
				char.socket.clilocmessage( 501971, '', GRAY )
				char.socket.attachtarget( "ore.response", [ ore.serial ] )
				return OK

def response( char, args, target ):
	direction = char.directionto( target.pos )
	if not char.direction == direction:
		char.direction = direction
		char.update()

	item = wolfpack.finditem( args[0] )

	if ( ( char.pos.x-target.pos.x )**2 + ( char.pos.y-target.pos.y )**2 > 4):
		char.socket.clilocmessage( 0x7A247 ) # You are too far away to do that.
		return OK

	if abs( char.pos.z - target.pos.z ) > 5:
		char.socket.clilocmessage( 0x7A247 ) # You are too far away to do that.
		return OK

	if not item.hastag( 'resname' ):
		return OOPS
	else:
		resname = item.gettag( 'resname' )

	if target.char:
		return OOPS

	targetitem = wolfpack.finditem( target.item.serial )

	# We go onto creating ingots here.
	if target.item.id in forgeids:
		if item.id in oreids:
			if char.pos.distance( target.pos ) > 3:
				char.socket.clilocmessage( 0x7A258 ) # You can't reach...
				return OK
			else:
				dosmelt( char, [ item, targetitem, resname ] )
				return OK

	# This is for merging the ore piles
	elif target.item.id in oreids:
		if targetitem.serial == item.serial:
			return OOPS
		if not targetitem.hastag('resname'):
			char.socket.clilocmessage( 501986, '', GRAY )
			return OOPS
		# Largest Ore Pile
		if item.id == oreids[3] and item.color == targetitem.color and item.gettag( 'resname' ) == targetitem.gettag( 'resname' ):
			if targetitem.getoutmostchar() != char:
				if char.pos.distance( target.pos ) > 2:
					# The ore is too far away.
					char.socket.clilocmessage( 501976, '', GRAY)
					return OK
				else:
					# Merge the ore piles
					if targetitem.id == oreids[2] or targetitem.id == oreids[1]:
						targetitem.amount += ( item.amount * 2 )
						targetitem.update()
						item.delete()
						# Select the forge on which to smelt the ore, or another pile of ore with which to combine it.
						char.socket.clilocmessage( 501971, '', GRAY )
					elif targetitem.id == oreids[0]:
						targetitem.amount += ( item.amount * 4 )
						targetitem.update()
						item.delete()
						# Select the forge on which to smelt the ore, or another pile of ore with which to combine it.
						char.socket.clilocmessage( 501971, '', GRAY )
					elif targetitem.id == item.id:
						targetitem.amount += item.amount
						targetitem.update()
						item.delete()
						# Select the forge on which to smelt the ore, or another pile of ore with which to combine it.
						char.socket.clilocmessage( 501971, '', GRAY )
					return OK
			else:
				# Merge the ore piles
				if targetitem.id == oreids[2] or targetitem.id == oreids[1]:
					targetitem.amount += ( item.amount * 2 )
					targetitem.update()
					item.delete()
					# Select the forge on which to smelt the ore, or another pile of ore with which to combine it.
					char.socket.clilocmessage( 501971, '', GRAY )
				elif targetitem.id == oreids[0]:
					targetitem.amount += ( item.amount * 4 )
					targetitem.update()
					item.delete()
					# Select the forge on which to smelt the ore, or another pile of ore with which to combine it.
					char.socket.clilocmessage( 501971, '', GRAY )
				elif targetitem.id == item.id:
					targetitem.amount += item.amount
					targetitem.update()
					item.delete()
					# Select the forge on which to smelt the ore, or another pile of ore with which to combine it.
					char.socket.clilocmessage( 501971, '', GRAY )
				return OK

		# Second Largest Ore
		elif item.id == oreids[2] and item.color == targetitem.color and item.gettag( 'resname' ) == targetitem.gettag( 'resname' ):
			if targetitem.getoutmostchar() != char:
				if char.pos.distance( target.pos ) > 2:
					# The ore is too far away.
					char.socket.clilocmessage( 501976, '', GRAY)
					return OK
				else:
					# Merge the ore piles
					if targetitem.id == oreids[2] or targetitem.id == oreids[1] or targetitem.id == oreids[0]:
						targetitem.amount += item.amount
						targetitem.update()
						item.delete()
						return OK
					elif targetitem.id == oreids[0]:
						targetitem.amount += ( item.amount * 2 )
						targetitem.update()
						item.delete()
						return OK
					elif targetitem.id == oreids[3]:
						return OOPS
					return OK
			else:
				# Merge the ore piles
				if targetitem.id == oreids[2] or targetitem.id == oreids[1]:
					targetitem.amount += item.amount
					targetitem.update()
					item.delete()
					return OK
				elif targetitem.id == oreids[0]:
					targetitem.amount += ( item.amount * 2 )
					targetitem.update()
					item.delete()
					return OK
				elif targetitem.id == oreids[3]:
					return OOPS
				return OK

		# Second Smallest
		elif item.id == oreids[1] and item.color == targetitem.color and item.gettag( 'resname' ) == targetitem.gettag( 'resname' ):
			if targetitem.getoutmostchar() != char:
				if char.pos.distance( target.pos ) > 2:
					char.socket.clilocmessage( 0x7A258 ) # You can't reach...
					return OK
				else:
					if targetitem.id == oreids[1]:
						targetitem.amount += item.amount
						targetitem.update()
						item.delete()
						return OK
					elif targetitem.id == oreids[0]:
						targetitem.amount += ( item.amount * 2 )
						targetitem.update()
						item.delete()
						return OK
					elif targetitem.id == oreids[2] or targetitem.id == oreids[3]:
						return OOPS
					return OK
			else:
				# Merge the ore piles
				if targetitem.id == oreids[1]:
					targetitem.amount += item.amount
					targetitem.update()
					item.delete()
					return OK
				elif targetitem.id == oreids[0]:
					targetitem.amount += ( item.amount * 2 )
					targetitem.update()
					item.delete()
					return OK
				elif targetitem.id == oreids[2] or targetitem.id == oreids[3]:
					return OOPS
				return OK

		# Smallest
		elif item.id == oreids[0] and item.color == targetitem.color and item.gettag( 'resname' ) == targetitem.gettag( 'resname' ):
			if targetitem.getoutmostchar() != char:
				if char.pos.distance( target.pos ) > 2:
					# The ore is too far away.
					char.socket.clilocmessage( 501976 )
					return OK
				else:
					# Merge the ore piles
					if targetitem.id == oreids[1] or targetitem.id == oreids[2] or targetitem.id == oreids[3]:
						char.socket.sysmessage( "You can not create a larger pile from a small pile of ore.", GRAY )
					elif targetitem.id == oreids[0]:
						targetitem.amount += item.amount
						targetitem.update()
						item.delete()
						char.socket.sysmessage( "You combine the two ore piles to create a single pile of ore.", GRAY )
					return OK
			else:
				# Merge the ore piles
				if targetitem.id == oreids[1] or targetitem.id == oreids[2] or targetitem.id == oreids[3]:
					char.socket.sysmessage( "You can not create a larger pile from a small pile of ore.", GRAY )
				elif targetitem.id == oreids[0]:
					targetitem.amount += item.amount
					targetitem.update()
					item.delete()
					char.socket.sysmessage( "You combine the two ore piles to create a single pile of ore.", GRAY )
				return OK

def dosmelt ( char, args ):
	ore = args[0]
	forge = args[1]
	resname = args[2]
	success = 0
	reqskill = ingottable[ resname ][ REQSKILL ]
	chance =  int( ( char.skill[ MINING ] - ingottable[ resname ][ MINSKILL ] )  / 10 )
	if chance > 100:
		chance = 100
	elif chance < 0:
		chance  = 0

	if not char.skill[ MINING ] >= reqskill:
		# You have no idea how to smelt this strange ore!
		char.socket.clilocmessage( 501986, '', GRAY )
		return OOPS

	if ore.amount >= 1 and char.skill[ MINING ] >= reqskill:
		if not skills.checkskill( char, forge, MINING, chance ):
			success = 0
			return
		elif chance >= randint(1, 100):
			if ore.id == oreids[3]:
				amount = ( ore.amount * 2 )
				successsmelt( char, ingottable, resname, amount )
				ore.delete()
			elif ore.id == oreids[2] or ore.id == oreids[1]:
				amount = ore.amount
				successsmelt( char, ingottable, resname, amount )
				ore.delete()
			elif ore.id == oreids[0]:
				if evenorodd( ore.amount ) == "even":
					amount = ( ore.amount / 2 )
					successsmelt( char, ingottable, resname, amount )
					ore.delete()
				elif evenorodd( ore.amount ) == "odd" and ore.amount > 1:
					amount = ( ( ore.amount - 1 ) / 2 )
					successsmelt( char, ingottable, resname, amount )
					ore.amount = 1
					ore.update()
				elif ore.amount == 1:
					# There is not enough metal-bearing ore in this pile to make an ingot.
					char.socket.clilocmessage( 501987, '', GRAY )
					return OOPS
			success = 1
		else:
			success = 0

	if success == 0:
		if ore.amount >= 2:
			# You burn away the impurities but are left with less useable metal.
			char.socket.clilocmessage( 501990, '', GRAY )
			ore.amount -= ( ore.amount / 2 )
			ore.update()
		else:
			# You burn away the impurities but are left with no useable metal.
			char.socket.clilocmessage( 501989, '', GRAY )
			ore.delete()

	return OK

def successsmelt( char, table, resname, amount ):
	socket = char.socket
	backpack = char.getbackpack()
	if not backpack:
		return OOPS
	resingot = "%s_ingot" % ( resname )
	resourceitem = wolfpack.additem( str(resingot) )
	resourceitem.decay = 1
	resourceitem.amount = amount

	if not wolfpack.utilities.tocontainer( resourceitem, char.getbackpack() ):
		resourceitem.update()

	# You smelt the ore removing the impurities and put the metal in your backpack.
	char.socket.clilocmessage( 501988, '', GRAY )
	char.soundeffect( SOUND_HAMMER_1 )

	return OK
