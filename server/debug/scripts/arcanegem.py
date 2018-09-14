import wolfpack
from wolfpack.consts import *
from wolfpack import tr
from math import ceil

DefaultArcaneHue = 2117

# leather gloves, cloaks, robe, tigh boots
arcaneequip = [
		[0x13c6, 0x26b0],
		[0x1515, 0x26ad],
		[0x1f04, 0x26ae],
		[0x1711, 0x1712]
	]

def possible():
	p = []
	for i in arcaneequip:
		for item in i:
			p.append(item)
	return p

def getCharges( char ):
	charges = int(char.skill[TAILORING] / 50)
	return charges

def onUse( char, item ):
	if not item.getoutmostchar() == char:
		char.socket.clilocmessage( 1042001 ) # That must be in your pack for you to use it.
		return True
	char.socket.sysmessage( tr("What do you wish to use the gem on?") )
	char.socket.attachtarget( "arcanegem.response", [item.serial] )
	return True

def isarcane( item ):
	if item.hastag( "arcane" ):
		return True
	return False

def MaxArcaneCharges( item ):
	Max = item.gettag( "maxArcaneCharges" )
	return Max

def CurArcaneCharges( item ):
	charges = item.gettag( "arcane" )
	return charges

def response( char, args, target ):
	item = wolfpack.finditem( args[0] )
	if not item:
		return False
	if target.item and not target.item.layer == 0 and target.item.getoutmostchar() == char:
		char.socket.clilocmessage( 1042001 ) # That must be in your pack for you to use it.
		return False
	if target.item and target.item.id in possible():
		if isarcane( target.item ):
			if CurArcaneCharges( target.item ) >= MaxArcaneCharges( target.item ):
				char.socket.sysmessage( tr("That item is already fully charged.") )
				return False
			else:
				if CurArcaneCharges <= 0:
					target.item.color = DefaultArcaneHue

				if (CurArcaneCharges( target.item ) + getCharges(char)) > MaxArcaneCharges( target.item ):
					target.item.settag( "arcane", MaxArcaneCharges( target.item ) )
				else:
					target.item.settag( "arcane", item.gettag("arcane") + getCharges(char) )

				char.socket.sysmessage( tr("You recharge the item.") )
				item.delete()
				target.item.resendtooltip()

		elif char.skill[TAILORING] >= 80.0:
			if target.item.hastag( "exceptional" ):
				charges = getCharges( char )
				target.item.settag( "exceptional", int(char.serial) )
				target.item.settag( "maxArcaneCharges", charges )
				target.item.settag( "arcane", charges )
				target.item.color = DefaultArcaneHue
				char.socket.sysmessage( tr("You enhance the item with your gem.") )
				item.delete()
				# new art look...
				for equipment in arcaneequip:
					if equipment[0] == target.item.id:
						target.item.id = equipment[1]
				target.item.update()
			else:
				char.socket.sysmessage( tr("Only exceptional items can be enhanced with the gem.") )
		else:
			char.socket.sysmessage( tr("You do not have enough skill in tailoring to enhance the item.") )
	else:
		char.socket.sysmessage( tr("You cannot use the gem on that.") )
	return

def canConsumeCharges( char ):
	avail = 0
	for i in range( 1, 25 ):
		item = char.itemonlayer( i )
		if item and item.hastag( "arcane" ):
			avail += CurArcaneCharges(item)
	return avail

def ConsumeCharges( char, amount = 1 ):
	avail = 0
	for i in range( 1, 25 ):
		item = char.itemonlayer( i )
		if item and item.hastag( "arcane" ):
			avail += CurArcaneCharges(item)
	if avail < amount:
		return False
	for i in range( 1, 25 ):
		item = char.itemonlayer( i )
		if item and item.hastag( "arcane" ):
			if CurArcaneCharges(item) > amount:
				item.settag( "arcane", CurArcaneCharges( item ) - amount)
				item.resendtooltip()
				break
			else:
				amount -= CurArcaneCharges( item )
				item.settag( "arcane", 0 )
				# back to normal look...
				for equipment in arcaneequip:
					if equipment[1] == item.id:
						item.id = equipment[0]
						item.color = 0
						item.update()
				item.resendtooltip()
	return True
