import wolfpack
from wolfpack.utilities import isweapon
import skills.poisoning

def onUse( char, item ):
	if not item.getoutmostchar() == char:
		char.socket.clilocmessage( 1042001 ) # That must be in your pack for you to use it.
		return True
	char.socket.attachtarget( 'oil_cloth.response', [item.serial] )
	char.socket.clilocmessage( 1005424 ) # Select the weapon or armor you wish to use the cloth on.
	return True

def response( char, args, target ):
	item = wolfpack.finditem( args[0] )
	if not item:
		return False
	if target.item:
		if isweapon( target.item ):
			if not target.item.getoutmostchar() == char:
				char.socket.clilocmessage( 1005425 ) # You may only wipe down items you are holding or carrying.
			elif not target.item.hastag( 'poisoning_strength' ):
				char.message( 1005422, "" ) # Hmmmm... this does not need to be cleaned.
			else:
				uses = 0
				if target.item.hastag( 'poisoning_uses' ):
					uses = target.item.gettag( 'poisoning_uses' )
				if uses <= 2:
					skills.poisoning.wearoff( target.item )
					char.socket.clilocmessage( 1010497 ) # You have cleaned the item.
				else:
					target.item.settag( 'poisoning_uses', uses - 2 )
					char.socket.clilocmessage( 1005423 ) # You have removed some of the caustic substance, but not all.
				if item.amount > 1:
					item.amount -= 1
				else:
					item.delete()
		else:
			char.message( 1005422, "" ) # Hmmmm... this does not need to be cleaned.

	elif target.char and target.char == char:
		char.socket.sysmessage( "11t")
		if target.char.id in [ 183, 184 ]:
			target.char.dispel(char, True, 'tribal_paint')
			char.socket.clilocmessage( 1040006 ) # You wipe away all of your body paint.
			if item.amount > 1:
				item.amount -= 1
			else:
				item.delete()
		else:
			char.message( 1005422, "" ) # Hmmmm... this does not need to be cleaned.
	else:
		char.socket.clilomessage( 1005426 ) # The cloth will not work on that.
	return
