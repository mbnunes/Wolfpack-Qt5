
import wolfpack
from wolfpack.consts import *
import random

# Baseid: ClilocBaseId, MaxQuantity, Empty Item Id
CONTAINERS = {
	0x9c8: [1042965, 10, 0], # Jug
	0x99f: [1042959, 5, 0], # Bottle of Ale
	0x99b: [1042959, 5, 0], # Bottle of Liquor
	0x9c7: [1042959, 5, 0], # Bottle of Wine
	
	# Ceramic Mugs
	0x995: [1042982, 1, 0x995],
	0x996: [1042982, 1, 0x995],
	0x997: [1042982, 1, 0x995],
	0x998: [1042982, 1, 0x995],
	0x999: [1042982, 1, 0x995],
	0x9CA: [1042982, 1, 0x995],
	
	# Pewter Mugs
	0xFFF: [1042994, 1, 0xFFF],
	0x1000: [1042994, 1, 0xFFF],
	0x1001: [1042994, 1, 0xFFF],
	0x1002: [1042994, 1, 0xFFF],
	
	# Goblet
	0x99A: [1043000, 1, 0x99A],
	0x9B3: [1043000, 1, 0x99A],
	0x9BF: [1043000, 1, 0x99A],
	0x9CB: [1043000, 1, 0x99A],
	
	# Glass Mug
	0x1F7D: [1042976, 5, 0x1F81],
	0x1F7E: [1042976, 5, 0x1F81],
	0x1F7F: [1042976, 5, 0x1F81],
	0x1F80: [1042976, 5, 0x1F81],
	0x1F81: [1042976, 5, 0x1F81],
	0x1F82: [1042976, 5, 0x1F81],
	0x1F83: [1042976, 5, 0x1F81],
	0x1F84: [1042976, 5, 0x1F81],
	0x1F85: [1042976, 5, 0x1F81],
	0x1F86: [1042976, 5, 0x1F81],
	0x1F87: [1042976, 5, 0x1F81],
	0x1F88: [1042976, 5, 0x1F81],
	0x1F89: [1042976, 5, 0x1F81],
	0x1F8A: [1042976, 5, 0x1F81],
	0x1F8B: [1042976, 5, 0x1F81],
	0x1F8C: [1042976, 5, 0x1F81],
	0x1F8D: [1042976, 5, 0x1F81],
	0x1F8E: [1042976, 5, 0x1F81],
	0x1F8F: [1042976, 5, 0x1F81],
	0x1F90: [1042976, 5, 0x1F81],
	0x1F91: [1042976, 5, 0x1F81],
	0x1F92: [1042976, 5, 0x1F81],
	0x1F93: [1042976, 5, 0x1F81],
	0x1F94: [1042976, 5, 0x1F81],
	0x9EE: [1042976, 5, 0x1F81],
	0x9EF: [1042976, 5, 0x1F81],
	
	# Pitcher
	0x9a7: [1048128, 5, 0xFF6],
	0xff6: [1048128, 5, 0xFF6],
	0xff7: [1048128, 5, 0xFF6],
	0xff8: [1048128, 5, 0xFF6],
	0xff9: [1048128, 5, 0xFF6],
	0x1f95: [1048128, 5, 0xFF6],
	0x1f96: [1048128, 5, 0xFF6],
	0x1f97: [1048128, 5, 0xFF6],
	0x1f98: [1048128, 5, 0xFF6],
	0x1f99: [1048128, 5, 0xFF6],
	0x1f9a: [1048128, 5, 0xFF6],
	0x1f9b: [1048128, 5, 0xFF6],
	0x1f9c: [1048128, 5, 0xFF6],
	0x1f9d: [1048128, 5, 0xFF6],
	0x1f9e: [1048128, 5, 0xFF6],
}

# Tag: Cliloc Offset, Intoxication Strength
FLUIDS = {
	'ale': [0, 3],
	'cider': [1, 7],
	'liquor': [2, 9],
	'milk': [3, 0],
	'wine': [4, 5],
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
# Intoxication
#
def intoxication(char, args):
	intoxication = 0
	if char.hastag('intoxication'):
		intoxication = int(char.gettag('intoxication'))
		
	intoxication -= 1
		
	# Otherwise no effect
	if char.socket:	
		if intoxication >= (char.strength + 9) / 10:
			if random.randint(0, 3) == 0 and not char.itemonlayer(LAYER_MOUNT):
				char.direction = random.randint(0, 7)
				char.update()				
				
				char.action(ANIM_BOW)
		
			if random.randint(0, 1) == 0:
				char.say('*hic*', 0x3B2)
				
			# Loose Mana and Stamina
			char.stamina = max(0, char.stamina - intoxication)
			char.updatestamina()
			char.mana = max(0, char.mana - intoxication)
			char.updatemana()
				
		if intoxication == max(0, ((char.strength + 9) / 10) - 1):
			char.socket.clilocmessage(500850)			

	if intoxication > 0:
		char.addtimer(3000, 'beverage.intoxication', [], 1, 0, 'intoxication')
		char.settag('intoxication', intoxication)
	else:
		char.deltag('intoxication')

#
# Take a nip of the nice fluid in the container
#
def drink(char, item):
	quantity = 0
	if item.hastag('quantity'):
		quantity = int(item.gettag('quantity'))

	btype = ''
	if item.hastag('fluid'):
		btype = unicode(item.gettag('fluid'))
		
	if not FLUIDS.has_key(btype):
		char.socket.sysmessage("You shouldn't drink this strange fluid.")
		return 1
		
	if not CONTAINERS.has_key(item.id):
		return 0
		
	cprop = CONTAINERS[item.id]
	fprop = FLUIDS[btype]

	if quantity > 0:
		# Drink
		char.soundeffect(0x30)
		char.action(ANIM_FIDGET3)
		
		# Intoxication
		if fprop[1] > 0:
			intoxication = 0
			if char.hastag('intoxication'):
				intoxication = int(char.gettag('intoxication'))
				char.dispel(None, 1, 'intoxication')
			
			intoxication += fprop[1]
			char.settag('intoxication', intoxication)
			char.addtimer(3000, 'beverage.intoxication', [], 1, 0, 'intoxication')
	
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
			item.deltag('fluid')
			item.resendtooltip()
	else:
		item.settag('quantity', int(quantity))
		item.resendtooltip()
		
	return 1	

#
# Show the tooltip for the item
#
def onShowTooltip(player, object, tooltip):
	if not CONTAINERS.has_key(object.id):
		return
		
	cprop = CONTAINERS[object.id]
	
	btype = ''
	if object.hastag('fluid'):
		btype = unicode(object.gettag('fluid'))
	
	if FLUIDS.has_key(btype):
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
