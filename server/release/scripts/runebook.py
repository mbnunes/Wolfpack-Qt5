#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-" {{ ;"`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.gumps import cGump
import wolfpack
from recall_rune import isrune
from wolfpack.utilities import fizzle
import whrandom

def onUse( char, item ):
	# char does not have it and distance > 2
	if( char.distanceto( item ) > 2 and item.getoutmostchar() != char ):
		# "Target cannot be seen."
		char.socket.clilocmessage( 502400 )
		return 1

	# check use delay
	if item.hastag( "dclickdelay" ):
		# "This book needs time to recharge."
		char.socket.clilomessage( 502403 )
		return 1

	sendGump( char, item )

	# set use delay 2 sec
	item.settag( "dclickdelay", 1 )
	wolfpack.addtimer( 2000, "runebook.rmdelay", [ item.serial ] )
	return 1

def onDropOnItem( book, item ):
	if not isrunebook( book ):
		return 0
	if not isrune( item ):
		return 0
	if( item.hastag( "marked" ) != 1 ):
		return 0

	# initialize rune serials to -1 if there's no rune
	# should not occur - will be removed later
	for i in range( 0, 16 ):
		if not book.hastag( "rune %i" % i ):
			book.settag( "rune %i" % i, -1 )

	# rune serials
	runes = [ -1 ] * 16
	for i in range( 0, 16 ):
		runes = book.gettag( "rune %i" % i )
	i = 0
	while( runes[ i ] > -1 and i < 16 ):
		i = i + 1
	# runebook is full
	if( i > 15 ):
		# can we access the char.socket ?
		# "This runebook is full."
		#char.socket.clilocmessage( 502401 )
		return 0
	book.settag( "rune %i" % i, item.serial )
	# insert rune - is runebook a container ?
	return 1

def onCreate( item, defstr ):
	if not item:
		return 0
	item.settag( "charges", 0 )
	item.settag( "default", 0 )
	# max charge calc will be added later
	item.settag( "maxcharges", 10 )
	# initialize rune serials to -1
	for i in range( 0, 16 ):
		item.settag( "rune %i" % i, -1 )
	item.settag( "runebook", 1 )
	return 1

def isrunebook( item ):
	if item.hastag( "runebook" ):
		return 1
	return 0

def rmdelay( self, args ):
	if( len( args ) < 1 ):
		return 1
	book = wolfpack.finditem( args[ 0 ] )
	if not book:
		return 1
	if not book.hastag( "dclickdelay" ):
		return 1
	book.deltag( "dclickdelay" )
	return 1

