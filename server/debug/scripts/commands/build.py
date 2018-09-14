"""
	\command build
	\description Fills a certain area with a given pattern.
	\usage - <code>build id z</code>
	Z is the height where the items should be created.
	Use .build to find out which build ids are there.
"""

import wolfpack
from wolfpack.consts import *
import random

# Place an item at the given position
def placeItem(x, y, z, mapid, itemid):
	if itemid == None:
		return
	elif type(itemid) == tuple:
		itemid = random.choice(itemid)
	elif type(itemid) == list:
		itemid = random.choice(itemid)

	item = wolfpack.additem(itemid)
	item.moveto(wolfpack.coord(x, y, z, mapid))
	item.movable = 2
	item.decay = False
	item.update()

# Build functions
# Arguments is a list of strings/lists/Nones in the following order:
# Upper Left, North, Upper Right, Left, Middle (Tiled), Right, Lower Left, Bottom, Lower Right
def buildFloor(x1, x2, y1, y2, z, mapid, args):
	# Place Corners
	placeItem(x1, y1, z, mapid, args[0]) # Upper Left
	if y2 != y1:
		placeItem(x1, y2, z, mapid, args[6]) # Lower Left
	if x1 != x2:
		placeItem(x2, y1, z, mapid, args[2]) # Upper Right
		if y2 != y1:
			placeItem(x2, y2, z, mapid, args[8]) # Lower Right

	# Place Horizonal Bars
	for x in range(x1 + 1, x2):
		placeItem(x, y1, z, mapid, args[1]) # Upper Bar
		if y2 != y1:
			placeItem(x, y2, z, mapid, args[7]) # Lower Bar

	# Place Vertical Bars
	for y in range(y1 + 1, y2):
		placeItem(x1, y, z, mapid, args[3]) # Left Bar
		if x2 != x1:
			placeItem(x2, y, z, mapid, args[5]) # Right Bar

	# Place tiled middle
	for x in range(x1 + 1, x2):
		for y in range(y1 + 1, y2):
			placeItem(x, y, z, mapid, args[4]) # Middle

# Build a red floor pattern
def redFloor(x1, x2, y1, y2, z, mapid, args):
	TABLE = ['4f9', '4f4', '4fa', '4f7', '4f3', '4f5', '4f8', '4f6', '4fb'] # 9 Entry for the 3x3 pattern

	for x in range(x1, x2 + 1):
		for y in range(y1, y2 + 2):
			xmod = (x - x1) % 3
			ymod = (y - y1) % 3
			itemid = TABLE[(ymod * 3 + xmod)]
			placeItem(x, y, z, mapid, itemid)

# Build a blue floor pattern
def blueFloor(x1, x2, y1, y2, z, mapid, args):
	TABLE = ['4ee', '4ed', '4ef', '4ec', '4f2', '4ea', '4f1', '4eb', '4f0'] # 9 Entry for the 3x3 pattern

	for x in range(x1, x2 + 1):
		for y in range(y1, y2 + 2):
			xmod = (x - x1) % 3
			ymod = (y - y1) % 3
			itemid = TABLE[(ymod * 3 + xmod)]
			placeItem(x, y, z, mapid, itemid)

# Build a standard wall with the given ids
# Arg List: Corner Bottom, Right/Left Wall, Top/Bottom Wall, North Pylon
def buildWall(x1, x2, y1, y2, z, mapid, args):	
	placeItem(x1, y1, z, mapid, args[3]) # Place Pylon first
	placeItem(x2, y2, z, mapid, args[0]) # Corner is second

	for y in range(y1 + 1, y2 + 1):
		placeItem(x1, y, z, mapid, args[1]) # Left/Right wall
		if y != y2:
			placeItem(x2, y, z, mapid, args[1]) # Left/Right wall

	for x in range(x1 + 1, x2 + 1):
		if x != x1:
			placeItem(x, y1, z, mapid, args[2]) # Left/Right wall
		if x != x2:
			placeItem(x, y2, z, mapid, args[2]) # Left/Right wall

def buildRoofE(x1, x2, y1, y2, z, mapid, args):
	for y in range(y1, y2 + 1):
		boff = y2 - y
		toff = y - y1

		# Which tile should we create?
		if boff == toff:
			itemid = args[2]
			tilez = z + toff * 3
		elif toff < boff:
			itemid = args[0]
			tilez = z + toff * 3
		else:
			itemid = args[1]
			tilez = z + boff * 3

		for x in range(x1, x2 + 1):
			placeItem(x, y, tilez, mapid, itemid) # Roof

