#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: Dreoth
#   )).-' {{ ;'`   | Revised by:
#  ( (  ;._ \\ ctr | Last Modification: Created
#===============================================================#
# .dye Command
#===============================================================#

"""
	\command freeze
	\description Toggles the frozen value of the target.
	\usage - <code>freeze</code>
"""

import wolfpack
from wolfpack.utilities import *

def freeze( socket, command, arguments ):
	try:
		socket.attachtarget( "commands.freeze.response", [arguments] )
	except:
		socket.sysmessage( 'Usage: freeze' )

def response( char, args, target ):
	if target.char:
		if target.char.frozen == 0:
			target.char.frozen = 1
		elif target.char.frozen == 1:
			target.char.frozen = 0
	else:
		char.socket.sysmessage( 'That was not a valid object.', GRAY )

def onLoad():
	wolfpack.registercommand( "freeze", freeze )
