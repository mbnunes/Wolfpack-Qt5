#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    
#  ((    _/{  "-;  | Created by: dreoth                
#   )).-' {{ ;'`   | Revised by:                                
#  ( (  ;._ \\ ctr | Last Modification: Created                 
#===============================================================#
# .sound Command                                               #
#===============================================================#

"""
	\command sound
	\description Play a soundeffect that can be heard by everyone.
	\usage - <code>sound id</code>
	Id is the decimal or hexadecimal id of the soundeffect you want to play.
"""

import wolfpack
from wolfpack.utilities import *
	
def sound( socket, command, arguments ):
	try:
		sound = hex2dec( str(arguments) )
		socket.player.soundeffect( sound )
	except:
		socket.sysmessage( 'Usage: sound <sound-id>' )

def onLoad():
	wolfpack.registercommand( "sound", sound )
