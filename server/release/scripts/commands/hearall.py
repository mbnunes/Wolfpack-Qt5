
import wolfpack

"""
	\command hearall
	\description You can hear what other chars say which are not in your visual range.
"""

def hearall(socket, command, arguments):
	if socket.player.hastag( 'hearall' ):
		socket.player.deltag( 'hearall' )
		socket.sysmessage( 'Hearall disabled' )
		return True

	socket.player.settag( 'hearall', 1 )
	socket.sysmessage( 'Hearall enabled' )

def onLoad():
	wolfpack.registercommand('hearall', hearall)
