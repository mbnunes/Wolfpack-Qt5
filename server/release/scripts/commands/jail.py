
import wolfpack
from wolfpack import tr
from wolfpack.consts import *

"""
	\command jail
	\description Sends the targetted player to jail after saving his current position. The jailed flag is set for the player too.
	If you want to release the player from jail, you have to use the forgive command.
"""

"""
	\command forgive
	\description Releases the targetted player from jail and sends him back where he was before being jailed.
"""

# Set this to the position of your jail
JAIL_POS = wolfpack.coord(5275, 1163, 0, 1)

def jailPlayer(player, target, tojail = True):
	if target.jailed == tojail:
		if not target.jailed:
			player.socket.sysmessage(tr('That player has been released from jail already.'))
		else:
			player.socket.sysmessage(tr('That player is in jail already.'))
		return

	target.jailed = tojail

	if tojail:
		target.settag('unjail_pos', str(target.pos))
		
		target.removefromview()
		target.moveto(JAIL_POS)
		target.update()
		if target.socket:
			target.socket.resendworld()		
		
		player.log(LOG_MESSAGE, 'Jailed player %s (0x%x).\n' % (target.orgname, target.serial))
		player.socket.sysmessage(tr('The targetted player has been jailed.'))
		target.message(503268) # You've been jailed
	else:
		player.log(LOG_MESSAGE,'Unjailed player %s (0x%x).\n' % (target.orgname, target.serial))
		player.socket.sysmessage(tr('The targetted player has been unjailed.'))	
	
		# Send back to original position
		unjail_pos = target.gettag('unjail_pos')		
		if unjail_pos and unjail_pos.count(',') == 3:
			(x, y, z, map) = unjail_pos.split(',')
		
			if map != 0xFF:		
				target.removefromview()
				target.moveto(wolfpack.coord(int(x), int(y), int(z), int(map)))
				target.update()
				if target.socket:
					target.socket.resendworld()

		target.deltag('unjail_pos')	
		target.message(503267) # You've been unjailed		

def response(player, arguments, target):
	if not target.char or not target.char.player:
		player.socket.sysmessage(tr('You have to target a player character.'))
		return

	if target.char.rank >= player.rank:
		player.socket.sysmessage(tr('You burn your fingers.'))
		return

	jailPlayer(player, target.char, arguments[0])

def jail(socket, command, arguments):
	socket.sysmessage(tr('Target the player you want to send to jail.'))
	socket.attachtarget('commands.jail.response', [1])
	
def forgive(socket, command, arguments):
	socket.sysmessage(tr('Target the player you want to release from jail.'))
	socket.attachtarget('commands.jail.response', [0])

def onLoad():
	wolfpack.registercommand('jail', jail)
	wolfpack.registercommand('forgive', forgive)