def buildRoofN(x1, x2, y1, y2, z, mapid, args):
	for x in range(x1, x2 + 1):
		roff = x2 - x
		loff = x - x1

		# Which tile should we create?
		if roff == loff:
			itemid = args[2]
			tilez = z + loff * 3
		elif loff < roff:
			itemid = args[0]
			tilez = z + loff * 3
		else:
			itemid = args[1]
			tilez = z + roff * 3

		for y in range(y1, y2 + 1):
			placeItem(x, y, tilez, mapid, itemid) # Roof			

# Build a 3x3 field
def build3x3(x1, x2, y1, y2, z, mapid, args):
	if x2 != x1 or y2 != y1:
		raise AssertionError, "You have to select a single coordinate."

	for x in range(x1 - 1, x1 + 2):
		for y in range(y1 - 1, y1 + 2):
			xmod = (x - (x1 - 1)) % 3
			ymod = (y - (y1 - 1)) % 3
			itemid = args[(ymod * 3 + xmod)]
			placeItem(x, y, z, mapid, itemid)

# Build a 4x4 field
def build4x4(x1, x2, y1, y2, z, mapid, args):
	if x2 != x1 or y2 != y1:
		raise AssertionError, "You have to select a single coordinate."

	for x in range(x1 - 1, x1 + 3):
		for y in range(y1 - 1, y1 + 3):
			xmod = (x - (x1 - 1)) % 4
			ymod = (y - (y1 - 1)) % 4
			itemid = args[(ymod * 4 + xmod)]
			placeItem(x, y, z, mapid, itemid)			

