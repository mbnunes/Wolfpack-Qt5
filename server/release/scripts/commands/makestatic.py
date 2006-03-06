#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: MagnusBr
#   )).-' {{ ;'`   | Revised by:
#  ( (  ;._ \\ ctr | Last Modification: Created
#===============================================================#
# .makestatic Command
#===============================================================#

"""
	\command makestatic
	\description Make an Item static.
	\usage - <code>freeze</code>
"""

import wolfpack
from wolfpack.utilities import booleantoggle
from wolfpack.consts import GRAY

def makestatic( socket, command, arguments ):
	try:
		socket.attachtarget( "commands.makestatic.response", [] )
	except:
		socket.sysmessage( 'Usage: makestatic' )

def response( char, args, target ):
	if target.item:
		target.item.movable = 3
		target.item.decay = False
		char.socket.sysmessage( 'Object is now Static.', GRAY )
	else:
		char.socket.sysmessage( 'That was not a valid object.', GRAY )
	return

def onLoad():
	wolfpack.registercommand( "makestatic", makestatic )
	return