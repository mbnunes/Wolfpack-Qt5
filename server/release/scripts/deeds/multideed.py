import datetime
from time import time, localtime

import wolfpack
import housing
from wolfpack.gumps import cGump
from wolfpack.consts import *
from housing.consts import CAN_HAVE_MULTIPLE_HOUSES
from housing.deed import getPlacementData

#2-Story houses
ch2story = [
[ 7, 7, 0x13ec, "foundation_2_7x7", 33000 ],
[ 7, 8, 0x13ed, "foundation_2_7x8", 37000 ],
[ 7, 9, 0x13ee, "foundation_2_7x9", 41000 ],
[ 7, 10, 0x13ef, "foundation_2_7x10", 45000 ],
[ 7, 11, 0x13f0, "foundation_2_7x11", 49000 ],
[ 7, 12, 0x13f1, "foundation_2_7x12", 53000 ],
[ 8, 7, 0x13f8, "foundation_2_8x7", 37500 ],
[ 8, 8, 0x13f9, "foundation_2_8x8", 42000 ],
[ 8, 9, 0x13fa, "foundation_2_8x9", 46500 ],
[ 8, 10, 0x13fb, "foundation_2_8x10", 51000 ],
[ 8, 11, 0x13fc, "foundation_2_8x11", 55500 ],
[ 8, 12, 0x13fd, "foundation_2_8x12", 60000 ],
[ 8, 13, 0x13fe, "foundation_2_8x13", 64500 ],
[ 9, 7, 0x1404, "foundation_2_9x7", 42000 ],
[ 9, 8, 0x1405, "foundation_2_9x8", 47000 ],
[ 9, 9, 0x1406, "foundation_2_9x9", 52000 ],
[ 9, 10, 0x1407, "foundation_2_9x10", 57000 ],
[ 9, 11, 0x1408, "foundation_2_9x11", 62000 ],
[ 9, 12, 0x1409, "foundation_2_9x12", 67000 ],
[ 9, 13, 0x140a, "foundation_2_9x13", 72000 ],
[ 10, 7, 0x1410, "foundation_2_10x7", 46500 ],
[ 10, 8, 0x1411, "foundation_2_10x8", 52000 ],
[ 10, 9, 0x1412, "foundation_2_10x9", 57500 ],
[ 10, 10, 0x1413, "foundation_2_10x10", 63000 ],
[ 10, 11, 0x1414, "foundation_2_10x11", 68500 ],
[ 10, 12, 0x1415, "foundation_2_10x12", 74000 ],
[ 10, 13, 0x1416, "foundation_2_10x13", 79500 ],
[ 11, 7, 0x141c, "foundation_2_11x7", 51000 ],
[ 11, 8, 0x141d, "foundation_2_11x8", 57000 ],
[ 11, 9, 0x141e, "foundation_2_11x9", 63000 ],
[ 11, 10, 0x141f, "foundation_2_11x10", 69000 ],
[ 11, 11, 0x1420, "foundation_2_11x11", 75000 ],
[ 11, 12, 0x1421, "foundation_2_11x12", 81000 ],
[ 11, 13, 0x1422, "foundation_2_11x13", 87000 ],
[ 12, 7, 0x1428, "foundation_2_12x7", 55500 ],
[ 12, 8, 0x1429, "foundation_2_12x8", 62000 ],
[ 12, 9, 0x142a, "foundation_2_12x9", 68500 ],
[ 12, 10, 0x142b, "foundation_2_12x10", 75000 ],
[ 12, 11, 0x142c, "foundation_2_12x11", 81500 ],
[ 12, 12, 0x142d, "foundation_2_12x12", 88000 ],
[ 12, 13, 0x142e, "foundation_2_12x13", 94500 ],
[ 13, 8, 0x1435, "foundation_2_13x8", 67000 ],
[ 13, 9, 0x1436, "foundation_2_13x9", 74000 ],
[ 13, 10, 0x1437, "foundation_2_13x10", 81000 ],
[ 13, 11, 0x1438, "foundation_2_13x11", 88000 ],
[ 13, 12, 0x1439, "foundation_2_13x12", 95000 ],
[ 13, 13, 0x143a, "foundation_2_13x13", 102000 ] ]

#3-Story houses
ch3story = [
[ 9, 14, 0x140b, "foundation_3_9x14", 77000 ],
[ 10, 14, 0x1417, "foundation_3_10x14", 85000 ],
[ 10, 15, 0x1418, "foundation_3_10x15", 90500 ],
[ 11, 14, 0x1423, "foundation_3_11x14", 93000 ],
[ 11, 15, 0x1424, "foundation_3_11x15", 99000 ],
[ 11, 16, 0x1425, "foundation_3_11x16", 105000 ],
[ 12, 14, 0x142f, "foundation_3_12x14", 101000 ],
[ 12, 15, 0x1430, "foundation_3_12x15", 107500 ],
[ 12, 16, 0x1431, "foundation_3_12x16", 114000 ],
[ 12, 17, 0x1432, "foundation_3_12x17", 120500 ],
[ 13, 14, 0x143b, "foundation_3_13x14", 109000 ],
[ 13, 15, 0x143c, "foundation_3_13x15", 116000 ],
[ 13, 16, 0x143d, "foundation_3_13x16", 123000 ],
[ 13, 17, 0x143e, "foundation_3_13x17", 130000 ],
[ 13, 18, 0x143f, "foundation_3_13x18", 137000 ],
[ 14, 9, 0x1442, "foundation_3_14x9", 79000 ],
[ 14, 10, 0x1443, "foundation_3_14x10", 87000 ],
[ 14, 11, 0x1444, "foundation_3_14x11", 94500 ],
[ 14, 12, 0x1445, "foundation_3_14x12", 102000 ],
[ 14, 13, 0x1446, "foundation_3_14x13", 109500 ],
[ 14, 14, 0x1447, "foundation_3_14x14", 117000 ],
[ 14, 15, 0x1448, "foundation_3_14x15", 124500 ],
[ 14, 16, 0x1449, "foundation_3_14x16", 132000 ],
[ 14, 17, 0x144a, "foundation_3_14x17", 139500 ],
[ 14, 18, 0x144b, "foundation_3_14x18", 147000 ],
[ 15, 10, 0x144f, "foundation_3_15x10", 93000 ],
[ 15, 11, 0x1450, "foundation_3_15x11", 101000 ],
[ 15, 12, 0x1451, "foundation_3_15x12", 109000 ],
[ 15, 13, 0x1452, "foundation_3_15x13", 117000 ],
[ 15, 14, 0x1453, "foundation_3_15x14", 125000 ],
[ 15, 15, 0x1454, "foundation_3_15x15", 133000 ],
[ 15, 16, 0x1455, "foundation_3_15x16", 141000 ],
[ 15, 17, 0x1456, "foundation_3_15x17", 149000 ],
[ 15, 18, 0x1457, "foundation_3_15x18", 157000 ],
[ 16, 11, 0x145c, "foundation_3_16x11", 107500 ],
[ 16, 12, 0x145d, "foundation_3_16x12", 116000 ],
[ 16, 13, 0x145e, "foundation_3_16x13", 124500 ],
[ 16, 14, 0x145f, "foundation_3_16x14", 133000 ],
[ 16, 15, 0x1460, "foundation_3_16x15", 141500 ],
[ 16, 16, 0x1461, "foundation_3_16x16", 150000 ],
[ 16, 17, 0x1462, "foundation_3_16x17", 158500 ],
[ 16, 18, 0x1463, "foundation_3_16x18", 167000 ],
[ 17, 12, 0x1469, "foundation_3_17x12", 123000 ],
[ 17, 13, 0x146a, "foundation_3_17x13", 132000 ],
[ 17, 14, 0x146b, "foundation_3_17x14", 141000 ],
[ 17, 15, 0x146c, "foundation_3_17x15", 150000 ],
[ 17, 16, 0x146d, "foundation_3_17x16", 159000 ],
[ 17, 17, 0x146e, "foundation_3_17x17", 168000 ],
[ 17, 18, 0x146f, "foundation_3_17x18", 177000 ],
[ 18, 13, 0x1476, "foundation_3_18x13", 139500 ],
[ 18, 14, 0x1477, "foundation_3_18x14"],
[ 18, 15, 0x1478, "foundation_3_18x15", 158500 ],
[ 18, 16, 0x1479, "foundation_3_18x16", 168000 ],
[ 18, 17, 0x147a, "foundation_3_18x17", 177500 ],
[ 18, 18, 0x147b, "foundation_3_18x18", 187000 ]]

