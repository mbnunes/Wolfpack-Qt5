
import wolfpack
from wolfpack import console
from wolfpack.consts import *
from wolfpack.utilities import hex2dec
import housing

#
# Is the given house public or private?
#
def isPublic(house):
	return True # TODO: Public or Private

#
# Register house
#
def onAttach(house):
	housing.registerHouse(house)

#
# Unregister house
#
def onDetach(house):
	housing.unregisterHouse(house)

#
# Create the housing items
#
def onCreate(house, definition):
	buildHouse(house, definition)

#
# Is the given character the owner of the given house?
#
def isOwner(house, player):
	if not house:
		return False
	
	if player.gm or house.owner == player:
		return True
	else:
		return False

#
# Place items in the house based on a given house definition
#
def buildHouse(house, definition):
	node = wolfpack.getdefinition(WPDT_MULTI, definition)
	
	if not node:
		return
		
	if node.hasattribute('inherit'):
		value = str(node.getattribute('inherit'))
		buildHouse(house, value) # Recursion

	for i in range(0, node.childcount):
		child = node.getchild(i)

		# Inherit another definition
		if child.name == 'inherit':
			if child.hasattribute('id'):
				buildHouse(house, child.getattribute('id'))
			else:
				buildHouse(house, child.value)

		# Add a normal item to the house		
		elif child.name == 'item':
			x = int(child.getattribute('x', '0'))
			y = int(child.getattribute('y', '0'))
			z = int(child.getattribute('z', '0'))
			id = str(child.getattribute('id', ''))
			
			item = wolfpack.additem(id)
			item.moveto(house.pos.x + x, house.pos.y + y, house.pos.z + z, house.pos.map)
			item.update()
			
		# Add a house door to the house
		elif child.name == 'door':
			x = int(child.getattribute('x', '0'))
			y = int(child.getattribute('y', '0'))
			z = int(child.getattribute('z', '0'))
			id = hex2dec(child.getattribute('id', ''))

			item = wolfpack.additem('housedoor')
			item.id = id
			item.moveto(house.pos.x + x, house.pos.y + y, house.pos.z + z, house.pos.map)
			item.update()

		# Add a sign to the house
		elif child.name == 'sign':
			x = int(child.getattribute('x', '0'))
			y = int(child.getattribute('y', '0'))
			z = int(child.getattribute('z', '0'))
			
			sign = wolfpack.additem('housesign')
			sign.moveto(house.pos.x + x, house.pos.y + y, house.pos.z + z, house.pos.map)
			sign.update()

#
# Check if access in this house is allowed to the given item
#
def onCheckSecurity(player, house, item):
	if not item.hasscript('housing.security'):
		return False
	else:
		level = housing.security.getLevel(item)
		
		if not checkAccess(player, house, level):
			if item.type == 1: # Container
				player.socket.clilocmessage(501647) # It's secure
			else:
				player.socket.clilocmessage(1061637) # It's not accessable
			
			return True
			
	return False

#
# Checks if the given player is an owner of the given house
# NOTE: This does not check higher access levels.
#
def isOwner(player, house):
	return player == house.owner
	
#
# Checks if the given player is an co-owner of the given house
# NOTE: This does not check higher access levels.
#
def isCoOwner(player, house):
	return False
	
#
# Checks if the given player is a friend of the house.
# NOTE: This does not check higher access levels.
#
def isFriend(player, house):
	return False

#
# Check the access level of the given char for this house
#
def checkAccess(player, house, level):
	if not player or not house or level < 0 or level > 3:
		return False # Error Checks
		
	if player.gm or level == ACCESS_ANYONE:
		return True
		
	if level >= ACCESS_OWNER and isOwner(player, house):
		return True
	elif level >= ACCESS_COOWNER and isCoOwner(player, house):
		return True
	elif level >= ACCESS_FRIEND and isFriend(player, house):
		return True
	
	return False
