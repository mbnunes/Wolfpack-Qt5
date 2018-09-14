
import wolfpack
from wolfpack import tr
import wolfpack.utilities
import potions.utilities
from potions.consts import *

# Pouring a potion.
def onUse( char, potionkeg ):
	kegtype = None
	backpack = char.getbackpack()

	if not char or not backpack:
		return False

	if not potionkeg.hastag( 'kegfill' ):
		kegfill = 0
		potionkeg.settag( 'kegfill', kegfill )
		potionkeg.resendtooltip()
	else:
		kegfill = int( potionkeg.gettag( 'kegfill' ) )
		if kegfill < 0: # Safeguard against negative fills
			kegfill = 0

	if potionkeg.hasintproperty( 'potiontype' ):
		kegtype = potionkeg.getintproperty( 'potiontype' )
	if potionkeg.hastag( 'potiontype' ):
		kegtype = int( potionkeg.gettag( 'potiontype' ) )

	if not kegtype and not kegtype == 0:
		char.socket.clilocmessage( 502222 )
		return True

	if potionkeg.name != POTIONS[ kegtype ][ KEG_NAME ]:
		potionkeg.name = POTIONS[ kegtype ][ KEG_NAME ]
		potionkeg.update()

	if kegfill > 0 and kegfill <= 100:
		count = backpack.countitems( [ 'f0e' ] )
		if count >= 1:
			potions.utilities.fillbottle(char, backpack, kegfill, kegtype, potionkeg)
			return True
		else:
			char.socket.clilocmessage( 500315 ) # You need an empty bottle to make a potion.
			return True

	else:
		if potionkeg.name != "#1041641":
			potionkeg.name = '#1041641'
		if potionkeg.hastag( 'potiontype' ):
			potionkeg.deltag( 'potiontype' )
		char.socket.clilocmessage( 502222 ) # The keg is empty.
		potionkeg.update()
		return True

# Returns the cliloc id for the fill message
def getfillid(kegfill):
	if kegfill == 100:
		return 502258 # The keg is completely full.
	elif kegfill <= 99 and kegfill >= 96:
		return 502257 # The liquid is almost to the top of the keg.
	elif kegfill <= 95 and kegfill >= 80:
		return 502256 # The keg is very full.
	elif kegfill <= 79 and kegfill >= 70:
		return 502255 # The keg is about three quarters full.
	elif kegfill <= 69 and kegfill >= 54:
		return 502253 # The keg is more than half full.
	elif kegfill <= 53 and kegfill >= 47:
		return 502254 # The keg is approximately half full.
	elif kegfill <= 46 and kegfill >= 40:
		return 502252 # The keg is almost half full.
	elif kegfill <= 39 and kegfill >= 30:
		return 502251 # The keg is about one third full.
	elif kegfill <= 29 and kegfill >= 20:
		return 502250 # The keg is about one quarter full.
	elif kegfill <= 19 and kegfill >= 5:
		return 502249 # The keg is not very full.
	elif kegfill <= 4 and kegfill >= 1:
		return 502248 # The keg is nearly empty.
	else:
		return 0
		#return 502259 # The keg is damaged.

# Potion Filling Messages
def kegfillmessage( char, kegfill ):
	socket = char.socket
	clilocid = getfillid(kegfill)
	if clilocid != 0:
		socket.clilocmessage(clilocid)
	return True

def onDropOnItem( keg, potion ):
	char = potion.container
	socket = char.socket
	potiontype = None
	kegtype = None

	if not char or not socket:
		return False

	if not isPotionkeg( keg ) or not isPotion( potion ):
		return False

	if isPotion( potion ):
		# Potion Type
		if potion.hasintproperty( 'potiontype' ):
			potiontype = potion.getintproperty( 'potiontype' )
		if potion.hastag( 'potiontype' ):
			potiontype = int( potion.gettag( 'potiontype' ) )
		# Keg Type
		if keg.hasintproperty( 'potiontype' ):
			kegtype = keg.getintproperty( 'potiontype' )
		if keg.hastag( 'potiontype' ):
			kegtype = int( keg.gettag( 'potiontype' ) )

		if not potiontype in POTIONS:
			socket.sysmessage( tr( "Only potions may be added to a potion keg!" ) )
			return True

		if not keg.hastag( 'kegfill' ):
			kegfill = 0
			keg.settag( 'kegfill', kegfill )
			keg.resendtooltip()
		else:
			kegfill = int( keg.gettag( 'kegfill' ) )
			if kegfill < 0: # Safeguard against negative fills
				kegfill = 0

		if kegfill >= 100:
			socket.clilocmessage( 502247 )
			return True

		if keg.baseid in [ 'potion_keg' ]:
			if kegtype >= 0:
				if potiontype == kegtype:
					if kegfill < 100 and kegfill >= 0:
						char.soundeffect( 0x240 )
						potions.utilities.fillPotion( char, keg, potion )
						keg.update()
						keg.resendtooltip()
						kegfillmessage( char, kegfill )
						socket.clilocmessage( 502239 )
						return True
					else:
						# The keg will not hold any more!
						socket.clilocmessage( 502233 )
						return True
				else:
					# You decide that it would be a bad idea to mix different types of otions.
					socket.clilocmessage( 502236 )
					return True
			else:
				kegtype = potions.utilities.getPotionType( potion )
				if kegtype >= 0:
					keg.settag( 'potiontype', kegtype )
					keg.name = POTIONS[ kegtype ][ KEG_NAME ]
					char.soundeffect( 0x240 )
					potions.utilities.fillPotion( char, keg, potion )
					keg.update()
					keg.resendtooltip()
				return True
	return True

def isPotionkeg( keg ):
	return keg.hasscript('potionkeg') and keg.baseid in ['potion_keg'] and keg.id in [ 0x1940 ]

def isPotion( potion ):
	if potion.hastag( 'potiontype' ) and not potion.hasscript('potionkeg'):
		return potion.hasscript('potions') and int( potion.gettag('potiontype') ) in POTIONS
	elif potion.hasintproperty( 'potiontype' ) and not potion.hasscript('potionkeg'):
		return potion.hasscript('potions') and potion.getintproperty('potiontype') in POTIONS
	else:
		return False

def onShowTooltip(viewer, potionkeg, tooltip):
	if not potionkeg.hastag( 'kegfill' ):
		kegfill = 0
		potionkeg.settag( 'kegfill', kegfill )
	else:
		kegfill = int( potionkeg.gettag( 'kegfill' ) )
		if kegfill < 0: # Safeguard against negative fills
			kegfill = 0

	clilocid = getfillid(kegfill)
	if clilocid != 0:
		tooltip.add(clilocid, "")
