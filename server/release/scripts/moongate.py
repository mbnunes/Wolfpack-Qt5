#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-" {{ ;"`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.gumps import cGump

def onUse( char, item ):
	# check distance
	if( char.distanceto( item ) > 3):
		char.socket.sysmessage( "You are too far away to use it." )
		return 1
	# check if the char is using a moongate
	if( char.hastag( "moongate" ) ):
		char.socket.sysmessage( "You are already using it." )
		return 1
	# send gate gump
	sendGump( char, item )
	return 1

def onCollideItem( char, item ):
	# npc would not travel through moongate ;)
	if( char.npc ):
		return 1
	if( char.hastag( "moongate" ) ):
		char.socket.sysmessage( "You are already using it." )
		return 1
	sendGump( char, item )
	char.settag( "moongate", 1 )
	return 1

def sendGump( char, item ):
	char.soundeffect( 0x58 )
	# noclose, nomove gump
	gateGump = cGump( 1, 1, 0, 30, 30 )
	# trammel
	gateGump.startPage( 1 )
	gateGump.addBackground( 9200, 300, 255 )
	gateGump.addText( 40, 40, "Pick your destination:" )
	gateGump.addButton( 47, 207, 0xFA5, 0xFA7, 0 )
	gateGump.addText( 80, 210, "CANCEL" )
	# facets
	gateGump.addText( 70, 70, "Trammel", 12 )
	gateGump.addPageButton( 48, 72, 0x4B9, 0x4BA, 1 )
	gateGump.addText( 70, 90, "Felucca" )
	gateGump.addPageButton( 48, 95, 0x4B9, 0x4BA, 2 )
	gateGump.addText( 70, 110, "Ilshenar" )
	gateGump.addPageButton( 48, 115, 0x4B9, 0x4BA, 3 )
	# gates
	gateGump.addText( 170, 70, "Britain" )
	gateGump.addButton( 150, 75, 0x837, 0x838, 5 )
	gateGump.addText( 170, 90, "Magincia" )
	gateGump.addButton( 150, 95, 0x837, 0x838, 6 )
	gateGump.addText( 170, 110, "Moonglow" )
	gateGump.addButton( 150, 115, 0x837, 0x838, 7 )
	gateGump.addText( 170, 130, "Skara Brae" )
	gateGump.addButton( 150, 135, 0x837, 0x838, 8 )
	gateGump.addText( 170, 150, "Trinsic" )
	gateGump.addButton( 150, 155, 0x837, 0x838, 9 )
	gateGump.addText( 170, 170, "Vesper" )
	gateGump.addButton( 150, 175, 0x837, 0x838, 10 )
	gateGump.addText( 170, 190, "Yew" )
	gateGump.addButton( 150, 195, 0x837, 0x838, 11 )
	gateGump.addText( 170, 210, "Jhelom" )
	gateGump.addButton( 150, 215, 0x837, 0x838, 12 )
	# felucca
	gateGump.startPage( 2 )
	gateGump.addBackground( 9200, 300, 255 )
	gateGump.addText( 40, 40, "Pick your destination:" )
	gateGump.addButton( 47, 207, 0xFA5, 0xFA7, 0 )
	gateGump.addText( 80, 210, "CANCEL" )
	# facets
	gateGump.addText( 70, 70, "Trammel" )
	gateGump.addPageButton( 48, 72, 0x4B9, 0x4BA, 1 )
	gateGump.addText( 70, 90, "Felucca", 12 )
	gateGump.addPageButton( 48, 95, 0x4B9, 0x4BA, 2 )
	gateGump.addText( 70, 110, "Ilshenar" )
	gateGump.addPageButton( 48, 115, 0x4B9, 0x4BA, 3 )
	# gates
	gateGump.addText( 170, 70, "Britain" )
	gateGump.addButton( 150, 75, 0x837, 0x838, 13 )
	gateGump.addText( 170, 90, "Magincia" )
	gateGump.addButton( 150, 95, 0x837, 0x838, 14 )
	gateGump.addText( 170, 110, "Moonglow" )
	gateGump.addButton( 150, 115, 0x837, 0x838, 15 )
	gateGump.addText( 170, 130, "Skara Brae" )
	gateGump.addButton( 150, 135, 0x837, 0x838, 16 )
	gateGump.addText( 170, 150, "Trinsic" )
	gateGump.addButton( 150, 155, 0x837, 0x838, 17 )
	gateGump.addText( 170, 170, "Vesper" )
	gateGump.addButton( 150, 175, 0x837, 0x838, 18 )
	gateGump.addText( 170, 190, "Yew" )
	gateGump.addButton( 150, 195, 0x837, 0x838, 19 )
	gateGump.addText( 170, 210, "Jhelom " )
	gateGump.addButton( 150, 215, 0x837, 0x838, 20 )
	# ilshenar
	gateGump.startPage( 3 )
	gateGump.addBackground( 9200, 300, 255 )
	gateGump.addText( 40, 40, "Pick your destination:" )
	gateGump.addButton( 47, 207, 0xFA5, 0xFA7, 0 )
	gateGump.addText( 80, 210, "CANCEL" )
	# facets
	gateGump.addText( 70, 70, "Trammel" )
	gateGump.addPageButton( 48, 72, 0x4B9, 0x4BA, 1 )
	gateGump.addText( 70, 90, "Felucca" )
	gateGump.addPageButton( 48, 95, 0x4B9, 0x4BA, 2 )
	gateGump.addText( 70, 110, "Ilshenar", 12 ) 
	gateGump.addPageButton( 48, 115, 0x4B9, 0x4BA, 3 )
	# #gates
	gateGump.addText( 170, 70, "Compassion" )
	gateGump.addButton( 150, 75, 0x837, 0x838, 21 )
	gateGump.addText( 170, 90, "Honesty" )
	gateGump.addButton( 150, 95, 0x837, 0x838, 22 )
	gateGump.addText( 170, 110, "Honor" )
	gateGump.addButton( 150, 115, 0x837, 0x838, 23 )
	gateGump.addText( 170, 130, "Humility" )
	gateGump.addButton( 150, 135, 0x837, 0x838, 24 )
	gateGump.addText( 170, 150, "Justice" )
	gateGump.addButton( 150, 155, 0x837, 0x838, 25 )
	gateGump.addText( 170, 170, "Sacrifice" )
	gateGump.addButton( 150, 175, 0x837, 0x838, 26 )
	gateGump.addText( 170, 190, "Spirituality" )
	gateGump.addButton( 150, 195, 0x837, 0x838, 27 )
	gateGump.addText( 170, 210, "Valor" )
	gateGump.addButton( 150, 215, 0x837, 0x838, 28 )
	gateGump.addText( 170, 230, "Chaos" )
	gateGump.addButton( 150, 235, 0x837, 0x838, 29 )
	# set callback function and its arguments
	gateGump.setCallback( "moongate.gateCallback" )
	# send it
	gateGump.setArgs( [ item ] )
	gateGump.send( char )
	# attach tag
	char.settag( "moongate", 1 )

