#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: Incanus
#   )).-' {{ ;'`   | Revised by:
#  ( (  ;._ \\ ctr | Last Modification: Created
#===============================================================#

"""
	\command tilemove
	\description Move objects relative to their current position.
	\usage - <code>tilemove [z0] x</code>
	- <code>tilemove [z0] x[,y]</code>
	- <code>tilemove [z0] x[,y][,z]</code>
	- <code>tilemove [z0] x[,y][,z][,map]</code>
	Z0 is the height where the items should be moved.
	X, Y and Z are the offsets the object should be moved by.
	MAP is the new map number.
"""

import wolfpack
from wolfpack.consts import *
import random

#
# Gather another corner if neccesary and start tiling
#
def tileResponse(player, arguments, target):
	if( len(arguments) < 6 ):
		player.socket.sysmessage( "Please select the second corner." )
		player.socket.attachtarget( "commands.tilemove.tileResponse", list(arguments) + [target.pos] )
		return

	socket = player.socket
	xmod = int( arguments[0] )
	ymod = int( arguments[1] )
	zmod = int( arguments[2] )
	if( arguments[3] == None ):
		newmap = None
	else:
		newmap = int( arguments[3] )
	if( arguments[4] == None ):
		z0 = None
	else:
		z0 = int( arguments[4] )

	x1 = min( arguments[5].x, target.pos.x )
	x2 = max( arguments[5].x, target.pos.x )
	y1 = min( arguments[5].y, target.pos.y )
	y2 = max( arguments[5].y, target.pos.y )
	
	unlimited = player.account.authorized("Misc", "Unlimited Tile")
	count = ((x2 - x1) + 1) * ((y2 - y1) + 1)
	
	# Cap at 250 items if not an admin is using it
	if( not unlimited and count > 250 ):
		player.socket.sysmessage( "You are not allowed to move more than 250 items at once." )
		return

	count = 0
	items = []
	iterator = wolfpack.itemregion( x1, y1, x2, y2, target.pos.map )
	item = iterator.first

	# We have to copy the items in a list
	# because moving them inside the rectangle will change the iterator
	while item:
		if( z0 == None or z0 == item.pos.z ):
			items.append( item )
		item = iterator.next

	for item in items:
		pos = item.pos
		if( newmap != None ):
			newposition = "%i,%i,%i,%i" % ( (pos.x + xmod) , (pos.y + ymod), (pos.z + zmod), newmap )
		else:
			newposition = "%i,%i,%i,%i" % ( (pos.x + xmod) , (pos.y + ymod), (pos.z + zmod), pos.map )
		item.pos = newposition
		item.update()
		count += 1

	socket.sysmessage( "%i items moved" % count )

#
# Gather arguments and validate id list.
#
def commandTileMove(socket, command, arguments):
	arguments = arguments.strip()
	if( len(arguments) == 0 or arguments.count(" ") > 1 ):
		socket.sysmessage( "Usage: tilemove [z0] x" )
		socket.sysmessage( "Usage: tilemove [z0] x[,y]" )
		socket.sysmessage( "Usage: tilemove [z0] x[,y][,z]" )
		socket.sysmessage( "Usage: tilemove [z0] x[,y][,z][,map]" )
		return False

	if( arguments.count(" ") == 0 ):
	        z0 = None
	else:
		(z0, arguments) = arguments.split( " " )
		try:
			z0 = int( z0 )
		except:
			socket.sysmessage( "'%s' must be a number." % z0 )
			return True
	
	arguments = arguments.split( "," )
	for i in arguments:
		try:
			i = int( i )
		except:
			socket.sysmessage( "'%s' must be a number." % i )
			return True
	        
	if( len(arguments) < 1 or len(arguments) > 4 ):
	        socket.sysmessage( "Usage: tilemove [z0] x" )
	        socket.sysmessage( "Usage: tilemove [z0] x[,y]" )
	        socket.sysmessage( "Usage: tilemove [z0] x[,y][,z]" )
	        socket.sysmessage( "Usage: tilemove [z0] x[,y][,z][,map]" )
	        return False

	if len( arguments ) == 4:
		xmod = int( arguments[0] )
		ymod = int( arguments[1] )
		zmod = int( arguments[2] )
		newmap = int( arguments[3] )
		if not wolfpack.hasmap( newmap ):
			newmap = None
	elif len( arguments ) == 3:
		xmod = int( arguments[0] )
		ymod = int( arguments[1] )
		zmod = int( arguments[2] )
		newmap = None
	elif len( arguments ) == 2:
		xmod = int( arguments[0] )
		ymod = int( arguments[1] )
		zmod = 0
		newmap = None
	elif len( arguments ) == 1:
		xmod = int( arguments[0] )
		ymod = 0
		zmod = 0
		newmap = None
	if newmap:
		socket.sysmesage( "Please select the first corner. [Move: %i,%i,%i,%i]" % ( xmod, ymod, zmod, newmap ) )
	else:
		socket.sysmessage( "Please select the first corner. [Move: %i,%i,%i]" % ( xmod, ymod, zmod ) )

	socket.attachtarget( "commands.tilemove.tileResponse", [ xmod, ymod, zmod, newmap, z0 ] )
	return True

#
# Register the command
#
def onLoad():
	wolfpack.registercommand( "tilemove", commandTileMove )
