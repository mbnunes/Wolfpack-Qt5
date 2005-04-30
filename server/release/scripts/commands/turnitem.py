
import wolfpack
from wolfpack import utilities
from wolfpack import properties

TURNABLES = {
	# Wooden Wall
	0xa: [0xc],
	0xc: [0xa],
	0xb: [0xd],
	0xd: [0xb],
	#incomplete...
	# Stone Wall
	0x1b: [0x1c],
	0x1c: [0x1b],
	0x1e: [0x21],
	0x21: [0x1e],
	0x1f: [0x20],
	0x20: [0x1f],
	0x22: [0x23],
	0x23: [0x22],
	0x25: [0x26],
	0x26: [0x25],
	0x29: [0x2c],
	0x2c: [0x29],
	0x2a: [0x2b],
	0x2b: [0x2a],
	# incomplete...
	# Marble Bench
	0x459: [0x45a],
	0x45a: [0x459],
	# Sandstone Bench
	0x45b: [0x45c],
	0x45c: [0x45b],
	# Small Crate
	0x9a9: [0xe7e],
	0xe7e: [0x9a9],
	# Wooden Chest
	0xe42: [0xe43],
	0xe43: [0xe42],
	# Chest of Drawers
	0xa2c: [0xa34],
	0xa34: [0xa2c],
	# Chest of Drawers fancy
	0xa30: [0xa38],
	0xa38: [0xa30],
	# Armoire
	0xa4c: [0xa50],
	0xa50: [0xa4e],
	0xa4e: [0xa52],
	0xa52: [0xa4c],
	# Red Armoire
	0xa4d: [0xa51],
	0xa51: [0xa4d],
	# Wooden Armoire
	0xa4f: [0xa53],
	0xa53: [0xa4f],
	# Bedroll
	0xa55: [0xa56],
	0xa56: [0xa55],
	# Bedroll 2
	0xa57: [0xa58],
	0xa58: [0xa59],
	0xa59: [0xa57],
	# Bookcases with books
	0xa97: [0xa98],
	0xa98: [0xa99],
	0xa99: [0xa9a],
	0xa9a: [0xa9b],
	0xa9b: [0xa9c],
	0xa9c: [0xa97],
	# Bookcases
	0xa9d: [0xa9e],
	0xa9e: [0xa9d],
	# Wooden Bench
	0xb2c: [0xb2d],
	0xb2d: [0xb2c],
	# Wooden Chair
	0xb2e: [0xb2f],
	0xb2f: [0xb30],
	0xb30: [0xb31],
	0xb31: [0xb2e],
	# Wooden Throne
	0xb32: [0xb33],
	0xb33: [0xb32],
	# Counter
	0xb3f: [0xb40],
	0xb40: [0xb3f],
	# Peg board
	0xc39: [0xc3a],
	0xc3a: [0xc39],
	# Small Wooden Table
	0xb34: [0xb35],
	0xb35: [0xb34],
	# Writing Table
	0xb49: [0xb4a],
	0xb4a: [0xb4b],
	0xb4b: [0xb4c],
	0xb4c: [0xb49],
	# Chair 1
	0xb4e: [0xb4f],
	0xb4f: [0xb50],
	0xb50: [0xb51],
	0xb51: [0xb4e],
	# Chair 2
	0xb52: [0xb53],
	0xb53: [0xb54],
	0xb54: [0xb55],
	0xb55: [0xb52],
	# Chair 3
	0xb56: [0xb57],
	0xb57: [0xb58],
	0xb58: [0xb59],
	0xb59: [0xb56],
	# Chair 4
	0xb5a: [0xb5b],
	0xb5b: [0xb5c],
	0xb5c: [0xb5d],
	0xb5d: [0xb5a],
	# Wooden Bench 2
	0xb5f: [0xb60],
	0xb60: [0xb61],
	0xb61: [0xb65],
	0xb65: [0xb66],
	0xb66: [0xb67],
	0xb67: [0xb5f],
	# Sandstone Bench 2
	0xb62: [0xb63],
	0xb63: [0xb64],
	0xb64: [0xb68],
	0xb68: [0xb69],
	0xb69: [0xb6a],
	0xb6a: [0xb62],
	# Sandstone Table
	0xb6b: [0xb6c],
	0xb6c: [0xb6d],
	0xb6d: [0xb7e],
	0xb7e: [0xb7f],
	0xb7f: [0xb80],
	0xb80: [0xb6b],
	# Wooden Table
	0xb6e: [0xb6f],
	0xb6f: [0xb81],
	0xb81: [0xb82],
	0xb82: [0xb6e],
	# Wooden Table 2
	0xb70: [0xb71],
	0xb71: [0xb72],
	0xb72: [0xb73],
	0xb73: [0xb74],
	0xb74: [0xb83],
	0xb83: [0xb84],
	0xb84: [0xb85],
	0xb85: [0xb86],
	0xb86: [0xb87],
	0xb87: [0xb70],
	# Wooden Table 3
	0xb75: [0xb76],
	0xb76: [0xb88],
	0xb88: [0xb89],
	0xb89: [0xb75],
	# Wooden Table 4
	0xb77: [0xb78],
	0xb78: [0xb79],
	0xb79: [0xb7a],
	0xb7a: [0xb7b],
	0xb7b: [0xb8a],
	0xb8a: [0xb8b],
	0xb8b: [0xb8c],
	0xb8c: [0xb8d],
	0xb8d: [0xb8e],
	0xb8e: [0xb77],
	# Wooden Table 5
	0xb7c: [0xb8f],
	0xb8f: [0xb7c],
	# Wooden Table 6
	0xb7d: [0xb90],
	0xb90: [0xb7d],
	# Wall Bench
	0xb91: [0xb92],
	0xb92: [0xb93],
	0xb93: [0xb94],
	0xb94: [0xb91],
	# Large Crate
	0xe3c: [0xe3d],
	0xe3d: [0xe3c],
	# Medium Crate
	0xe3e: [0xe3f],
	0xe3f: [0xe3e],
	# Small Crate
	0xe7e: [0x9a9],
	0x9a9: [0xe7e],
	# Wooden Box
	0xe7d: [0x9aa],
	0x9aa: [0xe7d],
	# Pure Metal Chest
	0x9ab: [0xe7c],
	0xe7c: [0x9ab],
	# Fancy Metal Chest
	0xe40: [0xe41],
	0xe41: [0xe40],
	# Gold Box
	0x9a8: [0xe80],
	0xe80: [0x9a8],
	# Table 1
	0xb7c: [0xb8f],
	0xb8f: [0xb7c],
	# Table 2
	0xb7d: [0xb90],
	0xb90: [0xb7d],
	# Wooden Bench
	0xb2c: [0xb2d],
	0xb2d: [0xb2c],
	# Wooden Throne
	0xb2e: [0xb2f],
	0xb2f: [0xb30],
	0xb30: [0xb31],
	0xb31: [0xb2e],
	# Fancy Throne
	0xb32: [0xb33],
	0xb33: [0xb32],
	# Big Crate
	0xe3c: [0xe3d],
	0xe3d: [0xe3c],
	# Medium Crate
	0xe3e: [0xe3f],
	0xe3f: [0xe3e],
	# Music Stand
	0xeb6: [0xeb8],
	0xeb8: [0xeb6],
	# Tall Music Stand
	0xebb: [0xebc],
	0xebc: [0xebb],
	# Dress Form
	0xec6: [0xec7],
	0xec7: [0xec6],
	# Wooden Box
	0xe7d: [0x9aa],
	0x9aa: [0xe7d],
	# Canvas
	0xf65: [0xf67],
	0xf67: [0xf69],
	0xf69: [0xf65],
	0xf66: [0xf68],
	0xf68: [0xf6a],
	0xf6a: [0xf66],
	# Globe
	0x1047: [0x1048],
	0x1048: [0x1047],
	# Loom Bench
	0x1049: [0x104a],
	0x104a: [0x1049],
	# Covered Counter
	0x118f: [0x1190],
	0x1190: [0x1191],
	0x1191: [0x1192],
	0x1192: [0x118f],
	# Bamboo Table
	0x11d6: [0x11d7],
	0x11d7: [0x11d8],
	0x11d8: [0x11d9],
	0x11d9: [0x11da],
	0x11da: [0x11db],
	0x11db: [0x11d6],
	# Log Table
	0x11dc: [0x11dd],
	0x11dd: [0x11de],
	0x11de: [0x11df],
	0x11df: [0x11e0],
	0x11e0: [0x11e1],
	0x11e1: [0x11dc],
	# Covered Table
	0x1667: [0x1668],
	0x1668: [0x1669],
	0x1669: [0x166a],
	0x166a: [0x166b],
	0x166b: [0x166c],
	0x166c: [0x1667], 
	# Stone Table
	0x1201: [0x1202],
	0x1202: [0x1203],
	0x1203: [0x1204],
	0x1204: [0x1205],
	0x1205: [0x1206],
	0x1206: [0x1201],
	# Stone Bench
	0x1207: [0x1208],
	0x1208: [0x1209],
	0x1209: [0x1207],
	# Stone Throne
	0x1218: [0x1219],
	0x1219: [0x121a],
	0x121a: [0x121b],
	0x121b: [0x1218],
	# Lord British Throne
	0x1526: [0x1527],
	0x1527: [0x1526],
	# Bar Door
	0x190e: [0x190f],
	0x190f: [0x190e],
	# Massage Table
	0x1916: [0x1917],
	0x1917: [0x1916],
	# Bar Corner
	0x1910: [0x1911],
	0x1911: [0x1912],
	0x1912: [0x1913],
	0x1913: [0x1910],
	# Bar
	0x1918: [0x1919],
	0x1919: [0x191a],
	0x191a: [0x191b],
	0x191b: [0x191c],
	0x191c: [0x191d],
	0x191d: [0x191e],
	0x191e: [0x191f],
	0x191f: [0x1918],
	# Wooden Tables
	0x1da5: [0x1da6],
	0x1da9: [0x1daa],
	0x1daa: [0x1da5],
	
	0x1da7: [0x1da8],
	0x1da8: [0x1dab],
	0x1dab: [0x1dac],
	0x1dac: [0x1da7],
	# Sandstone Table
	0x1dbb: [0x1dbc],
	0x1dbc: [0x1dbd],
	0x1dbd: [0x1dbe],
	0x1dbe: [0x1dbf],
	0x1dbf: [0x1dc0],
	0x1dc0: [0x1dbb],
	# Marble Table
	0x1dc1: [0x1dc2],
	0x1dc2: [0x1dc3],
	0x1dc3: [0x1dc4],
	0x1dc4: [0x1dc5],
	0x1dc5: [0x1dc6],
	0x1dc6: [0x1dc1],
	# Sandstone Bench 3
	0x1dc7: [0x1dc8],
	0x1dc8: [0x1dc9],
	0x1dc9: [0x1dca],
	0x1dca: [0x1dcb],
	0x1dcb: [0x1dcc],
	0x1dcc: [0x1dc7],
	# Marble Bench
	0x1dcd: [0x1dce],
	0x1dce: [0x1dcf],
	0x1dcf: [0x1dd0],
	0x1dd0: [0x1dd1],
	0x1dd1: [0x1dd2],
	0x1dd2: [0x1dcd],
	# Bulletin Board
	0x1e5e: [0x1e5f],
	0x1e5f: [0x1e5e],
	# Barrel Staves
	0x1eb1: [0x1eb2],
	0x1eb2: [0x1eb3],
	0x1eb4: [0x1eb1],
	# Stone Bench 2
	0x3dff: [0x3e00],
	0x3e00: [0x3dff]
}

