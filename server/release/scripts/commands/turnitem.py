
import wolfpack
from wolfpack import utilities
from wolfpack import properties

TURNABLES = {
	# Wooden Chest
	0xe42: [0xe43],
	0xe43: [0xe42],
	# Red Armoire
	0xa4d: [0xa51],
	0xa51: [0xa4d],
	# Wooden Armoire
	0xa4f: [0xa53],
	0xa53: [0xa4f],
	# Bookcases
	0xa9d: [0xa9e],
	0xa9e: [0xa9d],
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
	# Music Stand
	0xeb6: [0xeb8],
	0xeb8: [0xeb6],
	# Tall Music Stand
	0xebb: [0xebc],
	0xebc: [0xebb],
	# Dress Form
	0xec6: [0xec7],
	0xec7: [0xec6]
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

def onLoad():
	wolfpack.registercommand( "turnitem", turnitem )

def turnitem( socket, command, arguments ):
	socket.sysmessage( "Target the object you would like to turn." )
	socket.attachtarget( "commands.turnitem.targetitem", [] )

def targetitem( char, args, target ):
	socket = char.socket

	if target.item:
		# Find the target item.
		finditem = wolfpack.finditem( target.item.serial )
		# Safety Checks
		if not finditem:
			socket.sysmessage("You must target an item!")
			return 1
		if finditem.magic != 1:
			socket.sysmessage("This object is not movable by you!")
			return 1
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
					return 1
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
			return 1
	# Error
	else:
		socket.sysmessage("This item is not turnable.")
		return 1
