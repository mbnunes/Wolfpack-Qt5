
# Fear the wrath of the miners!
# ~ Dreoth

from wolfpack.consts import *
import whrandom
import wolfpack
import skills
from skills import mining
from wolfpack.time import *
from wolfpack.utilities import *
from random import randrange, randint
from system.lootlists import DEF_ORES # Gets BaseIDs

forgeids = [ 0xfb1, 0x197a, 0x197b, 0x197c, 0x197d, 0x197e, 0x197f, 0x1980,
				0x1981, 0x1982, 0x1983, 0x1984, 0x1985, 0x1986, 0x1987, 0x1988,
				0x1989, 0x198a, 0x198b, 0x198c, 0x198d, 0x198e, 0x198f, 0x1990,
				0x1991, 0x1992, 0x1993, 0x1994, 0x1995, 0x1996, 0x1997, 0x1998,
				0x1999, 0x199a, 0x199b, 0x199c, 0x199d, 0x199e, 0x199f, 0x19a0,
				0x19a1, 0x19a2, 0x19a3, 0x19a4, 0x19a5, 0x19a6, 0x19a7, 0x19a8,
				0x19a9 ]

def onShowTooltip(player, object, tooltip):
	name = 'Unknown Ore'

	if object.hastag('resname'):
		resname = str(object.gettag('resname'))
		if mining.ORES.has_key(resname):
			name = mining.ORES[resname][mining.ORENAME]

	tooltip.reset()
	tooltip.add(1050039, "%u\t%s" % (object.amount, name))

def onUse( char, ore ):
	if ore.baseid in DEF_ORES:
		if ore.getoutmostchar() != char:
			if char.pos.distance( ore.pos ) > 2:
				char.socket.clilocmessage( 501976, '', GRAY ) # You can't reach...
				return True
			else:
				if not ore.hastag( 'resname' ):
					char.socket.clilocmessage( 501986, '', GRAY ) # Strange ore.
					return True
				else:
					# Where do you want to smelt the ore?
					char.socket.clilocmessage( 501971, '', GRAY )
					char.socket.attachtarget( "ore.response", [ ore.serial ] )
					return True
		else:
			if not ore.hastag( 'resname' ):
				char.socket.clilocmessage( 501986, '', GRAY )
				return True
			else:
				# Where do you want to smelt the ore?
				char.socket.clilocmessage( 501971, '', GRAY )
				char.socket.attachtarget( "ore.response", [ ore.serial ] )
				return True