TURNDEEDS = {
	'small_bed_s_deed': ['small_bed_e_deed', 'small_bed_e', 'Small Bed East Deed'],
	'small_bed_e_deed': ['small_bed_s_deed', 'small_bed_s', 'Small Bed South Deed'],
	'large_bed_s_deed': ['large_bed_e_deed', 'large_bed_e', 'Large Bed East Deed'],
	'large_bed_e_deed': ['large_bed_s_deed', 'large_bed_s', 'Large Bed South Deed'],
	'dart_board_s_deed': ['dart_board_e_deed', 'dart_board_e', 'Dart Board East Deed'],
	'dart_board_e_deed': ['dart_board_s_deed', 'dart_board_s', 'Dart Board South Deed'],
	'large_forge_s_deed': ['large_forge_e_deed', 'large_forge_e', 'Large Forge East Deed'],
	'large_forge_e_deed': ['large_forge_s_deed', 'large_forge_s', 'Large Forge South Deed'],
	'anvil_s_deed': ['anvil_e_deed', 'anvil_e', 'Anvil East Deed'],
	'anvil_e_deed': ['anvil_s_deed', 'anvil_s', 'Anvil South Deed'],
	'training_dummy_s_deed': ['training_dummy_e_deed', 'training_dummy_e', 'Training Dummy East Deed'],
	'training_dummy_e_deed': ['training_dummy_s_deed', 'training_dummy_s', 'Training Dummy South Deed'],
	'spinning_wheel_s_deed': ['spinning_wheel_e_deed', 'spinning_wheel_e', 'Spinning Wheel East Deed'],
	'spinning_wheel_e_deed': ['spinning_wheel_s_deed', 'spinning_wheel_s', 'Spinning Wheel South Deed'],
	'loom_s_deed': ['loom_e_deed', 'loom_e', 'Loom East Deed'],
	'loom_e_deed': ['loom_s_deed', 'loom_s', 'Loom South Deed'],
	'stone_oven_s_deed': ['stone_oven_e_deed', 'stone_oven_e', 'Stone Oven East Deed'],
	'stone_oven_e_deed': ['stone_oven_s_deed', 'stone_oven_s', 'Stone Oven South Deed'],
	'flour_mill_s_deed': ['flour_mill_e_deed', 'flour_mill_e', 'Flour Mill East Deed'],
	'flour_mill_e_deed': ['flour_mill_s_deed', 'flour_mill_s', 'Flour Mill South Deed'],
	'water_trough_s_deed': ['water_trough_e_deed', 'water_trough_e', 'Water Trough East Deed'],
	'water_trough_e_deed': ['water_trough_s_deed', 'water_trough_s', 'Water Trough South Deed']
}

