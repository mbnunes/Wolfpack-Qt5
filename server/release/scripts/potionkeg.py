
import wolfpack
from wolfpack import tr
import wolfpack.utilities
from potions.utilities import *
from potions.consts import *

# Pouring a potion.
def onUse( char, potionkeg ):
	kegtype = None
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

	if potionkeg.hasintproperty( 'potiontype' ):
		kegtype = potionkeg.getintproperty( 'potiontype' )
	if potionkeg.hastag( 'potiontype' ):
		kegtype = int( potionkeg.gettag( 'potiontype' ) )

	if not kegtype:
		socket.clilocmessage( 502222 )
		return True

	if potionkeg.name != POTIONS[ kegtype ][ KEG_NAME ]:
		potionkeg.name = POTIONS[ kegtype ][ KEG_NAME ]
		potionkeg.update()

	if kegfill > 0 and kegfill <= 100:
		count = backpack.countitems( [ 'f0e' ] )
		if count >= 1:
			backpack.removeitems( [ 'f0e' ], 1 )
			newpot = wolfpack.additem( POTIONS[ kegtype ][ POT_DEF ] )
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

