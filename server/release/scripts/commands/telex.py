#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Naddel                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# .telex Command                                                #
#===============================================================#

import wolfpack

def telex( socket, command, arguments ):
	socket.sysmessage( "Which object do you want to move?" )
	socket.attachtarget( "commands.telex.telextarget_1", [] )
	return True

def telextarget_1( char, args, target ):
	if not (target.item or target.char):
		char.socket.sysmessage( "Only characters and items are valid objects." )
		return True

	if target.item:
		if target.item.container:
			char.socket.sysmessage( "You cannot move items which are in a container.")
			return True
		item = target.item
		char.socket.sysmessage( "Where do you want to move the item?" )
		char.socket.attachtarget( "commands.telex.telextarget_2", [ 0, item.serial ] )

	if target.char:
		ziel = target.char
		char.socket.sysmessage( "Where do you want to move the character?" )
		char.socket.attachtarget( "commands.telex.telextarget_2", [ 1, ziel.serial ] )

def telextarget_2( char, args, target ):
	# we teleport an item
	if args[0] == 0:
		if target.item:
			tile = wolfpack.tiledata(target.item.id)
			layer = tile['height']
		else:
			layer = 0
		item = wolfpack.finditem( args[1] )
		if not item:
			return True
		item.moveto( target.pos.x, target.pos.y, target.pos.z + layer )
		item.update()
	
	# we teleport a char
	if args[0] == 1:
		ziel = wolfpack.findchar( args[1] )
		if not ziel:
			return True
		ziel.moveto( target.pos.x, target.pos.y, target.pos.z )
		ziel.update()

def onLoad():
	wolfpack.registercommand( "telex", telex )

"""
	\command telex
	\description You can choose an item or char and move it where ever you want.
	\usage - <code>telex</code>
	Choose an item or char and select the target where you want to move it
"""