#Classical Houses
# The first two parameters are just garbage.
# 3rd Parameter is ID, 4th the ID Definition and 5th is the value of house
classical = [
[ 1, 1, 0x64, "small_stone_and_plaster_house", 37000 ],
[ 1, 1, 0x66, "small_fieldstone_house", 37000 ],
[ 1, 1, 0x68, "small_brick_house", 36750 ],
[ 1, 1, 0x6a, "small_wood_house", 35250 ],
[ 1, 1, 0x6c, "small_wood_and_plaster_house", 36750 ],
[ 1, 1, 0x6e, "small_thatched_roof_cottage", 36750 ],
[ 1, 1, 0xa0, "small_stone_workshop", 50500 ],
[ 1, 1, 0xa2, "small_marble_workshop", 52500 ],
[ 1, 1, 0x98, "small_tower", 73500 ],
[ 1, 1, 0x9e, "two_story_villa", 113750 ],
[ 1, 1, 0x9c, "sandstone_patio_house", 76500 ],
[ 1, 1, 0x9a, "log_cabin", 81750 ],
[ 1, 1, 0x68, "small_brick_house", 131500 ],
[ 1, 1, 0x76, "two_story_wood_and_plaster_house", 162750 ],
[ 1, 1, 0x78, "two_story_stone_and_plaster_house", 162000 ],
[ 1, 1, 0x8d, "large_patio_house", 129250 ],
[ 1, 1, 0x96, "large_marble_patio_house", 160500 ],
[ 1, 1, 0x7a, "large_tower", 366500 ],
[ 1, 1, 0x7c, "stone_keep", 572750 ],
[ 1, 1, 0x7e, "castle", 865250 ]]

#buttons
button_ok = 1
button_cancel = 0

def onUse( char, item ):
	gump0( char, gump0callback )
	return 1

# Selection of custom house type ( 2-story / 3-story )
# Its the first gump with Classical, 2-story and 3-story selection... accessed when use the dclick
def gump0callback( char, args, target ):
	button = target.button

	#Old style houses
	if button == 1:
		gump4( char, gump3callback, classical )
	#2-Story houses
	if button == 2:
		gump1( char, gump1callback, ch2story )
	#3-Story houses
	if button == 3:
		gump3( char, gump1callback, ch3story )

#Selection of 2-story / 3-story size
def gump1callback( char, args, target ):
	button = target.button
	if button > 0:
		char.socket.attachmultitarget( "deeds.multideed.response", args[0][button-1][2], args[0][button-1], 0, 0, 0 )


#Selection of a Classic House
def gump3callback( char, args, target ):
	button = target.button
	if button > 0:
		char.socket.attachmultitarget( "deeds.multideed.response2", args[0][button-1][2], args[0][button-1], 0, 0, 0 )


#Custom House placement warning gump callback
def gump2callback( char, args, target ):
	button = target.button
	if button == 1:
		foundation( char, args[1], args[0][0], args[0][1], args[0][2], args[0][3], args[0][4] )


#Classic House placement warning gump callback
def gump4callback( char, args, target ):
	button = target.button
	if button == 1:
		placement( char, args[1], args[0][3], args[0][2], args[0][4] )


# Response for a Custom House
def response( char, args, target ):

	# Checking if Player Can reach the Destination
	if not char.canreach(target.pos, 20):
		char.socket.sysmessage('You can\'t reach that.')
		return

	# Checking if Can Place the Multi
	(canplace, moveout) = wolfpack.canplace(target.pos, args[2], 4)

	if not canplace:
		char.socket.sysmessage('This is not a Valid Place')
		return

	gump2( char, gump2callback, args, target )
	return 1


# Response for a Classic House
def response2( char, args, target ):

	# Checking if Player Can reach the Destination
	if not char.canreach(target.pos, 20):
		char.socket.sysmessage('You can\'t reach that.')
		return

	# Checking if Can Place the Multi
	(canplace, moveout) = wolfpack.canplace(target.pos, args[2], 4)

	if not canplace:
		char.socket.sysmessage('This is not a Valid Place')
		return

	gump2( char, gump4callback, args, target )
	return 1


# Making the House
def placement(player, target, multiid, id, value):

	# Check if Player can Reach the Location
	if not player.canreach(target.pos, 20):
		player.socket.sysmessage('You can\'t reach that.')
		return

	# Check if Player Can Place Multi here
	(canplace, moveout) = wolfpack.canplace(target.pos, id, 4)

	if not canplace:
		player.socket.sysmessage('CAN\'T PLACE THERE')
		return

	# We may only own one house (Same for gamemasters -> registry)
	if not CAN_HAVE_MULTIPLE_HOUSES:
		house = housing.findHouse(player)
		if house:
			player.socket.clilocmessage(501271)
			return

	# The Amount of Gold in Bank
	bank = player.getbankbox()
	if bank:
		gold = bank.countresource( 0xEED, 0x0 )
	else:
		if not player.gm:
			player.socket.sysmessage('You dont have the correct amount in your Bank')
			return

	if (gold < value):
		if not player.gm:
			player.socket.sysmessage('You dont have the correct amount in your Bank')
			return

	# Consuming Gold
	if not player.gm:
		bank.useresource( value, 0xEED )

	
	####################################################
	# Placing the House
	####################################################
	house = wolfpack.addmulti( multiid )
	house.owner = player
	house.moveto(target.pos)
	house.update()

	# Registering Current Date and Time
	agora = datetime.date.today()
	h, m, s = localtime(time())[3:6]
	date = str(agora.year) + "-" + str(agora.month) + "-" + str(agora.day) + " " + str(h) + ":" + str(m) + ":" + str(s)

	house.settag('builton', date)

	# House Value
	house.settag('value', value)

	# Registering House
	housing.registerHouse(house)

	for obj in moveout:
		obj.removefromview()
		obj.moveto(player.pos)
		obj.update()
		if obj.ischar() and obj.socket:
			obj.socket.resendworld()

