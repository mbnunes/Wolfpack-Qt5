#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: ???
#   )).-' {{ ;'`   | Revised by: Dreoth
#  ( (  ;._ \\ ctr | Last Modification: Tweaks
#===============================================================#
"""
	\command invul
	\description Toggle or change your invulnerability flag.
	\usage - <code>invul</code>
	- <code>invul on</code>
	- <code>invul true</code>
	- <code>invul 1</code>
	- <code>invul off</code>
	- <code>invul false</code>
	- <code>invul 0</code>
	If no argument is given, the flag is toggled.
"""

import wolfpack
from wolfpack.utilities import booleantoggle

def invul(socket, command, arguments):
	socket.player.invulnerable = booleantoggle( socket.player.invulnerable )
	socket.sysmessage( "Invulnerable is now '%i'." % socket.player.invulnerable )
	socket.player.resendtooltip()
	return True

def onLoad():
	wolfpack.registercommand( 'invul', invul )
	return
