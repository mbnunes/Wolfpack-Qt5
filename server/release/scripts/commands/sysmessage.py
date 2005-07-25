
import wolfpack
from wolfpack import tr

def sysmessage( socket, command, arguments ):
	arguments = arguments.strip()
	if not len(arguments):
		socket.sysmessage( "Usage: sysmessage <message>" )
		return False
	socket.sysmessage( "Who should receive this message?" )
	socket.attachtarget( "commands.sysmessage.targetchar", [arguments] )
	return True

def targetchar( char, args, target ):
	if not target.char or not target.char.socket:
		return True
	target.char.socket.sysmessage( tr('A private message from %s: %s') % ( char.name, unicode(args[0]) ), char.saycolor )
	char.socket.sysmessage( 'You sent the following message: %s' % unicode(args[0]), char.saycolor )
	return True

def onLoad():
	wolfpack.registercommand("sysmessage", sysmessage)

"""
	\command sysmessage
	\description Sends a sysmessage to a player.
	\usage - <code>sysmessage message</code>
	Message is the message you want to send.
"""