# string id: callback, extra arguments
BUILD_TYPES = {
	'woodenboardse': (buildFloor, ('4b4', '4b0', '4b3', '4ad', ['4a9', '4aa', '4ab', '4ac'], '4af', '4b2', '4ae', '4b1')),
	'woodenboardsn': (buildFloor, ('4c0', '4bc', '4bf', '4b9', ['4b5', '4b6', '4b7', '4b8'], '4bb', '4be', '4ba', '4bd')),

	'woodenplankse': (buildFloor, ('4d1', ['4cb', '4cc'], '4d3', '4cf', ['4c6', '4c7', '4c8', '4c9'], '4ce', '4d0', ['4ca', '4cd'], '4d2')),
	'woodenplanksn': (buildFloor, ('4df', ['4d9', '4da'], '4e1', '4dd', ['4d4', '4d5', '4d6', '4d7'], '4dc', '4de', ['4d8', '4db'], '4e0')),

	'woodenlogse1': (buildFloor, ('504', '506', '507', '504', '506', '507', '504', '506', '507')),
	'woodenlogse2': (buildFloor, ('508', '506', '505', '508', '506', '505', '508', '506', '505')),

	'woodenlogsn1': (buildFloor, ('4c3', '4c3', '4c3', '4c1', '4c1', '4c1', '4c2', '4c2', '4c2')),
	'woodenlogsn2': (buildFloor, ('4c5', '4c5', '4c5', '4c1', '4c1', '4c1', '4c4', '4c4', '4c4')),

	# Carpets
	'bluecarpet1a': (buildFloor, ('ac3', 'af7', 'ac5', 'af6', 'abe', 'af8', 'ac4', 'af9', 'ac2')),
	'bluecarpet1b': (buildFloor, ('ac3', 'af7', 'ac5', 'af6', 'abd', 'af8', 'ac4', 'af9', 'ac2')),	
	'bluecarpet1c': (buildFloor, ('ac3', 'af7', 'ac5', 'af6', 'abf', 'af8', 'ac4', 'af9', 'ac2')),	
	'bluecarpet1d': (buildFloor, ('ac3', 'af7', 'ac5', 'af6', 'ac0', 'af8', 'ac4', 'af9', 'ac2')),	
	'bluecarpet2': (buildFloor, ('ad3', 'ad7', 'ad5', 'ad6', 'ad1', 'ad8', 'ad4', 'ad9', 'ad2')),
	'bluecarpet3a': (buildFloor, ('aef', 'af3', 'af1', 'af2', 'aec', 'af4', 'af0', 'af5', 'aee')),
	'bluecarpet3b': (buildFloor, ('aef', 'af3', 'af1', 'af2', 'aed', 'af4', 'af0', 'af5', 'aee')),
	'browncarpet': (buildFloor, ('adc', 'ae0', 'ade', 'adf', 'ada', 'ae1', 'add', 'ae2', 'adb')),

	'redcarpet1a': (buildFloor, ('aca', 'ace', 'acc', 'acd', 'ac8', 'acf', 'acb', 'ad0', 'ac9')),
	'redcarpet1b': (buildFloor, ('aca', 'ace', 'acc', 'acd', 'ac6', 'acf', 'acb', 'ad0', 'ac9')),
	'redcarpet1c': (buildFloor, ('aca', 'ace', 'acc', 'acd', 'ac7', 'acf', 'acb', 'ad0', 'ac9')),
	'redcarpet2': (buildFloor, ('ae4', 'ae8', 'ae6', 'ae7', 'aeb', 'ae9', 'ae5', 'aea', 'ae3')),
	'grayrug': (buildFloor, ('1df9', ['1df1', '1df2'], '1dfa', ['1df7', '1df8'], ['1def', '1df0'], ['1df3', '1df4'], '1dfc', ['1df5', '1df6'], '1dfb')),
	'greenrug1': (buildFloor, ('ab8', 'ab4', 'ab9', 'ab7', 'ab3', 'ab5', 'abb', 'ab6', 'aba')),
	'greenrug2': (buildFloor, ('1deb', ['1de3', '1de4'], '1dec', ['1de9', '1dea'], ['1de1', '1de2'], ['1de5', '1de6'], '1dee', ['1de7', '1de8'],  '1ded')),
	'greenrug3': (buildFloor, (	['1deb', 'ab8'], ['1de3', '1de4', 'ab4'], ['1dec', 'ab9'], ['1de9', '1dea', 'ab7'], ['1de1', '1de2', 'ab3'], ['1de5', '1de6', 'ab5'], ['1dee', 'abb'], ['1de7', '1de8', 'ab6'],  ['1ded', 'aba'])),

	# 3x3 matrix
	'bearskinn' : (build3x3, ('1e3e', '1e3d', '1e3c', '1e39', '1e3a', '1e3b', '1e38', '1e37', '1e36')),
	'bearskine' : (build3x3, ('1e48', '1e43', '1e42', '1e47', '1e44', '1e41', '1e46', '1e45', '1e40')),

	'polarskinn' : (build3x3, ('1e51', '1e50', '1e4f', '1e4c', '1e4d', '1e4e', '1e4b', '1e4a', '1e49')),
	'polarskine' : (build3x3, ('1e5b', '1e56', '1e55', '1e5a', '1e57', '1e54', '1e59', '1e58', '1e53')),

	'abbatoir' : (build3x3, ('120e', '120f', '1210', '1215', '1216', '1211', '1214', '1213', '1212')),

	# Fountains (4x4)
	'stonefountain': (build4x4, ('1741', '173f', '1738', '1737', 
								 '173d', '173e', '1739', '1736',
								 '173c', '173b', '173a', '1735',
								 '1731', '1732', '1733', '1734')),

	'sandstonefountain': (build4x4, ('19d3', '19d1', '19ca', '19c9', 
								 '19cf', '19d0', '19cb', '19c8',
								 '19ce', '19cd', '19cc', '19c7',
								 '19c3', '19c4', '19c5', '19c6')),								 

	'redfloor':	(redFloor, ()),
	'bluefloor': (blueFloor, ()),

	'wall1': (buildWall, ('1a', '1b', '1c', '1d')),
	'wall1small': (buildWall, ('24', '26', '25', '27')),
	'wall1tiny': (buildWall, ('2d', '2e', '2f', '30')),

	'wall2': (buildWall, ('59', '57', '58', '5a')),
	'wall2small': (buildWall, ('61', '60', '5f', '62')),
	'wall2smaller': (buildWall, ('65', '64', '63', '66')),
	'wall2tiny': (buildWall, ('6b', '6a', '69', '6c')),

	# Works like a floor
	'wall3small': (buildFloor, ('2ce', '2d0', '2d4', '2d3', None, '2d5', '2d1', '2d2', '2cf')),	

	'wall4': (buildWall, ('c7', 'c9', 'c8', 'cc')),
	'wall4small': (buildWall, ('dc', 'dd', 'de', 'df')),

	'wall5': (buildWall, ('1cf', '1d1', '1d0', '1d2')),
	'wall5small': (buildWall, ('1e8', '1ea', '1e9', '1eb')),

	'woodenwall1': (buildFloor, ('9', '7', 'c', '8', None, '8', 'd', '7', '6')),		
	'woodenwall1small': (buildWall, ('10', '11', '12', '13')),	
	'woodenwall1tiny': (buildWall, ('14', '15', '16', '17')),

	'woodenwall2': (buildWall, ('a6', 'a7', 'a8', 'a9')),			
	'woodenwall2small': (buildWall, ('b6', 'b8', 'b7', '13')),	
	'woodenwall2tiny': (buildWall, ('bd', 'be', 'bf', 'c0')),

	'plasterwall1': (buildWall, (['127', '12b', '132', '135'], ['12f', '131', '137', '137', '137'], ['12e', '130', '136', '136', '136'], '12a')),
	'plasterwall1wrecked': (buildWall, (['127', '12b', '132', '135', '37f', '383', '38a', '38d', '392'], ['387', '389', '38f', '38f', '38f', '394', '396'], ['386', '388', '38e', '38e', '38e', '393', '395'], '12a')),

	'brickwall': (buildWall, ('33', '35', '34', '36')),			
	'brickwallsmall': (buildWall, ('3d', '3f', '3e', '40')),	
	'brickwalltiny': (buildWall, ('41', '43', '42', '44')),

	# Roofs (Yay!)
	'slateroofe': (buildRoofE, (['217e', '217f', '5a3'], ['217c', '217d', '5a2'], '5a1')),
	'slateroofn': (buildRoofN, (['217a', '217b', '597'], ['2178', '2179', '596'], '595')),
	'stonetileroofe': (buildRoofE, ('193c', '193a', '193d')),
	'stonetileroofn': (buildRoofN, ('193b', '1939', '193e')),
	'thatchroofe': (buildRoofE, (['214e', '214f', '5a9'], ['2148', '2149', '5a7'], '5a8')),
	'thatchroofn': (buildRoofN, (['214a', '214b', '5a6'], ['214c', '214d', '5a4'], '5a5')),
	'tileroofe': (buildRoofE, (['21ef', '21f0', '5c1'], ['21ed', '21ee', '5c0'], '5bf')),
	'tileroofn': (buildRoofN, (['21eb', '21ec', '5b5'], ['21e9', '21ea', '5b4'], '5b3')),
	'woodenshinglese': (buildRoofE, (['2176', '2177', '5d0'], ['2172', '2173', '5cf'], '5ce')),
	'woodenshinglesn': (buildRoofN, (['2174', '2175', '5c4'], ['2170', '2171', '5c3'], '5c2')),	
	'palmroofe': (buildRoofE, (['2186', '2187', '594'], ['2184', '2185', '592'], '593')),
	'palmroofn': (buildRoofN, (['2182', '2183', '591'], ['2180', '2181', '58f'], '590')),		
	'logroofe': (buildRoofE, ('5fe', '5fd', '5fc')),
	'logroofn': (buildRoofN, ('5f2', '5f1', '5f0')),
	'hayroofe': (buildRoofE, ('26ec', '26eb', '26ea')),
	'hayroofn': (buildRoofN, ('26dc', '26db', '26da')),	
	'woodenslatee': (buildRoofE, ('28da', '28d9', '28d8')),
	'woodenslaten': (buildRoofN, ('28ca', '28c9', '28c8')),	
	'barkroofe': (buildRoofE, ('270a', '2709', '2708')),
	'barkroofn': (buildRoofN, ('26fa', '26f9', '26f8')),	
}

