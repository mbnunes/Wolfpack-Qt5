#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Utility functions used in Python scripts                      #
#===============================================================#

"""
	\library wolfpack.utilities
	\description Contains several useful functions for scripts.
"""

# Calculates the Magic Damage (Base Damage + Base + Source)
import wolfpack
from wolfpack import properties
from wolfpack.consts import *
import random
from types import *

"""
	\function wolfpack.utilities.rolldice
	\param dice
	\param sides
	\param bonus
	\return Integer
	\description Rolls some dice and returns a result.
"""
def rolldice(dice, sides, bonus=0):
	result = 0
	for i in range(0, dice):
		result += random.randint(1, sides)
	result += bonus
	return result

"""
	\function wolfpack.utilities.hex2dec
	\param value
	\param default
	\return Integer
	\description Converts a given hex value into a decimal value.
"""
def hex2dec(value, default = 0):
	if type( value ) is IntType:
		return value

	try:
		value = value.lower()

		if not value.startswith( "0x" ):
			return int( value )
		else:
			return int( value.replace( "0x", "" ), 16 )
	except:
		return default

"""
	\function wolfpack.utilities.evenorodd
	\param value
	\return "even" or "odd"
	\description Takes a given int value and determines if it is even or odd.
"""
def evenorodd( value ):
	if value % 2 == 0:
		return "even"
	else:
		return "odd"

"""
	\function wolfpack.utilities.itemsmatch
	\param item1
	\param item2
	\return Boolean
	\description Determines if the items match for stacking.
"""
def itemsmatch(a, b):
	return a.canstack(b)

"""
	\function wolfpack.utilities.tobackpack
	\param item
	\param char
	\return Boolean
	\description Moves an object into a character's backpack.
"""
def tobackpack(item, char):
	backpack = char.getbackpack()
	return tocontainer(item, backpack)

"""
	\function wolfpack.utilities.tocontainer
	\param item
	\param container
	\return Boolean
	\description Moves an item to a container, stacks if possible.
"""
def tocontainer( item, container ):
	for content in container.content:
		# Found an item to stack with
		if itemsmatch(content, item):
			if content.amount + item.amount <= 60000:
				content.amount = content.amount + item.amount
				content.update()
				item.delete()
				return 1 # Stacked

	# We couldn't stack
	container.additem( item, 1, 1, 0 )
	return 0 # Not stacked

"""
	\function wolfpack.utilities.cont2cont
	\param container1
	\param container2
	\description Moves objects from container1 to container2.
"""
def cont2cont( container1, container2 ):
	for item in container1.content:
		tocontainer( item, container2 )
		item.update()

"""
	\function wolfpack.utilities.isclothing
	\param item
	\return Boolean
	\description Returns if the item is a known clothing.
"""
def isclothing( item ):

	clothes = [ 0x152e, 0x152f, 0x1530, 0x1531, 0x152e, 0x1537, 0x1538, 0x1539, \
			0x153a, 0x153b, 0x153c, 0x153d, 0x153e, 0x1541, 0x1542, 0x1712, \
			0x170b, 0x170c, 0x170d, 0x170e, 0x170f, 0x1710, 0x1711, 0x1712, \
			0x1f00, 0x1f01, 0x1f02, 0x1f03, 0x1f04, 0x1f7b, 0x1f7c, 0x1f9f, \
			0x1fa0, 0x1fa1, 0x1fa2, 0x1ffd, 0x1ffe, 0x2649, 0x264a, 0x265f, \
			0x2660, 0x2661, 0x2662, 0x2663, 0x2664, 0x2665, 0x2666, 0x267b, \
			0x267c, 0x267d, 0x267e, 0x267f, 0x2680, 0x2681, 0x2682, 0x2683, \
			0x2684, 0x2685, 0x2686, 0x268d, 0x268e, 0x268f, 0x26b0, 0x1515, \
			0x1516, 0x1517, 0x1518, 0x1efe, 0x1eff ]

	if item.id in clothes:
		return 1
	else:
		return 0