def sendGump( char, item ):
	char.socket.closegump( 0x87654322 )
	char.soundeffect( 0x55 )

	# noclose, nomove gump
	runebook = cGump( 1, 1, 0, 100, 120 )

	runebook.startPage( 0 )
	runebook.addGump( 100, 10, 2200 )

	# decoration bar
	n = 125
	m = 0
	for i in range( 0, 16 ):
		if( i == 8 ):
			n = 170
		if( i % 8 == 0 ):
			m = 57
		elif( i % 8 == 7 ):
			m = 59
		else:
			m = 58
		runebook.addGump( n + i * 15, 50, m )

	# page number button at the bottom of the book
	for i in range( 0, 8 ):
		j = i / 4
		runebook.addPageButton( 130 + i * 35 + j * 30, 187, i + 2225, i + 2225, i + 2 )

	# charge / max charge
	if not item.hastag( "charges" ):
		item.settag( "charges", 0 )
	charges = item.gettag( "charges" )
	if not item.hastag( "maxcharges" ):
		item.settag( "maxcharges", 5 )
	maxcharges = item.gettag( "maxcharges" )
	runebook.addText( 160, 40, "Charges: %i" % charges )
	runebook.addText( 300, 40, "Max Charges: %i" % maxcharges )

	runebook.startPage( 1 )

	# rename button return code = 1000
	runebook.addButton( 130, 20, 2472, 2473, 1000 )
	runebook.addText( 158, 22, "Rename Book" )

	# next page - top right corner
	runebook.addPageButton( 393, 14, 2206, 2206, 2 )

	# get rune serials and names
	runes = [ -1 ] * 16
	for i in range( 0, 16 ):
		if not item.hastag( "rune %i" % i ):
			item.settag( "rune %i" % i, runes[ i ] )
		else:
			runes[ i ] = item.gettag( "rune %i" % i )
	runenum = 0
	while( runes[ runenum ] > -1 ):
		runenum = runenum + 1
	runenames = [ "Empty" ] * 16
	for i in range( 0, runenum ):
		rune = wolfpack.finditem( runes[ i ] )
		if rune and rune.isrune():
			runenames[ i ] = rune.name

	# blue button and rune name
	for i in range( 0, 16 ):
		if( i < 8 ):
			x = 130
		else:
			x = 290
		y = 60 + ( i % 8 ) * 15
		# blue button return code = rune number + 1
		# tramel color : 115, feluca color : ???
		namecolor = 115
		runebook.addButton( x, y + 5, 2103, 2104, i + 1 )
		if i < runenum:
			runebook.addText( x + 15, y, "%s" % runenames[ i ], namecolor )
		else:
			# no color - black
			runebook.addText( x + 15, y, "Empty" )

	if item.hastag( "default" ):
		default = item.gettag( "default" )
	else:
		item.settag( "default", 0 )
		default = 0

	# page 2 - 9
	for i in range( 2, 10 ):
		runebook.startPage( i )
		# next page - top right corner
		if( i < 9 ):
			runebook.addPageButton( 393, 14, 2206, 2206, i + 1 )
		# previous page - top left corner
		runebook.addPageButton( 125, 14, 2205, 2205, i - 1 )

		# for each sub-page
		for j in range( 0, 2 ):
			# rune number
			k = ( i - 2 ) * 2 + j
			# blue button return code = 1 + rune number
			runebook.addButton( 130 + j * 160, 65, 2103, 2104, 1 + k )
			# recall button return code = 301 + rune number
			runebook.addButton( 135 + j * 160, 140, 2271, 2271, 301 + k )
			# gate button return code = 401 + rune number
			runebook.addButton( 205 + j * 160, 140, 2291, 2291, 401 + k )
			# there's a recall rune
			# tramel color : 115, feluca : ???
			namecolor = 115
			if( k < runenum ):
				runebook.addText( 145 + j * 160, 60, runenames[ k ], namecolor )
				# set default button return code = 101 + rune number
				if( k == default ):
					runebook.addButton( 160 + j * 135, 20, 2360, 2360, 101 + k )
				else:
					runebook.addButton( 160 + j * 135, 20, 2360, 2361, 101 + k )
				runebook.addText( 175 + j * 135, 15, "Set default" )
				# drop button return code = 201 + rune number
				runebook.addButton( 135 + j * 160, 115, 2437, 2438, 201 + k )
				runebook.addText( 150 + j * 160, 115, "Drop rune" )
			# empty - no color : black
			else:
				runebook.addText( 145 + j * 160, 60, runenames[ k ] )

	runebook.setArgs( [ item ] )
	runebook.setType( 0x87654322 )
	runebook.setCallback( "runebook.callback" )

	# send it
	runebook.send( char )

