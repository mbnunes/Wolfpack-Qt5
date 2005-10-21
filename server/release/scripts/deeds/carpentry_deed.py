#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-" {{ ;"`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
from wolfpack.utilities import *
from wolfpack.consts import *

extended_carpentry = int( wolfpack.settings.getbool("General", "Extended Carpentry", False, True) )

TOOLTIP = 0
DIRECTION = 1
DYE = 2
DEED_ITEMS = 3

# { name:[ tooltip id, direction, item1[, item2, ... ]], ... }
# direction 0 : no dir / 1 : ns / 2 : we
deeds = {
	'small_bed_s':[ 1015123, 1, False, ["a5e", "a5f"] ],
	'small_bed_e':[ 1015124, 2, False, ["a60", "a61"] ],
	'large_bed_s':[ 1015125, 0, False, ["a77", "a76", "a74", "a75"] ],
	'large_bed_e':[ 1015126, 0, False, ["a73", "a70", "a72", "a71"] ],
	'dart_board_e':[ 1015128, 0, False, ["1e2f"] ],
	'dart_board_s':[ 1015127, 0, False, ["1e2e"] ],
	'ballot_box':[ 1015129, 0, False, ["ballot_box"] ],
	'pentagram':[ 1015130, 0, False, ["fe7", "fe6", "fe9", "fe8", "fea", "fec", "feb", "fee", "fed"] ],
	'abattoir':[ 1015131, 0, False, ["120e", "1215", "1214", "120f", "1216", "1213", "1210", "1211", "1212"] ],
	'small_forge':[ 1015133, 0, False, ["fb1"] ],
	'large_forge_e':[ 1015134, 2, False, ["197a", "197e", "1982"] ],
	'large_forge_s':[ 1015135, 1, False, ["1986", "198a", "198e"] ],
	'anvil_e':[ 1015136, 0, False, ["fb0"] ],
	'anvil_s':[ 1015137, 0, False, ["faf"] ],
	'training_dummy_e':[ 1015143, 0, False, ["1074"] ],
	'training_dummy_s':[ 1015144, 0, False, ["1070"] ],
	'pickpocket_dip_e':[ 1015145, 0, False, ["1ec0"] ],
	'spinning_wheel_e':[ 1015139, 0, True, ["1019"] ],
	'spinning_wheel_s':[ 1015140, 0, True, ["1015"] ],
	'loom_s':[ 1015141, 1, True, ["1060", "105f"] ],
	'loom_e':[ 1015142, 2, True, ["1061", "1062"] ],
	'stone_oven_s':[ 1015149, 1, False, ["92c", "92b"] ],
	'stone_oven_e':[ 1015150, 2, False, ["931", "930"] ],
	'flour_mill_e':[ 1015151, 2, False, ["1921", "1923"] ],
	'flour_mill_s':[ 1015152, 1, False, ["192d", "192f"] ],
	'water_trough_e':[ 1015153, 2, False, ["b43", "b44"] ],
	'water_trough_s':[ 1015154, 1, False, ["b41", "b42"] ],
	'bulletin_board':[ 1027774, 0, False, ["1e5e"] ],
	'dresser_e': [ 1022620, 2, False, ["a3d", "a3c"] ],
	'dresser_s': [ 1022620, 1, False, ["a45", "a44"] ],
	# not really carpentry...
	'stonetable_medium_e': [ 1044508, 1, False, ["1202", "1201"] ],
	'stonetable_medium_s': [ 1044509, 2, False, ["1205", "1204"] ],
	'stonetable_large_e': [ 1044511, 1, False, ["1202", "1203", "1201"] ],
	'stonetable_large_s': [ 1044512, 2, False, ["1205", "1206", "1204"] ],
	'goza_east': [ 1030404, 2, False, ["28a4", "28a5"] ],
	'goza_south': [ 1030405, 1, False, ["28a6", "28a7"] ],
	'square_goza_east': [ 1030407, 0, False, ["28a8"] ],
	'square_goza_south': [ 1030406, 0, False, ["28a9"] ],
	'brocade_goza_east': [ 1030408, 2, False, ["28ab", "28aa"] ],
	'brocade_goza_south': [ 1030409, 1, False, ["28ad", "28ac"] ],
	'brocade_square_goza_east': [ 1030411, 0, False, ["28ae"] ],
	'brocade_square_goza_south': [ 1030410, 0, False, ["28af"] ]
}

def onShowTooltip( sender, target, tooltip ):
	# name in tooltip
	if not target.hastag( 'carpentry_type' ):
		return
	name = target.gettag( 'carpentry_type' )
	if not deeds.has_key( name ):
		return
	tooltip_id = deeds[ name ][ TOOLTIP ]
	tooltip.add( tooltip_id, "" )
	# blessed
	tooltip.add( 1038021, "" )

def checkmulti(char):
	if char.gm:
		return True
	if not char.multi:
		#You can only build this in a house.
		char.socket.clilocmessage(500275)
		return False
	# check if this char is in it's own house
	if not char.multi.owner == char.serial:
		#You must own the house to do this.
		char.socket.clilocmessage(502096)
		return False
	return True

def onUse( char, item ):
	if not char or not item:
		return True

	if not checkmulti(char):
		return True

	# Where would you like to place this decoration?
	char.socket.clilocmessage( 1049780 )

	# send target cursor
	char.socket.attachtarget( "deeds.carpentry_deed.response", [ item.serial ] )

	return True