"""
	\function wolfpack.utilities.ishat
	\param item
	\return Boolean
	\description Returns if the item is a known hat.
"""
def ishat ( item ):

	hats = [ 0x153f, 0x1540, 0x1543, 0x1544, 0x1545, 0x1546, 0x1547, 0x1548, \
		0x1713, 0x1714, 0x1715, 0x1716, 0x1717, 0x1718, 0x1719, 0x171a, \
		0x171b, 0x171c ]

	if item.id in hats:
		return 1
	else:
		return 0

"""
	\function wolfpack.utilities.isarmor
	\param item
	\return Boolean
	\description Returns if the item is a known armor.
"""
def isarmor( item ):

	armors = [ 0x13be, 0x13bf, 0x13c0, 0x13c3, 0x13c4, 0x13c5, 0x13c6, 0x13c7, 0x13cb, \
			0x13cc, 0x13cd, 0x13ce, 0x13d2, 0x13d3, 0x13d4, 0x13d5, 0x13d6, \
			0x13da, 0x13db, 0x13dc, 0x13dd, 0x13e1, 0x13e2, 0x13d2, 0x13eb, \
			0x13ec, 0x13ed, 0x13ee, 0x13ef, 0x13f0, 0x13f1, 0x13f2, 0x1408, \
			0x1409, 0x140a, 0x140b, 0x140c, 0x140d, 0x140e, 0x140f, 0x1410, \
			0x1411, 0x1412, 0x1413, 0x1414, 0x1415, 0x1416, 0x1417, 0x1418, \
			0x1419, 0x141a, 0x144e, 0x144f, 0x1450, 0x1451, 0x1452, 0x1453, \
			0x1454, 0x1455, 0x1456, 0x1457, 0x2642, 0x2643, 0x2644, 0x2645, \
			0x2658, 0x2642, 0x2641, 0x2648, 0x2657, 0x1c00, 0x1c01, 0x1c02, \
			0x1c03, 0x1c04, 0x1c05, 0x1c06, 0x1c07, 0x1c08, 0x1c09, 0x1c0a, \
			0x1c0b, 0x1c0c, 0x1c0d, 0x1f0b, 0x1f0c, 0x1549, 0x154a, 0x154b, \
			0x154c, 0x2646, 0x2647, 0x13bb ]

	if item.id in armors:
		return 1
	else:
		return 0

"""
	\function wolfpack.utilities.isweapon
	\param item
	\return Boolean
	\description Returns if the item is a known weapon.
"""
def isweapon( item ):

	weapontypes = [ 1001, 1002, 1003, 1004, 1005, 1006, 1007 ]

	if item.type in weapontypes:
		return 1
	else:
		return 0

"""
	\function wolfpack.utilities.isshield
	\param item
	\return Boolean
	\description Returns if the item is a known shield.
"""
def isshield( item ):
	shields = [ 0x1b72, 0x1b73, 0x1b74, 0x1b75, 0x1b76, 0x1b77, 0x1b78, \
		0x1b79, 0x1b7a, 0x1b7b, 0x1bc3, 0x1bc4, 0x1bc5 ]

	if item.id in shields:
		return 1
	else:
		return 0

"""
	\function wolfpack.utilities.isspellbook
	\param item
	\return Boolean
	\description Returns if the item is a known spell book.
"""
def isspellbook( item ):
	sbtypes = [ 9 ]
	sbids = [ 0xe3b, 0xefa, 0x2252, 0x2253 ]
	if item.id in sbids and item.type in sbtypes:
		return 1
	else:
		return 0

"""
	\function wolfpack.utilities.isinstrument
	\param item
	\return Boolean
	\description Returns if the item is a known instrument.
"""
def isinstrument( item ):
	return item.id in [ 0xE9C, 0xE9D, 0xE9E, 0xEB1, 0xEB2, 0xEB3, 0xEB4 ]

"""
	\function wolfpack.utilities.isminingtool
	\param item
	\return Boolean
	\description Returns if the item is a known mining tool.
"""
def isminingtool( item ):
	return item.id in [ 0xf39, 0xf3a, 0xe85, 0xe86 ]

