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
from wolfpack.utilities import hex2dec, tobackpack, tobankbox, createkeysecret

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

def createBoatSpecialItem( definition, parentTag, boat ):
    offsets = parentTag.findchild( 'offsets' )
    xoffset = int( offsets.getattribute( 'x', '0' ) )
    yoffset = int( offsets.getattribute( 'y', '0' ) )
    item = wolfpack.additem( definition )
    itempos = wolfpack.coord( boat.pos.x + xoffset, boat.pos.y + yoffset, boat.pos.z, boat.pos.map )
    item.moveto( itempos )
    item.update()
    item.decay = 0
    item.settag( 'boat_serial', boat.serial )
    if not boat.hastag( 'boat_part_count' ):
        boat.settag( 'boat_part_count', 1 )
        boat.settag( 'boat_part1', item.serial )
    else:
        i = int( boat.gettag('boat_part_count') )
        i = i + 1
        boat.settag( 'boat_part_count', i )
        boat.settag( 'boat_part%i' % i, item.serial )

    return item

def createKeys( plank1, plank2, hold, boat, player ):
    rkeyid = createkeysecret()
    packkey = wolfpack.additem('1010')
    packkey.settag('lock', rkeyid)
    packkey.name = "a ship key"
    packkey.settag('recall.link', boat.serial)
    tobackpack( packkey, player )
    packkey.update()
    player.say( 502485, socket = player.socket ) # A ship's key is now in my backpack.
    bankkey = wolfpack.additem('1010')
    bankkey.settag('lock', rkeyid)
    bankkey.name = "a ship key"
    bankkey.settag('recall.link', boat.serial)
    tobankbox( bankkey, player )
    player.say( 502484, socket = player.socket ) # A ship's key is now in my safety deposit box.
    # Plank Section
    plank1.settag('lock', rkeyid)
    plank1.addscript( 'lock' )
    plank1.settag('locked',1)
    plank2.settag('lock', rkeyid)
    plank2.addscript( 'lock' )
    plank2.settag('locked',1)
    # Hold Section
    hold.settag('lock', rkeyid)
    hold.addscript( 'lock' )
    hold.settag('locked',1)

def applyKeys( plank1, plank2, hold, deed ):
    # Get KeyID
    rkeyid = deed.gettag('lock')
    # Plank Section
    plank1.settag('lock', rkeyid)
    plank1.addscript( 'lock' )
    plank1.settag('locked',1)
    plank2.settag('lock', rkeyid)
    plank2.addscript( 'lock' )
    plank2.settag('locked',1)
    # Hold Section
    hold.settag('lock', rkeyid)
    hold.addscript( 'lock' )
    hold.settag('locked',1)
	


#
# Create boat and items
#
def createBoat( player, deed, pos ):
	boat = wolfpack.addmulti(str(deed.gettag('multisection')))
        if boat == None:
                player.socket.sysmessage(tr('This deed is broken. Failed to create boat'))

	boat.owner = player
	boat.settag( 'boat_anchored', 1 )
	boat.settag( 'boat_facing', 0 ) # boat is facing north
	boat.settag( 'deedid', deed.baseid ) # For DryDock
	boat.moveto(pos)
	boat.update()
	boat.decay = 0

	if deed.hastag('hasname'):
		boat.name = deed.name

	splank = None
	pplank = None

        # Create special items
        node = wolfpack.getdefinition(WPDT_MULTI, str(deed.gettag('multisection')) )
	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'ids':
                    boat.settag('boat_id_north', hex2dec( subnode.getattribute( 'north', '0' ) ) )
                    boat.settag('boat_id_east', hex2dec( subnode.getattribute( 'east', '0' ) ) )                               
                    boat.settag('boat_id_south', hex2dec( subnode.getattribute( 'south', '0' ) ) )                               
                    boat.settag('boat_id_west', hex2dec( subnode.getattribute( 'west', '0' ) ) )                               
		elif subnode.name == 'special_items': # Found section
                        subsubnode = subnode.findchild('tillerman')
                        if subsubnode != None:
                                tillerman = createBoatSpecialItem( '3e4e', subsubnode, boat )
                                boat.settag('boat_tillerman', tillerman.serial)
				if deed.hastag('hasname'):
					tillerman.name = 'Tillerman of ' + boat.name
                        subsubnode = subnode.findchild('hold')
                        if subsubnode != None:
                                hold = createBoatSpecialItem( '3eae', subsubnode, boat )
                        subsubnode = subnode.findchild('planks')
                        if subsubnode != None:
                                portclosed = subsubnode.findchild('port_closed')
                                pplank = createBoatSpecialItem( '3eb1', portclosed, boat )
                                starclosed = subsubnode.findchild('star_closed')
                                splank = createBoatSpecialItem( '3eb2', starclosed, boat )
                                splank.settag('plank_starboard', 1)

	if not deed.hastag('lock'):
		createKeys( splank, pplank, hold, boat, player )
	else:
		applyKeys( splank, pplank, hold, deed )
                                
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

	if not canplace or player.pos.distance( target.pos ) > 30:
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
