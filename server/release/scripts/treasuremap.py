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
from wolfpack import tr

from map import sendmapcommand
from wolfpack.consts import CARTOGRAPHY

#####################################################################################
#######################   onCreate   ################################################
#####################################################################################

def onCreate(item, id):

	# Get Level
	level = item.gettag('level')

	# Set temporary Point (Just for test)
	item.settag('x', 1200)
	item.settag('y', 1500)

#####################################################################################
#######################   onUse   ###################################################
#####################################################################################

def onUse(player, item):
	# Has to belong to us.
	if item.getoutmostchar() != player:
		player.socket.clilocmessage(500685)
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