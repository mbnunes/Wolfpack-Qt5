
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

def onUse(char, keg):
	socket = char.socket
	kegfill = int(keg.gettag('kegfill'))
	backpack = char.getbackpack()

	if keg.hastag('potiontype'):
		checkkegstatus( char, keg, keg.gettag('potiontype') )

	if kegfill > 0:
		count = backpack.countitems(['f0e'])
		if count >= 1:
			backpack.removeitems(['f0e'], 1)
			newpot = wolfpack.additem(potions[keg.gettag('potiontype')][POT_DEF])
			newpot.decay = 1
			newpot.magic = 1
			if not tobackpack(newpot, char):
				newpot.update()
			kegfill -= 1
			keg.settag('kegfill', kegfill )
			if kegfill == 0:
				if keg.name != "#1041641":
					keg.name = '#1041641'
				keg.deltag('potiontype')
		else:
			socket.clilocmessage(500315)

	else:
		if keg.name != "#1041641":
			keg.name = '#1041641'
		if keg.hastag('potiontype'):
			keg.deltag('potiontype')

	return 1

def onDropOnItem(keg, potion):
	char = potion.container

	if 'potionkeg' not in keg.events or 'potion' not in potion.events:
		char.say("Error with events!")
		return 0

	if not potion.hastag('potiontype'):
		return 0

	if keg.id == hex2dec(0x1ad6) or keg.id == hex2dec(0x1ad7):
		if keg.hastag('kegfill') and keg.hastag('potiontype'):
			if potion.gettag('potiontype') == keg.gettag('potiontype'):
				kegfill = keg.gettag('kegfill')
				if kegfill < 100:
					kegfill += 1
					keg.settag('kegfill', kegfill)
					potion.delete()
					return 1
				else:
					# 502233 The keg will not hold any more!
					potion.say("Error!")
					return 1
			else:
				# 502236 You decide that it would be a bad idea to mix different types of potions.
				return 1
		else:
			keg.settag( 'potiontype', potion.gettag('potiontype') )
			keg.settag( 'kegfill', 1 )
			keg.name = potions[ potion.gettag('potiontype') ][ KEG_NAME ]
			consumePotion( char, potion, 1 )
			return 1
	else:
		return 0

def checkkegstatus(char, keg, potiontype):
	if keg.name != potions[ potiontype ][ KEG_NAME ]:
		keg.name = potions[ potiontype ][ KEG_NAME ]
	return 1