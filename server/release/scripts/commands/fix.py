#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: Dreoth
#   )).-' {{ ;'`   | Revised by:
#  ( (  ;._ \\ ctr | Last Modification: Created
#===============================================================#
"""
	\command fix
	\description Resend the player and the surrounding objects.
"""
"""
	\command resend
	\description Resend the player and the surrounding objects.
"""
import wolfpack

# Loads the command
def onLoad():
	wolfpack.registercommand( 'fix', commandFix )
	wolfpack.registercommand( 'resend', commandFix )
	return

def commandFix( socket, cmd, args ):
	socket.sysmessage( "Resending the world, please wait." )
	socket.player.update()
	socket.player.resendtooltip()
	socket.sysmessage( "Resend complete." )
	return True