"""
	\command turnitem
	\description Rotates an item or a deed's placement direction.
	\usage - <code>turnitem</code>
"""

def turnitem( socket, command, arguments ):
	socket.sysmessage( "Target the object you would like to turn." )
	socket.attachtarget( "commands.turnitem.targetitem", [] )
	return

def targetitem( char, args, target ):
	socket = char.socket

	if target.item:
		# Find the target item.
		finditem = wolfpack.finditem( target.item.serial )
		# Safety Checks
		if not finditem:
			socket.sysmessage("You must target an item!")
			return True
		if ( finditem.lockeddown ) or ( not char.gm and finditem.movable > 1 ) or ( finditem.movable == 2 and finditem.owner != char ):
			socket.sysmessage("This object is not movable by you!")
			return True
		# Object Exists
		if finditem:
			# Turnable Furniture
			if finditem.id in TURNABLES:
				finditem.id = utilities.hex2dec(TURNABLES[finditem.id][0])
				finditem.update()
				socket.sysmessage("You rotate the object.")
			# Turnable Deeds
			elif int(finditem.id) == utilities.hex2dec(0x14ef) and str(finditem.baseid) in TURNDEEDS:
				if finditem.container != char.getbackpack():
					socket.sysmessage("This deed needs to be in your backpack to turn it!")
					return True
				else:
					finditem.settag( 'carpentry_type', str(TURNDEEDS[finditem.baseid][1]) )
					finditem.name = str(TURNDEEDS[str(finditem.baseid)][2])
					# Update BaseID Last
					finditem.baseid = str(TURNDEEDS[finditem.baseid][0])
					finditem.update()
					socket.sysmessage("You rotate the deed's placement direction.")
			return
		# Error
		else:
			socket.sysmessage("This item is not turnable.")
			return True
	# Error
	else:
		socket.sysmessage("This item is not turnable.")
		return True

def onLoad():
	wolfpack.registercommand( "turnitem", turnitem )
	return
