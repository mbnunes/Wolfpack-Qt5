
import wolfpack
import wolfpack.utilities
# Imports the potion table.
from potions import POTIONS, POT_RETURN_BOTTLE, POT_AGGRESSIVE, \
	 POT_TARGET, POT_NAME, KEG_NAME, POT_DEF, consumePotion

### Filling the keg ###
def onDropOnItem( potionkeg, potion ):
	char = potion.container
	socket = char.socket

	if not char or not socket:
		return False

	if not potionkeg.hasscript( 'potionkeg' ) or not potion.hasscript( 'potions' ):
		return False

	if not potionkeg.hastag( 'kegfill' ):
		kegfill = 0
		potionkeg.settag( 'kegfill', kegfill )
	else:
		kegfill = int( potionkeg.gettag( 'kegfill' ) )
		if kegfill < 0: # Safeguard against negative fills
			kegfill = 0

	if not potion.hastag( 'potiontype' ):
		socket.sysmessage("Only potions may be added to a potion keg!")
		return True

	if kegfill >= 100:
		socket.clilocmessage( 502247 )
		return True

	if potionkeg.id in [ int(0x1ad6), int(0x1ad7), int(0x1940) ]:
		if potionkeg.hastag( 'potiontype' ):
			if potion.gettag( 'potiontype' ) == potionkeg.gettag( 'potiontype' ):
				if kegfill < 100 and kegfill >= 0:
					kegfill += 1
					potionkeg.settag( 'kegfill', kegfill )
					char.soundeffect( 0x240 )
					consumePotion( char, potion )
					potionkeg.update()
					kegfillmessage( char, kegfill )
					socket.clilocmessage( 502239 )
					return True
				else:
					socket.clilocmessage( 502233 ) # The keg will not hold any more!
					return True
			else:
				socket.clilocmessage( 502236 ) # You decide that it would be a bad idea to mix different types of potions.
				return True
		else:
			kegtype = potion.gettag( 'potiontype' )
			potionkeg.settag( 'potiontype', kegtype )
			potionkeg.settag( 'kegfill', 1 )
			potionkeg.name = POTIONS[ potion.gettag('potiontype') ][ KEG_NAME ]
			char.soundeffect( 0x240 )
			consumePotion( char, potion )
			potionkeg.update()
			return True
	else:
		return True


# Pouring a potion.
def onUse( char, potionkeg ):
	socket = char.socket
	backpack = char.getbackpack()

	if not char or not backpack:
		return False

	if not potionkeg.hasscript( 'potionkeg' ):
		return False

	if not potionkeg.hastag( 'kegfill' ):
		kegfill = 0
		potionkeg.settag( 'kegfill', kegfill )
	else:
		kegfill = int( potionkeg.gettag( 'kegfill' ) )
		if kegfill < 0: # Safeguard against negative fills
			kegfill = 0

	if potionkeg.hastag( 'potiontype' ) and potionkeg.name != POTIONS[ potionkeg.gettag( 'potiontype' ) ][ KEG_NAME ]:
		potionkeg.name = POTIONS[ potionkeg.gettag( 'potiontype' ) ][ KEG_NAME ]
		potionkeg.update()

	if kegfill > 0 and kegfill <= 100:
		count = backpack.countitems( [ 'f0e' ] )
		if count >= 1:
			backpack.removeitems( [ 'f0e' ], 1 )
			newpot = wolfpack.additem( POTIONS[ potionkeg.gettag( 'potiontype' ) ][ POT_DEF ] )
			if not newpot:
				return False
			newpot.decay = 1
			newpot.movable = 1
			socket.clilocmessage( 502242 ) # You pour some of the keg's contents into an empty bottle...
			socket.clilocmessage( 502243 ) # ...and place it into your backpack.
			if not wolfpack.utilities.tobackpack( newpot, char ):
				newpot.update()
			char.soundeffect(0x240)
			kegfill -= 1
			potionkeg.settag('kegfill', kegfill )
			if kegfill == 0:
				if potionkeg.name != "#1041641":
					potionkeg.name = '#1041641'
				potionkeg.deltag( 'potiontype' )
				potionkeg.update()
				socket.clilocmessage( 502245 ) # The keg is now empty
			return True
		else:
			socket.clilocmessage( 500315 )
			return True

	else:
		if potionkeg.name != "#1041641":
			potionkeg.name = '#1041641'
		if potionkeg.hastag( 'potiontype' ):
			potionkeg.deltag( 'potiontype' )
		socket.clilocmessage( 502222 )
		potionkeg.update()
		return True

# Potion Filling Messages
def kegfillmessage( char, kegfill ):
	socket = char.socket
	if kegfill == 100:
		socket.clilocmessage( 502258 ) # The keg is completely full.
	elif kegfill <= 99 and kegfill >= 96:
		socket.clilocmessage( 502257 ) # The liquid is almost to the top of the keg.
	elif kegfill <= 95 and kegfill >= 80:
		socket.clilocmessage( 502256 ) # The keg is very full.
	elif kegfill <= 79 and kegfill >= 70:
		socket.clilocmessage( 502255 ) # The keg is about three quarters full.
	elif kegfill <= 69 and kegfill >= 54:
		socket.clilocmessage( 502253 ) # The keg is more than half full.
	elif kegfill <= 53 and kegfill >= 47:
		socket.clilocmessage( 502254 ) # The keg is approximately half full.
	elif kegfill <= 46 and kegfill >= 40:
		socket.clilocmessage( 502252 ) # The keg is almost half full.
	elif kegfill <= 39 and kegfill >= 30:
		socket.clilocmessage( 502251 ) # The keg is about one third full.
	elif kegfill <= 29 and kegfill >= 20:
		socket.clilocmessage( 502250 ) # The keg is about one quarter full.
	elif kegfill <= 19 and kegfill >= 5:
		socket.clilocmessage( 502249 ) # The keg is not very full.
	elif kegfill <= 4 and kegfill >= 1:
		socket.clilocmessage( 502248 ) # The keg is nearly empty.
	else:
		socket.clilocmessage( 502259 ) # The keg is damaged.
	return
