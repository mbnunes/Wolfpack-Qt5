
from wolfpack import tr
import wolfpack
from wolfpack.consts import *

"""
	\command follow
	\description This command will let you select a character as the follow target. Then you will be teleported to the characters location every 1.5 seconds.
	To stop following, simply use this command again.
	\usage - <code>follow</code>
"""

def timer(char, args):
	socket = char.socket
	
	if not socket or not socket.hastag('follow_target'):
		return
	
	target_serial = socket.gettag('follow_target')
	target = wolfpack.findchar(target_serial)
	
	if not target or target.pos.map == 0xFF:
		message = tr('Stopped following 0x%x.') % target_serial
		socket.sysmessage(message)
		socket.log(LOG_MESSAGE, message + "\n")
		return

	# Move the gm
	if char.pos != target.pos:
		char.removefromview()
		char.moveto(target.pos)
		char.update()
		socket.resendworld()
	
	char.addtimer(1500, timer, [], False, False, 'FOLLOW_TIMER')

def target(player, arguments, target):
	if not target.char or target.char == player or target.pos.map == 0xFF:
		player.socket.sysmessage(tr('You chose an invalid follow target.'))
		return
		
	if target.rank > player.rank:
		player.socket.sysmessage(tr('You better don''t do that.'))
		return

	message = tr('Started following 0x%x.') % target.char.serial
	player.socket.sysmessage(message)
	player.socket.log(LOG_MESSAGE, message + "\n")	
	player.socket.settag('follow_target', target.char.serial)
	player.dispel(None, True, 'FOLLOW_TIMER')
	player.addtimer(1500, timer, [], False, False, 'FOLLOW_TIMER')

def follow(socket, command, arguments):
	socket.player.dispel(None, True, 'FOLLOW_TIMER')
	if socket.hastag('follow_target'):
		message = tr('Stopped following 0x%x.') % socket.gettag('follow_target')
		socket.sysmessage(message)
		socket.log(LOG_MESSAGE, message + "\n")
		socket.deltag('follow_target')
	socket.sysmessage(tr('Choose a new character to follow.'))
	socket.attachtarget('commands.follow.target', [])

def onLoad():
	wolfpack.registercommand('follow', follow)
