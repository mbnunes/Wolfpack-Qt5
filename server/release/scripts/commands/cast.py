#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: aszlig                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# cast Command  						#
#===============================================================#

import wolfpack
from magic import *

def CmdCast( socket, command, args ):
	try:
		args = int( args );
		castSpell( socket.player, args, MODE_CMD )
	except:
		socket.sysmessage( "Usage: cast <spell-id>" )
	return True


def onLoad():
	wolfpack.registercommand( "cast", CmdCast )

"""
	\command cast
	\description Directly cast a spell.
	\usage - <code>cast spell</code>
	Spell is the id of the spell you want to cast.
	\notes Spells casts with this command resemble spells cast from a scroll. So you still need the
		required magery skill, but you do not require reagents or the spell in your spellbook.
"""
