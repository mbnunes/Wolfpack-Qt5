#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: Dreoth
#   )).-' {{ ;'`   | Revised by:
#  ( (  ;._ \\ ctr | Last Modification: Ported from core
#===============================================================#
"""
	\command who
	\description Manage connected clients.
	\notes The gump shown will allow you to travel to the client, send messages or bring them directly to you.
"""
import wolfpack
import wolfpack.sockets
import wolfpack.accounts
import commands.info
import math
from wolfpack.gumps import cGump

def onLoad():
	wolfpack.registercommand( "who", cmdWho )
	return

def cmdWho( socket, command, argstring ):
	wholist = []
	worldsocketcount = wolfpack.sockets.count()
	worldsocket = wolfpack.sockets.first()
	while worldsocket:
		wholist += [ worldsocket.player.serial ]
		worldsocket = wolfpack.sockets.next()

	if len(wholist) == 0:
		return False

	gump = cGump( 0, 0, 0, 50, 50 )
	gump.addBackground( 0xE10, 380, 360 )
	gump.addCheckerTrans( 15, 15, 350, 330 );
	gump.addGump( 130, 18, 0xFA8 )
	gump.addText( 165, 20, unicode("Who Menu"), 0x530 )
	# Close Button
	gump.addButton( 30, 320, 0xFB1, 0xFB3, 0 )
	gump.addText( 70, 320, unicode("Close"), 0x834 )
	# Start The First Page
	# Notes, 10 per page
	# Pages to create, (( socketcount / 10) + 1 )
	# Player list increases by 22 pixels
	maxpages = ((worldsocketcount / 10) + 1)
	page = 0
	serialcount = 0
	while page <= maxpages:
		page += 1
		gump.startPage( page )
		gump.addText( 280, 320, unicode( "Page %i of %i" % ( page, maxpages ) ), 0x834 )
		if page < maxpages:
			gump.addPageButton( 260, 320, 0x0FA, 0x0FA, page + 1 )
		if page > 1:
			gump.addPageButton( 240, 320, 0x0FC, 0x0FC, page - 1 )
		upby = 22
		for serial in wholist:
			if not serial:
				break
			player = wolfpack.findchar( serial )
			if not player or not player.account or not player.socket:
				continue

			# serialcount + 10 for callback, we will -10 there and look at wholist.
			gump.addButton( 20, 40 + upby, 0xFA5, 0xFA7, ( serialcount + 10 ) )
			gump.addText( 50, 40 + upby, unicode( "%s [%s]" % ( player.name, player.account.name ) ), 0x834 )
			gump.addText( 240, 40 + upby, unicode( "%s" % player.socket.address ), 0x834 )
			upby += 22
			serialcount += 1
			if serialcount == ((page * 10) - 1):
				break

	gump.setArgs( [ wholist ] )
	gump.setCallback( "commands.who.callbackWho" )
	gump.send( socket )
	return
	
