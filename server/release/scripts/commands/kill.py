#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: Dreoth
#   )).-' {{ ;'`   | Revised by:
#  ( (  ;._ \\ ctr | Last Modification: Created
#===============================================================#
"""
	\command kill
	\description Kills the selected character.
	\notes You cannot kill invulnerable characters this way.
"""
"""
	\command bolt
	\description Lightning's the selected character.
	\notes This does not do any damage.
"""

import wolfpack
from wolfpack.consts import LOG_MESSAGE

def onLoad():
	wolfpack.registercommand( "kill", commandKill )
	wolfpack.registercommand( "bolt", commandBolt )
	return

def commandKill(socket, cmd, args):
	socket.sysmessage( "Please select the target to kill." )
	socket.attachtarget( "commands.kill.dokill", [] )
	return True

def dokill( char, args, target ):
	socket = char.socket
	if target.char and not target.char.dead:
		if target.char.invulnerable:
			socket.sysmessage( "This target is invulnerable!" )
			return False
		else:
			target.char.lightning()
			char.log( LOG_MESSAGE, "Used kill on 0x%x.\n" % target.char.serial )
			target.char.kill()
			return True

def commandBolt(socket, cmd, args):
	socket.sysmessage( "Please select the target to strike with lightning." )
	socket.attachtarget( "commands.kill.dobolt", [] )
	return True

def dobolt( char, args, target ):
	socket = char.socket
	if target.char and not target.char.dead:
		target.char.lightning()
		return True
