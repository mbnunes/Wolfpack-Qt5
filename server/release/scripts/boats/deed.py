#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts		        #
#  ((    _/{  "-;  | Created by: Correa			        #
#   )).-' {{ ;'`   | Revised by: see svn logs			#
#  ( (  ;._ \\ ctr | Last Modification: see svn logs	        #
#===============================================================#
# Boat deeds    					        #
#===============================================================#

from wolfpack import tr, console
import wolfpack
from wolfpack.consts import *
import boats
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
		player.socket.clilocmessage(1042001) # That must be in your pack for you to use it.
		return False

	# We may only own one house (Same for gamemasters -> registry)
	boat = boats.findBoat(player)
	if boat:
		player.socket.clilocmessage(501271) # You already own a house, you may not place another!
		return False

	# Does this deed have a multi section assigned to it?
	if not item.hastag('multisection'):
		player.socket.sysmessage(tr('This deed is broken.'))
		return False

	return True

#
# Do the basic checks first
#
def onUse(player, item):
	if not checkDeed(player, item):
		return True

	(dispid, xoffset, yoffset, zoffset) = getPlacementData(str(item.gettag('multisection')))

	if dispid == 0:
		player.socket.sysmessage(tr('This deed is broken. No disp ID'))
		return True

	player.socket.clilocmessage( 502482 ) # Where do you wish to place the ship?
	player.socket.attachmultitarget("boats.deed.placement", dispid - 0x4000, [item.serial, dispid, xoffset, yoffset, zoffset], xoffset, yoffset, zoffset)
	return True

#
# Create boat and items
#
def createBoat( player, deed, pos ):
	boat = wolfpack.addmulti(str(deed.gettag('multisection')))
        if boat == None:
                player.socket.sysmessage(tr('This deed is broken. Failed to create boat'))

	boat.owner = player
	boat.settag( 'boat_anchored', 1 )
	boat.moveto(pos)
	boat.update()
	boats.registerBoat(boat)

        # Create special items
        node = wolfpack.getdefinition(WPDT_MULTI, str(deed.gettag('multisection')) )
	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'special_items': # Found section
                        subsubnode = subnode.findchild('tillerman')
                        if subsubnode != None:
                                offsets = subsubnode.findchild('offsets')
                                xoffsets = int( offsets.getattribute( 'x', '0' ) )
                                yoffsets = int( offsets.getattribute( 'y', '0' ) )
                                player.socket.sysmessage('xoffsets = %i, yoffsets = %i' % (xoffsets, yoffsets) )
                                tillerman = wolfpack.additem('3e4e')
                                itempos = wolfpack.coord( pos.x + xoffsets, pos.y + yoffsets, pos.z, pos.map )
                                tillerman.moveto( itempos )
                                tillerman.update()
                                tillerman.settag('boat_serial', boat.serial)
                        subsubnode = subnode.findchild('hold')
                        if subsubnode != None:
                                offsets = subsubnode.findchild('offsets')
                                xoffsets = int( offsets.getattribute( 'x', '0' ) )
                                yoffsets = int( offsets.getattribute( 'y', '0' ) )
                                hold = wolfpack.additem('3eae')
                                itempos = wolfpack.coord( pos.x + xoffsets, pos.y + yoffsets, pos.z, pos.map )
                                hold.moveto( itempos )
                                hold.update()
                        subsubnode = subnode.findchild('planks')
                        if subsubnode != None:
                                portclosed = subsubnode.findchild('port_closed')
                                offsets = portclosed.findchild('offsets')
                                xoffsets = int( offsets.getattribute( 'x', '0' ) )
                                yoffsets = int( offsets.getattribute( 'y', '0' ) )
                                pplank = wolfpack.additem('3eb1')
                                itempos = wolfpack.coord( pos.x + xoffsets, pos.y + yoffsets, pos.z, pos.map )
                                pplank.moveto( itempos )
                                pplank.update()
                                pplank.settag( 'boat_serial', boat.serial )
                                starclosed = subsubnode.findchild('star_closed')
                                offsets = starclosed.findchild('offsets')
                                xoffsets = int( offsets.getattribute( 'x', '0' ) )
                                yoffsets = int( offsets.getattribute( 'y', '0' ) )
                                splank = wolfpack.additem('3eb2')
                                itempos = wolfpack.coord( pos.x + xoffsets, pos.y + yoffsets, pos.z, pos.map )
                                splank.moveto( itempos )
                                splank.update()
                                splank.settag( 'boat_serial', boat.serial )
                                
                                
#
# Target
#
def placement(player, arguments, target):
	deed = wolfpack.finditem(arguments[0])
	(dispid, xoffset, yoffset, zoffset) = arguments[1:] # Get the rest of the arguments

	if not checkDeed(player, deed):
		return	

        region = wolfpack.region( target.pos.x, target.pos.y, target.map )
        if region.cave:
		player.socket.clilocmessage( 502488 ) # You can not place a ship inside a dungeon.
		return
                
	(canplace, moveout) = wolfpack.canplaceboat(target.pos, dispid - 0x4000)

	if not canplace:
		player.socket.clilocmessage( 1043284 ) # A ship can not be created here.
		return

        createBoat( player, deed, target.pos )
	deed.delete()

	for obj in moveout:
		obj.removefromview()
		obj.moveto(player.pos)
		obj.update()
		if obj.ischar() and obj.socket:
			obj.socket.resendworld()
	return