def details(char, player):
	if not player.socket:
		char.socket.sysmessage('The player is currently offline.')
		return
	
	pos = player.pos
	account = player.account
	# Socket Information
	gump = cGump( 0, 0, 0, 50, 50 )
	gump.addBackground( 0xE10, 440, 340 )
	gump.addResizeGump( 195, 260, 0xBB8, 205, 20 )
	gump.addCheckerTrans( 15, 15, 410, 310 )
	gump.addGump( 160, 18, 0xFA2 )
	gump.addText( 195, 20, unicode( "Socket Menu" ), 0x530 )
	gump.addText( 70, 300, unicode("Close"), 0x834 )
	gump.addButton( 30, 300, 0xFB1, 0xFB3, 0 )
	gump.startPage( 1 )
	gump.addText( 50, 60, unicode( "Char name:" ), 0x834 )
	gump.addText( 250, 60, unicode( "%s" % player.name ), 0x834 )
	gump.addText( 50, 80, unicode( "IP:" ), 0x834 )
	gump.addText( 250, 80, unicode( "%s" % player.socket.address ), 0x834 )
	gump.addText( 50, 100, unicode( "Position:" ), 0x834 )
	gump.addText( 250, 100, unicode( "%i,%i,%i,%i" % ( pos.x, pos.y, pos.z, pos.map ) ), 0x834 )
	gump.addText( 50, 120, unicode( "Region:" ), 0x834 )
	if player.region:
		gump.addText( 250, 120, unicode( "%s" % unicode(player.region.name) ), 0x834 )
	else:
		gump.addText( 250, 120, "Unknown", 0x834 )
	gump.addText( 50, 140, unicode( "Account / ACL:" ), 0x834 )
	gump.addText( 250, 140, unicode( "%s / %s" % ( account.name, account.acl) ), 0x834 )
	# Actions
	# Go To Char
	gump.addButton( 20, 180, 0xFA5, 0xFA7, 1 )
	gump.addText( 50, 180, unicode( "Go to position" ), 0x834 )
	# Bring Char
	gump.addButton( 20, 200, 0xFA5, 0xFA7, 2 )
	gump.addText( 50, 200, unicode( "Bring char" ), 0x834 )
	# Jail Char
	gump.addButton( 20, 220, 0xFA5, 0xFA7, 3 )
	gump.addText( 50, 220, unicode( "Jail char" ), 0x834 )
	# Forgive Char
	gump.addButton( 220, 220, 0xFA5, 0xFA7, 4 )
	gump.addText( 250, 220, unicode( "Forgive char" ), 0x834 )
	# Show Char Info Gump
	gump.addButton( 220, 180, 0xFAB, 0xFAD, 5 )
	gump.addText( 250, 180, unicode( "Show char info gump" ), 0x834 )
	# Send Message
	gump.addButton( 20, 260, 0xFBD, 0xFBF, 6 )
	gump.addText( 50, 260, unicode( "Send message:" ), 0x834 )
	gump.addInputField( 200, 260, 190, 16, 0x834, 1, unicode( "<msg>" ) )
	# Disconnect
	gump.addButton( 220, 200, 0xFA5, 0xFA7, 7 )
	gump.addText( 250, 200, unicode( "Disconnect" ), 0x834 )
	# Stuff and Send
	gump.setCallback( "commands.who.callbackSocket" )
	gump.setArgs( [ player.serial ] )
	gump.send( char.socket )

def callbackWho( char, args, choice ):
	wholist = args[0]
	if choice.button == 0:
		return False
	elif wholist[ choice.button - 10 ]:
		player = wolfpack.findchar( wholist[ choice.button - 10 ] )
		details(char, player)
		return True
	else:
		return False


def callbackSocket( char, args, choice ):
	socket = char.socket
	player = wolfpack.findchar( args[0] )
	textentries = choice.text
	keys = textentries.keys()
	# Cancel
	if choice.button == 0:
		return False
	# Disconnect
	elif choice.button == 7:
		player.socket.disconnect()
		return True
	# Send Message
	elif choice.button == 6:
		for key in keys:
			if key == 1:
				player.socket.sysmessage( textentries[ key ] )
				break
		return True
	# Char Info
	elif choice.button == 5:
		commands.info.charinfo( socket, player )
		return True
	# Forgive player
	elif choice.button == 4:
		socket.sysmessage( "Jailing system is not yet complete." )
		return True
	# Jail player
	elif choice.button == 3:
		socket.sysmessage( "Jailing system is not yet complete." )
		return True
	# Bring player
	elif choice.button == 2:
		if player.serial != char.serial:
			player.removefromview()
			player.moveto(char.pos)
			player.update()
			player.socket.resendworld()
		return True
	# Go to player
	elif choice.button == 1:
		if player.serial != char.serial:
			char.removefromview()
			char.moveto(player.pos)
			char.update()
			char.socket.resendworld()
		return True

	return True
