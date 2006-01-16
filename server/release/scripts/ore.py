
# Fear the wrath of the miners!
# ~ Dreoth

import wolfpack
import skills
import wolfpack.utilities
from wolfpack.consts import GRAY, MINING, SOUND_HAMMER_1
from skills import mining
from random import randrange, randint
from system.lootlists import DEF_ORES # Gets BaseIDs
from wolfpack import tr
from skills.blacksmithing import FORGES

def onShowTooltip(player, object, tooltip):
	name = 'Unknown Ore'

	if object.hastag('resname'):
		resname = unicode(object.gettag('resname'))
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
	char.turnto(target.pos)

	if not char.canreach(target.pos, 4):
		char.socket.clilocmessage(500295)
		return

	item = wolfpack.finditem(args[0])

	if not item:
		return

	if not item.hastag( 'resname' ):
		return False
	else:
		resname = item.gettag( 'resname' )

	if target.item:
		targetitem = target.item

	elif target.char:
		if not target.char.baseid == 'fire_beetle':
			char.socket.clilocmessage(501973)
			return
		else:
			targetchar = target.char

	# Static Forges can be used, too
	else:
		if char.pos.distance( target.pos ) > 3:
			char.socket.clilocmessage( 0x7A258 ) # You can't reach...
			return True

		statics = wolfpack.statics(target.pos.x, target.pos.y, target.pos.map, True)
		for tile in statics:
			dispid = tile[0]
			if dispid in FORGES:
				dosmelt( char, [ item, resname ] )
				return True

		# We found no static forge
		char.socket.clilocmessage(501973)
		return

	# We go onto creating ingots here.
	if target.item and target.item.id in FORGES:
		if item.baseid in DEF_ORES:
			if char.pos.distance( target.pos ) > 3:
				char.socket.clilocmessage( 0x7A258 ) # You can't reach...
				return True
			else:
				dosmelt( char, [ item, resname ] )
				return True

	# We can use Fire Beetles like a forge
	if target.char and target.char.baseid == 'fire_beetle':
		if item.baseid in DEF_ORES:
			if char.pos.distance( target.pos ) > 3:
				char.socket.clilocmessage( 0x7A258 ) # You can't reach...
				return True
			else:
				dosmelt( char, [ item, resname ] )
				return True

	# This is for merging the ore piles
	elif target.item and target.item.baseid in DEF_ORES:
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
						char.socket.sysmessage( tr("You can not create a larger pile from a small pile of ore."), GRAY )
					elif targetitem.baseid == DEF_ORES[0]:
						targetitem.amount += item.amount
						targetitem.update()
						item.delete()
						char.socket.sysmessage( tr("You combine the two ore piles to create a single pile of ore."), GRAY )
					return True
			else:
				# Merge the ore piles
				if targetitem.baseid == DEF_ORES[1] or targetitem.baseid == DEF_ORES[2] or targetitem.baseid == DEF_ORES[3]:
					char.socket.sysmessage( tr("You can not create a larger pile from a small pile of ore."), GRAY )
				elif targetitem.baseid == DEF_ORES[0]:
					targetitem.amount += item.amount
					targetitem.update()
					item.delete()
					char.socket.sysmessage( tr("You combine the two ore piles to create a single pile of ore."), GRAY )
				return True

def dosmelt(char, args):
	ore = args[0]
	resname = args[1]

	if not mining.ORES.has_key(resname):
		char.socket.sysmessage( tr('You cannot smelt that kind of ore.') )
		return 0

	success = 0
	reqskill = mining.ORES[resname][mining.REQSKILL]
	minskill = reqskill - 250
	maxskill = reqskill + 250
	#chance = max(0, char.skill[MINING] - mining.ORES[resname][mining.MINSKILL]) / 1000.0

	if char.skill[MINING] < reqskill:
		char.socket.clilocmessage(501986, '', GRAY) # You have no idea how to smelt this strange ore!
		return False

	if ore.amount >= 1 and char.skill[ MINING ] >= reqskill:
		# Exploit fix. Otherwise the skillcheck would be before
		# consuming items.
		if ore.baseid == DEF_ORES[0] and ore.amount <= 1:
			char.socket.clilocmessage( 501987, '', GRAY )
			return False

		if not char.checkskill(MINING, minskill, maxskill):
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
				if ore.amount > 1 and ore.amount % 2 == 0:
					amount = ( ore.amount / 2 )
					successsmelt(char, resname, amount)
					ore.delete()
				elif ore.amount > 1:
					amount = ( ( ore.amount - 1 ) / 2 )
					successsmelt(char, resname, amount)
					ore.amount = 1
					ore.update()
				else:
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

	# Resend weight
	char.socket.resendstatus()

	return True

def successsmelt(char, resname, amount):
	item = wolfpack.additem( '%s_ingot' % resname)
	item.amount = amount

	ore = mining.ORES[resname]

	item.color = ore[mining.COLORID]
	item.settag('resname', resname)

	if not wolfpack.utilities.tobackpack(item, char):
		item.update()

	# You smelt the ore removing the impurities and put the metal in your backpack.
	char.socket.clilocmessage(501988, '', GRAY)
	char.soundeffect(SOUND_HAMMER_1)
	return True
