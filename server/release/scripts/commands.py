
import wolfpack

def onLoad():
	wolfpack.registercommand( "test", "commands" )
	wolfpack.registercommand( "sendpacket", "commands" )

def onCommand( socket, command, argstring ):
	if command == "sendpacket":
		sendpacket( socket, command, argstring )
	
def sendpacket( socket, command, argstring ):
	# Build a list
	packet = []

	pieces = argstring.split( " " )
	for piece in pieces:
		# Treat as hexadecimal digit (FF)
		packet.append( int( piece, 16 ) )

	socket.sendpacket( packet )