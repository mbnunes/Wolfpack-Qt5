#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-" {{ ;"`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
from wolfpack.utilities import *
from wolfpack.consts import *

# { name:[ tooltip id, direction, item1[, item2, ... ]], ... }
# direction 0 : no dir / 1 : ns / 2 : we
deeds = { 'small_bed_s':[ 1015123, 1, "a5e", "a5f" ],
	'small_bed_e':[ 1015124, 2, "a60", "a61" ],
	'large_bed_s':[ 1015125, 0, "a77", "a76", "a74", "a75" ],
	'large_bed_e':[ 1015126, 0, "a73", "a70", "a72", "a71" ],
	'dart_board_e':[ 1015128, 0, "1e2f" ],
	'dart_board_s':[ 1015127, 0, "1e2e" ],
	#'ballot_box':[ 1015129, ],
	'pentagram':[ 1015130, 0, "fe6", "fe7", "fe8", "fe9", "fea", "feb", "fec", "fed", "fee" ],
	#'abbatoir':[ 1015131, ],
	'small_forge':[ 1015133, 0, "fb1" ],
	'large_forge_e':[ 1015134, 2, "197a", "197e", "1982" ],
	'large_forge_s':[ 1015135, 1, "1986", "198a", "198e" ],
	'anvil_e':[ 1015136, 0, "fb0" ],
	'anvil_s':[ 1015137, 0, "faf" ],
	'training_dummy_e':[ 1015143, 0, "1074" ],
	'training_dummy_s':[ 1015144, 0, "1070" ],
	'pickpocket_dip_e':[ 1015145, 0, "1ec0" ],
	#'pickpocket_dip_s':[ 1015146, ],
	'spinning_wheel_e':[ 1015139, 0, "1019" ],
	'spinning_wheel_s':[ 1015140, 0, "1015" ],
	'loom_s':[ 1015141, 1, "1060", "105f" ],
	'loom_e':[ 1015142, 2, "1061", "1062" ],
	'stone_oven_s':[ 1015149, 1, "92c", "92b" ],
	'stone_oven_e':[ 1015150, 2, "931", "930" ],
	'flour_mill_e':[ 1015151, 2, "1921", "1923" ],
	'flour_mill_s':[ 1015152, 1, "192d", "192f" ],
	'water_trough_e':[ 1015153, 2, "b43", "b44" ],
	'water_trough_s':[ 1015154, 1, "b41", "b42" ] }

def onShowToolTip( sender, target, tooltip ):
	# name in tooltip
	if not target.hastag( 'carpentry_type' ):
		return
	name = target.gettag( 'carpentry_type' )
	if not deeds.haskey( name ):
		return
	tooltip_id = deeds[ name ][ 0 ]
	tooltip.add( tooltip_id, "" )
	# blessed
	tooltip.add( 1038021, "" )
	tooltip.send( sender )

def onUse( char, item ):
	if not char or not item:
		return
	# check if this char is in it's own house
	if char.multi == -1:
		# msg
		return
	multi = wolfpack.finditem( char.multi )
	if not multi or multi.ownserial != char.serial:
		# msg
		return
	# send target cursor
	char.socket.clilocmessage( 0xF55DA, "", 0x3b2, 3 )
	char.socket.attachtarget( "carpentry_deed.response", [ item ] )

def response( char, args, target ):
	item = args[0]
	if not item or item.getoutmostchar() != char:
		# msg
		return
	if not item.hastag( 'carpentry_type' ):
		return
	if not target.pos:
		return
	if char.distanceto( pos ) > 3:
		char.socket.clilocmessage( 500251, "", 0x3b2, 3 )
		return
	# check if there is sufficient room for the item
	if not check_room( item, target.pos ):
		# msg
		return
	setup_item( item, target.pos )
	item.delete()

def check_room( item, pos ):
	# check valid item
	if not item.hastag( 'carpentry_type' ):
		return 0
	name = item.gettag( 'carpentry_type' )
	if not deeds.haskey( name ):
		return 0
	dir = deeds[ name ][ 1 ]
	items = deeds[ name ][ 2: ]
	if not items:
		return 0
	num_item = len( items )
	if not num_item:
		return 0

	# check the spots
	ret0 = 0
	ret1 = 0
	ret2 = 0
	if num_item == 1:
		return check_spot( pos )
	elif num_item == 2:
		ret0 = check_spot( pos )
		if dir == 1:
			ret1 = check_spot( pos.x, pos.y + 1, pos.map )
		else:
			ret1 = check_spot( pos.x + 1, pos.y, pos.map )
		return ret0 & ret1
	elif num_item == 3:
		ret0 = check_spot( pos )
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
					return 0
		return 1
	# pentagram
	elif num_item == 9:
		for x in range( pos.x - 1, pos.x + 2 ):
			for y in range( pos.y - 1, pos.y + 2 ):
				ret0 = check_spot( x, y, pos.map )
				if not ret0:
					return 0
		return 1
	# no other case : error
	return 0

def check_spot( pos ):
	if len( wolfpack.items( pos.x, pos.y, pos.map ) ):
		return 0
	return 1

def check_spot( x, y, map ):
	if len( wolfpack.items( x, y, map ) ):
		return 0
	return 1

def setup_item( item, pos ):
	# check valid item
	if not item.hastag( 'carpentry_type' ):
		return
	name = item.gettag( 'carpentry_type' )
	if not deeds.haskey( name ):
		return
	dir = deeds[ name ][ 1 ]
	items = deeds[ name ][ 2: ]
	if not items:
		return
	num_item = len( items )
	if not num_items:
		return

	# put items
	if num_item == 1:
		put_item( items[ 0 ] )
	elif num_item == 2:
		put_item( items[ 0 ] )
		if dir == 1:
			x1 = pos.x
			y1 = pos.y + 1
		else:
			x1 = pos.x + 1
			y1 = pos.y
		put_item( items[ 1 ], x1, y1, pos.map )
	elif num_item == 3:
		put_item( items[ 1 ], pos.x, pos.y, pos.map )
		if dir == 1:
			put_item( items[ 0 ], pos.x, pos.y - 1, pos.map )
			put_item( items[ 2 ], pos.x, pos.y + 1, pos.map )
		else:
			put_item( items[ 0 ], pos.x - 1, pos.y, pos.map )
			put_item( items[ 2 ], pos.x + 1, pos.y, pos.map )
	elif num_item == 4:
		i = 0
		for x in range( pos.x, pos.x + 2 ):
			for y in range( pos.y, pos.y + 2 ):
				put_item( items[ i ], x, y, pos.map )
				i += 1
	elif num_item == 9:
		i = 0
		for y in range( pos.x - 1, pos.x + 2 ):
			for x in range( pos.y - 1, pos.y + 2 ):
				put_item( items[ i ], x, y, pos.map )
				i += 1

def put_item( str, pos ):
	item = wolfpack.additem( str )
	if not item:
		return 0
	item.moveto( pos )
	# locked down
	item.magic = 4
	item.update()
	return 1

def put_item( str, x, y, map ):
	item = wolfpack.additem( str )
	if not item:
		return 0
	item.moveto( x, y, map )
	# locked down
	item.magic = 4
	item.update()
	return 1
