
from wolfpack import tr
import random

DURATION = 5 * 60 * 1000 # Resistance lasts 5 minutes
RESIST_MAX_LEVEL = 2 # Resist at most level 2 poison (greater)

def onUse(player, item):
	if item.getoutmostitem() != player.getbackpack():
		player.socket.clilocmessage(1042038) # You must have the object in your backpack to use it.
		return True

	# If the player already is under the effect, deny another use
	if player.hastag('orangepetals'):
		player.message(1061904)
		return True
	
	player.message(1061905)
	player.soundeffect(0x3b)
	
	if item.amount <= 1:
		item.delete()
	else:
		item.amount -= 1
		item.update()

	magic = random.random()
	player.settag('orangepetals', magic)
	player.addtimer(DURATION, callback, [magic], True)

	return True

def callback(player, args):
	if not player.hastag('orangepetals'):
		return
		
	if player.gettag('orangepetals') != args[0]:
		return

	player.deltag('orangepetals')	
	player.message(tr('* You feel the effects of your poison resistance wearing off *'), 0x3f)
