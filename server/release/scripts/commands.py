
import wolfpack

def onLoad():
	wolfpack.registercommand( "test", "commands" )
	wolfpack.registercommand( "sendpacket", "commands" )
	wolfpack.registercommand( "testpacket", "commands" )

def onCommand( socket, command, argstring ):
	if command == "SENDPACKET":
		sendpacket( socket, command, argstring )
	elif command == "TESTPACKET":
		testpacket( socket, command, argstring )

	return 1

def testpacket( socket, command, argstring ):
	file = open( 'packet.dat', 'rb' )
	content = file.read()
	file.close()

	packet = []

	for char in content:
		packet.append( ord( char ) )

	socket.sendpacket( packet )

def sendpacket( socket, command, argstring ):
	# Build a list
	packet = []

	pieces = argstring.split( " " )
	for piece in pieces:
		# Treat as hexadecimal digit (FF)
		packet.append( int( piece, 16 ) )

	socket.sendpacket( packet )