# callback function
def gateCallback( char, args, target ):
	if not char.hastag( "moongate" ):
		char.socket.sysmessage( "script error 0. contact GM." )
		return 1
	if( len( args ) < 1 ):
		char.deltag( "moongate" )
		char.socket.sysmessage( "script error 1. contact GM." )
		return 1
	item = args[0]
	if not item:
		char.deltag( "moongate" )
		char.socket.sysmessage( "script error 2. contact GM." )
		return 1
	if( char.distanceto( item ) > 3):
		char.deltag( "moongate" )
		char.socket.sysmessage( "You are too far away to use it." )
		return 1
	button = target.button
	if not button:
		char.deltag( "moongate" )
		char.socket.sysmessage( "Canceled." )
		return 1
	elif( button > 29 ):
		char.deltag( "moongate" )
		char.socket.sysmessage( "script error 3. contact GM." )
		char.socket.sysmessage( "button = " + button )
		return 1
	elif( ( button > 4 ) and ( button < 30 ) ):
		char.deltag( "moongate" )
	# set world number
	numWorld = 0
	if( ( button > 4 ) and ( button < 13 ) ):
		numWorld = 0
	elif ( ( button > 12 ) and ( button < 21 ) ):
		numWorld = 1
	elif( ( button > 20 ) and ( button < 30 ) ):
		numWorld = 2
	# teleport
	coord = []
	oldcoord = [ char.pos.x, char.pos.y, char.pos.z ]
	if( ( button == 5 ) or ( button == 13 ) ):
		coord = [ 1336, 1998, 5, numWorld ]
	elif( ( button == 6 ) or ( button == 14 ) ):
		coord = [ 3564, 2141, 33, numWorld ]
	elif( ( button == 7 ) or ( button == 15 ) ):
		coord = [ 4467, 1284, 5, numWorld ]
	elif( ( button == 8 ) or ( button == 16 ) ):
		char.moveto( 643, 2068, 5, numWorld )
		coord = [ 643, 2068, 5, numWorld ]
	elif( ( button == 9 ) or ( button == 17 ) ):
		coord = [ 1828, 2949, -20, numWorld ]
	elif( ( button == 10 ) or ( button == 18 ) ):
		coord = [ 2701, 694, 4, numWorld ]
	elif( ( button == 11 ) or ( button == 19 ) ):
		coord = [ 771, 754, 4, numWorld ]
	elif( ( button == 12 ) or ( button == 20 ) ):
		coord = [ 771, 754, 4, numWorld ]
	elif( button == 21 ):
		coord = [ 1216, 468, -13, 2 ]
	elif( button == 22 ):
		coord = [ 722, 1364, -60, 2 ]
	elif( button == 23 ):
		coord = [ 750, 724, -28, 2 ]
	elif( button == 24 ):
		coord = [ 282, 1015, 0, 2 ]
	elif( button == 25 ):
		coord = [ 987, 1010, -32, 2 ]
	elif( button == 26 ):
		coord = [ 1174, 1286, -30, 2 ]
	elif( button == 27 ):
		coord = [1532, 1340, -4, 2 ]
	elif( button == 28 ):
		coord = [ 528, 219, -42, 2 ]
	elif( button == 29 ):
		coord = [ 1721, 218, 96, 2 ]
	char.soundeffect( 0x1fc )
	char.removefromview()
	char.moveto( coord[0], coord[1], coord[2], coord[3] )
	char.update()
	char.soundeffect( 0x1fc )
	char.effect( 0x372a )
	# move followers
	followers = char.followers
	if( len( followers ) > 0 ):
		for i in range( 0, len( char.followers ) ):
			follower = char.followers[i]
			# only transport follower which is within 5 tile from the character
			if( char.distanceto( follower ) < 5 ):
				follower.removefromview()
				follower.moveto( coord[0], coord[1], coord[2], coord[3] )
				follower.update()
				follower.effect( 0x372a )
			# else it will not follow him/her
			else:
				char.removefollower( follower )
	char.socket.resendworld()
	return 1
