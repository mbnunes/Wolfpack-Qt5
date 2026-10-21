#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: codex                          #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# .fly Command                                                  #
#===============================================================#

r"""
	\command fly
	\description Toggles flying mode for Gargoyles.
"""

import wolfpack

def commandFly( socket, cmd, args ):
	char = socket.player
	if not char:
		return False

	if not char.isgargoyle():
		socket.sysmessage( "Only gargoyles can fly." )
		return True

	if char.hastag("flying"):
		char.deltag("flying")
		socket.sysmessage( "You land." )
	else:
		char.settag("flying", 1)
		socket.sysmessage( "You are flying." )

	char.resend()
	return True

def onLoad():
	wolfpack.registercommand( 'fly', commandFly )
