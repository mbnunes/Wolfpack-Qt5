
# http://uo.stratics.com/alchemy/potions/potionkegs.shtml

import wolfpack
from random import randint
from wolfpack.time import *
from wolfpack.consts import *
from wolfpack.utilities import hex2dec, tobackpack
from potions import potions # Imports the potion table.

POT_RETURN_BOTTLE = 0
POT_AGGRESSIVE = 1
POT_TARGET = 2
POT_NAME = 3
KEG_NAME = 4
POT_DEF = 5

### Filling the keg ###
def onDropOnItem(potionkeg, potion):
	if ('potionkeg' not in potionkeg.events) or ('potions' not in potion.events):
		return 0

	char = potion.container
	socket = char.socket

	if not potion.hastag('potiontype') or not potionkeg.hastag('kegfill'):
		socket.sysmessage("Only potions may be added to a potion keg!")
		return 0

	char.say("Test")
	"""
	char.say("Step 1")
	if potionkeg.id in [ hex2dec(0x1ad6), hex2dec(0x1ad7) ]:
		char.say("Step 2")
		if potionkeg.hastag('kegfill') and potionkeg.hastag('potiontype'):
			char.say("Step 3")
			if potion.gettag('potiontype') == potionkeg.gettag('potiontype'):
				kegfill = potionkeg.gettag('kegfill')
				if kegfill < 100:
					kegfill += 1
					potionkeg.settag('kegfill', kegfill)
					potion.delete()
					return 1
				else:
					# The keg will not hold any more!
					socket.clilocmessage(502233)
					return 1
			else:
				# You decide that it would be a bad idea to mix different types of potions.
				socket.clilocmessage(502236)
				return 1
		else:
			potionkeg.settag( 'potiontype', potion.gettag('potiontype') )
			potionkeg.settag( 'kegfill', 1 )
			potionkeg.name = potions[ potion.gettag('potiontype') ][ KEG_NAME ]
			consumePotion( char, potion, 1 )
			return 1
	else:
		return 0
	"""
	return 1

### End Filling ###

def onUse(char, potionkeg):
	socket = char.socket
	kegfill = int(potionkeg.gettag('kegfill'))
	backpack = char.getbackpack()

	if potionkeg.hastag('potiontype'):
		checkkegstatus( char, keg, potionkeg.gettag('potiontype') )

	if kegfill > 0:
		count = backpack.countitems(['f0e'])
		if count >= 1:
			backpack.removeitems(['f0e'], 1)
			newpot = wolfpack.additem(potions[potionkeg.gettag('potiontype')][POT_DEF])
			newpot.decay = 1
			newpot.magic = 1
			if not tobackpack(newpot, char):
				newpot.update()
			kegfill -= 1
			potionkeg.settag('kegfill', kegfill )
			if kegfill == 0:
				if potionkeg.name != "#1041641":
					potionkeg.name = '#1041641'
				potionkeg.deltag('potiontype')
		else:
			socket.clilocmessage(500315)

	else:
		if potionkeg.name != "#1041641":
			potionkeg.name = '#1041641'
		if potionkeg.hastag('potiontype'):
			potionkeg.deltag('potiontype')

	return 1

def checkkegstatus(char, potionkeg, potiontype):
	if potionkeg.name != potions[ potiontype ][ KEG_NAME ]:
		potionkeg.name = potions[ potiontype ][ KEG_NAME ]
	return 1