"""
	\function wolfpack.utilities.isoregem
	\param item
	\return Boolean
	\description Returns if the item is a known ore gem.
"""
def isoregem( item ):
	return item.id == 0x1ea7 and item.hastag( 'resource' ) and item.gettag( 'resource' ) == 'ore'

"""
	\function wolfpack.utilities.iswoodgem
	\param item
	\return Boolean
	\description Returns if the item is a known wood gem.
"""
def iswoodgem( item ):
	return item.id == 0x1ea7 and item.hastag( 'resource' ) and item.gettag( 'resource' ) == 'wood'

"""
	\function wolfpack.utilities.ismountainorcave
	\param tile
	\return Boolean
	\description Returns if the tile is a mountain or cave tile.
"""
def ismountainorcave( tile ):
	return tile in range( 220, 232 ) + \
			range( 236, 248 ) + \
			range( 252, 264 ) + \
			range( 268, 280 ) + \
			range( 286, 298 ) + \
			range( 321, 325 ) + \
			range( 467, 475 ) + \
			range( 476, 488 ) + \
			range( 492, 496 ) + \
			range( 543, 580 ) + \
			range( 581, 614 ) + \
			range( 1010, 1011 ) + \
			range( 1339, 1360 ) + \
			range( 1361, 1364 ) + \
			range( 1741, 1758) + \
			range( 1771, 1791 ) + \
			range( 1801, 1825 ) + \
			range( 1831, 1855 ) + \
			range( 1861, 1885 ) + \
			range( 1981, 2005 ) + \
			range( 2028, 2034 ) + \
			range( 2100, 2106 ) + \
			range( 13121, 13178 ) + \
			range( 13354, 13356 ) + \
			range( 13361, 13371 ) + \
			range( 13625, 13638 ) + \
			range( 16233, 16245 )

"""
	\function wolfpack.utilities.issand
	\param tile
	\return Boolean
	\description Returns if the tile is a sand tile.
"""
def issand( tile ):
	return tile in range( 22, 63 ) + \
			range( 68, 76 ) + \
			range( 286, 302 ) + \
			range( 402, 403 ) + \
			range( 424, 428 ) + \
			range( 441, 466 ) + \
			range( 642, 646 ) + \
			range( 650, 658 ) + \
			range( 821, 829 ) + \
			range( 833, 837 ) + \
			range( 845, 853 ) + \
			range( 857, 861 ) + \
			range( 951, 959 ) + \
			range( 967, 971 ) + \
			range( 1447, 1459 ) + \
			range( 1611, 1619 ) + \
			range( 1623, 1627 ) + \
			range( 1635, 1643 ) + \
			range( 1647, 1651 )

"""
	\function wolfpack.utilities.iscrystal
	\param tile
	\return Boolean
	\description Returns if the tile is a crystal tile, AoS artwork.
"""
def iscrystal( tile ):
	return tile in range( 8710, 8750 )

"""
	\function wolfpack.utilities.istree
	\param tile
	\return Boolean
	\description Returns if the tile is a tree object..
"""
def istree( tile ):
	return tile in range( 3221, 3223 ) + \
			range( 3230, 3231 ) + \
			range( 3242, 3243 ) + \
			range( 3274, 3278 ) + \
			range( 3230, 3231 ) + \
			range( 3242, 3243 ) + \
			range( 3274, 3275 ) + \
			range( 3280, 3281 ) + \
			range( 3283, 3284 ) + \
			range( 3286, 3287 ) + \
			range( 3288, 3289 ) + \
			range( 3290, 3291 ) + \
			range( 3293, 3294 ) + \
			range( 3296, 3297 ) + \
			range( 3299, 3300 ) + \
			range( 3302, 3303 ) + \
			range( 3323, 3324 ) + \
			range( 3326, 3327 ) + \
			range( 3329, 3330 ) + \
			range( 3393, 3394 ) + \
			range( 3415, 3420 ) + \
			range( 3438, 3443 ) + \
			range( 3460, 3463 ) + \
			range( 3476, 3477 ) + \
			range( 3480, 3481 ) + \
			range( 3484, 3485 ) + \
			range( 3486, 3487 ) + \
			range( 3488, 3489 ) + \
			range( 3492, 3493 ) + \
			range( 3496, 3497 ) + \
			range( 4789, 4798 ) + \
			range( 8778, 8782 )

