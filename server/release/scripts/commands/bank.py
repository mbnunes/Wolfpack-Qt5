#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# .bank Command                                                 #
#===============================================================#

import wolfpack
from wolfpack.utilities import hex2dec

def bank( socket, command, arguments ):
	arguments = arguments.strip()	# Remove trailing and leading whitespaces
	layer = 0x1d

	if len( arguments ) > 0:
		try:
			layer = hex2dec( arguments )
		except:
			socket.sysmessage( 'Usage: bank <layer-id>' )
			return

	socket.sysmessage( 'Please choose a target.' )
	socket.attachtarget( "commands.bank.callback", [ layer ] )

def callback( char, args, target ):
	if not target.char:
		char.socket.sysmessage( 'You have to target a character.' )
		return

	container = target.char.itemonlayer( args[0] )

	if not container:
		char.socket.sysmessage( 'The target has no item on layer 0x%02x' % args[0] )
		return

	char.socket.sendcontainer( container )

def onLoad():
	wolfpack.registercommand( "bank", bank )

"""
	\command bank
	\description Shows the content of a container on another character.
	\usage - <code>bank [layer]</code>
	If layer is omitted, the bank box of the targetted character is shown. Otherwise
	the server sends the contents of the container on the given layer.
	\notes The container on the given layer is not automatically created if it doesn't exist.
"""