# callback function - process gump callback
def callback( char, args, target ):
	# close the book
	char.socket.closegump( 0x87654322 )

	# error checkings - will be removed later
	if( len( args ) < 1 ):
		char.socket.sysmessage( "script error 1. contact GM." )
		return 1
	item = args[0]
	if not item:
		char.socket.sysmessage( "script error 2. contact GM." )
		return 1

	# distance check
	if( char.distanceto( item ) > 2 and item.getoutmostchar() != char ):
		char.socket.clilocmessage( 502400 )
		return 1

	button = int( target.button )

	# rename book
	if( button == 1000 ):
		# "Please enter a title for the runebook:"
		char.socket.clilocmessage( 502414 )
		# how do we request input and catch the response ?
		char.socket.sysmessage( "not implemented yet - will be added." )
		return 1
	runes = [ -1 ] * 16
	for i in range( 0, 16 ):
		# should not occur - will be removed later
		if not item.hastag( "rune %i" % i ):
			item.settag( "rune %i" % i, -1 )
		else:
			runes[ i ] = item.gettag( "rune %i" % i )

	# selected rune number
	runenum = ( button - 1 ) % 100
	if( runenum < 0 or runenum > 15 ):
		# "Request cancelled."
		char.socket.clilocmessage( 502415 )
		return 1

	# there's no rune
	if( runes[ runenum ] < 0 ):
		# "This place in the book is empty."
		char.socket.clilocmessage( 502411 )
		return 1

	# recall button - using charges : 1 - 16
	if( button > 0 and button < 17 ):
		charges = item.gettag( "charges" )
		if( charges < 1 ):
			# "There are no charges left on that item."
			char.socket.clilocmessage( 502412 )
		else:
			# recall to the rune
			# char action / power word
			charges = charges - 1
			item.settag( "charges", charges )
			char.say( "Kal Ort Por" )
			wolfpack.addtimer( 2000, "runebook.recall0", [ char.serial, runes[ runenum ] ] )
	# set default button : 101 - 116
	elif( button > 100 and button < 117 ):
		item.settag( "default", runenum )
	# drop button : 201 - 216
	elif( button > 200 and button < 217 ):
		# will be added
		char.socket.sysmessage( "drop rune %i" % runenum )
		char.socket.sysmessage( "not implemented yet" )
	# recall button - spell : 301 - 316
	elif( button > 300 and button < 317 ):
		# char action / power word
		char.say( "Kal Ort Por" )
		wolfpack.addtimer( 2000, "runebook.recall1", [ char.serial, runes[ runenum ] ] )
	# gate button : 401 - 416
	elif( button > 400 and button < 417 ):
		char.say( "Vas Rel Por" )
		wolfpack.addtimer( 3500, "runebook.gate", [ char.serial, runes[ runenum ] ] )
	# button number error - should not occur
	else:
		char.socket.sysmessage( "script error 4. contact GM" )

	return 1

# recall using charges
def recall0( self, args ):
	if( len( args ) < 2 ):
		return 1
	char = wolfpack.findchar( args[ 0 ] )
	rune = wolfpack.finditem( args[ 1 ] )
	if not char:
		return 1
	if not rune:
		char.socket.sysmessage( "runebook script error." )
		return 1
	x = rune.gettag( "target_x" )
	y = rune.gettag( "target_y" )
	z = rune.gettag( "target_z" )
	map = rune.gettag( "target_map" )
	# check the spot if anything blocks or is valid location
	char.soundeffect( 0x1fd )
	char.removefromview()
	# AoS : now we can recall between the worlds
	char.moveto( x, y, z, map )
	char.update()
	char.soundeffect( 0x1fd )
	char.resendworld()
	return 1

# recall spell to the selected rune
def recall1( self, args ):
	if( len( args ) < 2 ):
		return 1
	char = wolfpack.findchar( args[ 0 ] )
	rune = wolfpack.finditem( args[ 1 ] )
	if not char:
		return 1
	if not rune:
		char.socket.sysmessage( "runebook script error." )
		return 1
	# cast spell
	if( char.mana < 11 ):
		char.socket.sysmessage( )
		return 1
	char.soundeffect( 0x1fd )
	char.socket.sysmessage( "not implemented yet." )
	return 1

def gate( self, args ):
	if( len( args ) < 2 ):
		return 1
	char = wolfpack.findchar( args[ 0 ] )
	rune = wolfpack.finditem( args[ 1 ] )
	if not char:
		return 1
	if not rune:
		char.socket.sysmessage( "runebook script error." )
		return 1
	char.socket.sysmessage( "not implemented yet" )
	return 1

