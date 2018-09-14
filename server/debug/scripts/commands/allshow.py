import wolfpack
from wolfpack import tr

"""
	\command allshow
	\description Reveals the offline characters on your screen.
"""

def allshow(socket, command, arguments):
	player = socket.player
	offlineChars = wolfpack.chars(player.pos.x, player.pos.y, player.pos.map, player.visrange, True)
	for oc in offlineChars:
		socket.removeobject(oc)
		socket.sendobject(oc)
	socket.sysmessage( tr( "The offline characters on your screen have been revealed." ) )


def onLoad():
	wolfpack.registercommand('allshow', allshow)
