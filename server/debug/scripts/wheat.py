
import wolfpack
import random
import wolfpack.utilities
from wolfpack import tr

# How much wheat you need in order to make one sack of flour
WHEATAMOUNT = 4

MILL_RANGE = 2
flour_mill = [ 0x1920, 0x1921, 0x1922, 0x1923, 0x1924, 0x1925, 0x1926, 0x1927, 0x1928, 0x1929, 0x192a, 0x192b, 0x192c, 0x192d, 0x192e, 0x192f, 0x1930, 0x1931, 0x1932, 0x1933 ]
# not used flours
flours = [ "1039", "1045" ]

def checkmill( pos ):
	found = 0
	# Dynamic items
	items = wolfpack.items( pos.x, pos.y, pos.map, MILL_RANGE )
	for item in items:
		if item.id in flour_mill:
			found = 1
			break
	# static items
	if not found:
		# Check for static items
		for x in range(-2, 3):
			for y in range(-2, 3):
				statics = wolfpack.statics(pos.x + x, pos.y + y, pos.map, True)
	
				for tile in statics:
					dispid = tile[0]
					if dispid in flour_mill:
						found = 1
						break
	return found

def onUse( char, item ):
	if item.getoutmostchar() != char:
		char.socket.clilocmessage(500364) # You can't use that, it belongs to someone else.
		return True
	if not checkmill( char.pos ):
		char.socket.clilocmessage(1044491) # You must be near a flour mill to do that.
		return True
	if char.countresource(item.id) < WHEATAMOUNT:
		char.socket.clilocmessage(500997) # You need more wheat to make a sack of flour.
		return True
	char.useresource(WHEATAMOUNT, item.id)
	flour = wolfpack.additem(random.choice(flours))
	char.socket.sysmessage(tr("You use some of the wheat and make a sack of flour."))
	char.soundeffect(0x57)
	if not wolfpack.utilities.tobackpack( flour, char ):
		flour.update()
	return True