def response( char, args, target ):
	if not target.item:
		char.socket.clilocmessage(501973)
		return

	char.turnto(target.item)

	if not char.canreach(target.item, 4):
		char.socket.clilocmessage(500295)
		return

	item = wolfpack.finditem(args[0])

	if not item:
		return

	if not item.hastag( 'resname' ):
		return False
	else:
		resname = item.gettag( 'resname' )

	targetitem = wolfpack.finditem( target.item.serial )

	# We go onto creating ingots here.
	if target.item.baseid in forgeids:
		if item.baseid in DEF_ORES:
			if char.pos.distance( target.pos ) > 3:
				char.socket.clilocmessage( 0x7A258 ) # You can't reach...
				return True
			else:
				dosmelt( char, [ item, targetitem, resname ] )
				return True

	# This is for merging the ore piles
	elif target.item.baseid in DEF_ORES:
		if targetitem.serial == item.serial:
			return False
		if not targetitem.hastag('resname'):
			char.socket.clilocmessage( 501986, '', GRAY )
			return False
		# Largest Ore Pile
		if item.baseid == DEF_ORES[3] and item.color == targetitem.color and item.gettag( 'resname' ) == targetitem.gettag( 'resname' ):
			if targetitem.getoutmostchar() != char:
				if char.pos.distance( target.pos ) > 2:
					# The ore is too far away.
					char.socket.clilocmessage( 501976, '', GRAY)
					return True
				else:
					# Merge the ore piles
					if targetitem.baseid == DEF_ORES[2] or targetitem.baseid == DEF_ORES[1]:
						targetitem.amount += ( item.amount * 2 )
						targetitem.update()
						item.delete()
						# Select the forge on which to smelt the ore, or another pile of ore with which to combine it.
						char.socket.clilocmessage( 501971, '', GRAY )
					elif targetitem.baseid == DEF_ORES[0]:
						targetitem.amount += ( item.amount * 4 )
						targetitem.update()
						item.delete()
						# Select the forge on which to smelt the ore, or another pile of ore with which to combine it.
						char.socket.clilocmessage( 501971, '', GRAY )
					elif targetitem.baseid == item.baseid:
						targetitem.amount += item.amount
						targetitem.update()
						item.delete()
						# Select the forge on which to smelt the ore, or another pile of ore with which to combine it.
						char.socket.clilocmessage( 501971, '', GRAY )
					return True
			else:
				# Merge the ore piles
				if targetitem.baseid == DEF_ORES[2] or targetitem.baseid == DEF_ORES[1]:
					targetitem.amount += ( item.amount * 2 )
					targetitem.update()
					item.delete()
					# Select the forge on which to smelt the ore, or another pile of ore with which to combine it.
					char.socket.clilocmessage( 501971, '', GRAY )
				elif targetitem.baseid == DEF_ORES[0]:
					targetitem.amount += ( item.amount * 4 )
					targetitem.update()
					item.delete()
					# Select the forge on which to smelt the ore, or another pile of ore with which to combine it.
					char.socket.clilocmessage( 501971, '', GRAY )
				elif targetitem.baseid == item.baseid:
					targetitem.amount += item.amount
					targetitem.update()
					item.delete()
					# Select the forge on which to smelt the ore, or another pile of ore with which to combine it.
					char.socket.clilocmessage( 501971, '', GRAY )
				return True

		# Second Largest Ore
		elif item.baseid == DEF_ORES[2] and item.color == targetitem.color and item.gettag( 'resname' ) == targetitem.gettag( 'resname' ):
			if targetitem.getoutmostchar() != char:
				if char.pos.distance( target.pos ) > 2:
					# The ore is too far away.
					char.socket.clilocmessage( 501976, '', GRAY)
					return True
				else:
					# Merge the ore piles
					if targetitem.baseid == DEF_ORES[2] or targetitem.baseid == DEF_ORES[1] or targetitem.baseid == DEF_ORES[0]:
						targetitem.amount += item.amount
						targetitem.update()
						item.delete()
						return True
					elif targetitem.baseid == DEF_ORES[0]:
						targetitem.amount += ( item.amount * 2 )
						targetitem.update()
						item.delete()
						return True
					elif targetitem.baseid == DEF_ORES[3]:
						return False
					return True
			else:
				# Merge the ore piles
				if targetitem.baseid == DEF_ORES[2] or targetitem.baseid == DEF_ORES[1]:
					targetitem.amount += item.amount
					targetitem.update()
					item.delete()
					return True
				elif targetitem.baseid == DEF_ORES[0]:
					targetitem.amount += ( item.amount * 2 )
					targetitem.update()
					item.delete()
					return True
				elif targetitem.baseid == DEF_ORES[3]:
					return False
				return True

		# Second Smallest
		elif item.baseid == DEF_ORES[1] and item.color == targetitem.color and item.gettag( 'resname' ) == targetitem.gettag( 'resname' ):
			if targetitem.getoutmostchar() != char:
				if char.pos.distance( target.pos ) > 2:
					char.socket.clilocmessage( 0x7A258 ) # You can't reach...
					return True
				else:
					if targetitem.baseid == DEF_ORES[1]:
						targetitem.amount += item.amount
						targetitem.update()
						item.delete()
						return True
					elif targetitem.baseid == DEF_ORES[0]:
						targetitem.amount += ( item.amount * 2 )
						targetitem.update()
						item.delete()
						return True
					elif targetitem.baseid == DEF_ORES[2] or targetitem.baseid == DEF_ORES[3]:
						return False
					return True
			else:
				# Merge the ore piles
				if targetitem.baseid == DEF_ORES[1]:
					targetitem.amount += item.amount
					targetitem.update()
					item.delete()
					return True
				elif targetitem.baseid == DEF_ORES[0]:
					targetitem.amount += ( item.amount * 2 )
					targetitem.update()
					item.delete()
					return True
				elif targetitem.baseid == DEF_ORES[2] or targetitem.baseid == DEF_ORES[3]:
					return False
				return True

		# Smallest
		elif item.baseid == DEF_ORES[0] and item.color == targetitem.color and item.gettag( 'resname' ) == targetitem.gettag( 'resname' ):
			if targetitem.getoutmostchar() != char:
				if char.pos.distance( target.pos ) > 2:
					# The ore is too far away.
					char.socket.clilocmessage( 501976 )
					return True
				else:
					# Merge the ore piles
					if targetitem.baseid == DEF_ORES[1] or targetitem.baseid == DEF_ORES[2] or targetitem.baseid == DEF_ORES[3]:
						char.socket.sysmessage( "You can not create a larger pile from a small pile of ore.", GRAY )
					elif targetitem.baseid == DEF_ORES[0]:
						targetitem.amount += item.amount
						targetitem.update()
						item.delete()
						char.socket.sysmessage( "You combine the two ore piles to create a single pile of ore.", GRAY )
					return True
			else:
				# Merge the ore piles
				if targetitem.baseid == DEF_ORES[1] or targetitem.baseid == DEF_ORES[2] or targetitem.baseid == DEF_ORES[3]:
					char.socket.sysmessage( "You can not create a larger pile from a small pile of ore.", GRAY )
				elif targetitem.baseid == DEF_ORES[0]:
					targetitem.amount += item.amount
					targetitem.update()
					item.delete()
					char.socket.sysmessage( "You combine the two ore piles to create a single pile of ore.", GRAY )
				return True

