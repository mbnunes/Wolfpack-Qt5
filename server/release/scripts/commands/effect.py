#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: Dreoth
#   )).-' {{ ;'`   | Revised by:
#  ( (  ;._ \\ ctr | Last Modification: Created
#===============================================================#
# .effect Command                                               #
#===============================================================#

import wolfpack
from wolfpack.utilities import hex2dec

def effect(socket, command, arguments):
	try:
		effect = hex2dec(arguments)
		socket.player.effect(effect, 20, 10)
	except:
		socket.sysmessage('Usage: effect <effect-id>')

def onLoad():
	wolfpack.registercommand( "effect", effect )

"""
	\command effect
	\description Lets your character perform an effect visible to everyone.
	\usage - <code>effect id</code>
	Id is the effect you want your character to perform.
"""
