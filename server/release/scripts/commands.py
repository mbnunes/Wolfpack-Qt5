
import wolfpack

def onLoad():
	wolfpack.registercommand( "test", "commands" )


def onCommand( socket, command, argstring ):
	socket.sysmessage( "test" )
	
