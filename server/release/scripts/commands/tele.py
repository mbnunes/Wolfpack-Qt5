"""
	\command tele
	\description Transports you directly to the targetted location.
"""

import wolfpack
from wolfpack import utilities

def onLoad():
	wolfpack.registercommand( "tele", commandTele )
	return

def commandTele( socket, cmd, args ):
	socket.sysmessage( 'Select your teleport destination.' )
	socket.attachtarget( "commands.tele.teleport", [] )
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
	return True
