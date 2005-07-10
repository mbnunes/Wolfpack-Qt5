# ToDo: Change ids of items when they become arcane

import wolfpack
from wolfpack.consts import *

DefaultArcaneHue = 2117

# leather gloves, cloaks, robe, tigh boots
arcaneequip = ["13c6", "1515", "1f04", "1711"]

def getCharges( char ):
	charges = char.skill[TAILORING] / 5
	if charges < 16:
		charges = 16
	elif charges > 24:
		charges = 24
	return charges

def onUse( char, item ):
	if not item.getoutmostchar() == char:
		char.socket.clilocmessage( 1042001 ) # That must be in your pack for you to use it.
		return True
	char.socket.sysmessage( "What do you wish to use the gem on?" )
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
	if target.item and target.item.baseid in arcaneequip:
		if isarcane( target.item ):
			if CurArcaneCharges( target.item ) >= MaxArcaneCharges( target.item ):
				char.socket.sysmessage( "That item is already fully charged." )
				return False
			else:
				if CurArcaneCharges <= 0:
					target.item.color = DefaultArcaneHue

				if (CurArcaneCharges( target.item ) + getCharges(char)) > MaxArcaneCharges( target.item ):
					target.item.settag( "arcane", MaxArcaneCharges( target.item ) )
				else:
					target.item.settag( "arcane", item.gettag("arcane") + getCharges(char) )

				char.socket.sysmessage( "You recharge the item." )
				item.delete()
				target.item.resendtooltip()

		elif char.skill[TAILORING] >= 80.0:
			if target.item.hastag( "exceptional" ):
				charges = getCharges( char )
				target.item.settag( "exceptional", int(char.serial) )
				target.item.settag( "maxArcaneCharges", charges )
				target.item.settag( "arcane", charges )
				target.item.color = DefaultArcaneHue
				char.socket.sysmessage( "You enhance the item with your gem." )
				item.delete()
				target.item.update()
			else:
				char.socket.sysmessage( "Only exceptional items can be enhanced with the gem." )
		else:
			char.socket.sysmessage( "You do not have enough skill in tailoring to enhance the item." )
	else:
		char.socket.sysmessage( "You cannot use the gem on that." )
	return

def ConsumeCharges( char, amount ):
	avail = 0
	for i in range( 1, 25 ):
		item = char.itemonlayer( i )
		if item.hastag( "arcane" ):
			avail += CurArcaneCharges(item)
	if avail < amount:
		return False
	for i in range( 1, 25 ):
		item = char.itemonlayer( i )
		if item.hastag( "arcane" ):
			if CurArcaneCharges(item) > amount:
				item.settag( "arcane", CurArcaneCharges( item ) - amount)
				break
			else:
				amount -= CurArcaneCharges( item )
				item.settag( "arcane", 0 )
	return True