#
# Gather another corner if neccesary and start tiling
#
def buildResponse(player, arguments, target):
	if len(arguments) < 3:
		player.socket.sysmessage('Please select the second corner.')
		player.socket.attachtarget("commands.build.buildResponse", list(arguments) + [target.pos])
		return

	x1 = min(arguments[2].x, target.pos.x)
	x2 = max(arguments[2].x, target.pos.x)
	y1 = min(arguments[2].y, target.pos.y)
	y2 = max(arguments[2].y, target.pos.y)
	z = arguments[0]
	id = arguments[1]

	unlimited = player.account.authorized('Misc', 'Unlimited Tile')
	count = ((x2 - x1) + 1) * ((y2 - y1) + 1)

	# Cap at 250 items if not an admin is using it
	if not unlimited and count > 250:
		player.socket.sysmessage('You are not allowed to tile more than 250 items at once.')
		return

	try:
		BUILD_TYPES[id][0](x1, x2, y1, y2, z, player.pos.map, BUILD_TYPES[id][1])	
		player.log(LOG_MESSAGE, "Building %u items (%s) from %u,%u to %u,%u at z=%d.\n" % (count, id, x1, y1, x2, y2, z))
		player.socket.sysmessage('Building %u items from %u,%u to %u,%u at z=%d.' % (count, x1, y1, x2, y2, z))		
	except AssertionError, e:
		player.socket.sysmessage(str(e))
	except:
		raise

#
# Gather arguments and validate id list.
#
def commandBuild(socket, command, arguments):
	if arguments.count(' ') != 1:
		socket.sysmessage('Usage: .build [%s] z' % ", ".join(BUILD_TYPES.keys()))
		return

	(id, z) = arguments.split(' ')

	try:
		z = int(z)
	except:
		socket.sysmessage('Invalid z value.')
		return

	if id not in BUILD_TYPES:
		socket.sysmessage('Usage: .build [%s] z' % ", ".join(BUILD_TYPES.keys()))
		return

	socket.sysmessage('Please select the first corner.')
	socket.attachtarget('commands.build.buildResponse', [z, id])

#
# Register the command
#
def onLoad():
	wolfpack.registercommand("build", commandBuild)