"""
	\function wolfpack.utilities.iswater
	\param tile
	\return Boolean
	\description Returns if the tile is a water tile.
"""
def iswater( tile ):
	return tile in range( 168, 172 ) + \
		range( 6038, 6067 ) + \
		range( 13422, 13446 ) + \
		range( 13456, 13484 ) + \
		range( 13493, 13526 )

"""
	\function wolfpack.utilities.isdirt
	\param item
	\return Boolean
	\description Returns if the tile is a dirt tile..
"""
def isdirt( tile ):
	return tile in range( 113, 125 ) + \
			range( 130, 168 ) + \
			range( 220, 228 ) + \
			range( 232, 236 ) + \
			range( 321, 336 ) + \
			range( 361, 377 ) + \
			range( 476, 496 ) + \
			range( 622, 634 ) + \
			range( 638, 642 ) + \
			range( 720, 857 ) + \
			range( 871, 891  ) + \
			range( 901, 937 ) + \
			range( 1035, 1054 ) + \
			range( 1351, 1366 ) + \
			range( 1431, 1447 ) + \
			range( 1571, 1594 ) + \
			range( 1779, 1787 ) + \
			range( 1911, 1937 ) + \
			range( 2444, 2495 ) + \
			range( 12788, 12796 ) + \
			range( 13683, 13695 ) + \
			range( 13742, 13746 )

"""
	\function wolfpack.utilities.cleartag
	\param self
	\param args[ char, tagname ]
	\description Removes a tagname from a character.
"""
def cleartag( self, args ):
	char = args[0]
	tagname = args[1]
	self.deltag( tagname )
	return OK

"""
	\function wolfpack.utilities.rusmsg
	\param player
	\param locmsg
	\description Sends a localized system message to a given player.
"""
def rusmsg( player, locmsg ):
	player.socket.sysmessage( unicode( locmsg, 'cp1251') )
	return OK

#
# Internal function for resource searches
#
"""
	\function wolfpack.utilities.checkresources
	\param container
	\param baseid
	\param amount
	\return Ineger
	\description Returns the amount of a resource to consume in a container from a given baseid.
"""
def checkresources(container, baseid, amount):
  for item in container.content:
    if item.baseid == baseid:
      amount -= item.amount
    else:
      amount = checkresources(item, baseid, amount)

    # Don't count negative values
    if amount <= 0:
      amount = 0
      break

  return amount

#
# Internal Recursive function for consuming resources.
# Return the amount left to consume.
#
"""
	\function wolfpack.utilities.consumeresourcesinternal
	\param container
	\param baseid
	\param amount
	\return Integer
	\description Consumes an amount of resources from a container with the given baseid.
"""
def consumeresourcesinternal(container, baseid, amount):
  for item in container.content:
    if item.baseid == baseid:
      # Enough to statisfy our consumption
      if item.amount > amount:
        item.amount -= amount
        item.update()
        return 0

      amount -= item.amount
      item.delete()
    else:
      amount = consumeresourcesinternal(item, baseid, amount)

  return amount

#
# Consume a certain amount of a certain resource.
# Return true if the resources have been consumed
# correctly.
#
"""
	\function wolfpack.utilities.consumeresources
	\param container
	\param baseid
	\param amount
	\return Integer or False
	\description Consumes the amount of resources from a container with a given baseid.
"""
def consumeresources(container, baseid, amount):
  if checkresources(container, baseid, amount) == 0:
    return consumeresourcesinternal(container, baseid, amount) == 0

  return 0

