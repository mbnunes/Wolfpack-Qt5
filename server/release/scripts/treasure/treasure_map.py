#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: MagnusBr                       #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

####################################################################################
###################     Imports      ###############################################
####################################################################################

import wolfpack
import random
from wolfpack import tr

from map import sendmapcommand
from wolfpack.consts import CARTOGRAPHY, ANIM_ATTACK3

from treasure.treasure_coords import TREASURECOORDS
from treasure.treasure_spawn import treasinitialspawn
from treasure.treasure_templates import filltreasure

#####################################################################################
#######################   onCreate   ################################################
#####################################################################################

def onCreate(item, id):

	# Get Level
	level = item.gettag('level')

	# Get a point in list
	(x,y) = random.choice(TREASURECOORDS)	

	item.settag('x', x)
	item.settag('y', y)

#####################################################################################
#######################   onUse   ###################################################
#####################################################################################

def onUse(player, item):
	# Has to belong to us.
	if item.getoutmostchar() != player:
		player.socket.clilocmessage(500685)
		return 1

	# Already finished?
	if item.hastag('founded'):
		player.socket.sysmessage('Someone already founded this treasure!')
		return 1

	# Its decoded?
	if item.hastag('decoded'):
		sendtreasmap(player, item)
	else:
		trydecodemap(player, item)
	
	return 1

#####################################################################################
##################   Trying to Decode the Map   #####################################
#####################################################################################

def trydecodemap(player, item):
	# Get Level
	level = int(item.gettag('level'))

	# Now lets try to decode
	if not level:
		player.socket.sysmessage("You decoded the map!")
		item.settag('decoded', 1)
		item.name = "tattered treasure map"
	elif level == 1:
		if player.skill[CARTOGRAPHY] >= 270:
			player.socket.sysmessage("You decoded the map!")
			item.settag('decoded', 1)
			item.name = "tattered treasure map"
		else:
			player.socket.sysmessage("You cannot decode this map yet!")
	elif level == 2:
		if player.skill[CARTOGRAPHY] >= 710:
			player.socket.sysmessage("You decoded the map!")
			item.settag('decoded', 1)
			item.name = "tattered treasure map"
		else:
			player.socket.sysmessage("You cannot decode this map yet!")
	elif level == 3:
		if player.skill[CARTOGRAPHY] >= 810:
			player.socket.sysmessage("You decoded the map!")
			item.settag('decoded', 1)
			item.name = "tattered treasure map"
		else:
			player.socket.sysmessage("You cannot decode this map yet!")
	elif level == 4:
		if player.skill[CARTOGRAPHY] >= 910:
			player.socket.sysmessage("You decoded the map!")
			item.settag('decoded', 1)
			item.name = "tattered treasure map"
		else:
			player.socket.sysmessage("You cannot decode this map yet!")
	elif level == 5 or level == 6:
		if player.skill[CARTOGRAPHY] >= 1000:
			player.socket.sysmessage("You decoded the map!")
			item.settag('decoded', 1)
			item.name = "tattered treasure map"
		else:
			player.socket.sysmessage("You cannot decode this map yet!")
	
#####################################################################################
#######################   Sending Map to Client   ###################################
#####################################################################################

def sendtreasmap(player, item):

	# Get tags from map
	x = item.gettag('x')
	y = item.gettag('y')
	
	# Get Parameters
	width = 200
	height = 200
	xtop = x - 300
	ytop = y - 300
	xbottom = x + 300
	ybottom = y + 300

	# Send a map detail packet
	details = wolfpack.packet(0x90, 19)
	details.setint(1, item.serial)
	details.setshort(5, 0x139d)
	details.setshort(7, xtop) # Upper Left X
	details.setshort(9, ytop) # Upper Left Y
	details.setshort(11, xbottom) # Lower Right X
	details.setshort(13, ybottom) # Lower Right Y
	details.setshort(15, width) # Gump Width
	details.setshort(17, height) # Gump Height
	details.send(player.socket)

	# Remove all pins
	sendmapcommand(player.socket, item, 5)

	# Send all pins anew
	sendmapcommand(player.socket, item, 1, 0, 100, 100)

	# You cant edit this map
	sendmapcommand(player.socket, item, 7, 0)

#####################################################################################
#######################   Check for Treasure Point   ################################
#####################################################################################

def checktreaspoint(x, y, z, map, item, char):

	# Just Felluca and Trammel (At least for a while)
	if map > 1:
		return 0
	else:
		xtreas = item.gettag('x')
		ytreas = item.gettag('y')
		level = item.gettag('level')

		# Checking if we had correct point
		if xtreas == x and ytreas == y:
			if not item.hastag('founded'):
				item.settag('founded', 1)
				# Animation
				char.action( ANIM_ATTACK3 )
				# Dirt
				dirt = wolfpack.additem("911")
				dirt.addtimer( 1500, generatechest, [x,y,z,map,char,level])
				dirt.moveto(x, y, z, map)
				dirt.update()
				# Returning
				return 1
			else:
				return 0
		else:
			return 0

#####################################################################################
#######################   Timer for Dirt   ##########################################
#####################################################################################

def generatechest(item, args):

	x = args[0]
	y = args[1]
	z = args[2]
	map = args[3]
	char = args[4]
	level = args[5]

	item.delete()

	createtreaschest(x, y, z, map, char, level)

#####################################################################################
#######################   Create Treasure Chest   ###################################
#####################################################################################

def createtreaschest(x, y, z, map, char, level):

	item = wolfpack.additem("treasure chest")
	item.settag('level', level)
	item.moveto(x, y, z, map)
	item.update()

	# Lets fill the Chest with correct template
	filltreasure( item, level )

	# Lets set the member and, if possible, the party leader for this chest
	item.settag('owner', char.serial)
	if char.party:
		item.settag('party', char.party.leader.serial)

	# Lets set the Lock difficult fot this Chest
	if level == 0:
		item.removescript( 'lock' )
		item.deltag('lock')
		item.deltag('locked')
		item.resendtooltip()
	elif level == 1:
		item.settag('lockpick_difficult', 360)
	elif level == 2:
		item.settag('lockpick_difficult', 760)
	elif level == 3:
		item.settag('lockpick_difficult', 840)
	elif level == 4:
		item.settag('lockpick_difficult', 920)
	else:
		item.settag('lockpick_difficult', 1000)

	# Spawn some creatures
	treasinitialspawn( item )