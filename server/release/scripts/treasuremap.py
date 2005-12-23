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
from treasurechest import treasmonsterspawn

####################################################################################
###################     Coords      ################################################
####################################################################################

TREASURECOORDS = [
	[2643,851],
	[2672,392],
	[3404, 238],
	[3369, 637],
	[2740, 435],
	[2770, 345],
	[2781, 289],
	[2836, 233],
	[3014, 250],
	[3082, 202],
	[3246, 245],
	[3375, 458],
	[1314, 317],
	[1470, 229],
	[1504, 366],
	[2340, 645],
	[2350, 688],
	[2395, 723],
	[2433, 767],
	[1319, 889],
	[1414, 771],
	[1529, 753],
	[1555, 806],
	[1511, 967],
	[1510, 1071],
	[1562, 1058],
	[961, 506],
	[1162, 189],
	[2337, 1159],
	[2392, 1154],
	[2517, 1066],
	[2458, 1042],
	[1658, 2030],
	[2062, 1962],
	[2089, 1987],
	[2097, 1975],
	[2066, 1979],
	[2058, 1992],
	[2071, 2007],
	[2093, 2006],
	[2187, 1991],
	[1689, 1993],
	[1709, 1964],
	[1726, 1998],
	[1731, 2016],
	[1743, 2028],
	[1754, 2020],
	[2033, 1942],
	[2054, 1962],
	[581, 1455],
	[358, 1337],
	[208, 1444],
	[199, 1461],
	[348, 1565],
	[620, 1706],
	[1027, 1180],
	[1037, 1975],
	[1042, 1960],
	[1042, 1903],
	[1034, 1877],
	[1018, 1859],
	[980, 1849],
	[962, 1858],
	[977, 1879],
	[968, 1884],
	[969, 1893],
	[974, 1992],
	[989, 1992],
	[1024, 1990],
	[2648, 2167],
	[2956, 2200],
	[2968, 2170],
	[2957, 2150],
	[2951, 2177],
	[2629, 2221],
	[2642, 2289],
	[2682, 2290],
	[2727, 2309],
	[2782, 2293],
	[2804, 2255],
	[2850, 2252],
	[2932, 2240],
	[3568, 2402],
	[3702, 2825],
	[3511, 2421],
	[3533, 2471],
	[3417, 2675],
	[3425, 2723],
	[3476, 2761],
	[3541, 2784],
	[3558, 2819],
	[3594, 2825],
	[4419, 3117],
	[4471, 3188],
	[4507, 3227],
	[4496, 3241],
	[4642, 3369],
	[4694, 3485],
	[4448, 3130],
	[4453, 3148],
	[4500, 3108],
	[4513, 3103],
	[4424, 3152],
	[4466, 3209],
	[4477, 3230],
	[4477, 3282],
	[2797, 3452],
	[2803, 3488],
	[2793, 3519],
	[2831, 3511],
	[2989, 3606],
	[3035, 3603],
	[1427, 2405],
	[1466, 2181],
	[1519, 2214],
	[1523, 2150],
	[1541, 2115],
	[1534, 2189],
	[1546, 2221],
	[1595, 2193],
	[1619, 2236],
	[1654, 2268],
	[1655, 2304],
	[1433, 2381],
	[1724, 2288],
	[1703, 2318],
	[1772, 2321],
	[1758, 2333],
	[1765, 2430],
	[1647, 2641],
	[1562, 2705],
	[1670, 2808],
	[1471, 2340],
	[1562, 2312],
	[1450, 2301],
	[1437, 2294],	
	[1478, 2273],
	[1464, 2245],
	[1439, 2217],
	[1383, 2840],
	[1388, 2984],
	[1415, 3059],
	[1602, 3012],
	[1664, 3062],
	[2062, 2144],
	[2178, 2151],
	[2104, 2123],
	[2098, 2101],
	[2123, 2120],
	[2130, 2108],
	[2129, 2132],
	[2153, 2120],
	[2162, 2148],
	[2186, 2144],
	[492, 2027],
	[477, 2044],
	[451, 2053],
	[468, 2087],
	[465, 2100],
	[958, 2504],
	[1025, 2702],
	[1290, 2735],
	[2013, 3269],
	[2149, 3362],
	[2097, 3384],
	[2039, 3427],
	[2516, 3999],
	[2453, 3942],
	[2528, 3982],
	[2513, 3962],
	[2512, 3918],
	[2513, 3901],
	[2480, 3908],
	[2421, 3902],
	[2415, 3920],
	[2422, 3928],
	[2370, 3428],
	[2341, 3482],
	[2360, 3507],
	[2387, 3505],
	[2152, 3950],
	[2157, 3924],
	[2140, 3940],
	[2143, 3986],
	[2154, 3983],
	[2162, 3988],
	[2467, 3581],
	[2527, 3585],
	[2482, 3624],
	[2535, 3608],
	[1090, 3110],
	[1094, 3131],
	[1073, 3133],
	[1076, 3156],
	[1068, 3182],
	[1096, 3178],
	[1129, 3403],
	[1135, 3445],
	[1162, 3469],
	[1128, 3500]
]

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

	# Lets set the member and, if possible, the party leader for this chest
	item.settag('owner', char.serial)
	if char.party:
		item.settag('party', char.party.leader.serial)

	# Spawn some creatures
	treasmonsterspawn( item )