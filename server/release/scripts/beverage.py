
import wolfpack
from wolfpack.consts import *
import random

# Baseid: ClilocBaseId, MaxQuantity, Empty Item Id
CONTAINERS = {
	0x9c8: [1042965, 10, 0], # Jug
}

# Tag: Cliloc Offset, Intoxication Strength
FLUIDS = {
	'ale': [0, 1],
	'cider': [1, 3],
	'liquor': [2, 4],
	'milk': [3, 0],
	'wine': [4, 2],
	'water': [5, 0],
}

#
# Use the item
#
def onUse(char, item):
	if item.getoutmostchar() != char:
		char.socket.clilocmessage(502946)
		return 1
	
	quantity = 0
	if item.hastag('quantity'):
		quantity = int(item.gettag('quantity'))
	
	if quantity == 0:
		return refill(char, item)
	else:
		return drink(char, item)	

#
# Refill the container with something
#
def refill(char, item):	
	return 0
	
#
# Take a nip of the nice fluid in the container
#
def drink(char, item):
	quantity = 0
	if item.hastag('quantity'):
		quantity = int(item.gettag('quantity'))
		
	if quantity == 0:
		char.socket.sysmessage('This seems to be empty.')
		return 1
	
	btype = ''
	if item.hastag('fluid'):
		btype = unicode(item.gettag('fluid'))
		
	if not FLUIDS.has_key(btype):
		char.socket.sysmessage("You shouldn't drink this strange fluid.")
		return 0
		
	if not CONTAINERS.has_key(item.id):
		return 0
		
	cprop = CONTAINERS[item.id]
	fprop = FLUIDS[btype]

	# Drink
	char.soundeffect(0x30)
	char.action(ANIM_FIDGET3)

	# Reduce Quantity
	quantity -= 1
	
	# Empty
	if quantity == 0:
		if cprop[2] == 0:
			item.delete()
		else:
			item.id = cprop[2]
			item.update()
			item.deltag('quantity')
			item.resendtooltip()
	else:
		item.settag('quantity', int(quantity))
		item.resendtooltip()
		
	return 1	

#
# Show the tooltip for the item
#
def onShowTooltip(player, object, tooltip):
	btype = ''
	if object.hastag('fluid'):
		btype = unicode(object.gettag('fluid'))
	
	if not FLUIDS.has_key(btype):
		return
		
	if not CONTAINERS.has_key(object.id):
		return
		
	cprop = CONTAINERS[object.id]
	fprop = FLUIDS[btype]
			
	tooltip.reset()
	tooltip.add(cprop[0] + fprop[0], '')
	
	quantity = 0
	if object.hastag('quantity'):
		quantity = int(object.gettag('quantity'))
		
	perc = (quantity * 100) / cprop[1]

	if perc <= 0:
		tooltip.add(1042975, '')
	elif perc <= 33:
		tooltip.add(1042974, '')
	elif perc <= 66:
		tooltip.add(1042973, '')
	else:
		tooltip.add(1042972, '')
