
from wolfpack import tr, console
import wolfpack
from wolfpack.consts import *
import housing
from wolfpack.utilities import hex2dec

#
# Get placement information from the multi definition
#
def getPlacementData(definition, dispid = 0, xoffset = 0, yoffset = 0, zoffset = 0):
	node = wolfpack.getdefinition(WPDT_MULTI, definition)
	
	if not node:
		return (dispid, xoffset, yoffset, zoffset) # Return, wrong definition
		
	if node.hasattribute('inherit'):
		(dispid, xoffset, yoffset, zoffset) = getPlacementData(node.getattribute('inherit'), dispid, xoffset, yoffset, zoffset)
	
	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'id': # Found the display id
			dispid = hex2dec(subnode.value)
		elif subnode.name == 'inherit': # Inherit another definition
			if subnode.hasattribute('id'):
				(dispid, xoffset, yoffset, zoffset) = getPlacementData(subnode.getattribute('id'), dispid, xoffset, yoffset, zoffset)
			else:
				(dispid, xoffset, yoffset, zoffset) = getPlacementData(subnode.value, dispid, xoffset, yoffset, zoffset)
		elif subnode.name == 'placement': # Placement info
			xoffset = hex2dec(subnode.getattribute('xoffset', '0'))
			yoffset = hex2dec(subnode.getattribute('yoffset', '0'))
			zoffset = hex2dec(subnode.getattribute('zoffset', '0'))
				
	return (dispid, xoffset, yoffset, zoffset)

#
# Checks if the deed is ok
#
def checkDeed(player, item):
	# Has to be in our belongings
	if not player.canreach(item, -1):
		player.socket.clilocmessage(1042001)
		return False
		
	# We may only own one house (Same for gamemasters -> registry)
	house = housing.findHouse(player)
	if house:
		player.socket.clilocmessage(501271)
		return False
	
	# Does this deed have a multi section assigned to it?
	if not item.hastag('section'):
		player.socket.sysmessage(tr('This deed is broken.'))
		return False
	
	return True

#
# Do the basic checks first
#
def onUse(player, item):
	if not checkDeed(player, item):
		return True
	
	(dispid, xoffset, yoffset, zoffset) = getPlacementData(str(item.gettag('section')))
	
	if dispid == 0:
		player.socket.sysmessage(tr('This deed is broken.'))
		return True
	
	player.socket.clilocmessage(1010433)
	player.socket.attachmultitarget("housing.deed.placement", dispid - 0x4000, [item.serial], xoffset, yoffset, zoffset)
	return True

#
# Target
#
def placement(player, arguments, target):
	deed = wolfpack.finditem(arguments[0])
	if not checkDeed(player, deed):
		return

	if not player.canreach(target.pos, 20):
		player.socket.sysmessage('You can\'t reach that.')
		return
		
	house = wolfpack.addmulti(str(deed.gettag('section')))
	house.owner = player
	house.moveto(target.pos)
	house.update()
	
	housing.registerHouse(house)

