
# Fear the wrath of the miners!
# ~ Dreoth

from wolfpack.consts import *
import whrandom
import wolfpack
import skills
from wolfpack.time import *
from wolfpack.utilities import *
from random import randrange

# 0x19b7, 0x19b8, 0x19ba give 1 ingot.
# 0x19b9 gives 2 ingots.
oreids = [ 0x19b7, 0x19b8, 0x19ba, 0x19b9 ]

forgeids = [ 0xfb1, 0x197a, 0x197b, 0x197c, 0x197d, 0x197e, 0x197f, 0x1980, 
				0x1981, 0x1982, 0x1983, 0x1984, 0x1985, 0x1986, 0x1987, 0x1988,
				0x1989, 0x198a, 0x198b, 0x198c, 0x198d, 0x198e, 0x198f, 0x1990,
				0x1991, 0x1992, 0x1993, 0x1994, 0x1995, 0x1996, 0x1997, 0x1998, 
				0x1999, 0x199a, 0x199b, 0x199c, 0x199d, 0x199e, 0x199f, 0x19a0,
				0x19a1, 0x19a2, 0x19a3, 0x19a4, 0x19a5, 0x19a6, 0x19a7, 0x19a8, 
				0x19a9 ]

# Name, reqSkill, minSkill, maxSkill, color, item name
ingottable = \
{
	'iron':			[ 0, 0, 1000, 0x0, 'iron ingot' ],
	'dullcopper':	[ 650, 250, 1050, 0x973, 'dull copper ingot' ],
	'shadowiron':	[ 700, 300, 1100, 0x966, 'shadow iron ingot' ], 
	'copper':		[ 750, 350, 1150, 0x96d, 'copper ingot' ],
	'bronze':		[ 800, 400, 1200, 0x972, 'bronze ingot' ],
	'gold':			[ 850, 450, 1250, 0x8a5, 'gold ingot' ],
	'silver':			[ 875, 475, 1275, 0x430, 'silver ingot' ],
	'agapite':		[ 900, 500, 1300, 0x979, 'agapite ingot' ],
	'verite':			[ 950, 550, 1350, 0x89f, 'verite ingot' ],
	'valorite':		[ 990, 590, 1390, 0x8ab, 'valorite ingot' ],
	'merkite':		[ 995, 600, 1400, 0x8b0, 'merkite ingot' ],
	'mythril':		[ 995, 600, 1400, 0x84c, 'mythril ingot' ]
}

REQSKILL = 0
MINSKILL = 1
MAXSKILL = 2
COLORID = 3
NAME = 4

