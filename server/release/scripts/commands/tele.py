"""
	\command tele
	\description Transports you directly to the targetted location.
"""

"""
	\command telem
	\description Transports you directly to the targetted location.
	In addition, the target cursor is displayed again once you choose
	a new location to allow you consecutive teleportation.
"""

import wolfpack
from wolfpack import utilities

def onLoad():
	wolfpack.registercommand( "tele", commandTele )
	wolfpack.registercommand( "telem", commandTeleM )
	return

def commandTeleM( socket, cmd, args ):
	socket.sysmessage( 'Select your teleport destination.' )
	socket.attachtarget( "commands.tele.teleport", [True] )
	return True

def commandTele( socket, cmd, args ):
	socket.sysmessage( 'Select your teleport destination.' )
	socket.attachtarget( "commands.tele.teleport", [False] )
	return True

def teleport( char, args, target ):
	source = char.pos
	# Keeps you from teleporting to weird places.
	if target.item and target.item.container:
		target = char.pos
	else:
		target = target.pos

	char.removefromview()
	char.moveto( target )
	char.update()
	if char.socket:
		char.socket.resendworld()
	utilities.smokepuff(char, source)
	utilities.smokepuff(char, target)
	
	if args[0]:
		char.socket.sysmessage( 'Select your teleport destination.' )
		char.socket.attachtarget( "commands.tele.teleport", [True] )
	
	return True