# Making the Foundation
def foundation( player, target, width, height, id, multiid, value ):
	# Check if Player can Reach the Location
	if not player.canreach(target.pos, 20):
		player.socket.sysmessage('You can\'t reach that.')
		return

	# Check if Player Can Place Multi here
	(canplace, moveout) = wolfpack.canplace(target.pos, id, 4)

	if not canplace:
		player.socket.sysmessage('CAN\'T PLACE THERE')
		return

	# We may only own one house (Same for gamemasters -> registry)
	if not CAN_HAVE_MULTIPLE_HOUSES:
		house = housing.findHouse(player)
		if house:
			player.socket.clilocmessage(501271)
			return

	# The Amount of Gold in Bank
	bank = player.getbankbox()
	if bank:
		gold = bank.countresource( 0xEED, 0x0 )
	else:
		gold = 0
		if not player.gm:
			player.socket.sysmessage('You dont have the correct amount in your Bank')
			return

	if (gold < value):
		if not player.gm:
			player.socket.sysmessage('You dont have the correct amount in your Bank')
			return

	# Consuming Gold
	if not player.gm:
		bank.useresource( value, 0xEED )

	####################################################
	# Placing the Foundation Multi
	####################################################
	multi = wolfpack.addmulti(str(multiid))
	if not multi:
		player.socket.sysmessage("Multi '%s' not yet implemented" % multiid)
		return True

	multi.owner = player
	multi.decay = False
	multi.moveto( target.pos )

	# Registering Current Date and Time
	agora = datetime.date.today()
	h, m, s = localtime(time())[3:6]
	date = str(agora.year) + "-" + str(agora.month) + "-" + str(agora.day) + " " + str(h) + ":" + str(m) + ":" + str(s)

	multi.settag('builton', date)
	multi.settag('value', value)

	# Registering Foundation in Housing registry
	housing.registerHouse(multi)

	import housing.customhouse as ch
	design = ch.get_or_create_design(multi)
	multi.settag('ch_revision', design['revision'])

	multi.update()

	left = int(width // 2)
	right = int(left - ( width - 1 ))
	bottom = int(height // 2)
	top = int(bottom - ( height - 1 ))

	signpost = wolfpack.additem( "b98" )
	sign = wolfpack.additem( "bd2" )
	signpost.decay = False
	sign.decay = False

	x = multi.pos.x + right
	y = multi.pos.y + bottom + 1
	z = multi.pos.z + 7
	map = multi.pos.map
	newpos = wolfpack.coord( x, y, z, map )

	signpost.moveto( newpos )
	sign.moveto( newpos )
	sign.settag( 'house', multi.serial )
	sign.addscript( 'signpost' )
	signpost.update()
	sign.update()

	for obj in moveout:
		obj.removefromview()
		obj.moveto(player.pos)
		obj.update()
		if obj.ischar() and obj.socket:
			obj.socket.resendworld()


#House type selection common/2-story/3-story
def gump0( char, callback ):
	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 270 145}" )
	mygump.addRawLayout( "{gumppictiled 10 10 250 125 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 250 125}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 10 10 250 20 1060239 0 0 32767}" )
	mygump.addRawLayout( "{button 10 110 4017 4019 1 0 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 110 150 20 3000363 0 0 32767}" )
	mygump.addRawLayout( "{button 10 40 4005 4007 1 0 1}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 40 200 20 1060390 0 0 32767}" )
	mygump.addRawLayout( "{button 10 60 4005 4007 1 0 2}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 60 200 20 1060391 0 0 32767}" )
	mygump.addRawLayout( "{button 10 80 4005 4007 1 0 3}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 80 200 20 1060392 0 0 32767}" )
	mygump.setCallback( callback )
	mygump.send( char )

#2-story house size selection
def gump1( char, callback, chsizes ):
	bank = char.getbankbox()
	if bank:
		gold = bank.countresource( 0xEED, 0x0 )
	gump_params = [
	str( gold ),
	"425",
	"212",
	"30500",
	"580",
	"290",
	"34500",
	"650",
	"325",
	"38500",
	"700",
	"350",
	"42500",
	"750",
	"375",
	"46500",
	"800",
	"400",
	"50500",
	"39000",
	"43500",
	"48000",
	"52500",
	"850",
	"57000",
	"1100",
	"550",
	"61500",
	"48500",
	"53500",
	"58500",
	"63500",
	"68500",
	"59000",
	"64500",
	"70000",
	"1150",
	"575",
	"75500",
	"70500",
	"76500",
	"1200",
	"600",
	"82500",
	"83000",
	"1250",
	"625",
	"89500",
	"1300",
	"96500" ]
	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 520 420}" )
	mygump.addRawLayout( "{gumppictiled 10 10 500 20 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 500 20}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 10 10 500 20 1060239 0 0 32767}" )
	mygump.addRawLayout( "{gumppictiled 10 40 500 20 2624}" )
	mygump.addRawLayout( "{checkertrans 10 40 500 20}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 40 225 20 1060235 0 0 32767}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 275 40 75 20 1060236 0 0 32767}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 350 40 75 20 1060237 0 0 32767}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 425 40 75 20 1060034 0 0 32767}" )
	mygump.addRawLayout( "{gumppictiled 10 70 500 280 2624}" )
	mygump.addRawLayout( "{checkertrans 10 70 500 280}" )
	mygump.addRawLayout( "{gumppictiled 10 360 500 20 2624}" )
	mygump.addRawLayout( "{checkertrans 10 360 500 20}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 10 360 250 20 1060645 0 0 32767}" )
	mygump.addRawLayout( "{text 250 360 1152 0}" )
	mygump.addRawLayout( "{gumppictiled 10 390 500 20 2624}" )
	mygump.addRawLayout( "{checkertrans 10 390 500 20}" )
	mygump.addRawLayout( "{button 10 390 4017 4019 1 0 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 390 100 20 3000363 0 0 32767}" )
	mygump.addRawLayout( "{page 1}" )
	mygump.addRawLayout( "{button 10 70 4005 4007 1 0 1}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 70 225 20 1060241 0 0 32767}" )
	mygump.addRawLayout( "{text 275 70 1152 1}" )
	mygump.addRawLayout( "{text 350 70 1152 2}" )
	mygump.addRawLayout( "{text 425 70 1152 3}" )
	mygump.addRawLayout( "{button 10 90 4005 4007 1 0 2}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 90 225 20 1060242 0 0 32767}" )
	mygump.addRawLayout( "{text 275 90 1152 4}" )
	mygump.addRawLayout( "{text 350 90 1152 5}" )
	mygump.addRawLayout( "{text 425 90 1152 6}" )
	mygump.addRawLayout( "{button 10 110 4005 4007 1 0 3}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 110 225 20 1060243 0 0 32767}" )
	mygump.addRawLayout( "{text 275 110 1152 7}" )
	mygump.addRawLayout( "{text 350 110 1152 8}" )
	mygump.addRawLayout( "{text 425 110 1152 9}" )
	mygump.addRawLayout( "{button 10 130 4005 4007 1 0 4}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 130 225 20 1060244 0 0 32767}" )
	mygump.addRawLayout( "{text 275 130 1152 10}" )
	mygump.addRawLayout( "{text 350 130 1152 11}" )
	mygump.addRawLayout( "{text 425 130 1152 12}" )
	mygump.addRawLayout( "{button 10 150 4005 4007 1 0 5}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 150 225 20 1060245 0 0 32767}" )
	mygump.addRawLayout( "{text 275 150 1152 13}" )
	mygump.addRawLayout( "{text 350 150 1152 14}" )
	mygump.addRawLayout( "{text 425 150 1152 15}" )
	mygump.addRawLayout( "{button 10 170 4005 4007 1 0 6}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 170 225 20 1060246 0 0 32767}" )
	mygump.addRawLayout( "{text 275 170 1152 16}" )
	mygump.addRawLayout( "{text 350 170 1152 17}" )
	mygump.addRawLayout( "{text 425 170 1152 18}" )
	mygump.addRawLayout( "{button 10 190 4005 4007 1 0 7}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 190 225 20 1060253 0 0 32767}" )
	mygump.addRawLayout( "{text 275 190 1152 4}" )
	mygump.addRawLayout( "{text 350 190 1152 5}" )
	mygump.addRawLayout( "{text 425 190 1152 6}" )
	mygump.addRawLayout( "{button 10 210 4005 4007 1 0 8}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 210 225 20 1060254 0 0 32767}" )
	mygump.addRawLayout( "{text 275 210 1152 7}" )
	mygump.addRawLayout( "{text 350 210 1152 8}" )
	mygump.addRawLayout( "{text 425 210 1152 19}" )
	mygump.addRawLayout( "{button 10 230 4005 4007 1 0 9}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 230 225 20 1060255 0 0 32767}" )
	mygump.addRawLayout( "{text 275 230 1152 10}" )
	mygump.addRawLayout( "{text 350 230 1152 11}" )
	mygump.addRawLayout( "{text 425 230 1152 20}" )
	mygump.addRawLayout( "{button 10 250 4005 4007 1 0 10}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 250 225 20 1060256 0 0 32767}" )
	mygump.addRawLayout( "{text 275 250 1152 13}" )
	mygump.addRawLayout( "{text 350 250 1152 14}" )
	mygump.addRawLayout( "{text 425 250 1152 21}" )
	mygump.addRawLayout( "{button 10 270 4005 4007 1 0 11}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 270 225 20 1060257 0 0 32767}" )
	mygump.addRawLayout( "{text 275 270 1152 16}" )
	mygump.addRawLayout( "{text 350 270 1152 17}" )
	mygump.addRawLayout( "{text 425 270 1152 22}" )
	mygump.addRawLayout( "{button 10 290 4005 4007 1 0 12}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 290 225 20 1060258 0 0 32767}" )
	mygump.addRawLayout( "{text 275 290 1152 23}" )
	mygump.addRawLayout( "{text 350 290 1152 1}" )
	mygump.addRawLayout( "{text 425 290 1152 24}" )
	mygump.addRawLayout( "{button 10 310 4005 4007 1 0 13}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 310 225 20 1060259 0 0 32767}" )
	mygump.addRawLayout( "{text 275 310 1152 25}" )
	mygump.addRawLayout( "{text 350 310 1152 26}" )
	mygump.addRawLayout( "{text 425 310 1152 27}" )
	mygump.addRawLayout( "{button 10 330 4005 4007 1 0 14}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 330 225 20 1060265 0 0 32767}" )
	mygump.addRawLayout( "{text 275 330 1152 7}" )
	mygump.addRawLayout( "{text 350 330 1152 8}" )
	mygump.addRawLayout( "{text 425 330 1152 9}" )
	mygump.addRawLayout( "{button 450 390 4005 4007 0 2 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 400 390 100 20 3000406 0 0 32767}" )
	mygump.addRawLayout( "{page 2}" )
	mygump.addRawLayout( "{button 200 390 4014 4016 0 1 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 250 390 100 20 3000405 0 0 32767}" )
	mygump.addRawLayout( "{button 10 70 4005 4007 1 0 15}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 70 225 20 1060266 0 0 32767}" )
	mygump.addRawLayout( "{text 275 70 1152 10}" )
	mygump.addRawLayout( "{text 350 70 1152 11}" )
	mygump.addRawLayout( "{text 425 70 1152 20}" )
	mygump.addRawLayout( "{button 10 90 4005 4007 1 0 16}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 90 225 20 1060267 0 0 32767}" )
	mygump.addRawLayout( "{text 275 90 1152 13}" )
	mygump.addRawLayout( "{text 350 90 1152 14}" )
	mygump.addRawLayout( "{text 425 90 1152 28}" )
	mygump.addRawLayout( "{button 10 110 4005 4007 1 0 17}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 110 225 20 1060268 0 0 32767}" )
	mygump.addRawLayout( "{text 275 110 1152 16}" )
	mygump.addRawLayout( "{text 350 110 1152 17}" )
	mygump.addRawLayout( "{text 425 110 1152 29}" )
	mygump.addRawLayout( "{button 10 130 4005 4007 1 0 18}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 130 225 20 1060269 0 0 32767}" )
	mygump.addRawLayout( "{text 275 130 1152 23}" )
	mygump.addRawLayout( "{text 350 130 1152 1}" )
	mygump.addRawLayout( "{text 425 130 1152 30}" )
	mygump.addRawLayout( "{button 10 150 4005 4007 1 0 19}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 150 225 20 1060270 0 0 32767}" )
	mygump.addRawLayout( "{text 275 150 1152 25}" )
	mygump.addRawLayout( "{text 350 150 1152 26}" )
	mygump.addRawLayout( "{text 425 150 1152 31}" )
	mygump.addRawLayout( "{button 10 170 4005 4007 1 0 20}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 170 225 20 1060271 0 0 32767}" )
	mygump.addRawLayout( "{text 275 170 1152 25}" )
	mygump.addRawLayout( "{text 350 170 1152 26}" )
	mygump.addRawLayout( "{text 425 170 1152 32}" )
	mygump.addRawLayout( "{button 10 190 4005 4007 1 0 21}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 190 225 20 1060277 0 0 32767}" )
	mygump.addRawLayout( "{text 275 190 1152 10}" )
	mygump.addRawLayout( "{text 350 190 1152 11}" )
	mygump.addRawLayout( "{text 425 190 1152 12}" )
	mygump.addRawLayout( "{button 10 210 4005 4007 1 0 22}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 210 225 20 1060278 0 0 32767}" )
	mygump.addRawLayout( "{text 275 210 1152 13}" )
	mygump.addRawLayout( "{text 350 210 1152 14}" )
	mygump.addRawLayout( "{text 425 210 1152 21}" )
	mygump.addRawLayout( "{button 10 230 4005 4007 1 0 23}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 230 225 20 1060279 0 0 32767}" )
	mygump.addRawLayout( "{text 275 230 1152 16}" )
	mygump.addRawLayout( "{text 350 230 1152 17}" )
	mygump.addRawLayout( "{text 425 230 1152 29}" )
	mygump.addRawLayout( "{button 10 250 4005 4007 1 0 24}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 250 225 20 1060280 0 0 32767}" )
	mygump.addRawLayout( "{text 275 250 1152 23}" )
	mygump.addRawLayout( "{text 350 250 1152 1}" )
	mygump.addRawLayout( "{text 425 250 1152 33}" )
	mygump.addRawLayout( "{button 10 270 4005 4007 1 0 25}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 270 225 20 1060281 0 0 32767}" )
	mygump.addRawLayout( "{text 275 270 1152 25}" )
	mygump.addRawLayout( "{text 350 270 1152 26}" )
	mygump.addRawLayout( "{text 425 270 1152 34}" )
	mygump.addRawLayout( "{button 10 290 4005 4007 1 0 26}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 290 225 20 1060282 0 0 32767}" )
	mygump.addRawLayout( "{text 275 290 1152 25}" )
	mygump.addRawLayout( "{text 350 290 1152 26}" )
	mygump.addRawLayout( "{text 425 290 1152 35}" )
	mygump.addRawLayout( "{button 10 310 4005 4007 1 0 27}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 310 225 20 1060283 0 0 32767}" )
	mygump.addRawLayout( "{text 275 310 1152 36}" )
	mygump.addRawLayout( "{text 350 310 1152 37}" )
	mygump.addRawLayout( "{text 425 310 1152 38}" )
	mygump.addRawLayout( "{button 10 330 4005 4007 1 0 28}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 330 225 20 1060289 0 0 32767}" )
	mygump.addRawLayout( "{text 275 330 1152 13}" )
	mygump.addRawLayout( "{text 350 330 1152 14}" )
	mygump.addRawLayout( "{text 425 330 1152 15}" )
	mygump.addRawLayout( "{button 450 390 4005 4007 0 3 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 400 390 100 20 3000406 0 0 32767}" )
	mygump.addRawLayout( "{page 3}" )
	mygump.addRawLayout( "{button 200 390 4014 4016 0 2 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 250 390 100 20 3000405 0 0 32767}" )
	mygump.addRawLayout( "{button 10 70 4005 4007 1 0 29}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 70 225 20 1060290 0 0 32767}" )
	mygump.addRawLayout( "{text 275 70 1152 16}" )
	mygump.addRawLayout( "{text 350 70 1152 17}" )
	mygump.addRawLayout( "{text 425 70 1152 22}" )
	mygump.addRawLayout( "{button 10 90 4005 4007 1 0 30}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 90 225 20 1060291 0 0 32767}" )
	mygump.addRawLayout( "{text 275 90 1152 23}" )
	mygump.addRawLayout( "{text 350 90 1152 1}" )
	mygump.addRawLayout( "{text 425 90 1152 30}" )
	mygump.addRawLayout( "{button 10 110 4005 4007 1 0 31}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 110 225 20 1060292 0 0 32767}" )
	mygump.addRawLayout( "{text 275 110 1152 25}" )
	mygump.addRawLayout( "{text 350 110 1152 26}" )
	mygump.addRawLayout( "{text 425 110 1152 34}" )
	mygump.addRawLayout( "{button 10 130 4005 4007 1 0 32}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 130 225 20 1060293 0 0 32767}" )
	mygump.addRawLayout( "{text 275 130 1152 25}" )
	mygump.addRawLayout( "{text 350 130 1152 26}" )
	mygump.addRawLayout( "{text 425 130 1152 39}" )
	mygump.addRawLayout( "{button 10 150 4005 4007 1 0 33}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 150 225 20 1060294 0 0 32767}" )
	mygump.addRawLayout( "{text 275 150 1152 36}" )
	mygump.addRawLayout( "{text 350 150 1152 37}" )
	mygump.addRawLayout( "{text 425 150 1152 40}" )
	mygump.addRawLayout( "{button 10 170 4005 4007 1 0 34}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 170 225 20 1060295 0 0 32767}" )
	mygump.addRawLayout( "{text 275 170 1152 41}" )
	mygump.addRawLayout( "{text 350 170 1152 42}" )
	mygump.addRawLayout( "{text 425 170 1152 43}" )
	mygump.addRawLayout( "{button 10 190 4005 4007 1 0 35}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 190 225 20 1060301 0 0 32767}" )
	mygump.addRawLayout( "{text 275 190 1152 16}" )
	mygump.addRawLayout( "{text 350 190 1152 17}" )
	mygump.addRawLayout( "{text 425 190 1152 18}" )
	mygump.addRawLayout( "{button 10 210 4005 4007 1 0 36}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 210 225 20 1060302 0 0 32767}" )
	mygump.addRawLayout( "{text 275 210 1152 23}" )
	mygump.addRawLayout( "{text 350 210 1152 1}" )
	mygump.addRawLayout( "{text 425 210 1152 24}" )
	mygump.addRawLayout( "{button 10 230 4005 4007 1 0 37}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 230 225 20 1060303 0 0 32767}" )
	mygump.addRawLayout( "{text 275 230 1152 25}" )
	mygump.addRawLayout( "{text 350 230 1152 26}" )
	mygump.addRawLayout( "{text 425 230 1152 31}" )
	mygump.addRawLayout( "{button 10 250 4005 4007 1 0 38}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 250 225 20 1060304 0 0 32767}" )
	mygump.addRawLayout( "{text 275 250 1152 25}" )
	mygump.addRawLayout( "{text 350 250 1152 26}" )
	mygump.addRawLayout( "{text 425 250 1152 35}" )
	mygump.addRawLayout( "{button 10 270 4005 4007 1 0 39}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 270 225 20 1060305 0 0 32767}" )
	mygump.addRawLayout( "{text 275 270 1152 36}" )
	mygump.addRawLayout( "{text 350 270 1152 37}" )
	mygump.addRawLayout( "{text 425 270 1152 40}" )
	mygump.addRawLayout( "{button 10 290 4005 4007 1 0 40}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 290 225 20 1060306 0 0 32767}" )
	mygump.addRawLayout( "{text 275 290 1152 41}" )
	mygump.addRawLayout( "{text 350 290 1152 42}" )
	mygump.addRawLayout( "{text 425 290 1152 44}" )
	mygump.addRawLayout( "{button 10 310 4005 4007 1 0 41}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 310 225 20 1060307 0 0 32767}" )
	mygump.addRawLayout( "{text 275 310 1152 45}" )
	mygump.addRawLayout( "{text 350 310 1152 46}" )
	mygump.addRawLayout( "{text 425 310 1152 47}" )
	mygump.addRawLayout( "{button 10 330 4005 4007 1 0 42}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 330 225 20 1060314 0 0 32767}" )
	mygump.addRawLayout( "{text 275 330 1152 25}" )
	mygump.addRawLayout( "{text 350 330 1152 26}" )
	mygump.addRawLayout( "{text 425 330 1152 27}" )
	mygump.addRawLayout( "{button 450 390 4005 4007 0 4 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 400 390 100 20 3000406 0 0 32767}" )
	mygump.addRawLayout( "{page 4}" )
	mygump.addRawLayout( "{button 200 390 4014 4016 0 3 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 250 390 100 20 3000405 0 0 32767}" )
	mygump.addRawLayout( "{button 10 70 4005 4007 1 0 43}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 70 225 20 1060315 0 0 32767}" )
	mygump.addRawLayout( "{text 275 70 1152 25}" )
	mygump.addRawLayout( "{text 350 70 1152 26}" )
	mygump.addRawLayout( "{text 425 70 1152 32}" )
	mygump.addRawLayout( "{button 10 90 4005 4007 1 0 44}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 90 225 20 1060316 0 0 32767}" )
	mygump.addRawLayout( "{text 275 90 1152 36}" )
	mygump.addRawLayout( "{text 350 90 1152 37}" )
	mygump.addRawLayout( "{text 425 90 1152 38}" )
	mygump.addRawLayout( "{button 10 110 4005 4007 1 0 45}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 110 225 20 1060317 0 0 32767}" )
	mygump.addRawLayout( "{text 275 110 1152 41}" )
	mygump.addRawLayout( "{text 350 110 1152 42}" )
	mygump.addRawLayout( "{text 425 110 1152 43}" )
	mygump.addRawLayout( "{button 10 130 4005 4007 1 0 46}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 130 225 20 1060318 0 0 32767}" )
	mygump.addRawLayout( "{text 275 130 1152 45}" )
	mygump.addRawLayout( "{text 350 130 1152 46}" )
	mygump.addRawLayout( "{text 425 130 1152 47}" )
	mygump.addRawLayout( "{button 10 150 4005 4007 1 0 47}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 150 225 20 1060319 0 0 32767}" )
	mygump.addRawLayout( "{text 275 150 1152 48}" )
	mygump.addRawLayout( "{text 350 150 1152 7}" )
	mygump.addRawLayout( "{text 425 150 1152 49}" )

	#Add params
	for line in gump_params:
		mygump.addRawText( line )

	mygump.setArgs( [chsizes] )
	mygump.setCallback( callback )
	mygump.send( char )