def onUse( char, ore ):
	if ore.id in oreids:
		if ore.getoutmostchar() != char:
			if char.pos.distance( ore.pos ) > 2:
				char.socket.clilocmessage( 0x7A258 ) # You can't reach...
				return OK
			else:
				if not ore.hastag( 'resname' ):
					char.socket.sysmessage( "This ore is not of workable quality...", GRAY )
					return OK
				else:
					char.socket.sysmessage( "Where do you want to smelt the ore?", GRAY )
					char.socket.attachtarget( "ore.response", [ ore.serial ] )
					return OK
		else:
			if not ore.hastag( 'resname' ):
				char.socket.sysmessage( "This ore is not of workable quality...", GRAY )
				return OK
			else:
				char.socket.sysmessage( "Where do you want to smelt the ore?", GRAY )
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
			char.socket.sysmessage( "You can not combine a pile of ore with itself...", GRAY )
			return OOPS
		if not targetitem.hastag('resname'):
			char.socket.sysmessage( "That ore is not of workable quality...", GRAY )
			return OOPS
		# Largest Ore Pile
		if item.id == oreids[3] and item.color == targetitem.color and item.gettag('resname') == targetitem.gettag('resname'):
			if targetitem.getoutmostchar() != char:
				if char.pos.distance( target.pos ) > 2:
					char.socket.clilocmessage( 0x7A258 ) # You can't reach...
					return OK
				else:
					# Merge the ore piles
					if targetitem.id == oreids[2] or targetitem.id == oreids[1]:
						targetitem.amount += ( item.amount * 2 )
						targetitem.update()
						item.delete()
						char.socket.sysmessage( "You combine the two ore piles to create a single pile of ore.", GRAY )
					elif targetitem.id == oreids[0]:
						targetitem.amount += ( item.amount * 4 )
						targetitem.update()
						item.delete()
						char.socket.sysmessage( "You combine the two ore piles to create a single pile of ore.", GRAY )
					elif targetitem.id == item.id:
						targetitem.amount += item.amount
						targetitem.update()
						item.delete()
						char.socket.sysmessage( "You combine the two ore piles to create a single pile of ore.", GRAY )
					return OK
			else:
				# Merge the ore piles
				if targetitem.id == oreids[2] or targetitem.id == oreids[1]:
					targetitem.amount += ( item.amount * 2 )
					targetitem.update()
					item.delete()
					char.socket.sysmessage( "You combine the two ore piles to create a single pile of ore.", GRAY )
				elif targetitem.id == oreids[0]:
					targetitem.amount += ( item.amount * 4 )
					targetitem.update()
					item.delete()
					char.socket.sysmessage( "You combine the two ore piles to create a single pile of ore.", GRAY )
				elif targetitem.id == item.id:
						targetitem.amount += item.amount
						targetitem.update()
						item.delete()
						char.socket.sysmessage( "You combine the two ore piles to create a single pile of ore.", GRAY )
				return OK
		
		# Second Largest Ore
		elif item.id == oreids[2] and item.color == targetitem.color and item.gettag('resname') == targetitem.gettag('resname'):
			if targetitem.getoutmostchar() != char:
				if char.pos.distance( target.pos ) > 2:
					char.socket.clilocmessage( 0x7A258 ) # You can't reach...
					return OK
				else:
					# Merge the ore piles
					if targetitem.id == oreids[2] or targetitem.id == oreids[1] or targetitem.id == oreids[0]:
						targetitem.amount += item.amount
						targetitem.update()
						item.delete()
						char.socket.sysmessage( "You combine the two ore piles to create a single pile of ore.", GRAY )
					elif targetitem.id == oreids[0]:
						targetitem.amount += ( item.amount * 2 )
						targetitem.update()
						item.delete()
						char.socket.sysmessage( "You combine the two ore piles to create a single pile of ore.", GRAY )
					elif targetitem.id == oreids[3]:
						char.socket.sysmessage( "You can not create a larger pile from a small pile of ore.", GRAY )
					return OK
			else:
				# Merge the ore piles
				if targetitem.id == oreids[2] or targetitem.id == oreids[1]:
					targetitem.amount += item.amount
					targetitem.update()
					item.delete()
					char.socket.sysmessage( "You combine the two ore piles to create a single pile of ore.", GRAY )
				elif targetitem.id == oreids[0]:
					targetitem.amount += ( item.amount * 2 )
					targetitem.update()
					item.delete()
					char.socket.sysmessage( "You combine the two ore piles to create a single pile of ore.", GRAY )
				elif targetitem.id == oreids[3]:
						char.socket.sysmessage( "You can not create a larger pile from a small pile of ore.", GRAY )
				return OK
		
		# Second Smallest
		elif item.id == oreids[1] and item.color == targetitem.color and item.gettag('resname') == targetitem.gettag('resname'):
			if targetitem.getoutmostchar() != char:
				if char.pos.distance( target.pos ) > 2:
					char.socket.clilocmessage( 0x7A258 ) # You can't reach...
					return OK
				else:
					# Merge the ore piles
					if targetitem.id == oreids[1]:
						targetitem.amount += item.amount
						targetitem.update()
						item.delete()
						char.socket.sysmessage( "You combine the two ore piles to create a single pile of ore.", GRAY )
					elif targetitem.id == oreids[0]:
						targetitem.amount += ( item.amount * 2 )
						targetitem.update()
						item.delete()
						char.socket.sysmessage( "You combine the two ore piles to create a single pile of ore.", GRAY )
					elif targetitem.id == oreids[2] or targetitem.id == oreids[3]:
						char.socket.sysmessage( "You can not create a larger pile from a small pile of ore.", GRAY )
					return OK
			else:
				# Merge the ore piles
				if targetitem.id == oreids[1]:
					targetitem.amount += item.amount
					targetitem.update()
					item.delete()
					char.socket.sysmessage( "You combine the two ore piles to create a single pile of ore.", GRAY )
				elif targetitem.id == oreids[0]:
					targetitem.amount += ( item.amount * 2 )
					targetitem.update()
					item.delete()
					char.socket.sysmessage( "You combine the two ore piles to create a single pile of ore.", GRAY )
				elif targetitem.id == oreids[2] or targetitem.id == oreids[3]:
						char.socket.sysmessage( "You can not create a larger pile from a small pile of ore.", GRAY )
				return OK
		
		# Smallest
		elif item.id == oreids[0] and item.color == targetitem.color and item.gettag('resname') == targetitem.gettag('resname'):
			if targetitem.getoutmostchar() != char:
				if char.pos.distance( target.pos ) > 2:
					char.socket.clilocmessage( 0x7A258 ) # You can't reach...
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
	ingotdef = "1bf2"
	success = 0
	reqskill = ingottable[ resname ][ REQSKILL ]
	
	if ore.amount >= 1 and char.skill[ MINING ] >= reqskill:
		skills.checkskill( char, forge, MINING, 0 )
		if ore.id == oreids[3]:
			amount = ( ore.amount * 2 )
			successsmelt( char, ingottable, resname, amount, ingotdef )
			ore.delete()
		elif ore.id == oreids[2] or ore.id == oreids[1]:
			amount = ore.amount
			successsmelt( char, ingottable, resname, amount, ingotdef )
			ore.delete()
		elif ore.id == oreids[0]:
			if evenorodd( ore.amount ) == "even":
				amount = ( ore.amount / 2 )
				successsmelt( char, ingottable, resname, amount, ingotdef )
				ore.delete()
			elif evenorodd( ore.amount ) == "odd" and ore.amount > 1:
				amount = ( ( ore.amount - 1 ) / 2 )
				successsmelt( char, ingottable, resname, amount, ingotdef )
				ore.amount = 1
				ore.update()
			elif ore.amount == 1:
				char.socket.sysmessage( "You require more ore to produce any ingots.", GRAY )
		success = 1
		
	if success == 0:
		char.socket.sysmessage( "You smelt the ore but fail to produce any ingots.", GRAY )
		ore.amount -= ( ore.amount / 2 )
		ore.update()
	
	return OK

def successsmelt( char, table, resname, amount, ingotdef ):
	socket = char.socket
	backpack = char.getbackpack()
	if not backpack:
		return OOPS
	resourceitem = wolfpack.additem( ingotdef )
	resourceitem.decay = 1
	resourceitem.name = str( table[ resname ][ NAME ] )
	resourceitem.color = table[ resname ][ COLORID ]
	resourceitem.amount = amount
	resourceitem.settag( 'resname', str( resname ) ) # Used when smelting
	message = "You smelt the ore and place some " + table[ resname ][ NAME] + "s into your backpack."
	
	if not wolfpack.utilities.tocontainer( resourceitem, char.getbackpack() ):
		resourceitem.update()

	socket.sysmessage( message, GRAY )
	char.soundeffect(  0x2a )
	
	return OK
