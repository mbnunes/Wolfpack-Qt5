#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: unknown                        #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Doors                                                         #                    									#
#===============================================================#

import wolfpack

# Sound constants for opening and closing a door
SOUND_OPENWOOD = 0xEA
SOUND_OPENGATE = 0xEB
SOUND_OPENSTEEL = 0xEC
SOUND_OPENSECRET = 0xED
SOUND_CLOSEWOOD = 0xF1
SOUND_CLOSEGATE = 0xF2
SOUND_CLOSESTEEL = 0xF3
SOUND_CLOSESECRET = 0xF4

# How long does it take to close a door automatically (in ms)
CLOSEDOOR_DELAY = 5000

# Inco
doors = [
			# ID (closed), ID (opened), x offset (opened), y offset (opened), OpenSound, CloseSound
			[ 0x675, 0x676, -1, 1, SOUND_OPENSTEEL, SOUND_CLOSESTEEL ],
			[ 0x677, 0x678, 1, 1, SOUND_OPENSTEEL, SOUND_CLOSESTEEL ],
			[ 0x679, 0x67A, -1, 0, SOUND_OPENSTEEL, SOUND_CLOSESTEEL ],
			[ 0x67B, 0x67C, 1, -1, SOUND_OPENSTEEL, SOUND_CLOSESTEEL ],
			[ 0x67D, 0x67E, 1, 1, SOUND_OPENSTEEL, SOUND_CLOSESTEEL ],
			[ 0x67F, 0x680, 1, -1, SOUND_OPENSTEEL, SOUND_CLOSESTEEL ],
			[ 0x681, 0x682, 0, 0, SOUND_OPENSTEEL, SOUND_CLOSESTEEL ],
			[ 0x683, 0x684, 0, -1, SOUND_OPENSTEEL, SOUND_CLOSESTEEL ],

			[ 0x685, 0x686, -1, 1, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x687, 0x688, 1, 1, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x689, 0x68A, -1, 0, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x68B, 0x68C, 1, -1, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x68D, 0x68E, 1, 1, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x68F, 0x690, 1, -1, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x691, 0x692, 0, 0, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x693, 0x694, 0, -1, SOUND_OPENGATE, SOUND_CLOSEGATE ],

			[ 0x695, 0x696, -1, 1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x697, 0x698, 1, 1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x699, 0x69A, -1, 0, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x69B, 0x69C, 1, -1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x69D, 0x69E, 1, 1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x69F, 0x6a0, 1, -1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6a1, 0x6a2, 0, 0, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6a3, 0x6a4, 0, -1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],

			[ 0x6a5, 0x6a6, -1, 1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6a7, 0x6a8, 1, 1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6a9, 0x6aA, -1, 0, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6aB, 0x6aC, 1, -1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6aD, 0x6aE, 1, 1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6aF, 0x6b0, 1, -1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6b1, 0x6b2, 0, 0, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6b3, 0x6b4, 0, -1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],

			[ 0x6b5, 0x6b6, -1, 1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6b7, 0x6b8, 1, 1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6b9, 0x6bA, -1, 0, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6bB, 0x6bC, 1, -1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6bD, 0x6bE, 1, 1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6bF, 0x6c0, 1, -1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6c1, 0x6c2, 0, 0, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6c3, 0x6c4, 0, -1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],

			[ 0x6c5, 0x6c6, -1, 1, SOUND_OPENSTEEL, SOUND_CLOSESTEEL ],
			[ 0x6c7, 0x6c8, 1, 1, SOUND_OPENSTEEL, SOUND_CLOSESTEEL ],
			[ 0x6c9, 0x6cA, -1, 0, SOUND_OPENSTEEL, SOUND_CLOSESTEEL ],
			[ 0x6cB, 0x6cC, 1, -1, SOUND_OPENSTEEL, SOUND_CLOSESTEEL ],
			[ 0x6cD, 0x6cE, 1, 1, SOUND_OPENSTEEL, SOUND_CLOSESTEEL ],
			[ 0x6cF, 0x6d0, 1, -1, SOUND_OPENSTEEL, SOUND_CLOSESTEEL ],
			[ 0x6d1, 0x6d2, 0, 0, SOUND_OPENSTEEL, SOUND_CLOSESTEEL ],
			[ 0x6d3, 0x6d4, 0, -1, SOUND_OPENSTEEL, SOUND_CLOSESTEEL ],

			[ 0x6d5, 0x6d6, -1, 1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6d7, 0x6d8, 1, 1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6d9, 0x6dA, -1, 0, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6dB, 0x6dC, 1, -1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6dD, 0x6dE, 1, 1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6dF, 0x6e0, 1, -1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6e1, 0x6e2, 0, 0, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6e3, 0x6e4, 0, -1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],

			[ 0x6e5, 0x6e6, -1, 1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6e7, 0x6e8, 1, 1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6e9, 0x6eA, -1, 0, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6eB, 0x6eC, 1, -1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6eD, 0x6eE, 1, 1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6eF, 0x6f0, 1, -1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6f1, 0x6f2, 0, 0, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x6f3, 0x6f4, 0, -1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],

			# Iron Gate 1
			[ 0x824, 0x825, -1, 1, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x826, 0x827, 1, 1, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x828, 0x829, -1, 0, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x82a, 0x82b, 1, -1, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x82c, 0x82d, 1, 1, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x82e, 0x82f, 1, -1, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x830, 0x831, 0, 0, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x832, 0x833, 0, -1, SOUND_OPENGATE, SOUND_CLOSEGATE ],

			# Wooden Gate 1
			[ 0x839, 0x83a, -1, 1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x83b, 0x83c, 1, 1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x83d, 0x83e, -1, 0, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x83f, 0x840, 1, -1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x841, 0x842, 1, 1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x843, 0x844, 1, -1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x845, 0x846, 0, 0, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x847, 0x848, 0, -1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],

			# Iron Gate 2
			[ 0x84c, 0x84d, -1, 1, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x84e, 0x84f, 1, 1, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x850, 0x851, -1, 0, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x852, 0x853, 1, -1, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x854, 0x855, 1, 1, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x856, 0x857, 1, -1, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x858, 0x859, 0, 0, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x85a, 0x85b, 0, -1, SOUND_OPENGATE, SOUND_CLOSEGATE ],

			# Wooden Gate 2
			[ 0x866, 0x867, -1, 1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x868, 0x869, 1, 1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x86a, 0x86b, -1, 0, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x86c, 0x86d, 1, -1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x86e, 0x86f, 1, 1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x870, 0x871, 1, -1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x872, 0x873, 0, 0, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],
			[ 0x874, 0x875, 0, -1, SOUND_OPENWOOD, SOUND_CLOSEWOOD ],

			# Barred Metal Door
			[ 0x1fed, 0x1fee, -1, 1, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x1fef, 0x1ff0, 1, 1, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x1ff1, 0x1ff2, -1, 0, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x1ff3, 0x1ff4, 1, -1, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x1ff5, 0x1ff6, 1, 1, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x1ff7, 0x1ff8, 1, -1, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x1ff9, 0x1ffa, 0, 0, SOUND_OPENGATE, SOUND_CLOSEGATE ],
			[ 0x1ffb, 0x1ffc, 0, -1, SOUND_OPENGATE, SOUND_CLOSEGATE ],

			# Secret Doors
			[ 0xE8, 0xE9, -1, 1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0xEA, 0xEB, 1, 1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0xEC, 0xED, -1, 0, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0xEE, 0xEF, 1, -1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0xF0, 0xF1, 1, 1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0xF2, 0xF3, 1, -1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0xF4, 0xF5, 0, 0, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0xF6, 0xF7, 0, -1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],

			[ 0x314, 0x315, -1, 1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x316, 0x317, 1, 1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x318, 0x319, -1, 0, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x31A, 0x31B, 1, -1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x31C, 0x31D, 1, 1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x31E, 0x31F, 1, -1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x320, 0x321, 0, 0, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x322, 0x323, 0, -1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],

			[ 0x324, 0x325, -1, 1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x326, 0x327, 1, 1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x328, 0x329, -1, 0, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x32A, 0x32B, 1, -1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x32C, 0x32D, 1, 1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x32E, 0x32F, 1, -1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x330, 0x331, 0, 0, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x332, 0x333, 0, -1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],

			[ 0x334, 0x335, -1, 1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x336, 0x337, 1, 1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x338, 0x339, -1, 0, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x33A, 0x33B, 1, -1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x33C, 0x33D, 1, 1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x33E, 0x33F, 1, -1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x340, 0x341, 0, 0, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x342, 0x343, 0, -1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],

			[ 0x344, 0x345, -1, 1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x346, 0x347, 1, 1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x348, 0x349, -1, 0, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x34A, 0x34B, 1, -1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x34C, 0x34D, 1, 1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x34E, 0x34F, 1, -1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x350, 0x351, 0, 0, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x352, 0x353, 0, -1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],

			[ 0x354, 0x355, -1, 1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x356, 0x357, 1, 1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x358, 0x359, -1, 0, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x35A, 0x35B, 1, -1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x35C, 0x35D, 1, 1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x35E, 0x35F, 1, -1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x360, 0x361, 0, 0, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
			[ 0x362, 0x363, 0, -1, SOUND_OPENSECRET, SOUND_CLOSESECRET ],
		]

def searchkey( container, lock ):
	for item in container.content:
		# Check if it's a key
		if 'key' in item.events and item.hastag( 'lock' ) and int( item.gettag( 'lock' ) ) == lock:
			return 1

		if item.type == 1 and searchkey( item, lock ):
			return 1

	return 0

def opendoor( char, item ):
	pos = item.pos

	# Search the door information
	for door in doors:
		# Use this door information
		if door[0] == item.id:
			# Change the Door id and resend
			item.id = door[1]
			item.moveto( pos.x + door[2], pos.y + door[3], pos.z )
			item.update()

			# Soundeffect (open)
			char.soundeffect( door[4] )

			# This is used to track multiple open/close actions
			# We cannot dispel the effect on the item, so we have
			# to do it this way.
			if not item.hastag('opencount'):
				opencount = 1
			else:
				opencount = int(item.gettag('opencount')) + 1

			item.settag('opencount', str(opencount))

			if not item.hastag('opened'):
				item.settag('opened', 'true')

			# Add an autoclose tempeffect
			item.addtimer(CLOSEDOOR_DELAY, "door.autoclose", [ opencount ], 1)

			# Refresh a House if we opened it
			# Multis are completely broken right now. add refresh later here
			#multi = wolfpack.findmulti( item.pos )

			#if multi and multi.ishouse():
				# Only Owners and Friends can refresh a house
			#	if multi.owner == char or char in multi.friends:
			#		char.message( 'You refresh the house' )

			return True

		# Return to the original state
		elif door[1] == item.id:
			blocked = 0
			chars = None
			chars = wolfpack.chars( item.pos.x - door[2], item.pos.y - door[3], item.pos.map, 0 )
			if chars:
				for bchar in chars:
					if bchar.pos.z == item.pos.z:
						blocked = 1
						break
					elif bchar.pos.z < item.pos.z and bchar.pos.z >= ( item.pos.z - 5):
						blocked = 1
						break
					elif bchar.pos.z > item.pos.z and bchar.pos.z <= ( item.pos.z + 5):
						blocked = 1
						break

			if blocked == 1:
				char.socket.sysmessage( "There is someone blocking the door!" )
				return False
			else:
				pos = item.pos
				# Change the door id and update the clients around it
				item.id = door[0]
				item.moveto( pos.x - door[2], pos.y - door[3], pos.z )
				if item.hastag('opened'):
					item.deltag( 'opened' )
				item.update()
				# Soundeffect (close)
				char.soundeffect( door[5] )

			return True

	return False

def autoclose( item, args ):
	if not item or not item.hastag( 'opencount' ):
		return

	# Are we supposed to close this door?
	opencount = int( item.gettag( 'opencount' ) )

	# We are not supposed to close this door
	if opencount != args[0]:
		return

	"""
	# NOTE!
	# We need to put in a form of character checking for the location
	# where the door closes, this way the door doesn't close ON people.
	"""
	blocked = 0
	chars = []
	for door in doors:
		if door[1] == item.id:
			chars = wolfpack.chars( item.pos.x - door[2], item.pos.y - door[3], item.pos.map, 0 )
	for bchar in chars:
		if bchar.pos.z == item.pos.z:
			blocked = 1
			break
		elif bchar.pos.z < item.pos.z and bchar.pos.z >= ( item.pos.z - 5):
			blocked = 1
			break
		elif bchar.pos.z > item.pos.z and bchar.pos.z <= ( item.pos.z + 5):
			blocked = 1
			break

	if blocked == 1:
		item.addtimer(CLOSEDOOR_DELAY, "door.autoclose", [ int(item.gettag('opencount')) ], 1)
	else:
		# Find the door definition for this item
		for door in doors:
			if door[1] == item.id:
				pos = item.pos
				# Change the door id and update the clients around it
				item.id = door[0]
				item.moveto( pos.x - door[2], pos.y - door[3], pos.z )
				if item.hastag( 'opened' ):
					item.deltag( 'opened' )
				item.update()

				# Soundeffect (close)
				item.soundeffect( door[5] )


def onUse(char, item, norange=0):
	# Using doors doesnt count against the object-delay
	char.objectdelay = 0

	# In Range?
	if not norange and not char.gm and not char.canreach(item, 2):
		char.message( "You cannot reach the handle from here." )
		return 1

	# Do we have a linked door, is this door not open?
	if item.hastag('link') and not item.hastag('opened'):
		doubledoor = wolfpack.finditem( int(item.gettag('link')) )
		# Double check to make sure either door isn't open.
		if not item.hastag('opened') and not doubledoor.hastag('opened'):
			opendoor( char, doubledoor )
	return opendoor( char, item )

def onTelekinesis(char, item):
	return onUse(char, item, 1)
