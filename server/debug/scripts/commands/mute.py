
"""
	\command mute
	\description Stops the targetted player from talking until you unmute him.
"""

"""
	\command unmute
	\description Allows the targetted player to talk again.
"""

import wolfpack
from wolfpack import tr
from wolfpack.consts import *

def response(player, arguments, target):
	if not target.char or not target.char.player:
		player.socket.sysmessage(tr('You have to target a player character.'))
		return

	if target.char.rank >= player.rank:
		player.socket.sysmessage(tr('You burn your fingers.'))
		return

	target.char.squelched = arguments[0]

	if arguments[0]:
		player.log(LOG_MESSAGE, 'Muted player %s (0x%x).\n' % (target.char.orgname, target.char.serial))
		player.socket.sysmessage(tr('The targetted player can no longer talk.'))
	else:
		player.log(LOG_MESSAGE, 'Unmuted player %s (0x%x).\n' % (target.char.orgname, target.char.serial))
		player.socket.sysmessage(tr('The targetted player can now talk again.'))
		target.char.message(500006)
	return

def mute(socket, command, arguments):
	socket.sysmessage(tr('Target the player you want to mute.'))
	socket.attachtarget('commands.mute.response', [1])
	return

def unmute(socket, command, arguments):
	socket.sysmessage(tr('Target the player you want to unmute.'))
	socket.attachtarget('commands.mute.response', [0])
	return

def onLoad():
	wolfpack.registercommand('mute', mute)
	wolfpack.registercommand('unmute', unmute)
	return
