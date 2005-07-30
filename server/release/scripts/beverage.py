
import wolfpack
from wolfpack.consts import *
import random
from system import poison
import skills
from skills.fishing import mapWater, staticWater
from wolfpack.utilities import tobackpack

# Baseid: ClilocBaseId, MaxQuantity, Empty Item Id, Fillable
CONTAINERS = {
	0x9c8: [1042965, 10, 0, False, ''], # Jug
	0x99f: [1042959, 5, 0, False, ''], # Bottle of Ale
	0x99b: [1042959, 5, 0, False, ''], # Bottle of Liquor
	0x9c7: [1042959, 5, 0, False, ''], # Bottle of Wine

	# Ceramic Mugs
	0x995: [1042982, 1, 0x995, True, ''],
	0x996: [1042982, 1, 0x995, True, ''],
	0x997: [1042982, 1, 0x995, True, ''],
	0x998: [1042982, 1, 0x995, True, ''],
	0x999: [1042982, 1, 0x995, True, ''],
	0x9CA: [1042982, 1, 0x995, True, ''],

	# Skull Mugs
	0xFFB: [1042988, 1, 0xFFB, True, ''],
	0xFFC: [1042988, 1, 0xFFC, True, ''],
	0xFFD: [1042988, 1, 0xFFD, True, ''],
	0xFFE: [1042988, 1, 0xFFE, True, ''],

	# Pewter Mugs
	0xFFF: [1042994, 1, 0xFFF, True, ''],
	0x1000: [1042994, 1, 0x1000, True, ''],
	0x1001: [1042994, 1, 0x1001, True, ''],
	0x1002: [1042994, 1, 0x1002, True, ''],

	# Goblet
	0x99A: [1043000, 1, 0x99A, True, ''],
	0x9B3: [1043000, 1, 0x9B3, True, ''],
	0x9BF: [1043000, 1, 0x9BF, True, ''],
	0x9CB: [1043000, 1, 0x9CB, True, ''],

	# Glass Mug
	0x1F7D: [1042976, 5, 0x1F81, True, 'cider'], # cider
	0x1F7E: [1042976, 5, 0x1F82, True, 'cider'], # cider
	0x1F7F: [1042976, 5, 0x1F83, True, 'cider'], # cider
	0x1F80: [1042976, 5, 0x1F84, True, 'cider'], # cider
	0x1F81: [1042976, 5, 0x1F81, True, ''], # empty
	0x1F82: [1042976, 5, 0x1F82, True, ''], # empty
	0x1F83: [1042976, 5, 0x1F83, True, ''], # empty
	0x1F84: [1042976, 5, 0x1F84, True, ''], # empty
	0x1F85: [1042976, 5, 0x1F81, True, 'liquor'], # liquor
	0x1F86: [1042976, 5, 0x1F82, True, 'liquor'], # liquor
	0x1F87: [1042976, 5, 0x1F83, True, 'liquor'], # liquor
	0x1F88: [1042976, 5, 0x1F84, True, 'liquor'], # liquor
	0x1F89: [1042976, 5, 0x1F81, True, 'milk'], # milk
	0x1F8A: [1042976, 5, 0x1F82, True, 'milk'], # milk
	0x1F8B: [1042976, 5, 0x1F83, True, 'milk'], # milk
	0x1F8C: [1042976, 5, 0x1F84, True, 'milk'], # milk
	0x1F8D: [1042976, 5, 0x1F81, True, 'wine'], # wine
	0x1F8E: [1042976, 5, 0x1F82, True, 'wine'], # wine
	0x1F8F: [1042976, 5, 0x1F83, True, 'wine'], # wine
	0x1F90: [1042976, 5, 0x1F84, True, 'wine'], # wine
	0x1F91: [1042976, 5, 0x1F81, True, 'water'], # water
	0x1F92: [1042976, 5, 0x1F82, True, 'water'], # water
	0x1F93: [1042976, 5, 0x1F83, True, 'water'], # water
	0x1F94: [1042976, 5, 0x1F84, True, 'water'], # water
	0x9EE: [1042976, 5, 0x1F82, True, 'ale'], # ale
	0x9EF: [1042976, 5, 0x1F83, True, 'ale'], # ale

	# Pitcher
	0x9a7: [1048128, 5, 0x9a7, True, ''], # empty
	0x9ad: [1048128, 5, 0x9a7, True, ''], # milk
	0xff6: [1048128, 5, 0xff6, True, ''], # empty
	0xff7: [1048128, 5, 0xff7, True, ''], # empty
	0xff8: [1048128, 5, 0xff7, True, 'water'], # water
	0xff9: [1048128, 5, 0xff6, True, 'water'], # water
	0x1f95: [1048128, 5, 0xff6, True, 'ale'], # ale
	0x1f96: [1048128, 5, 0xff7, True, 'ale'], # ale 
	0x1f97: [1048128, 5, 0xff6, True, 'cider'], # cider
	0x1f98: [1048128, 5, 0xff7, True, 'cider'], # cider 
	0x1f99: [1048128, 5, 0xff6, True, 'liquor'], # liquor 
	0x1f9a: [1048128, 5, 0xff7, True, 'liquor'], # liquor 
	0x1f9b: [1048128, 5, 0xff6, True, 'wine'], # wine
	0x1f9c: [1048128, 5, 0xff7, True, 'wine'], # wine
	0x1f9d: [1048128, 5, 0xff6, True, 'water'], # water
	0x1f9e: [1048128, 5, 0xff7, True, 'water'], # water
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

def consumeUse(item):
	quantity = 0
	if item.hastag('quantity'):
		quantity = int(item.gettag('quantity'))

	if quantity == 0:
		if CONTAINERS.has_key(item.id):
			cprops = CONTAINERS[item.id]

			if cprops[3]:
				char.socket.clilocmessage(500837)
				char.socket.attachtarget('beverage.refill_target', [item.serial])
				return True
		return False

#
# Use the item
#
def onUse(char, item):
	if item.getoutmostchar() != char:
		char.socket.clilocmessage(502946)
		return True

	# poisoned drink
	if item.hastag( 'poisoning_char' ):
		poison.poison( char, item.gettag( 'poisoning_strength' ) )
		char.socket.clilocmessage( 1010512 )
		skills.poisoning.wearoff( item )

	quantity = 0
	if item.hastag('quantity'):
		quantity = int(item.gettag('quantity'))

	if quantity == 0:
		if CONTAINERS.has_key(item.id):
			cprops = CONTAINERS[item.id]

			if cprops[3]:
				char.socket.clilocmessage(500837)
				char.socket.attachtarget('beverage.refill_target', [item.serial])
				return True
		return False
	# pitcher filled with water
	elif item.baseid in ["1f9d", "1f9e"] and CONTAINERS[item.id][4] == 'water':
		char.socket.attachtarget('beverage.water', [item.serial])
	else:
		return drink(char, item)
	return True

def water(char, args, target):
	item = wolfpack.finditem( args[0] )
	if not item:
		return False
	if target.char and target.char == char:
		return drink(char, item)
	# Bowl Flour -> make dough
	elif target.item and target.item.baseid == 'a1e':
		if not target.item.getoutmostchar() == char:
			char.socket.clilocmessage( 1042001 ) # That must be in your pack for you to use it.
			return True
		dough = wolfpack.additem( "103d" )
		if not tobackpack( dough, char ):
			dough.update()
		wooden_bowl = wolfpack.additem( "15f8" )
		if not tobackpack( wooden_bowl, char ):
			wooden_bowl.update()
		target.item.delete()
		consume(item)
	return True

#
# Find a valid container item id for the given fluid
# 
def updateItemIdFromFluid(item, fluid):
	if not FLUIDS.has_key(fluid):
		return

	origid = item.id

	# Search the container list
	for (key, value) in CONTAINERS.items():
		# Refillable, EmptyId of the container are we, fluid is the same
		if value[3] and value[2] == origid and value[4] == fluid:
			item.id = key
			item.update()
			return True

	return False

#
# Refill the container with something
#
def refill_target(char, args, target):
	item = wolfpack.finditem(args[0])
	if not item:
		return

	if not CONTAINERS.has_key(item.id):
		return

	cprops = CONTAINERS[item.id]

	if not char.canreach(target.pos, 3):
		char.socket.clilocmessage(1019045)
		return

	if target.item:
		if CONTAINERS.has_key(target.item.id):
			quantity = 0
			if target.item.hastag('quantity'):
				quantity = int(target.item.gettag('quantity'))

			fluid = ''			
			if target.item.hastag('fluid'):
				fluid = str(target.item.gettag('fluid'))

			if quantity > 0 and FLUIDS.has_key(fluid):
				item.settag('fluid', fluid)

				# How much can we refill?
				if quantity > cprops[1]:
					quantity -= cprops[1]
					target.item.settag('quantity', quantity)
					target.item.resendtooltip()
					item.settag('quantity', cprops[1])					
					updateItemIdFromFluid(item, 'water')
					item.resendtooltip()

				# The source will be depleted
				else:
					item.settag('quantity', quantity)
					item.resendtooltip()

					cprop = CONTAINERS[target.item.id]										
					if cprop[2] == 0:
						target.item.delete()
					else:
						target.item.id = cprop[2]
						target.item.update()
						target.item.deltag('quantity')
						target.item.deltag('fluid')
						target.item.resendtooltip()

				return # We refilled

		# A watersource
		if target.item.watersource:
			# Check if its depletable
			quantity = cprops[1]
			if target.item.hastag('quantity'):
				quantity = int(item.gettag('quantity'))

			if quantity > 0:
				item.settag('fluid', 'water')
				item.settag('quantity', quantity)
				updateItemIdFromFluid(item, 'water')
				item.resendtooltip()
		return

	# Check map if its a water source
	elif not target.char:
		mapTile = wolfpack.map( target.pos.x, target.pos.y, target.pos.map )
		if target.model in staticWater or mapTile[ "id" ] in mapWater:
			quantity = cprops[1]
			if quantity > 0:
				item.settag('fluid', 'water')
				item.settag('quantity', quantity)
				updateItemIdFromFluid(item, 'water')
				item.resendtooltip()

#
# Intoxication
#
def intoxication_func(char, args):
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
		char.addtimer(3000, intoxication_func, [], 1, 0, 'intoxication')
		char.settag('intoxication', intoxication)
	else:
		char.deltag('intoxication')

#
# Consume one unit of this beverage
#
def consume(item):
	quantity = 0
	if item.hastag('quantity'):
		quantity = int(item.gettag('quantity'))

	if quantity <= 0:
		return False # Couldn't consume

	btype = ''
	if item.hastag('fluid'):
		btype = unicode(item.gettag('fluid'))

	if not FLUIDS.has_key(btype):
		return True

	if not CONTAINERS.has_key(item.id):
		return False

	cprop = CONTAINERS[item.id]
	fprop = FLUIDS[btype]

	quantity -= 1

	# Empty
	if quantity <= 0:
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
	return True

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
		return True

	if not CONTAINERS.has_key(item.id):
		return False

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
			char.addtimer(3000, intoxication_func, [], 1, 0, 'intoxication')

		# Reduce Quantity
		quantity -= 1

	# Empty
	char.socket.sysmessage( str(quantity))
	char.socket.sysmessage( str(cprop[2]))
	if quantity == 0:
		if cprop[2] == 0:
			char.socket.sysmessage( "S" )
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

	return True	

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

	if object.name == '' and FLUIDS.has_key(btype):
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