def gump3( char, callback, chsizes ):
	bank = char.getbankbox()
	if bank:
		gold = bank.countresource( 0xEED, 0x0 )
	gump_params = [
	str( gold ),
	"1150",
	"575",
	"73500",
	"1200",
	"600",
	"81000",
	"1250",
	"625",
	"86500",
	"88500",
	"1300",
	"650",
	"94500",
	"1350",
	"675",
	"100500",
	"96000",
	"102500",
	"1370",
	"685",
	"109000",
	"115500",
	"103500",
	"110500",
	"117500",
	"2119",
	"1059",
	"124500",
	"131500",
	"111000",
	"118500",
	"126000",
	"133500",
	"141000",
	"126500",
	"134500",
	"142500",
	"150500",
	"143000",
	"151500",
	"160000",
	"160500",
	"169500",
	"179000"]

	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 520 420}" )
	mygump.addRawLayout( "{gumppictiled 10 10 500 20 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 500 20}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 10 10 500 20 1060239 0 0 32767}" )
	mygump.addRawLayout( "{gumppictiled 10 40 500 20 2624}" )
	mygump.addRawLayout( "{checkertrans 10 40 500 20}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 40 225 20 1060235 0 0 32767}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 275 40 75 20 1060236 0 0 32767}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 350 40 75 20 1060237 0 0 32767}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 425 40 75 20 1060034 0 0 32767}" )
	mygump.addRawLayout( "{gumppictiled 10 70 500 280 2624}" )
	mygump.addRawLayout( "{checkertrans 10 70 500 280}" )
	mygump.addRawLayout( "{gumppictiled 10 360 500 20 2624}" )
	mygump.addRawLayout( "{checkertrans 10 360 500 20}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 10 360 250 20 1060645 0 0 32767}" )
	mygump.addRawLayout( "{text 250 360 1152 0}" )
	mygump.addRawLayout( "{gumppictiled 10 390 500 20 2624}" )
	mygump.addRawLayout( "{checkertrans 10 390 500 20}" )
	mygump.addRawLayout( "{button 10 390 4017 4019 1 0 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 390 100 20 3000363 0 0 32767}" )
	mygump.addRawLayout( "{page 1}" )
	mygump.addRawLayout( "{button 10 70 4005 4007 1 0 1}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 70 225 20 1060272 0 0 32767}" )
	mygump.addRawLayout( "{text 275 70 1152 1}" )
	mygump.addRawLayout( "{text 350 70 1152 2}" )
	mygump.addRawLayout( "{text 425 70 1152 3}" )
	mygump.addRawLayout( "{button 10 90 4005 4007 1 0 2}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 90 225 20 1060284 0 0 32767}" )
	mygump.addRawLayout( "{text 275 90 1152 4}" )
	mygump.addRawLayout( "{text 350 90 1152 5}" )
	mygump.addRawLayout( "{text 425 90 1152 6}" )
	mygump.addRawLayout( "{button 10 110 4005 4007 1 0 3}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 110 225 20 1060285 0 0 32767}" )
	mygump.addRawLayout( "{text 275 110 1152 7}" )
	mygump.addRawLayout( "{text 350 110 1152 8}" )
	mygump.addRawLayout( "{text 425 110 1152 9}" )
	mygump.addRawLayout( "{button 10 130 4005 4007 1 0 4}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 130 225 20 1060296 0 0 32767}" )
	mygump.addRawLayout( "{text 275 130 1152 7}" )
	mygump.addRawLayout( "{text 350 130 1152 8}" )
	mygump.addRawLayout( "{text 425 130 1152 10}" )
	mygump.addRawLayout( "{button 10 150 4005 4007 1 0 5}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 150 225 20 1060297 0 0 32767}" )
	mygump.addRawLayout( "{text 275 150 1152 11}" )
	mygump.addRawLayout( "{text 350 150 1152 12}" )
	mygump.addRawLayout( "{text 425 150 1152 13}" )
	mygump.addRawLayout( "{button 10 170 4005 4007 1 0 6}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 170 225 20 1060298 0 0 32767}" )
	mygump.addRawLayout( "{text 275 170 1152 14}" )
	mygump.addRawLayout( "{text 350 170 1152 15}" )
	mygump.addRawLayout( "{text 425 170 1152 16}" )
	mygump.addRawLayout( "{button 10 190 4005 4007 1 0 7}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 190 225 20 1060308 0 0 32767}" )
	mygump.addRawLayout( "{text 275 190 1152 11}" )
	mygump.addRawLayout( "{text 350 190 1152 12}" )
	mygump.addRawLayout( "{text 425 190 1152 17}" )
	mygump.addRawLayout( "{button 10 210 4005 4007 1 0 8}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 210 225 20 1060309 0 0 32767}" )
	mygump.addRawLayout( "{text 275 210 1152 14}" )
	mygump.addRawLayout( "{text 350 210 1152 15}" )
	mygump.addRawLayout( "{text 425 210 1152 18}" )
	mygump.addRawLayout( "{button 10 230 4005 4007 1 0 9}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 230 225 20 1060310 0 0 32767}" )
	mygump.addRawLayout( "{text 275 230 1152 19}" )
	mygump.addRawLayout( "{text 350 230 1152 20}" )
	mygump.addRawLayout( "{text 425 230 1152 21}" )
	mygump.addRawLayout( "{button 10 250 4005 4007 1 0 10}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 250 225 20 1060311 0 0 32767}" )
	mygump.addRawLayout( "{text 275 250 1152 19}" )
	mygump.addRawLayout( "{text 350 250 1152 20}" )
	mygump.addRawLayout( "{text 425 250 1152 22}" )
	mygump.addRawLayout( "{button 10 270 4005 4007 1 0 11}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 270 225 20 1060320 0 0 32767}" )
	mygump.addRawLayout( "{text 275 270 1152 14}" )
	mygump.addRawLayout( "{text 350 270 1152 15}" )
	mygump.addRawLayout( "{text 425 270 1152 23}" )
	mygump.addRawLayout( "{button 10 290 4005 4007 1 0 12}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 290 225 20 1060321 0 0 32767}" )
	mygump.addRawLayout( "{text 275 290 1152 19}" )
	mygump.addRawLayout( "{text 350 290 1152 20}" )
	mygump.addRawLayout( "{text 425 290 1152 24}" )
	mygump.addRawLayout( "{button 10 310 4005 4007 1 0 13}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 310 225 20 1060322 0 0 32767}" )
	mygump.addRawLayout( "{text 275 310 1152 19}" )
	mygump.addRawLayout( "{text 350 310 1152 20}" )
	mygump.addRawLayout( "{text 425 310 1152 25}" )
	mygump.addRawLayout( "{button 10 330 4005 4007 1 0 14}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 330 225 20 1060323 0 0 32767}" )
	mygump.addRawLayout( "{text 275 330 1152 26}" )
	mygump.addRawLayout( "{text 350 330 1152 27}" )
	mygump.addRawLayout( "{text 425 330 1152 28}" )
	mygump.addRawLayout( "{button 450 390 4005 4007 0 2 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 400 390 100 20 3000406 0 0 32767}" )
	mygump.addRawLayout( "{page 2}" )
	mygump.addRawLayout( "{button 200 390 4014 4016 0 1 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 250 390 100 20 3000405 0 0 32767}" )
	mygump.addRawLayout( "{button 10 70 4005 4007 1 0 15}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 70 225 20 1060324 0 0 32767}" )
	mygump.addRawLayout( "{text 275 70 1152 26}" )
	mygump.addRawLayout( "{text 350 70 1152 27}" )
	mygump.addRawLayout( "{text 425 70 1152 29}" )
	mygump.addRawLayout( "{button 10 90 4005 4007 1 0 16}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 90 225 20 1060327 0 0 32767}" )
	mygump.addRawLayout( "{text 275 90 1152 1}" )
	mygump.addRawLayout( "{text 350 90 1152 2}" )
	mygump.addRawLayout( "{text 425 90 1152 3}" )
	mygump.addRawLayout( "{button 10 110 4005 4007 1 0 17}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 110 225 20 1060328 0 0 32767}" )
	mygump.addRawLayout( "{text 275 110 1152 4}" )
	mygump.addRawLayout( "{text 350 110 1152 5}" )
	mygump.addRawLayout( "{text 425 110 1152 6}" )
	mygump.addRawLayout( "{button 10 130 4005 4007 1 0 18}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 130 225 20 1060329 0 0 32767}" )
	mygump.addRawLayout( "{text 275 130 1152 7}" )
	mygump.addRawLayout( "{text 350 130 1152 8}" )
	mygump.addRawLayout( "{text 425 130 1152 10}" )
	mygump.addRawLayout( "{button 10 150 4005 4007 1 0 19}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 150 225 20 1060330 0 0 32767}" )
	mygump.addRawLayout( "{text 275 150 1152 11}" )
	mygump.addRawLayout( "{text 350 150 1152 12}" )
	mygump.addRawLayout( "{text 425 150 1152 17}" )
	mygump.addRawLayout( "{button 10 170 4005 4007 1 0 20}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 170 225 20 1060331 0 0 32767}" )
	mygump.addRawLayout( "{text 275 170 1152 14}" )
	mygump.addRawLayout( "{text 350 170 1152 15}" )
	mygump.addRawLayout( "{text 425 170 1152 23}" )
	mygump.addRawLayout( "{button 10 190 4005 4007 1 0 21}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 190 225 20 1060332 0 0 32767}" )
	mygump.addRawLayout( "{text 275 190 1152 19}" )
	mygump.addRawLayout( "{text 350 190 1152 20}" )
	mygump.addRawLayout( "{text 425 190 1152 30}" )
	mygump.addRawLayout( "{button 10 210 4005 4007 1 0 22}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 210 225 20 1060333 0 0 32767}" )
	mygump.addRawLayout( "{text 275 210 1152 19}" )
	mygump.addRawLayout( "{text 350 210 1152 20}" )
	mygump.addRawLayout( "{text 425 210 1152 31}" )
	mygump.addRawLayout( "{button 10 230 4005 4007 1 0 23}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 230 225 20 1060334 0 0 32767}" )
	mygump.addRawLayout( "{text 275 230 1152 26}" )
	mygump.addRawLayout( "{text 350 230 1152 27}" )
	mygump.addRawLayout( "{text 425 230 1152 32}" )
	mygump.addRawLayout( "{button 10 250 4005 4007 1 0 24}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 250 225 20 1060335 0 0 32767}" )
	mygump.addRawLayout( "{text 275 250 1152 26}" )
	mygump.addRawLayout( "{text 350 250 1152 27}" )
	mygump.addRawLayout( "{text 425 250 1152 33}" )
	mygump.addRawLayout( "{button 10 270 4005 4007 1 0 25}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 270 225 20 1060336 0 0 32767}" )
	mygump.addRawLayout( "{text 275 270 1152 26}" )
	mygump.addRawLayout( "{text 350 270 1152 27}" )
	mygump.addRawLayout( "{text 425 270 1152 34}" )
	mygump.addRawLayout( "{button 10 290 4005 4007 1 0 26}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 290 225 20 1060340 0 0 32767}" )
	mygump.addRawLayout( "{text 275 290 1152 7}" )
	mygump.addRawLayout( "{text 350 290 1152 8}" )
	mygump.addRawLayout( "{text 425 290 1152 9}" )
	mygump.addRawLayout( "{button 10 310 4005 4007 1 0 27}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 310 225 20 1060341 0 0 32767}" )
	mygump.addRawLayout( "{text 275 310 1152 11}" )
	mygump.addRawLayout( "{text 350 310 1152 12}" )
	mygump.addRawLayout( "{text 425 310 1152 13}" )
	mygump.addRawLayout( "{button 10 330 4005 4007 1 0 28}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 330 225 20 1060342 0 0 32767}" )
	mygump.addRawLayout( "{text 275 330 1152 14}" )
	mygump.addRawLayout( "{text 350 330 1152 15}" )
	mygump.addRawLayout( "{text 425 330 1152 18}" )
	mygump.addRawLayout( "{button 450 390 4005 4007 0 3 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 400 390 100 20 3000406 0 0 32767}" )
	mygump.addRawLayout( "{page 3}" )
	mygump.addRawLayout( "{button 200 390 4014 4016 0 2 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 250 390 100 20 3000405 0 0 32767}" )
	mygump.addRawLayout( "{button 10 70 4005 4007 1 0 29}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 70 225 20 1060343 0 0 32767}" )
	mygump.addRawLayout( "{text 275 70 1152 19}" )
	mygump.addRawLayout( "{text 350 70 1152 20}" )
	mygump.addRawLayout( "{text 425 70 1152 24}" )
	mygump.addRawLayout( "{button 10 90 4005 4007 1 0 30}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 90 225 20 1060344 0 0 32767}" )
	mygump.addRawLayout( "{text 275 90 1152 19}" )
	mygump.addRawLayout( "{text 350 90 1152 20}" )
	mygump.addRawLayout( "{text 425 90 1152 31}" )
	mygump.addRawLayout( "{button 10 110 4005 4007 1 0 31}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 110 225 20 1060345 0 0 32767}" )
	mygump.addRawLayout( "{text 275 110 1152 26}" )
	mygump.addRawLayout( "{text 350 110 1152 27}" )
	mygump.addRawLayout( "{text 425 110 1152 35}" )
	mygump.addRawLayout( "{button 10 130 4005 4007 1 0 32}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 130 225 20 1060346 0 0 32767}" )
	mygump.addRawLayout( "{text 275 130 1152 26}" )
	mygump.addRawLayout( "{text 350 130 1152 27}" )
	mygump.addRawLayout( "{text 425 130 1152 36}" )
	mygump.addRawLayout( "{button 10 150 4005 4007 1 0 33}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 150 225 20 1060347 0 0 32767}" )
	mygump.addRawLayout( "{text 275 150 1152 26}" )
	mygump.addRawLayout( "{text 350 150 1152 27}" )
	mygump.addRawLayout( "{text 425 150 1152 37}" )
	mygump.addRawLayout( "{button 10 170 4005 4007 1 0 34}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 170 225 20 1060348 0 0 32767}" )
	mygump.addRawLayout( "{text 275 170 1152 26}" )
	mygump.addRawLayout( "{text 350 170 1152 27}" )
	mygump.addRawLayout( "{text 425 170 1152 38}" )
	mygump.addRawLayout( "{button 10 190 4005 4007 1 0 35}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 190 225 20 1060353 0 0 32767}" )
	mygump.addRawLayout( "{text 275 190 1152 14}" )
	mygump.addRawLayout( "{text 350 190 1152 15}" )
	mygump.addRawLayout( "{text 425 190 1152 16}" )
	mygump.addRawLayout( "{button 10 210 4005 4007 1 0 36}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 210 225 20 1060354 0 0 32767}" )
	mygump.addRawLayout( "{text 275 210 1152 19}" )
	mygump.addRawLayout( "{text 350 210 1152 20}" )
	mygump.addRawLayout( "{text 425 210 1152 21}" )
	mygump.addRawLayout( "{button 10 230 4005 4007 1 0 37}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 230 225 20 1060355 0 0 32767}" )
	mygump.addRawLayout( "{text 275 230 1152 19}" )
	mygump.addRawLayout( "{text 350 230 1152 20}" )
	mygump.addRawLayout( "{text 425 230 1152 25}" )
	mygump.addRawLayout( "{button 10 250 4005 4007 1 0 38}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 250 225 20 1060356 0 0 32767}" )
	mygump.addRawLayout( "{text 275 250 1152 26}" )
	mygump.addRawLayout( "{text 350 250 1152 27}" )
	mygump.addRawLayout( "{text 425 250 1152 32}" )
	mygump.addRawLayout( "{button 10 270 4005 4007 1 0 39}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 270 225 20 1060357 0 0 32767}" )
	mygump.addRawLayout( "{text 275 270 1152 26}" )
	mygump.addRawLayout( "{text 350 270 1152 27}" )
	mygump.addRawLayout( "{text 425 270 1152 36}" )
	mygump.addRawLayout( "{button 10 290 4005 4007 1 0 40}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 290 225 20 1060358 0 0 32767}" )
	mygump.addRawLayout( "{text 275 290 1152 26}" )
	mygump.addRawLayout( "{text 350 290 1152 27}" )
	mygump.addRawLayout( "{text 425 290 1152 39}" )
	mygump.addRawLayout( "{button 10 310 4005 4007 1 0 41}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 310 225 20 1060359 0 0 32767}" )
	mygump.addRawLayout( "{text 275 310 1152 26}" )
	mygump.addRawLayout( "{text 350 310 1152 27}" )
	mygump.addRawLayout( "{text 425 310 1152 40}" )
	mygump.addRawLayout( "{button 10 330 4005 4007 1 0 42}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 330 225 20 1060360 0 0 32767}" )
	mygump.addRawLayout( "{text 275 330 1152 26}" )
	mygump.addRawLayout( "{text 350 330 1152 27}" )
	mygump.addRawLayout( "{text 425 330 1152 41}" )
	mygump.addRawLayout( "{button 450 390 4005 4007 0 4 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 400 390 100 20 3000406 0 0 32767}" )
	mygump.addRawLayout( "{page 4}" )
	mygump.addRawLayout( "{button 200 390 4014 4016 0 3 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 250 390 100 20 3000405 0 0 32767}" )
	mygump.addRawLayout( "{button 10 70 4005 4007 1 0 43}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 70 225 20 1060366 0 0 32767}" )
	mygump.addRawLayout( "{text 275 70 1152 19}" )
	mygump.addRawLayout( "{text 350 70 1152 20}" )
	mygump.addRawLayout( "{text 425 70 1152 22}" )
	mygump.addRawLayout( "{button 10 90 4005 4007 1 0 44}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 90 225 20 1060367 0 0 32767}" )
	mygump.addRawLayout( "{text 275 90 1152 26}" )
	mygump.addRawLayout( "{text 350 90 1152 27}" )
	mygump.addRawLayout( "{text 425 90 1152 28}" )
	mygump.addRawLayout( "{button 10 110 4005 4007 1 0 45}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 110 225 20 1060368 0 0 32767}" )
	mygump.addRawLayout( "{text 275 110 1152 26}" )
	mygump.addRawLayout( "{text 350 110 1152 27}" )
	mygump.addRawLayout( "{text 425 110 1152 33}" )
	mygump.addRawLayout( "{button 10 130 4005 4007 1 0 46}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 130 225 20 1060369 0 0 32767}" )
	mygump.addRawLayout( "{text 275 130 1152 26}" )
	mygump.addRawLayout( "{text 350 130 1152 27}" )
	mygump.addRawLayout( "{text 425 130 1152 37}" )
	mygump.addRawLayout( "{button 10 150 4005 4007 1 0 47}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 150 225 20 1060370 0 0 32767}" )
	mygump.addRawLayout( "{text 275 150 1152 26}" )
	mygump.addRawLayout( "{text 350 150 1152 27}" )
	mygump.addRawLayout( "{text 425 150 1152 40}" )
	mygump.addRawLayout( "{button 10 170 4005 4007 1 0 48}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 170 225 20 1060371 0 0 32767}" )
	mygump.addRawLayout( "{text 275 170 1152 26}" )
	mygump.addRawLayout( "{text 350 170 1152 27}" )
	mygump.addRawLayout( "{text 425 170 1152 42}" )
	mygump.addRawLayout( "{button 10 190 4005 4007 1 0 49}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 190 225 20 1060372 0 0 32767}" )
	mygump.addRawLayout( "{text 275 190 1152 26}" )
	mygump.addRawLayout( "{text 350 190 1152 27}" )
	mygump.addRawLayout( "{text 425 190 1152 43}" )
	mygump.addRawLayout( "{button 10 210 4005 4007 1 0 50}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 210 225 20 1060379 0 0 32767}" )
	mygump.addRawLayout( "{text 275 210 1152 26}" )
	mygump.addRawLayout( "{text 350 210 1152 27}" )
	mygump.addRawLayout( "{text 425 210 1152 29}" )
	mygump.addRawLayout( "{button 10 230 4005 4007 1 0 51}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 230 225 20 1060380 0 0 32767}" )
	mygump.addRawLayout( "{text 275 230 1152 26}" )
	mygump.addRawLayout( "{text 350 230 1152 27}" )
	mygump.addRawLayout( "{text 425 230 1152 34}" )
	mygump.addRawLayout( "{button 10 250 4005 4007 1 0 52}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 250 225 20 1060381 0 0 32767}" )
	mygump.addRawLayout( "{text 275 250 1152 26}" )
	mygump.addRawLayout( "{text 350 250 1152 27}" )
	mygump.addRawLayout( "{text 425 250 1152 38}" )
	mygump.addRawLayout( "{button 10 270 4005 4007 1 0 53}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 270 225 20 1060382 0 0 32767}" )
	mygump.addRawLayout( "{text 275 270 1152 26}" )
	mygump.addRawLayout( "{text 350 270 1152 27}" )
	mygump.addRawLayout( "{text 425 270 1152 41}" )
	mygump.addRawLayout( "{button 10 290 4005 4007 1 0 54}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 290 225 20 1060383 0 0 32767}" )
	mygump.addRawLayout( "{text 275 290 1152 26}" )
	mygump.addRawLayout( "{text 350 290 1152 27}" )
	mygump.addRawLayout( "{text 425 290 1152 43}" )
	mygump.addRawLayout( "{button 10 310 4005 4007 1 0 55}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 310 225 20 1060384 0 0 32767}" )
	mygump.addRawLayout( "{text 275 310 1152 26}" )
	mygump.addRawLayout( "{text 350 310 1152 27}" )
	mygump.addRawLayout( "{text 425 310 1152 44}" )

	#Add params
	for line in gump_params:
		mygump.addRawText( line )

	mygump.setArgs( [chsizes] )
	mygump.setCallback( callback )
	mygump.send( char )


