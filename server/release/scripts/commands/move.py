"""
	\command move
	\description Move an object relative to its current position.
	\usage - <code>move [x]</code>
	- <code>move [x],[y]</code>
	- <code>move [x],[y],[z]</code>
	- <code>move [x],[y],[z],[map]</code>
	X, Y and Z are the offsets the object should be moved by.
	MAP is the new map number.
"""

import wolfpack

def onLoad():
	wolfpack.registercommand( "move", commandMove )
	return

def commandMove( socket, cmd, args ):
	char = socket.player
	args = args.strip()
	if len(args) == 0:
		socket.sysmessage( "Moves an object relative to its current position." )
		socket.sysmessage( "Usage: move [x]" )
		socket.sysmessage( "Usage: move [x],[y]" )
		socket.sysmessage( "Usage: move [x],[y],[z]" )
		return False
	else:
		args = args.split( "," )
		for i in args:
			try:
				i = int(i)
			except:
				socket.sysmessage( "'%s' must be a number." % i )
				return True
			
		if len( args ) >= 1 and len( args ) <= 4:
			if len( args ) == 4:
				xmod = int( args[0] )
				ymod = int( args[1] )
				zmod = int( args[2] )
				newmap = int( args[3] )
				if not wolfpack.hasmap( newmap ):
					newmap = None
			elif len( args ) == 3:
				xmod = int( args[0] )
				ymod = int( args[1] )
				zmod = int( args[2] )
				newmap = None
			elif len( args ) == 2:
				xmod = int( args[0] )
				ymod = int( args[1] )
				zmod = 0
				newmap = None
			elif len( args ) == 1:
				xmod = int( args[0] )
				ymod = 0
				zmod = 0
				newmap = None
			if newmap:
				socket.sysmesage( "Please select a target to move %i,%i,%i,%i" % ( xmod, ymod, zmod, newmap ) )
			else:
				socket.sysmessage( "Please select a target to move %i,%i,%i" % ( xmod, ymod, zmod ) )
			socket.attachtarget( "commands.move.response", [ int(xmod), int(ymod), int(zmod), newmap ] )
			return True
		else:
			socket.sysmessage( "Moves an object relative to its current position." )
			socket.sysmessage( "Usage: move [x]" )
			socket.sysmessage( "Usage: move [x],[y]" )
			socket.sysmessage( "Usage: move [x],[y],[z]" )
			socket.sysmessage( "Usage: move [x],[y],[z],[map]" )
			return False

def response( char, args, target ):
	socket = char.socket
	xmod = args[0]
	ymod = args[1]
	zmod = args[2]
	newmap = args[3]
	if not newmap:
		newmap = False

	if newmap:
		newmap = int(newmap)
	xmod = int( xmod )
	ymod = int( ymod )
	zmod = int( zmod )

	if target.item:
		item = target.item
		pos = item.pos
		if type(newmap) == int:
			newposition = "%i,%i,%i,%i" % ( (pos.x + xmod) , (pos.y + ymod ), (pos.z + zmod), newmap )
		else:
			newposition = "%i,%i,%i,%i" % ( (pos.x + xmod) , (pos.y + ymod ), (pos.z + zmod), pos.map )
		item.pos = newposition
		item.update()
		return True
	elif target.char:
		char = target.char
		pos = char.pos
		if type(newmap) == int:
			newposition = wolfpack.coord( (pos.x + xmod) , (pos.y + ymod ), (pos.z + zmod), newmap )
		else:
			newposition = wolfpack.coord( (pos.x + xmod) , (pos.y + ymod ), (pos.z + zmod), pos.map )
		char.removefromview()
		char.moveto( newposition )
		char.update()
		if char.socket:
			char.socket.resendworld()
		return True
	return True