def response( char, args, target ):
	item = wolfpack.finditem(args[0])
	if not item or item.getoutmostchar() != char:
		# msg
		return
	if not item.hastag( 'carpentry_type' ):
		return
	if not target.pos:
		return

	if( target.item ):
		#we don't want to target items in containers
		if( target.item.getoutmostchar() ):
			#The decoration cannot be placed there.
			char.socket.clilocmessage( 1049782 )
			return

	if not( char.canreach( target, 3 ) ):
		#That location is too far away.
		char.socket.clilocmessage( 500251 )
		return

	# check if there is sufficient room for the item
	if not check_room( item, target.pos ):
		#The decoration cannot be placed there.
		char.socket.clilocmessage( 1049782 )
		return

	setup_item( item, target.pos )
	item.delete()

def check_room( item, pos ):
	# check valid item
	if not item.hastag( 'carpentry_type' ):
		return False
	name = item.gettag( 'carpentry_type' )
	if not deeds.has_key( name ):
		return False
	dir = deeds[ name ][ DIRECTION ]
	items = deeds[ name ][ DEED_ITEMS ]
	if not items:
		return False
	num_item = len( items )
	if not num_item:
		return False

	# check the spots
	ret0 = 0
	ret1 = 0
	ret2 = 0
	if num_item == 1:
		return check_spot( pos.x, pos.y, pos.map )
	elif num_item == 2:
		ret0 = check_spot( pos.x, pos.y, pos.map )
		if dir == 1:
			ret1 = check_spot( pos.x, pos.y + 1, pos.map )
		else:
			ret1 = check_spot( pos.x + 1, pos.y, pos.map )
		return ret0 & ret1
	elif num_item == 3:
		ret0 = check_spot( pos.x, pos.y, pos.map )
		if dir == 1:
			ret1 = check_spot( pos.x, pos.y + 1, pos.map )
			ret2 = check_spot( pos.x, pos.y - 1, pos.map )
		else:
			ret1 = check_spot( pos.x + 1, pos.y, pos.map )
			ret2 = check_spot( pos.x -1, pos.y, pos.map )
		return ret0 & ret1 & ret2
	elif num_item == 4:
		ret0 = 1
		for x in range( pos.x, pos.x + 2 ):
			for y in range( pos.y, pos.y + 2 ):
				ret0 = check_spot( x, y, pos.map )
				if not ret0:
					return False
		return True
	# pentagram
	elif num_item == 9:
		ret0 = wolfpack.items( pos.x, pos.y, pos.map, 2 )
		if ret0:
			return False
		return True
	# no other case : error
	return False

def check_spot( x, y, map ):
	#TODO: needs check for z value too, Check for static items
	# Check for dynamic items
	if len( wolfpack.items( x, y, map ) ):
		return False
	return True

def setup_item( item, pos ):
	# check valid item
	if not item.hastag( 'carpentry_type' ):
		return
	name = item.gettag( 'carpentry_type' )
	if not deeds.has_key( name ):
		return
	dir = deeds[ name ][ DIRECTION ]
	items = deeds[ name ][ DEED_ITEMS ]
	if not items:
		return
	num_item = len( items )
	if not num_item:
		return

	try:
		resname = item.gettag('resname')
	except:
		resname = None

	if deeds[ name ][ DYE ]:
		color = item.color
	else:
		color = None

	# put items
	if num_item == 1:
		put_item( items[ 0 ], resname, color, pos.x, pos.y, pos.z, pos.map )
	elif num_item == 2:
		put_item( items[ 0 ], resname, color, pos.x, pos.y, pos.z, pos.map )
		if dir == 1:
			x1 = pos.x
			y1 = pos.y + 1
		else:
			x1 = pos.x + 1
			y1 = pos.y
		put_item( items[ 1 ], resname, color, x1, y1, pos.z, pos.map )
	elif num_item == 3:
		put_item( items[ 1 ], resname, color, pos.x, pos.y, pos.z, pos.map )
		if dir == 1:
			put_item( items[ 0 ], resname, color, pos.x, pos.y - 1, pos.z, pos.map )
			put_item( items[ 2 ], resname, color, pos.x, pos.y + 1, pos.z, pos.map )
		else:
			put_item( items[ 0 ], resname, color, pos.x - 1, pos.y, pos.z, pos.map )
			put_item( items[ 2 ], resname, color, pos.x + 1, pos.y, pos.z, pos.map )
	elif num_item == 4:
		i = 0
		for x in range( pos.x, pos.x + 2 ):
			for y in range( pos.y, pos.y + 2 ):
				put_item( items[ i ], resname, color, x, y, pos.z, pos.map )
				i += 1
	elif num_item == 9:
		i = 0
		for x in range( pos.x - 1, pos.x + 2 ):
			for y in range( pos.y - 1, pos.y + 2 ):
				put_item( items[ i ], resname, color, x, y, pos.z, pos.map )
				i += 1

def put_item( str, resname, color, x, y, z, map ):
	item = wolfpack.additem( str )
	if not item:
		return False

	if extended_carpentry and resname and len(resname) > 0:
		item.settag( 'resname', resname )
		if color != None:
			item.color = color

	item.moveto( x, y, z, map )
	# locked down
	item.movable = 3
	item.decay = 0
	item.update()
	return True