def dosmelt(char, args):
	ore = args[0]
	forge = args[1]
	resname = args[2]

	if not mining.ORES.has_key(resname):
		char.socket.sysmessage('You cannot smelt that kind of ore.')
		return 0

	success = 0
	reqskill = mining.ORES[resname][mining.REQSKILL]
	chance = max(0, char.skill[MINING] - mining.ORES[resname][mining.MINSKILL]) / 1000.0

	if not char.skill[MINING] >= reqskill:
		char.socket.clilocmessage(501986, '', GRAY) # You have no idea how to smelt this strange ore!
		return False

	if ore.amount >= 1 and char.skill[ MINING ] >= reqskill:
		if not skills.checkskill(char, MINING, chance):
			success = 0
		else:
			if ore.baseid == DEF_ORES[3]:
				amount = (ore.amount * 2)
				successsmelt(char, resname, amount)
				ore.delete()
			elif ore.baseid == DEF_ORES[2] or ore.baseid == DEF_ORES[1]:
				amount = ore.amount
				successsmelt(char, resname, amount)
				ore.delete()
			elif ore.baseid == DEF_ORES[0]:
				if evenorodd( ore.amount ) == "even":
					amount = ( ore.amount / 2 )
					successsmelt(char, resname, amount)
					ore.delete()
				elif evenorodd( ore.amount ) == "odd" and ore.amount > 1:
					amount = ( ( ore.amount - 1 ) / 2 )
					successsmelt(char, resname, amount)
					ore.amount = 1
					ore.update()
				elif ore.amount == 1:
					# There is not enough metal-bearing ore in this pile to make an ingot.
					char.socket.clilocmessage( 501987, '', GRAY )
					return False
			success = 1

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

	return True

def successsmelt(char, resname, amount):
	item = wolfpack.additem('1bf2')
	item.amount = amount
	item.baseid = '%s_ingot' % resname

	ore = mining.ORES[resname]

	item.color = ore[mining.COLORID]
	item.settag('resname', resname)

	if not wolfpack.utilities.tobackpack(item, char):
		item.update()

	# You smelt the ore removing the impurities and put the metal in your backpack.
	char.socket.clilocmessage(501988, '', GRAY)
	char.soundeffect(SOUND_HAMMER_1)
	return True