def gump2( char, callback, args, target ):
	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 420 280}" )
	mygump.addRawLayout( "{gumppictiled 10 10 400 20 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 400 20}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 10 10 400 20 1060635 0 0 30720}" )
	mygump.addRawLayout( "{gumppictiled 10 40 400 200 2624}" )
	mygump.addRawLayout( "{checkertrans 10 40 400 200}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 10 40 400 200 1049583 0 1 32512}" )
	mygump.addRawLayout( "{gumppictiled 10 250 400 20 2624}" )
	mygump.addRawLayout( "{checkertrans 10 250 400 20}" )
	mygump.addRawLayout( "{button 10 250 4005 4007 1 0 1}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 40 250 170 20 1011036 0 0 32767}" )
	mygump.addRawLayout( "{button 210 250 4005 4007 1 0 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 240 250 170 20 1011012 0 0 32767}" )
	mygump.setCallback( callback )
	mygump.setArgs( [args, target] )
	mygump.send( char )


def gump4( char, callback, chsizes ):
	bank = char.getbankbox()
	if bank:
		gold = bank.countresource( 0xEED, 0x0 )
	gump_params = [
	str( gold ),
	"489",
	"244",
	"37000",
	"36750",
	"35250",
	"50500",
	"52500",
	"667",
	"333",
	"73500",
	"1265",
	"632",
	"113750",
	"76500",
	"81750",
	"1576",
	"788",
	"131500",
	"162750",
	"162000",
	"129250",
	"160500",
	"2437",
	"1218",
	"366500",
	"3019",
	"1509",
	"572750",
	"4688",
	"2344",
	"865250"]

	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 520 420}" )
	mygump.addRawLayout( "{gumppictiled 10 10 500 20 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 500 20}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 10 10 500 20 1060239 0 0 32767}" )
	mygump.addRawLayout( "{gumppictiled 10 40 500 20 2624}" )
	mygump.addRawLayout( "{checkertrans 10 40 500 20}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 40 225 20 1060235 0 0 32767}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 275 40 75 20 1060236 0 0 32767}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 350 40 75 20 1060237 0 0 32767}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 425 40 75 20 1060034 0 0 32767}" )
	mygump.addRawLayout( "{gumppictiled 10 70 500 280 2624}" )
	mygump.addRawLayout( "{checkertrans 10 70 500 280}" )
	mygump.addRawLayout( "{gumppictiled 10 360 500 20 2624}" )
	mygump.addRawLayout( "{checkertrans 10 360 500 20}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 10 360 250 20 1060645 0 0 32767}" )
	mygump.addRawLayout( "{text 250 360 1152 0}" )
	mygump.addRawLayout( "{gumppictiled 10 390 500 20 2624}" )
	mygump.addRawLayout( "{checkertrans 10 390 500 20}" )
	mygump.addRawLayout( "{button 10 390 4017 4019 1 0 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 390 100 20 3000363 0 0 32767}" )
	mygump.addRawLayout( "{page 1}" )
	mygump.addRawLayout( "{button 10 70 4005 4007 1 0 1}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 70 225 20 1011303 0 0 32767}" )
	mygump.addRawLayout( "{text 275 70 1152 1}" )
	mygump.addRawLayout( "{text 350 70 1152 2}" )
	mygump.addRawLayout( "{text 425 70 1152 3}" )
	mygump.addRawLayout( "{button 10 90 4005 4007 1 0 2}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 90 225 20 1011304 0 0 32767}" )
	mygump.addRawLayout( "{text 275 90 1152 1}" )
	mygump.addRawLayout( "{text 350 90 1152 2}" )
	mygump.addRawLayout( "{text 425 90 1152 3}" )
	mygump.addRawLayout( "{button 10 110 4005 4007 1 0 3}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 110 225 20 1011305 0 0 32767}" )
	mygump.addRawLayout( "{text 275 110 1152 1}" )
	mygump.addRawLayout( "{text 350 110 1152 2}" )
	mygump.addRawLayout( "{text 425 110 1152 4}" )
	mygump.addRawLayout( "{button 10 130 4005 4007 1 0 4}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 130 225 20 1011306 0 0 32767}" )
	mygump.addRawLayout( "{text 275 130 1152 1}" )
	mygump.addRawLayout( "{text 350 130 1152 2}" )
	mygump.addRawLayout( "{text 425 130 1152 5}" )
	mygump.addRawLayout( "{button 10 150 4005 4007 1 0 5}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 150 225 20 1011307 0 0 32767}" )
	mygump.addRawLayout( "{text 275 150 1152 1}" )
	mygump.addRawLayout( "{text 350 150 1152 2}" )
	mygump.addRawLayout( "{text 425 150 1152 4}" )
	mygump.addRawLayout( "{button 10 170 4005 4007 1 0 6}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 170 225 20 1011308 0 0 32767}" )
	mygump.addRawLayout( "{text 275 170 1152 1}" )
	mygump.addRawLayout( "{text 350 170 1152 2}" )
	mygump.addRawLayout( "{text 425 170 1152 4}" )
	mygump.addRawLayout( "{button 10 190 4005 4007 1 0 7}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 190 225 20 1011321 0 0 32767}" )
	mygump.addRawLayout( "{text 275 190 1152 1}" )
	mygump.addRawLayout( "{text 350 190 1152 2}" )
	mygump.addRawLayout( "{text 425 190 1152 6}" )
	mygump.addRawLayout( "{button 10 210 4005 4007 1 0 8}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 210 225 20 1011322 0 0 32767}" )
	mygump.addRawLayout( "{text 275 210 1152 1}" )
	mygump.addRawLayout( "{text 350 210 1152 2}" )
	mygump.addRawLayout( "{text 425 210 1152 7}" )
	mygump.addRawLayout( "{button 10 230 4005 4007 1 0 9}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 230 225 20 1011317 0 0 32767}" )
	mygump.addRawLayout( "{text 275 230 1152 8}" )
	mygump.addRawLayout( "{text 350 230 1152 9}" )
	mygump.addRawLayout( "{text 425 230 1152 10}" )
	mygump.addRawLayout( "{button 10 250 4005 4007 1 0 10}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 250 225 20 1011319 0 0 32767}" )
	mygump.addRawLayout( "{text 275 250 1152 11}" )
	mygump.addRawLayout( "{text 350 250 1152 12}" )
	mygump.addRawLayout( "{text 425 250 1152 13}" )
	mygump.addRawLayout( "{button 10 270 4005 4007 1 0 11}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 270 225 20 1011320 0 0 32767}" )
	mygump.addRawLayout( "{text 275 270 1152 11}" )
	mygump.addRawLayout( "{text 350 270 1152 12}" )
	mygump.addRawLayout( "{text 425 270 1152 14}" )
	mygump.addRawLayout( "{button 10 290 4005 4007 1 0 12}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 290 225 20 1011318 0 0 32767}" )
	mygump.addRawLayout( "{text 275 290 1152 11}" )
	mygump.addRawLayout( "{text 350 290 1152 12}" )
	mygump.addRawLayout( "{text 425 290 1152 15}" )
	mygump.addRawLayout( "{button 10 310 4005 4007 1 0 13}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 310 225 20 1011309 0 0 32767}" )
	mygump.addRawLayout( "{text 275 310 1152 16}" )
	mygump.addRawLayout( "{text 350 310 1152 17}" )
	mygump.addRawLayout( "{text 425 310 1152 18}" )
	mygump.addRawLayout( "{button 10 330 4005 4007 1 0 14}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 330 225 20 1011310 0 0 32767}" )
	mygump.addRawLayout( "{text 275 330 1152 16}" )
	mygump.addRawLayout( "{text 350 330 1152 17}" )
	mygump.addRawLayout( "{text 425 330 1152 19}" )
	mygump.addRawLayout( "{button 450 390 4005 4007 0 2 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 400 390 100 20 3000406 0 0 32767}" )
	mygump.addRawLayout( "{page 2}" )
	mygump.addRawLayout( "{button 200 390 4014 4016 0 1 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 250 390 100 20 3000405 0 0 32767}" )
	mygump.addRawLayout( "{button 10 70 4005 4007 1 0 15}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 70 225 20 1011311 0 0 32767}" )
	mygump.addRawLayout( "{text 275 70 1152 16}" )
	mygump.addRawLayout( "{text 350 70 1152 17}" )
	mygump.addRawLayout( "{text 425 70 1152 20}" )
	mygump.addRawLayout( "{button 10 90 4005 4007 1 0 16}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 90 225 20 1011315 0 0 32767}" )
	mygump.addRawLayout( "{text 275 90 1152 16}" )
	mygump.addRawLayout( "{text 350 90 1152 17}" )
	mygump.addRawLayout( "{text 425 90 1152 21}" )
	mygump.addRawLayout( "{button 10 110 4005 4007 1 0 17}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 110 225 20 1011316 0 0 32767}" )
	mygump.addRawLayout( "{text 275 110 1152 16}" )
	mygump.addRawLayout( "{text 350 110 1152 17}" )
	mygump.addRawLayout( "{text 425 110 1152 22}" )
	mygump.addRawLayout( "{button 10 130 4005 4007 1 0 18}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 130 225 20 1011312 0 0 32767}" )
	mygump.addRawLayout( "{text 275 130 1152 23}" )
	mygump.addRawLayout( "{text 350 130 1152 24}" )
	mygump.addRawLayout( "{text 425 130 1152 25}" )
	mygump.addRawLayout( "{button 10 150 4005 4007 1 0 19}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 150 225 20 1011313 0 0 32767}" )
	mygump.addRawLayout( "{text 275 150 1152 26}" )
	mygump.addRawLayout( "{text 350 150 1152 27}" )
	mygump.addRawLayout( "{text 425 150 1152 28}" )
	mygump.addRawLayout( "{button 10 170 4005 4007 1 0 20}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 50 170 225 20 1011314 0 0 32767}" )
	mygump.addRawLayout( "{text 275 170 1152 29}" )
	mygump.addRawLayout( "{text 350 170 1152 30}" )
	mygump.addRawLayout( "{text 425 170 1152 31}" )
	
	#Add params
	for line in gump_params:
		mygump.addRawText( line )

	mygump.setArgs( [chsizes] )
	mygump.setCallback( callback )
	mygump.send( char )