"""
	\function magic.utilities.energydamage
	\param target The target that is taking damage. Only characters are allowed.
	\param source The source of the damage. May be None.
	\param amount The amount of damage dealt.
	\param physical The physical fraction of the damage.
	\param fire The fire fraction of the damage.
	\param cold The cold fraction of the damage.
	\param poison The poison fraction of the damage.
	\param energy The energy fraction of the damage.
	\description Deal damage to a target and take the targets energy resistances into account.
"""
def energydamage(target, source, amount, physical=0, fire=0, cold=0, poison=0, energy=0, noreflect=0, damagetype=DAMAGE_MAGICAL):
	if not target:
		raise RuntimeError, "Invalid arguments for energydamage."

	amount = max(1, amount)

	if physical + fire + cold + poison + energy == 0:
		raise RuntimeError, "Invalid arguments for energydamage."

	damage = 0

	if physical > 0:
		physical = amount * (physical / 100.0)
		resistance = properties.fromchar(target, RESISTANCE_PHYSICAL) / 100.0
		damage = max(0, physical - (physical * resistance))

		if source and not noreflect and damage > 0:
			reflectphysical = properties.fromchar(target, REFLECTPHYSICAL)
			reflect = reflectphysical / 100.0 * damage

			if reflect > 0:
				energydamage(source, target, reflect, physical=100, noreflect=1)
				#target.effect(0x22c6, 10, 16)
				#target.movingeffect(0x22c6, source, 0, 0, 14)

	if fire > 0:
		fire = amount * (fire / 100.0)
		resistance = properties.fromchar(target, RESISTANCE_FIRE) / 100.0
		damage += max(0, fire - (fire * resistance))

	if cold > 0:
		cold = amount * (cold / 100.0)
		resistance = properties.fromchar(target, RESISTANCE_COLD) / 100.0
		damage += max(0, cold - (cold * resistance))

	if poison > 0:
		poison = amount * (poison / 100.0)
		resistance = properties.fromchar(target, RESISTANCE_POISON) / 100.0
		damage += max(0, poison - (poison * resistance))

	if energy > 0:
		energy = amount * (energy / 100.0)
		resistance = properties.fromchar(target, RESISTANCE_ENERGY) / 100.0
		damage += max(0, energy - (energy * resistance))

	damage = max(1, damage)
	target.damage(damagetype, damage, source)

"""
	\function wolfpack.utilities.createlockandkey
	\param item
	\description Makes a current container lockable and creates a key.
"""
def createlockandkey( container ):
	if container.type == 1:
		if not container.hastag('lock'):
			parts = ['A','B','C','D','E','F','G','H','J','K','L','M','N','P','Q','R','S','T','U','V','W','X','Y','Z','2','3','4','5','6','7','8','9']
			rkeyid = ''
			for i in range(1,8):
				rkeyid += random.choice(parts)
			key = wolfpack.additem('1010')
			key.settag('lock', rkeyid)
			tocontainer( key, container )
			container.settag( 'lock', rkeyid )
			if not 'lock' in container.events:
				events = container.events
				events[:0] = ['lock']
				container.events = events
	return

"""
	\function wolfpack.utilities.throwobject
	\param char
	\param object
	\param target
	\param sendobject
	\param speed
	\param fixeddir
	\param explodes
	\param hue
	\param rendermode
	\return none
	\description Animates a character's throwing of an object at a given target.
"""
def throwobject( char, object, target, sendobject=0, speed=10, fixeddir=0, explodes=0, hue=0, rendermode=0 ):
	char.turnto(target.pos)
	char.action(0x9)
	char.movingeffect(object.id, target.pos, fixeddir, explodes, speed, hue, rendermode)
	# This will make the object leave the character's pack and land at the target location.
	if sendobject > 0:
		char.say('Woooooooooo!')
		if target.char:
			object.moveto(target.char.pos)
		elif target.item and target.item.type != 1 and (not target.item.container):
			object.moveto( target.item.pos )
		else:
			object.moveto( target.pos )
		object.update()
	return

