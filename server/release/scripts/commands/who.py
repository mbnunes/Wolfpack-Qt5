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
from wolfpack import tr
import math
from wolfpack.gumps import cGump

def onLoad():
	wolfpack.registercommand( "who", cmdWho )
	return
	
def cmdWho(socket, command, arguments):
	showWhoGump(socket.player, 0)

def showWhoGump(player, page):
	# Collect the current list of sockets first
	wholist = []

	worldsocket = wolfpack.sockets.first()
	while worldsocket:
		char = worldsocket.player
		if char.invisible and char.rank > player.rank:
			continue
		if not char.account:
			continue	
		wholist.append(char)
		worldsocket = wolfpack.sockets.next()
		
	count = len(wholist)
		
	# Skip page * 10 users
	newwholist = wholist[page * 10:]

	# Reduce the page id while page > 0 and len(newwholist) == 0
	while len(newwholist) == 0 and page > 0:
		page -= 1
		newwholist = wholist[page * 10:]
	
	wholist = newwholist
		
	gump = cGump( 0, 0, 0, 50, 50 )
	gump.addBackground( 0xE10, 380, 360 )
	gump.addCheckerTrans( 15, 15, 350, 330 );
	gump.addGump( 130, 18, 0xFA8 )
	gump.addText( 165, 20, tr("Who Menu"), 0x530 )
	gump.addButton( 30, 320, 0xFB1, 0xFB3, 0 ) 	# Close Button
	gump.addText( 70, 320, tr("Close"), 0x834 )
	gump.addText( 145, 320, tr("Players: %u") % count, 0x834 )

	# Player list increases by 22 pixels
	pages = (count + 9) / 10 # 10 per page

	player.socket.sysmessage(tr('Total sockets: %u') % count)
	gump.addText( 280, 320, tr( "Page %i of %i" % ( page + 1, pages ) ), 0x834 )

	if page + 1 < pages:
		gump.addButton( 260, 320, 0x0FA, 0x0FA, 1 ) # Next Page
		
	if page > 0:
		gump.addButton( 240, 320, 0x0FC, 0x0FC, 2 ) # Previous Page
		
	offset = 22
	for item in wholist[:10]:
		gump.addButton( 20, 40 + offset, 0xFA5, 0xFA7, 2 + player.serial )
		gump.addText( 54, 40 + offset, tr("%s [%s]") % ( player.name, player.account.name ), 0x834 )
		gump.addText( 257, 40 + offset, unicode(player.socket.address), 0x834 )
		offset += 24
	
	gump.setArgs( [ page ] )
	gump.setCallback( "commands.who.callbackWho" )
	gump.send( player.socket )

def callbackWho( char, args, choice ):
	page = args[0]
		
	# Next page
	if choice.button == 1:
		showWhoGump(char, page + 1)
	
	elif choice.button == 2 and page > 0:
		showWhoGump(char, page - 1)
		
	elif choice.button > 2:
		serial = choice.button - 2
		player = wolfpack.findchar(serial)
		details(char, player)

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
