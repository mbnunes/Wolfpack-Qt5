"""
	\command tele
	\description Transports you directly to the targetted location.
"""

import wolfpack
from wolfpack.consts import *

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
	wolfpack.effect(0x3728, source, 10, 15)
	wolfpack.effect(0x3728, target, 10, 15)
	char.soundeffect(0x1fe)
	return True
