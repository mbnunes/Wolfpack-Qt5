#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# .action Command                                               #
#===============================================================#

import wolfpack
from wolfpack.utilities import hex2dec
	
def action(socket, command, arguments):
	try:
		action = hex2dec(arguments)
		socket.player.action(action)
	except:
		socket.sysmessage('Usage: action <animation-id>')

def onLoad():
	wolfpack.registercommand( "action", action )

"""
	\command action
	\description Lets your character perform an animation visible to everyone.
	\usage - <code>action id</code>	
	Id is the animation you want your character to perform.
	The animation will be visible to everyone seeing you. You can also 
	specify the animation id as a hexadecimal number (0x0, 0x01, etc.).
"""
